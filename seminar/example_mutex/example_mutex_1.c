MODULE_IMPORT_NS(mutex_lock);
/*
 * example_mutex_1.c
 *
 * 1. 
 */
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/printk.h>

static DEFINE_MUTEX(shared_mutex);
struct mutex *get_shared_mutex(void)
{
    return &shared_mutex;
}
EXPORT_SYMBOL(get_shared_mutex);

static int __init example_mutex_init(void)
{

    pr_info("example_mutex_1 init\n");

    while(mutex_trylock(&shared_mutex) != 0) continue;
    
    pr_info("example_mutex_1 is locked\n");

    if (mutex_is_locked(&shared_mutex) == 0)
        pr_info("The example_mutex_1 failed to lock!\n");

    return 0;
}

static void __exit example_mutex_exit(void)
{

    // Unlocks mutex, other module have to wait 
    mutex_unlock(&shared_mutex);
    pr_info("example_mutex_1 is unlocked\n");
    pr_info("example_mutex_1 exit\n");
}

module_init(example_mutex_init);
module_exit(example_mutex_exit);

MODULE_DESCRIPTION("Mutex example 1");
MODULE_LICENSE("GPL");
