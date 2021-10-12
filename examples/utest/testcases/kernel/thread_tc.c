/*
 * Copyright (c) 2006-2019, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2019-03-12     WillianChan  the first version
 * 2019-06-14     flybreak     add Priority preemption and time slice rotation test.
 */
 
#include <rtthread.h>
#include "utest.h"

#define THREAD_STACK_SIZE 4096
#define THREAD_TIMESLICE  5

ALIGN(RT_ALIGN_SIZE)
static char static_thread_stack[THREAD_STACK_SIZE];
static struct rt_thread thread2;
static rt_thread_t tid1 = RT_NULL;
static rt_thread_t tid3 = RT_NULL;
static rt_thread_t tid4 = RT_NULL;
static rt_thread_t tid5 = RT_NULL;
static rt_thread_t tid6 = RT_NULL;
static rt_thread_t tid7 = RT_NULL;
static rt_uint32_t delay_pass_flag = 0;
static rt_uint32_t tid3_finish_flag = 0;
static rt_uint32_t tid4_finish_flag = 0;
static rt_uint32_t tid6_finish_flag = 0;
static rt_uint32_t thread5_source = 0;
#ifndef RT_USING_SMP
static rt_uint32_t thread_yield_flag = 0;
#endif
static rt_uint32_t entry_idle_hook_times = 0;
static rt_thread_t __current_thread;
static rt_uint8_t change_priority;
static rt_uint32_t count = 0;
static rt_uint32_t time_slice1 = 0, time_slice2 = 0;
static rt_uint32_t thread_exit = 0;

static void thread1_entry(void *parameter)
{
    while (1);
}

static void test_thread_dynamic(void)
{
    rt_err_t rst_startup = -RT_ERROR;
    rt_err_t rst_delete = -RT_ERROR;

    tid1 = rt_thread_create("thread1",
                            thread1_entry,
                            RT_NULL,
                            THREAD_STACK_SIZE,
                            __current_thread->current_priority + 1,
                            THREAD_TIMESLICE);
    if (tid1 == RT_NULL)
    {
        LOG_E("rt_thread_create failed!");
        uassert_false(tid1 == RT_NULL);
        goto __exit;
    }
    else
    {
        rst_startup = rt_thread_startup(tid1);
        if (rst_startup != RT_EOK)
        {
            LOG_E("rt_thread_startup failed!");
            uassert_false(rst_startup != RT_EOK);
            goto __exit;
        }
    }
    rst_delete = rt_thread_delete(tid1);
    if (rst_delete != RT_EOK)
    {
        LOG_E("rt_thread_delete failed!");
        uassert_false(rst_delete != RT_EOK);
        goto __exit;
    }
    uassert_true(tid1 != RT_NULL && rst_startup == RT_EOK && rst_delete == RT_EOK);

__exit:
    if (tid1 != RT_NULL && rst_delete != RT_EOK)
    {
        rt_thread_delete(tid1);
    }
    return;
}

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

    uassert_true(rst_init == RT_EOK && rst_startup == RT_EOK && rst_detach == RT_EOK);

__exit:
    if (rst_init == RT_EOK && rst_detach != RT_EOK)
    {
        rt_thread_detach(&thread2);
    }
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
    tid3 = rt_thread_create("thread3",
                            thread3_entry,
                            RT_NULL,
                            THREAD_STACK_SIZE,
                            __current_thread->current_priority - 1,
                            THREAD_TIMESLICE);
    if (tid3 == RT_NULL)
    {
        LOG_E("rt_thread_create failed!");
        uassert_false(tid3 == RT_NULL);
        goto __exit;
    }
    else
    {
        if (rt_thread_startup(tid3) != RT_EOK)
        {
            LOG_E("rt_thread_startup failed!");
            uassert_false(1);
            goto __exit;
        }
    }

    while (tid3_finish_flag != 1);
    uassert_true(delay_pass_flag == 1);

__exit:
    return;
}

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
    rt_thread_idle_sethook(idle_hook);

    tid4 = rt_thread_create("thread4",
                            thread4_entry,
                            RT_NULL,
                            THREAD_STACK_SIZE,
                            __current_thread->current_priority - 1,
                            THREAD_TIMESLICE);
    if (tid4 == RT_NULL)
    {
        LOG_E("rt_thread_create failed!");
        uassert_false(tid4 == RT_NULL);
        goto __exit;
    }
    else
    {
        if (rt_thread_startup(tid4) != RT_EOK)
        {
            LOG_E("rt_thread_startup failed!");
            uassert_false(1);
            goto __exit;
        }
    }

    while (tid4_finish_flag != 1)
    {
        rt_thread_mdelay(200);
    }
    uassert_true(entry_idle_hook_times > 0);

__exit:
    return;
}

static void thread5_entry(void *parameter)
{
    while (1)
    {
        thread5_source ++;
        rt_thread_delay(5);
        if (thread5_source == 5)
        {
            rt_thread_yield();
        }
    }
}

static void thread6_entry(void *parameter)
{
    while (++ thread5_source <= 9);
    tid6_finish_flag = 1;
}

static void test_thread_yield(void)
{
    tid5 = rt_thread_create("thread5",
                            thread5_entry,
                            RT_NULL,
                            THREAD_STACK_SIZE,
                            __current_thread->current_priority - 1,
                            THREAD_TIMESLICE);
    if (tid5 == RT_NULL)
    {
        LOG_E("rt_thread_create failed!");
        uassert_false(tid5 == RT_NULL);
        goto __exit;
    }
    else
    {
        
#ifdef RT_USING_SMP
        /* bound on current cpu to make sure can be run on this cpu */
        rt_kprintf("bound on cpu %d\n", __current_thread->oncpu);
        rt_thread_control(tid5, RT_THREAD_CTRL_BIND_CPU, __current_thread->oncpu);
#endif
        if (rt_thread_startup(tid5) != RT_EOK)
        {
            LOG_E("rt_thread_startup failed!");
            uassert_false(1);
            goto __exit;
        }
    }
    tid6 = rt_thread_create("thread6",
                            thread6_entry,
                            RT_NULL,
                            THREAD_STACK_SIZE,
                            __current_thread->current_priority - 1,
                            THREAD_TIMESLICE);
    if (tid6 == RT_NULL)
    {
        LOG_E("rt_thread_create failed!");
        uassert_false(tid6 == RT_NULL);
        goto __exit;
    }
    else
    {
        
#ifdef RT_USING_SMP
        rt_kprintf("bound on cpu %d\n", __current_thread->oncpu);
        /* bound on current cpu to make sure can be run on this cpu */
        rt_thread_control(tid6, RT_THREAD_CTRL_BIND_CPU, __current_thread->oncpu);
#endif
        if (rt_thread_startup(tid6) != RT_EOK)
        {
            LOG_E("rt_thread_startup failed!");
            uassert_false(1);
            goto __exit;
        }
    }

    while (tid6_finish_flag != 1);
    uassert_true(thread5_source == 10);

__exit:
    if (tid5 != RT_NULL)
    {
        rt_thread_delete(tid5);
    }
    return;
}

#ifndef RT_USING_SMP
static volatile rt_uint32_t yield_count;

static void test_thread_yield_inc_entry(void* parameter)
{
    rt_uint32_t loop = 0;

    while (1)
    {
        if (loop++ > 10001)
            break;
        yield_count++;
        rt_thread_yield();
    }
}

static void test_thread_yield_entry(void* parameter)
{
    rt_thread_t tid;
    rt_uint32_t loop = 0;
    rt_uint32_t count_before;

    tid = rt_thread_create("inc", test_thread_yield_inc_entry, RT_NULL,
            2048, 1, 10);
    if (!tid)
    {
        LOG_E("rt_thread_create failed!");
        return;
    }
    rt_thread_startup(tid);

    while (1)
    {
        if (loop++ > 10000)
            break;

        count_before = yield_count;
        rt_thread_yield();
        if (yield_count == count_before)
        {
            LOG_E("yield error!");
            return;
        }
    }
    thread_yield_flag = 1;
}

void test_thread_yield_nosmp(void)
{
    rt_thread_t tid;

    yield_count = 0;

    tid = rt_thread_create("chkcnt", test_thread_yield_entry, RT_NULL,
            2048, 1, 10);
    if (!tid)
    {
        LOG_E("rt_thread_create failed!");
        return;
    }
    rt_thread_startup(tid);

    uassert_true(thread_yield_flag == 1);
}
#endif

static void thread7_entry(void *parameter)
{
    while (1);
}

static void test_thread_control(void)
{
    rt_err_t rst_delete = -RT_ERROR;

    tid7 = rt_thread_create("thread7",
                            thread7_entry,
                            RT_NULL,
                            THREAD_STACK_SIZE,
                            __current_thread->current_priority + 1,
                            THREAD_TIMESLICE);
    if (tid7 == RT_NULL)
    {
        LOG_E("rt_thread_create failed!");
        uassert_false(tid7 == RT_NULL);
        goto __exit;
    }
    else
    {
        if (rt_thread_control(tid7, RT_THREAD_CTRL_STARTUP, RT_NULL) != RT_EOK)
        {
            LOG_E("rt_thread_control failed!");
            uassert_false(1);
            goto __exit;
        }
        rt_thread_mdelay(200);
    }
    rt_thread_control(tid7, RT_THREAD_CTRL_CHANGE_PRIORITY, &change_priority);
    if (tid7->current_priority != change_priority)
    {
        LOG_E("rt_thread_control failed!");
        uassert_false(1);
        goto __exit;
    }
    rst_delete = rt_thread_control(tid7, RT_THREAD_CTRL_CLOSE, RT_NULL);
    if (rst_delete != RT_EOK)
    {
        LOG_E("rt_thread_control failed!");
        uassert_false(rst_delete != RT_EOK);
        goto __exit;
    }

    uassert_true(1);

__exit:
    if (tid7 != RT_NULL && rst_delete != RT_EOK)
    {
        rt_thread_delete(tid7);
    }
    return;
}

static void thread8_entry(void *parameter)
{
    rt_thread_yield();

    while (1)
    {
        rt_enter_critical();
        time_slice1 ++;
        rt_exit_critical();
    };
}

static void thread9_entry(void *parameter)
{
    while (1)
    {
        rt_enter_critical();
        time_slice2 ++;
        rt_exit_critical();
    };
}

static void thread10_entry(void *parameter)
{
    for (; count < 10; count ++);
    thread_exit = 1;
}

static void test_thread_time_slice(void)
{
    rt_thread_t tid8 = RT_NULL, tid9 = RT_NULL;
    rt_uint8_t times = 3;
    
    tid8 = rt_thread_create("thread8",
                            thread8_entry,
                            RT_NULL,
                            THREAD_STACK_SIZE,
                            __current_thread->current_priority + 1,
                            THREAD_TIMESLICE);
    if (tid8 == RT_NULL)
    {
        LOG_E("rt_thread_create failed!");
        uassert_false(tid8 == RT_NULL);
        goto __exit;
    }
    tid9 = rt_thread_create("thread9",
                            thread9_entry,
                            RT_NULL,
                            THREAD_STACK_SIZE,
                            __current_thread->current_priority + 1,
                            THREAD_TIMESLICE);
    if (tid9 == RT_NULL)
    {
        LOG_E("rt_thread_create failed!");
        uassert_false(tid9 == RT_NULL);
        goto __exit;
    }
    rt_thread_startup(tid8);
    rt_thread_startup(tid9);
    
    time_slice1 = 0;
    time_slice2 = 0;

    while (times --)
    {
        rt_bool_t is_success = RT_FALSE;

        /* init count times */
        rt_enter_critical();
        time_slice1 = 0;
        time_slice2 = 0;
        rt_exit_critical();

        /* test 30 times about thread slice */
        rt_thread_delay(THREAD_TIMESLICE * 30);

        rt_enter_critical();
        if (time_slice2 > time_slice1)
        {
            is_success = (time_slice2 - time_slice1 < time_slice2 / 10) ? RT_TRUE : RT_FALSE;
        }
        else
        {
            is_success = (time_slice1 - time_slice2 < time_slice1 / 10) ? RT_TRUE : RT_FALSE;
        }
        if (is_success == RT_FALSE)
            LOG_E("1= %d 2 = %d\n", time_slice1, time_slice2);
        rt_exit_critical();

        uassert_true(is_success == RT_TRUE);
    }

__exit:
    if (tid8)
        rt_thread_delete(tid8);
    if (tid9)
        rt_thread_delete(tid9);
    return;
}

static void test_thread_priority(void)
{
    rt_thread_t tid10 = RT_NULL;
    thread_exit = 0;

    tid10 = rt_thread_create("thread10",
                            thread10_entry,
                            RT_NULL,
                            THREAD_STACK_SIZE,
                            __current_thread->current_priority - 1,
                            THREAD_TIMESLICE);
    if (tid10 == RT_NULL)
    {
        LOG_E("rt_thread_create failed!");
        uassert_false(tid10 == RT_NULL);
        return;
    }
    count = 0;
    rt_thread_startup(tid10);

    /* wait thread exit */
    while (!thread_exit);

    uassert_true(count == 10);
    uassert_true(thread_exit == 1);

    return;
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

    UTEST_UNIT_RUN(test_thread_dynamic);
    UTEST_UNIT_RUN(test_thread_static);
    UTEST_UNIT_RUN(test_thread_delay);
    UTEST_UNIT_RUN(test_idle_hook);
    UTEST_UNIT_RUN(test_thread_yield);
    UTEST_UNIT_RUN(test_thread_control);
    UTEST_UNIT_RUN(test_thread_priority);
    // UTEST_UNIT_RUN(test_thread_time_slice);
}
UTEST_TC_EXPORT(testcase, "src.thread.thread_tc", utest_tc_init, utest_tc_cleanup, 60); 
