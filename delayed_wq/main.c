#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/workqueue.h>
#include <linux/mutex.h>

#define DBG_KERNEL 0

/* This module shows that a given delayed work item is schedule large
 * number of times, but it is actually executed for very small number of
 * times.
 *
 * At the same time it also shows that mod_delayed_work() schedules the work
 * even if it is getting executed. Otherwise total number of total_work_sched_cnt
 * should be 1000, but it is more than that.
 *
 */
struct addr_req {
	struct delayed_work work;
	struct list_head list;
	int a;
	u32 pattern;
	bool executed;
};

static int total_work_sched_cnt = 0;
static int total_mod_delay_cnt = 0;
static DEFINE_MUTEX(lock);
static LIST_HEAD(req_list);
static struct workqueue_struct *addr_wq;
static struct workqueue_struct *speed_wq;
static void process_req(struct work_struct *work);
static DECLARE_DELAYED_WORK(dwork, process_req);

static void process_req(struct work_struct *work)
{
	struct addr_req *req;

	mutex_lock(&lock);
	list_for_each_entry(req, &req_list, list) {
		mod_delayed_work(addr_wq, &req->work, 0);
		total_mod_delay_cnt++;
	}
	mutex_unlock(&lock);
	mod_delayed_work(speed_wq, &dwork, 5);
}

static void process_one_req(struct work_struct *_work)
{
	struct addr_req *req;

	/* Don't need atomic update for ordered WQ with one outstanding entry */
	total_work_sched_cnt++; 
#if DBG_KERNEL
	if (_work->execute_started && _work->execute_ended)
		printk("%s work %p request duplicate execution\n", __func__, _work);
#endif

	mutex_lock(&lock);
	req = container_of(_work, struct addr_req, work.work);
	if (req->pattern == 0xbeefbeef && req->executed) {
		/* We should never come here, if work is executed
		 * only once.
		 */
		printk("%s executed work again a=%d\n", __func__, req->a);
		mutex_unlock(&lock);
		return;
	}
	req->executed = true;
	req->pattern = 0xbeefbeef;
	list_del(&req->list);
	mutex_unlock(&lock);
	msleep(10);
	/* Don't free memory for debug, so that _work address never repeats. */
	//kfree(req);
}

static void enqueue_work(void)
{
	struct addr_req *req;
	int i;

	for (i = 0; i < 1000; i++) {
		req = kzalloc(sizeof(*req), GFP_KERNEL);
		if (!req)
			break;
		INIT_DELAYED_WORK(&req->work, process_one_req);
#if DBG_KERNEL
		req->work.work.enable_debug = true;
#endif
		req->pattern = 0xdeaddead;
		req->a = i;

		mutex_lock(&lock);
		list_add_tail(&req->list, &req_list);
		mod_delayed_work(addr_wq, &req->work, 1000);
		mutex_unlock(&lock);
	}
}

static int main_init(void)
{
	addr_wq = alloc_ordered_workqueue("addr_wq", 0);
	if (!addr_wq)
		return -1;

	speed_wq = alloc_ordered_workqueue("speed_wq", 0);
	if (!speed_wq) {
		destroy_workqueue(addr_wq);
		return -1;
	}
	enqueue_work();
	mod_delayed_work(speed_wq, &dwork, 0);
	return 0;
}

static void main_cleanup(void)
{
	cancel_delayed_work_sync(&dwork);
	destroy_workqueue(speed_wq);
	destroy_workqueue(addr_wq);
	printk("%s total_work_cnt=%d\n", __func__, 1000);
	printk("%s total_work_sched_cnt=%d\n", __func__, total_work_sched_cnt);
	printk("%s total_mod_delay_cnt=%d\n", __func__, total_mod_delay_cnt);
}

module_init(main_init);
module_exit(main_cleanup);
MODULE_LICENSE("GPL");
