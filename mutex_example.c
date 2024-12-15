/*
 * mutex_example.c
*/
#include <linux/init.h>
#include <linux/module.h>
#include <linux/printk.h>
#include <linux/delay.h>
#include <linux/mutex.h>
#include <linux/kthread.h>

static DEFINE_MUTEX(mutex);

static struct task_struct *thread1;
static struct task_struct *thread2;

int thread_1(void *arg);
int thread_2(void *arg);

int thread_1(void *arg)
{
    pr_info("In thread 1 \n");
    while (!mutex_trylock(&mutex)) continue;
    pr_info("Thread 1: mutex is locked\n");
    msleep(3000);
    mutex_unlock(&mutex);
    pr_info("Thread 1: mutex is unlocked\n");
    return 0;
}

int thread_2(void *arg)
{
    pr_info("In thread 2 \n");
    while (!mutex_trylock(&mutex)) continue;
    pr_info("Thread 2: mutex is locked\n");
    msleep(1000);
    mutex_unlock(&mutex);
    pr_info("Thread 2: mutex is unlocked\n");
    return 0;
}


static int __init mutex_example_init(void){

    //pthread_t thread1, thread2;
    

    pr_info("mutex_example started\n");
    
    thread1 = kthread_create(thread_1 , NULL, "Thread1");
    if (IS_ERR(thread1))
        goto ERROR_THREAD_1;

    thread2 = kthread_create(thread_2, NULL, "Thread2");
    if (IS_ERR(thread2))
        goto ERROR_THREAD_2;

    wake_up_process(thread1);
    wake_up_process(thread2);

    return 0;

    ERROR_THREAD_2:
        kthread_stop(thread2);
        pr_err("Failed to create thread 2\n");
    ERROR_THREAD_1:
        kthread_stop(thread1);
        pr_err("Failed to create thread 1\n");
    return -1;


    /*
    if (pthread_create(&thread1, NULL, thread_1, NULL) != 0) {
        pr_info("Failed to create thread 1\n");
        return -1;
    }

    if (pthread_create(&thread2, NULL, thread_2, NULL) != 0) {
        pr_info("Failed to create thread 2\n");
        return -1;
    }

    if (pthread_join(thread1, NULL) != 0) {
        pr_info("Failed to join thread 1\n");
        return -1;
    }

    if (pthread_join(thread2, NULL) != 0) {
        pr_info("Failed to join thread 2\n");
        return -1;
    }

    return 0;
    */
}

static void __exit mutex_example_exit(void){
    pr_info("mutex_example exit\n");
}

module_init(mutex_example_init);
module_exit(mutex_example_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Mutex example");

