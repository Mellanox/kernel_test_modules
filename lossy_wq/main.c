#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/delay.h>

/** This modules shows that delayed work is scheduled 5 times,
 * but it is executed only twice.
 * Therefore work item handler and users need to take care of
 * this.
 */

static void process_req(struct work_struct *work);
static DECLARE_DELAYED_WORK(work, process_req);
//static DECLARE_WORK(work, process_req);
static struct workqueue_struct *addr_wq;
atomic_t fire_cnt;

static void process_req(struct work_struct *work2)
{
	printk("%s entered\n", __func__);
	atomic_inc(&fire_cnt);
	printk("%s fire_cnt = %d\n", __func__, atomic_read(&fire_cnt));
	msleep(5000);
	#if 1
	if (atomic_read(&fire_cnt) <= 10)
		//queue_work(addr_wq, work);
		//mod_delayed_work(addr_wq, &work, 0);
	#endif

	printk("%s exit\n", __func__);
}

static void enqueue_work(void)
{
	#if 0
	printk("scheduling 1st time\n");
	queue_work(addr_wq, &work);
	msleep(1000);

	printk("scheduling 2nd time\n");
	queue_work(addr_wq, &work);
	msleep(1000);

	printk("scheduling 3nd time\n");
	queue_work(addr_wq, &work);
	msleep(1000);

	printk("scheduling 4th time\n");
	queue_work(addr_wq, &work);
	printk("scheduling 5th time\n");
	queue_work(addr_wq, &work);
	#endif

	printk("scheduling 1st time\n");
	mod_delayed_work(addr_wq, &work, 0);
	msleep(1000);

	printk("scheduling 2nd time\n");
	mod_delayed_work(addr_wq, &work, 0);
	msleep(1000);

	printk("scheduling 3nd time\n");
	mod_delayed_work(addr_wq, &work, 0);
	msleep(1000);

	printk("scheduling 4th time\n");
	mod_delayed_work(addr_wq, &work, 0);
	printk("scheduling 5th time\n");
	mod_delayed_work(addr_wq, &work, 0);
}

static int main_init(void)
{
	atomic_set(&fire_cnt, 0);
	addr_wq = alloc_ordered_workqueue("ib_addr", WQ_MEM_RECLAIM);
	if (addr_wq)
		enqueue_work();
	return 0;
}

static void main_cleanup(void)
{
	if (addr_wq)
		destroy_workqueue(addr_wq);
}

module_init(main_init);
module_exit(main_cleanup);
MODULE_LICENSE("GPL");
