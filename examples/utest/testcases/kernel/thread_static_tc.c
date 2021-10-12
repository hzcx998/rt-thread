/*
 * Copyright (c) 2006-2019, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2019-03-12     WillianChan  the first version
 * 2019-06-14     flybreak     add Priority preemption and time slice rotation test.
 * 2020-09-10     Ernest Chen  fix some bug and add test below slice rotation test.
 */

#include <rtthread.h>
#include <cpu.h>
#include "utest.h"

#define THREAD_STACK_SIZE 4096
#define THREAD_TIMESLICE  2

ALIGN(RT_ALIGN_SIZE)
static char static_thread_stack[THREAD_STACK_SIZE];
static struct rt_thread thread2;

static rt_uint32_t delay_pass_flag = 0;
static rt_uint32_t tid3_finish_flag = 0;
static rt_uint32_t tid4_finish_flag = 0;
static rt_uint32_t tid6_finish_flag = 0;
static rt_uint32_t thread5_source = 0;
static rt_uint32_t entry_idle_hook_times = 0;
static rt_thread_t __current_thread;
static rt_uint8_t change_priority;
static rt_uint32_t count = 0;
static rt_uint32_t times_slice1 = 0, times_slice2 = 0;

static struct rt_thread tc_thread_thread3;
static char tc_thread_thread3_stack[THREAD_STACK_SIZE];

#ifdef RT_USING_IDLE_HOOK
static struct rt_thread tc_thread_thread4;
static char tc_thread_thread4_stack[THREAD_STACK_SIZE];
#endif

static struct rt_thread tc_thread_thread5;
static char tc_thread_thread5_stack[THREAD_STACK_SIZE];
static struct rt_thread tc_thread_thread6;
static char tc_thread_thread6_stack[THREAD_STACK_SIZE];
static struct rt_thread tc_thread_thread7;
static char tc_thread_thread7_stack[THREAD_STACK_SIZE];
static struct rt_thread tc_thread_thread8;
static char tc_thread_thread8_stack[THREAD_STACK_SIZE];
static struct rt_thread tc_thread_thread9;
static char tc_thread_thread9_stack[THREAD_STACK_SIZE];
static struct rt_thread tc_thread_thread10;
static char tc_thread_thread10_stack[THREAD_STACK_SIZE];
static struct rt_thread tc_thread_thread11;
static char tc_thread_thread11_stack[THREAD_STACK_SIZE];

static void thread2_entry(void *parameter)
{
    while (1);
}

static void test_thread_static(void)
{
    rt_err_t rst_init = -RT_ERROR;
    rt_err_t rst_startup = -RT_ERROR;
    rt_err_t rst_detach = - RT_ERROR;

    rst_init = rt_thread_init(&thread2,
                              "thread2",
                              thread2_entry,
                              RT_NULL,
                              &static_thread_stack[0],
                              sizeof(static_thread_stack),
                              __current_thread->current_priority + 1,
                              THREAD_TIMESLICE);
    if (rst_init != RT_EOK)
    {
        LOG_E("static_thread_init failed!");
        uassert_false(rst_init != RT_EOK);
        goto __exit;
    }
    else
    {
        rst_startup = rt_thread_startup(&thread2);
        if (rst_startup != RT_EOK)
        {
            LOG_E("static_thread_startup failed!");
            uassert_false(rst_startup != RT_EOK);
            goto __exit;
        }
    }
    rst_detach = rt_thread_detach(&thread2);
    if (rst_detach != RT_EOK)
    {
        LOG_E("rt_thread_detach failed!");
        uassert_false(rst_detach != RT_EOK);
        goto __exit;
    }

__exit:
    if (rst_init == RT_EOK && rst_detach != RT_EOK)
    {
        rt_thread_detach(&thread2);
    }

    uassert_true(rst_init == RT_EOK && rst_startup == RT_EOK && rst_detach == RT_EOK);
    return;
}

static void thread3_entry(void *parameter)
{
    rt_tick_t curtk = rt_tick_get();

    rt_thread_delay(RT_TICK_PER_SECOND * 3);

    int delta = rt_tick_get() - curtk;

    rt_kprintf("dealy: delta %d\n", delta);
    if (delta < RT_TICK_PER_SECOND * 3)
    {
        tid3_finish_flag = 1;
        delay_pass_flag = 0;
        return;
    }

    delay_pass_flag = 1;
    tid3_finish_flag = 1;
}

static void test_thread_delay(void)
{
    rt_err_t result = RT_EOK;

    result = rt_thread_init(&tc_thread_thread3, "thread3",
                            thread3_entry,
                            RT_NULL,
                            tc_thread_thread3_stack,
                            sizeof(tc_thread_thread3_stack),
                            __current_thread->current_priority - 1,
                            THREAD_TIMESLICE);
    if (result != RT_EOK)
    {
        LOG_E("rt_thread_init failed!");
        uassert_false(result == RT_EOK);
        goto __exit;
    }
    else
    {
        if (rt_thread_startup(&tc_thread_thread3) != RT_EOK)
        {
            LOG_E("rt_thread_startup failed!");
            uassert_false(1);
            goto __exit;
        }
    }

    /* wait for thread3 exit */
    while (tid3_finish_flag != 1)
    {
        rt_thread_mdelay(10);
    };

    /* check if the delay time as expect */
    uassert_true(delay_pass_flag == 1);

__exit:
    return;
}

#ifdef RT_USING_IDLE_HOOK
static void idle_hook(void)
{
    entry_idle_hook_times ++;
}

static void thread4_entry(void *parameter)
{
    rt_uint32_t delay_times = 5;
    while (delay_times --)
    {
        rt_thread_mdelay(300);
    }

    rt_thread_idle_delhook(idle_hook);
    tid4_finish_flag = 1;
}

static void test_idle_hook(void)
{
    rt_err_t result = RT_EOK;
    rt_thread_idle_sethook(idle_hook);

    result = rt_thread_init(&tc_thread_thread4, "thread4",
                            thread4_entry,
                            RT_NULL,
                            tc_thread_thread4_stack,
                            sizeof(tc_thread_thread4_stack),
                            __current_thread->current_priority - 1,
                            THREAD_TIMESLICE);
    if (result != RT_EOK)
    {
        LOG_E("rt_thread_init failed!");
        uassert_false(result == RT_EOK);
        goto __exit;
    }
    else
    {
        if (rt_thread_startup(&tc_thread_thread4) != RT_EOK)
        {
            LOG_E("rt_thread_startup failed!");
            uassert_false(1);
            goto __exit;
        }
    }

    /* wait for thread4 exit */
    while (tid4_finish_flag != 1)
    {
        rt_thread_mdelay(200);
    }

    /* check if really enter into thread hook function */
    uassert_true(entry_idle_hook_times > 0);

__exit:
    return;
}
#endif /* RT_USING_IDLE_HOOK */

static void thread5_entry(void *parameter)
{
    while (1)
    {
        thread5_source ++;
        thread5_source ++;
        thread5_source ++;
        thread5_source ++;
        thread5_source ++;

        /* now the value of thread5_source should be 5 */
        rt_thread_yield();

        /* thread5 exit */
        return;
    }
}

static void thread6_entry(void *parameter)
{
    if (thread5_source != 5)
    {
        LOG_E("thread yield error!");
        uassert_false(1);
        return;
    }

    while (++ thread5_source <= 9);
    tid6_finish_flag = 1;
}

static void test_thread_yield(void)
{
    rt_err_t result = RT_EOK;
    result = rt_thread_init(&tc_thread_thread5, "thread5",
                            thread5_entry,
                            RT_NULL,
                            tc_thread_thread5_stack,
                            sizeof(tc_thread_thread5_stack),
                            __current_thread->current_priority - 1,
                            THREAD_TIMESLICE);

    if (result == -RT_ERROR)
    {
        LOG_E("rt_thread_init failed!");
        uassert_false(result == -RT_ERROR);
        goto __exit;
    }
    else
    {
        if (rt_thread_startup(&tc_thread_thread5) != RT_EOK)
        {
            LOG_E("rt_thread_startup failed!");
            uassert_false(1);
            goto __exit;
        }
    }

    result = rt_thread_init(&tc_thread_thread6, "thread6",
                            thread6_entry,
                            RT_NULL,
                            tc_thread_thread6_stack,
                            sizeof(tc_thread_thread6_stack),
                            __current_thread->current_priority - 1,
                            THREAD_TIMESLICE);
    if (result == -RT_ERROR)
    {
        LOG_E("rt_thread_init failed!");
        uassert_false(result == -RT_ERROR);
        goto __exit;
    }
    else
    {
        if (rt_thread_startup(&tc_thread_thread6) != RT_EOK)
        {
            LOG_E("rt_thread_startup failed!");
            uassert_false(1);
            goto __exit;
        }
    }

    /* wait for thread6 exit */
    while (tid6_finish_flag != 1);

    /* check if thread5 actually yield CPU to thread6  */
    uassert_true(thread5_source == 10);
    return;

__exit:

    rt_thread_detach(&tc_thread_thread5);

    return;
}

static void thread7_entry(void *parameter)
{
    while (1);
}

static void test_thread_control(void)
{
    rt_err_t rst_delete = -RT_ERROR;
    rt_err_t result = RT_EOK;

    result = rt_thread_init(&tc_thread_thread7, "thread7",
                            thread7_entry,
                            RT_NULL,
                            tc_thread_thread7_stack,
                            sizeof(tc_thread_thread7_stack),
                            __current_thread->current_priority + 1,
                            THREAD_TIMESLICE);
    if (result == -RT_ERROR)
    {
        LOG_E("rt_thread_init failed!");
        uassert_false(result == -RT_ERROR);
        goto __exit;
    }
    else
    {
        if (rt_thread_control(&tc_thread_thread7, RT_THREAD_CTRL_STARTUP, RT_NULL) != RT_EOK)
        {
            LOG_E("rt_thread_control failed!");
            uassert_false(1);
            goto __exit;
        }
        rt_thread_mdelay(200);
    }

    /* control null cmd */
    result = rt_thread_control(&tc_thread_thread7, 0x55, RT_NULL);
    if (result != RT_EOK)
    {
        LOG_E("rt_thread_control failed!");
        uassert_false(result != RT_EOK);
        goto __exit;
    }

    /* change thread7 to new priority */
    rt_thread_control(&tc_thread_thread7, RT_THREAD_CTRL_CHANGE_PRIORITY, &change_priority);

    if (tc_thread_thread7.current_priority != change_priority)
    {
        LOG_E("rt_thread_control failed!");
        uassert_false(1);
        goto __exit;
    }

    rt_thread_suspend(&tc_thread_thread7);
    change_priority = __current_thread->current_priority;
    /* change thread7 to new priority */
    rt_thread_control(&tc_thread_thread7, RT_THREAD_CTRL_CHANGE_PRIORITY, &change_priority);

    if (tc_thread_thread7.current_priority != change_priority)
    {
        LOG_E("rt_thread_control failed!");
        uassert_false(1);
        goto __exit;
    }

    /* try to delete a thread */
    rst_delete = rt_thread_control(&tc_thread_thread7, RT_THREAD_CTRL_CLOSE, RT_NULL);
    if (rst_delete != RT_EOK)
    {
        LOG_E("rt_thread_control failed!");
        uassert_false(rst_delete != RT_EOK);
        goto __exit;
    }

    uassert_true(1);
    return;

__exit:
    rt_thread_detach(&tc_thread_thread7);

    return;
}

static void thread8_entry(void *parameter)
{
    rt_thread_yield();

    while (1)
    {
        rt_enter_critical();
        times_slice1 ++;
        rt_exit_critical();
    };
}

static void thread9_entry(void *parameter)
{
    while (1)
    {
        rt_enter_critical();
        times_slice2 ++;
        rt_exit_critical();
    };
}

static void thread10_entry(void *parameter)
{
    for (; count < 100; count ++);
}

static void test_thread_time_slice(void)
{
    rt_thread_t tid8 = &tc_thread_thread8, tid9 = &tc_thread_thread9;
    rt_uint8_t times = 3;
    rt_err_t result = RT_EOK;

    result = rt_thread_init(&tc_thread_thread8, "thread8",
                            thread8_entry,
                            RT_NULL,
                            tc_thread_thread8_stack,
                            sizeof(tc_thread_thread8_stack),
                            __current_thread->current_priority + 1,
                            THREAD_TIMESLICE);

    if (result == -RT_ERROR)
    {
        LOG_E("rt_thread_init failed!");
        uassert_false(tid8 == RT_NULL);
        goto __exit;
    }

    result = rt_thread_init(&tc_thread_thread9, "thread9",
                            thread9_entry,
                            RT_NULL,
                            tc_thread_thread9_stack,
                            sizeof(tc_thread_thread9_stack),
                            __current_thread->current_priority + 1,
                            THREAD_TIMESLICE);

    if (result == -RT_ERROR)
    {
        LOG_E("rt_thread_init failed!");
        uassert_false(result == -RT_ERROR);
        goto __exit;
    }

    if (rt_thread_startup(&tc_thread_thread8) != RT_EOK)
    {
        LOG_E("rt_thread_startup failed!");
        uassert_false(1);
        goto __exit;
    }

    if (rt_thread_startup(&tc_thread_thread9) != RT_EOK)
    {
        LOG_E("rt_thread_startup failed!");
        uassert_false(1);
        goto __exit;
    }

    while (times --)
    {
        rt_bool_t is_success = RT_FALSE;

        /* init count times */
        rt_enter_critical();
        times_slice1 = 0;
        times_slice2 = 0;
        rt_exit_critical();

        /* test 30 times about thread slice */
        rt_thread_delay(THREAD_TIMESLICE * 30);

        rt_enter_critical();
        if (times_slice2 > times_slice1)
        {
            is_success = (times_slice2 - times_slice1 < times_slice2 / 2) ? RT_TRUE : RT_FALSE;
        }
        else
        {
            is_success = (times_slice1 - times_slice2 < times_slice1 / 2) ? RT_TRUE : RT_FALSE;
        }
        rt_exit_critical();

        uassert_true(is_success == RT_TRUE);
    }

__exit:
    if (tid8)
        rt_thread_detach(tid8);
    if (tid9)
        rt_thread_detach(tid9);
    return;
}


static rt_uint8_t thread_keep_value;
static rt_uint8_t thread_exit_flag;

static void thread11_entry(void *param)
{
    while (1)
    {
        thread_keep_value = 10;
        rt_thread_mdelay(10);

        if (thread_exit_flag == 1)
        {
            return;
        }
    }
}

static void test_thread_schedule_lock(void)
{
    rt_err_t result = RT_EOK;

    thread_keep_value = 0;
    thread_exit_flag = 0;
    result = rt_thread_init(&tc_thread_thread11, "thread11",
                            thread11_entry,
                            RT_NULL,
                            tc_thread_thread11_stack,
                            sizeof(tc_thread_thread11_stack),
                            __current_thread->current_priority - 1,
                            THREAD_TIMESLICE);

    if (result == -RT_ERROR)
    {
        LOG_E("rt_thread_init failed!");
        uassert_false(result == -RT_ERROR);
    }
    else
    {
#ifdef RT_USING_SMP
        rt_kprintf("bound on cpu %d\n", __current_thread->oncpu);
        uassert_true(rt_thread_control(&tc_thread_thread11, RT_THREAD_CTRL_BIND_CPU, (void *)__current_thread->oncpu) == RT_EOK);
#endif
        if (rt_thread_startup(&tc_thread_thread11) != RT_EOK)
        {
            LOG_E("rt_thread_startup failed!");
            uassert_false(1);
        }
    }

    /* lock the thread scheduler */
    rt_enter_critical();

    thread_keep_value = 100;
    rt_thread_mdelay(1000);
    uassert_true(thread_keep_value == 100);
    thread_exit_flag = 1;

    /* unlock the thread scheduler */
    rt_exit_critical();

    return;
}

static void test_thread_priority(void)
{
    rt_err_t result = RT_EOK;

    result = rt_thread_init(&tc_thread_thread10, "thread10",
                            thread10_entry,
                            RT_NULL,
                            tc_thread_thread10_stack,
                            sizeof(tc_thread_thread10_stack),
                            __current_thread->current_priority - 1,
                            THREAD_TIMESLICE);

    if (result == -RT_ERROR)
    {
        LOG_E("rt_thread_init failed!");
        uassert_false(result == -RT_ERROR);
        return;
    }
    else
    {
        /* bound on current cpu to make sure can be run on this cpu */
        uassert_true(rt_thread_control(&tc_thread_thread10, RT_THREAD_CTRL_BIND_CPU, (void *)rt_hw_cpu_id()) == RT_EOK);

        count = 0;
        if (rt_thread_startup(&tc_thread_thread10) != RT_EOK)
        {
            LOG_E("rt_thread_startup failed!");
            uassert_false(1);
            return;
        }
    }
    LOG_I("count val:%d\n", count);
    uassert_true(count == 100);

    return;
}

static void test_thread_resume_ready(void)
{
    rt_err_t result = RT_EOK;

    result = rt_thread_init(&tc_thread_thread7, "thread7",
                            thread7_entry,
                            RT_NULL,
                            tc_thread_thread7_stack,
                            sizeof(tc_thread_thread7_stack),
                            __current_thread->current_priority,
                            THREAD_TIMESLICE);
    if (result == -RT_ERROR)
    {
        LOG_E("rt_thread_init failed!");
        uassert_false(result == -RT_ERROR);
    }
    result = rt_thread_startup(&tc_thread_thread7);
    uassert_true(result == RT_EOK);

    result = rt_thread_resume(&tc_thread_thread7);
    uassert_true(result != RT_EOK);
    result =  rt_thread_detach(&tc_thread_thread7);

    uassert_true(result == RT_EOK);
}

static void test_thread_find_not_exist(void)
{
    rt_thread_t thread = rt_thread_find("not_exist");

    uassert_true(thread == RT_NULL);
}

/* excute for destroy defunct */
static void thread_entry_delay(void *parameter)
{
    while (1)
    {
        rt_thread_mdelay(1);
    }
}

/* excute for destroy defunct */
static void thread_entry_excute_idle(void *parameter)
{
    while (1)
    {
        /* perform idle */
        // rt_thread_idle_excute();

        rt_thread_mdelay(1);
    }
}

static void test_thread_close_and_start_again(void)
{
    rt_err_t result = RT_EOK;
    rt_uint32_t i;

    rt_thread_init(&tc_thread_thread7, "thread7",
                   thread_entry_delay,
                   RT_NULL,
                   tc_thread_thread7_stack,
                   sizeof(tc_thread_thread7_stack),
                   __current_thread->current_priority + 1,
                   1);
    rt_thread_startup(&tc_thread_thread7);

    /* for next special test */
    rt_thread_init(&tc_thread_thread8, "thread8",
                   thread_entry_excute_idle,
                   RT_NULL,
                   tc_thread_thread8_stack,
                   sizeof(tc_thread_thread8_stack),
                   __current_thread->current_priority + 1,
                   1);
    rt_thread_startup(&tc_thread_thread8);

    /* Test special branches:in rt_thread_idle_excute() function, the defunct is not empty, but it is empty when run into.
     * The situation cannot be controlled to make sure this happends.
     */
    for (i = 0; i < 100; i++)
    {
        result = rt_thread_detach(&tc_thread_thread7);
        if (result == -RT_ERROR)
        {
            LOG_E("rt_thread_detach failed!");
            break;
        }
        /* wait for idle running */
        rt_thread_mdelay(1);

        rt_thread_init(&tc_thread_thread7, "thread7",
                       thread7_entry,
                       RT_NULL,
                       tc_thread_thread7_stack,
                       sizeof(tc_thread_thread7_stack),
                       __current_thread->current_priority + 1,
                       THREAD_TIMESLICE);
        if (result == -RT_ERROR)
        {
            LOG_E("rt_thread_init failed!");
            break;
        }

        result = rt_thread_startup(&tc_thread_thread7);
        if (result == -RT_ERROR)
        {
            LOG_E("rt_thread_startup failed!");
            break;
        }
    }

    rt_thread_detach(&tc_thread_thread7);
    rt_thread_detach(&tc_thread_thread8);

    uassert_true(result == RT_EOK);

    return;
}

static void tc_cleanup(struct rt_thread *tid)
{
    count = __current_thread->current_priority;
}

static void test_thread_cleanup_detach(void)
{
    rt_thread_init(&tc_thread_thread7, "thread7",
                   thread7_entry,
                   RT_NULL,
                   tc_thread_thread7_stack,
                   sizeof(tc_thread_thread7_stack),
                   __current_thread->current_priority + 1,
                   THREAD_TIMESLICE);

    /* set cleanup function */
    tc_thread_thread7.cleanup = tc_cleanup;

    rt_thread_startup(&tc_thread_thread7);

    /* wait for thread running and detaching */
    rt_thread_mdelay(10);
    rt_thread_detach(&tc_thread_thread7);
    /* wait for cleaning up */
    rt_thread_mdelay(100);

    uassert_true(count == __current_thread->current_priority);
}

static void thread_entry_cleanup(void *parameter)
{
    /* Just run over, and clean up this thread by rt_thread_exit. */
}

static void test_thread_cleanup_thread_exit(void)
{
    rt_thread_init(&tc_thread_thread7, "thread7",
                   thread_entry_cleanup,
                   RT_NULL,
                   tc_thread_thread7_stack,
                   sizeof(tc_thread_thread7_stack),
                   __current_thread->current_priority + 1,
                   THREAD_TIMESLICE);

    /* set cleanup function */
    tc_thread_thread7.cleanup = tc_cleanup;

    rt_thread_startup(&tc_thread_thread7);

    /* wait for thread running and exit */
    rt_thread_mdelay(100);

    uassert_true(count == __current_thread->current_priority);
}

static void test_thread_cleanup(void)
{
    /* Set cleanup function, and detach this thread. */
    test_thread_cleanup_detach();

    /* Set cleanup function, and wait this thread exit by itself running over. */
    test_thread_cleanup_thread_exit();

    return;
}

static void test_thread_get_idle(void)
{
    rt_thread_t idle;

    idle = rt_thread_idle_gethandler();

    uassert_str_equal(idle->name, "tidle0");

    return;
}

static void test_thread_suspend_error(void)
{
    rt_err_t result = RT_EOK;

    rt_thread_init(&tc_thread_thread7, "thread7",
                   thread7_entry,
                   RT_NULL,
                   tc_thread_thread7_stack,
                   sizeof(tc_thread_thread7_stack),
                   __current_thread->current_priority + 1,
                   THREAD_TIMESLICE);

    /* It should be error. */
    result = rt_thread_suspend(&tc_thread_thread7);
    if (result == RT_EOK)
    {
        LOG_E("rt_thread_suspend failed!");
    }
    else
    {
        uassert_false(result == RT_EOK);
    }

    rt_thread_detach(&tc_thread_thread7);

    return;
}

#ifdef RT_USING_HOOK
enum test_thread_hook
{
    TC_THREAD_START = 1,
    TC_THREAD_INIT,
    TC_THREAD_SUSPEND,
    TC_THREAD_RESUME,
    TC_THREAD_SCHEDULE,
};

static enum test_thread_hook tc_enum_thread;

static void test_thread_inited_hook(rt_thread_t thread)
{
    tc_enum_thread = TC_THREAD_INIT;
}

static void test_thread_suspend_hook(rt_thread_t thread)
{
    tc_enum_thread = TC_THREAD_SUSPEND;
}

static void test_thread_resume_hook(rt_thread_t thread)
{
    tc_enum_thread = TC_THREAD_RESUME;
}

static void thread_entry_hook(void *parameter)
{
    /* wait for the thread suspending itself */
    rt_thread_mdelay(50);
}

static void test_thread_api_hook(void)
{
    /* init test status */
    tc_enum_thread = TC_THREAD_START;
    count = 0;

    /* set the hook */
    rt_thread_inited_sethook(test_thread_inited_hook);
    rt_thread_suspend_sethook(test_thread_suspend_hook);
    rt_thread_resume_sethook(test_thread_resume_hook);

    rt_thread_init(&tc_thread_thread7, "thread7",
                   thread_entry_hook,
                   RT_NULL,
                   tc_thread_thread7_stack,
                   sizeof(tc_thread_thread7_stack),
                   __current_thread->current_priority + 1,
                   THREAD_TIMESLICE);
    /* After initialization, the thread is init */
    uassert_true(tc_enum_thread == TC_THREAD_INIT);

    rt_thread_startup(&tc_thread_thread7);
    /* After startup, the thread is resume */
    uassert_true(tc_enum_thread == TC_THREAD_RESUME);

    /* wait for suspending test thread */
    rt_thread_mdelay(10);
    uassert_true(tc_enum_thread == TC_THREAD_SUSPEND);

    /* wait for exit test thread */
    rt_thread_detach(&tc_thread_thread7);
    /* set the hook null */
    rt_thread_inited_sethook(RT_NULL);
    rt_thread_suspend_sethook(RT_NULL);
    rt_thread_resume_sethook(RT_NULL);

}

static void test_scheduler_hook(struct rt_thread *from, struct rt_thread *to)
{
    tc_enum_thread = TC_THREAD_SCHEDULE;
}

static void test_thread_schedule_hook(void)
{
    /* init tc_enum_thread status */
    tc_enum_thread = TC_THREAD_START;

    rt_scheduler_sethook(test_scheduler_hook);

    rt_thread_init(&tc_thread_thread7, "thread7",
                   thread_entry_delay,
                   RT_NULL,
                   tc_thread_thread7_stack,
                   sizeof(tc_thread_thread7_stack),
                   __current_thread->current_priority + 1,
                   THREAD_TIMESLICE);
    rt_thread_startup(&tc_thread_thread7);

    /* wait for test thread running */
    rt_thread_delay(THREAD_TIMESLICE);
    uassert_true(tc_enum_thread == TC_THREAD_SCHEDULE);

    rt_thread_detach(&tc_thread_thread7);
    rt_scheduler_sethook(RT_NULL);
}
#endif /* RT_USING_HOOK */

/* Skip the warning for test the stack close to overflow. */
static void tc_thread_stack_warning(void *parameter)
{
    rt_uint8_t arr[THREAD_STACK_SIZE - 96];
    rt_uint32_t i;

    for (i = 0; i < THREAD_STACK_SIZE - 96; i++)
    {
        arr[i] = i;
    }

    while (1);
}

static void test_thread_stack_warning(void)
{
    rt_err_t result = RT_EOK;

    rt_thread_init(&tc_thread_thread7, "thread7",
                   tc_thread_stack_warning,
                   RT_NULL,
                   tc_thread_thread7_stack,
                   sizeof(tc_thread_thread7_stack),
                   FINSH_THREAD_PRIORITY + 1,
                   THREAD_TIMESLICE);

    rt_thread_startup(&tc_thread_thread7);

    rt_thread_mdelay(10);

    result = rt_thread_detach(&tc_thread_thread7);
    uassert_true(result == RT_EOK);

    return;
}

static void test_thread_critical_level(void)
{
    rt_uint16_t level;

    /* read lock level */
    level = rt_critical_level();
    if (level != 0)
    {
        uassert_false(level != 0);
    }

    /* lock the thread scheduler */
    rt_enter_critical();
    /* read lock level */
    level = rt_critical_level();
    if (level != 1)
    {
        uassert_false(level != 1);
    }

    /* unlock the thread scheduler */
    rt_exit_critical();
    /* read lock level */
    level = rt_critical_level();
    uassert_true(level == 0);
}

static void test_thread_critical_multiple(void)
{
    rt_int16_t tc_scheduler_lock_nest;

    /* rt_scheduler_lock_nest is 1. */
    rt_enter_critical();
    /* rt_scheduler_lock_nest is 2. */
    rt_enter_critical();

    tc_scheduler_lock_nest = (rt_int16_t)rt_critical_level();

    /* Unlock the thread scheduler, the rt_scheduler_lock_nest will be above zero. */
    rt_exit_critical();
    /* Unlock the thread scheduler, the rt_scheduler_lock_nest will equal zero. */
    rt_exit_critical();

    uassert_true(tc_scheduler_lock_nest > 1);
}

static rt_err_t utest_tc_init(void)
{
    __current_thread = rt_thread_self();
    change_priority = __current_thread->current_priority + 5;
    delay_pass_flag = 0;
    tid3_finish_flag = 0;
    tid4_finish_flag = 0;
    tid6_finish_flag = 0;
    thread5_source = 0;
    entry_idle_hook_times = 0;
    count = 0;
    return RT_EOK;
}

static rt_err_t utest_tc_cleanup(void)
{
    return RT_EOK;
}

static void testcase(void)
{
    LOG_I("in testcase func...");

    UTEST_UNIT_RUN(test_thread_static);
    UTEST_UNIT_RUN(test_thread_delay);
#ifdef RT_USING_IDLE_HOOK
    UTEST_UNIT_RUN(test_idle_hook);
#endif
    UTEST_UNIT_RUN(test_thread_yield);
    // UTEST_UNIT_RUN(test_thread_control);
    // UTEST_UNIT_RUN(test_thread_time_slice);
    UTEST_UNIT_RUN(test_thread_priority);
    UTEST_UNIT_RUN(test_thread_schedule_lock);
    UTEST_UNIT_RUN(test_thread_resume_ready);
    UTEST_UNIT_RUN(test_thread_find_not_exist);
    // UTEST_UNIT_RUN(test_thread_close_and_start_again);
    // UTEST_UNIT_RUN(test_thread_cleanup);
    // UTEST_UNIT_RUN(test_thread_get_idle);
    // UTEST_UNIT_RUN(test_thread_suspend_error);
#ifdef RT_USING_HOOK
    // UTEST_UNIT_RUN(test_thread_api_hook);
    // UTEST_UNIT_RUN(test_thread_schedule_hook);
#endif
    // UTEST_UNIT_RUN(test_thread_stack_warning);
    UTEST_UNIT_RUN(test_thread_critical_level);
    UTEST_UNIT_RUN(test_thread_critical_multiple);
}
UTEST_TC_EXPORT(testcase, "src.thread.thread_static_tc", utest_tc_init, utest_tc_cleanup, 60);
