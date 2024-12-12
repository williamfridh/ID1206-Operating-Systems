MODULE_IMPORT_NS(mutex_lock);
/*
 * example_mutex_2.c
 *
 * 1. 
 */
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/printk.h>

extern struct mutex *get_shared_mutex(void);

static int __init example_mutex_init(void)
{

    if (!get_shared_mutex()) {
        pr_err("example_mutex_2 Failed to get shared mutex\n");
        return -1;
    }

    struct mutex *shared_mutex = get_shared_mutex();

    if (!shared_mutex) {
        pr_err("example_mutex_2 Failed to get shared mutex\n");
        return -1;
    }

    pr_info("example_mutex_2 init\n");

    while(mutex_trylock(shared_mutex) != 0) continue;
    
    pr_info("example_mutex_2 is locked\n");

    if (mutex_is_locked(shared_mutex) == 0)
        pr_info("The example_mutex_2 failed to lock!\n");

    return 0;
}

static void __exit example_mutex_exit(void)
{
    pr_info("example_mutex_2 exit\n");
}

module_init(example_mutex_init);
module_exit(example_mutex_exit);

MODULE_DESCRIPTION("Mutex example 1");
MODULE_LICENSE("GPL");
