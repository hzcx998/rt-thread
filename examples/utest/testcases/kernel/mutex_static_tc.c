/*
 * Copyright (c) 2006-2019, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author          Notes
 * 2019-01-26     GuanWenliang    the first version
 * 2019-06-06     WillianChan     Make the test more comprehensive
 *                                and add error resource recovery.
 * 2020-06-01     ErnestChen      update the test and add more test
 */

#include <rtthread.h>
#include "utest.h"

#define UTEST_THREAD_STACK_SIZE 4096

static struct rt_mutex test_mutex;

static volatile int exit_flag;
static volatile int th1_is_run, th2_is_run;
static volatile int th1_is_finish, th2_is_finish, th3_is_finish;
static int hthread1_is_finish, hthread2_is_finish;
static int number1 = 0, number2 = 0;

static struct rt_thread tc_mutex_thread1;
static char tc_mutex_thread1_stack[UTEST_THREAD_STACK_SIZE];
static struct rt_thread tc_mutex_thread2;
static char tc_mutex_thread2_stack[UTEST_THREAD_STACK_SIZE];
static struct rt_thread tc_mutex_thread3;
static char tc_mutex_thread3_stack[UTEST_THREAD_STACK_SIZE];

static void mutex_t1_entry(void *arg)
{
    while (1)
    {
        rt_mutex_take(&test_mutex, RT_WAITING_FOREVER);
        number1 ++;
        rt_thread_mdelay(50);
        number2 ++;
        rt_mutex_release(&test_mutex);

        if (number1 >= 50)
        {
            hthread1_is_finish = 1;
            return;
        }
    }
}

static void mutex_t2_entry(void *arg)
{
    while (1)
    {
        rt_mutex_take(&test_mutex, RT_WAITING_FOREVER);
        if (number1 != number2)
        {
            LOG_E("number1 != number2, mutex test failed!");
            rt_mutex_release(&test_mutex);
            uassert_false(1);
            return;
        }

        number1++;
        number2++;
        rt_mutex_release(&test_mutex);

        if (number1 >= 50)
        {
            hthread2_is_finish = 1;
            return;
        }
    }
}

static void test_mutex_threads(void)
{
    number1 = 0;
    number2 = 0;
    hthread1_is_finish = 0;
    hthread2_is_finish = 0;
    rt_err_t rst;

    rst = rt_mutex_init(&test_mutex, "test_mutex", RT_IPC_FLAG_FIFO);
    if (rst != RT_EOK)
    {
        LOG_E("rt_mutex_init failed");
        uassert_false(1);
        goto __exit;
    }

    rst = rt_thread_init(&tc_mutex_thread1, "mutex thread 1",
                         mutex_t1_entry, RT_NULL,
                         tc_mutex_thread1_stack,
                         sizeof(tc_mutex_thread1_stack),
                         RT_THREAD_PRIORITY_MAX / 2,
                         5);
    if (rst == RT_EOK)
    {
        rt_thread_startup(&tc_mutex_thread1);
    }
    else
    {
        LOG_E("create thr_take failed");
        goto __exit;
    }

    rst = rt_thread_init(&tc_mutex_thread2, "mutex thread 2",
                         mutex_t2_entry, RT_NULL,
                         tc_mutex_thread2_stack,
                         sizeof(tc_mutex_thread2_stack),
                         RT_THREAD_PRIORITY_MAX / 2,
                         5);
    if (rst == RT_EOK)
    {
        rt_thread_startup(&tc_mutex_thread2);
    }
    else
    {
        LOG_E("create thr_release failed");
        goto __exit;
    }

    while (!hthread1_is_finish || !hthread2_is_finish)
    {
        rt_thread_mdelay(100);
    }

    if (number1 != number2)
    {
        LOG_E("number1 != number2, mutex test failed!");
        uassert_false(1);
        goto __exit;
    }

    if (rt_mutex_detach(&test_mutex) != RT_EOK)
    {
        LOG_E("rt_mutex_detach failed");
        uassert_true(RT_FALSE);

        return;
    }

    uassert_true(RT_TRUE);
    return;

__exit:
    if (rst == RT_EOK)
    {
        rt_mutex_detach(&test_mutex);
    }

    uassert_true(RT_FALSE);
    return;
}

static void prio_inv_entry1(void *arg)
{
    rt_err_t res = RT_EOK;

    while (th2_is_run == 0)
    {
        rt_thread_mdelay(10);
    }

    th1_is_run = 1;

    res = rt_mutex_take(&test_mutex, RT_WAITING_FOREVER);
    if (res == RT_EOK)
    {
        rt_mutex_release(&test_mutex);
    }

    th1_is_finish = 1;
}

static void prio_inv_entry2(void *arg)
{
    rt_err_t res;

    res = rt_mutex_take(&test_mutex, RT_WAITING_FOREVER);

    th2_is_run = 1;

    if (res != RT_EOK)
    {
        LOG_E("prio_inv_entry2 get mutex failed");
        goto __exit;
    }

    while (exit_flag == 0)
    {
        rt_thread_mdelay(10);
    }

    rt_mutex_release(&test_mutex);

__exit:
    th2_is_finish = 1;
}

static void prio_inv_entry3(void *arg)
{
    while (th1_is_run == 0 || th2_is_run == 0)
    {
        rt_thread_mdelay(10);
    }

    rt_thread_mdelay(1000);

    uassert_true(tc_mutex_thread1.current_priority == tc_mutex_thread2.current_priority);

    exit_flag = 1;
    th3_is_finish = 1;
}

static void test_priority_inversion(void)
{
    exit_flag = 0;
    th1_is_run = 0;
    th2_is_run = 0;
    th1_is_finish = 0;
    th2_is_finish = 0;
    th3_is_finish = 0;
    rt_err_t rst;

    rst = rt_mutex_init(&test_mutex, "test_mutex", RT_IPC_FLAG_FIFO);
    if (rst != RT_EOK)
    {
        LOG_E("rt_mutex_init failed");
        uassert_true(RT_FALSE);

        return;
    }

    rst = rt_thread_init(&tc_mutex_thread3, "mutex thread 3",
                         prio_inv_entry3, RT_NULL,
                         tc_mutex_thread3_stack, sizeof(tc_mutex_thread3_stack),
                         RT_THREAD_PRIORITY_MAX / 2 - 1,
                         5);

    if (rst == RT_EOK)
    {
        rt_thread_startup(&tc_mutex_thread3);
    }
    else
    {
        LOG_E("create thr_release failed");
        goto __exit;
    }

    rst = rt_thread_init(&tc_mutex_thread1, "mutex thread 1",
                         prio_inv_entry1, RT_NULL,
                         tc_mutex_thread1_stack, sizeof(tc_mutex_thread1_stack),
                         RT_THREAD_PRIORITY_MAX / 2,
                         5);

    if (rst == RT_EOK)
    {
        rt_thread_startup(&tc_mutex_thread1);
    }
    else
    {
        LOG_E("create thr_release failed");
        goto __exit;
    }

    rst = rt_thread_init(&tc_mutex_thread2, "mutex thread 2",
                         prio_inv_entry2, RT_NULL,
                         tc_mutex_thread2_stack, sizeof(tc_mutex_thread2_stack),
                         RT_THREAD_PRIORITY_MAX / 2 + 1,
                         5);

    if (rst == RT_EOK)
    {
        rt_thread_startup(&tc_mutex_thread2);
    }
    else
    {
        LOG_E("create thr_release failed");
        goto __exit;
    }

    while (th1_is_finish != 1 || th2_is_finish != 1 || th3_is_finish != 1)
    {
        rt_thread_mdelay(100);
    }

    if (rt_mutex_detach(&test_mutex) != RT_EOK)
    {
        LOG_E("rt_mutex_detach failed");
        uassert_true(RT_FALSE);

        return;
    }

    uassert_true(RT_TRUE);
    return;

__exit:
    if (rst == RT_EOK)
    {
        rt_mutex_detach(&test_mutex);
    }
    uassert_true(RT_FALSE);

    return;
}

static struct rt_thread tc_mutex_thread4;
static char tc_mutex_thread4_stack[UTEST_THREAD_STACK_SIZE];
static struct rt_mutex thread_mutex;

static void mutex_take_variable_time(void *param)
{
    rt_err_t rst = RT_EOK;

    /* After other thread has take the mutex, the other one take zero time again. */
    rst = rt_mutex_take(&thread_mutex, RT_WAITING_NO);
    uassert_true(rst != RT_EOK);

    /* try to take the mutex and wait 1000 ticks time */
    rst = rt_mutex_take(&thread_mutex, 1000);
    uassert_true(rst != RT_EOK);
}

static void test_mutex_take_variable_time(void)
{
    rt_err_t rst;

    rst = rt_mutex_init(&thread_mutex, "test_mutex", RT_IPC_FLAG_FIFO);
    uassert_true(rst == RT_EOK);

    rst = rt_mutex_take(&thread_mutex, RT_WAITING_FOREVER);
    uassert_true(rst == RT_EOK);

    rst = rt_thread_init(&tc_mutex_thread4, "mutex",
                         mutex_take_variable_time, RT_NULL,
                         tc_mutex_thread4_stack,
                         sizeof(tc_mutex_thread4_stack),
                         rt_thread_self()->current_priority - 1, 10);
    uassert_true(rst == RT_EOK);

    rst = rt_thread_startup(&tc_mutex_thread4);
    uassert_true(rst == RT_EOK);

    rt_thread_mdelay(100);

    rst =  rt_thread_detach(&tc_mutex_thread4);
    uassert_true(rst == RT_EOK);

    rst = rt_mutex_detach(&thread_mutex);
    uassert_true(rst == RT_EOK);
}

static void test_mutex_take_deep(void)
{
    rt_err_t rst;

    rst = rt_mutex_init(&thread_mutex, "test_mutex", RT_IPC_FLAG_FIFO);
    uassert_true(rst == RT_EOK);

    /* try to take the mutex and wait 1000 ticks 1th time */
    rst = rt_mutex_take(&thread_mutex, 1000);
    uassert_true(rst == RT_EOK);

    /* try to take the mutex and wait 1000 ticks 2th time */
    rst = rt_mutex_take(&thread_mutex, 1000);
    uassert_true(rst == RT_EOK);

    rst = rt_mutex_detach(&thread_mutex);
    uassert_true(rst == RT_EOK);
}

static void mutex_entry_error_release(void *param)
{
    rt_err_t rst;
    rst = rt_mutex_release(&thread_mutex);
    uassert_true(rst != RT_EOK);
}

static void test_mutex_error_release(void)
{
    rt_err_t rst;

    rst = rt_mutex_init(&thread_mutex, "test_mutex", RT_IPC_FLAG_FIFO);
    if (rst != RT_EOK)
    {
        LOG_E("rt_mutex_init failed");
        uassert_true(rst != RT_EOK);
        return;
    }

    rst = rt_thread_init(&tc_mutex_thread4, "mutex",
                         mutex_entry_error_release, RT_NULL,
                         tc_mutex_thread4_stack,
                         sizeof(tc_mutex_thread4_stack),
                         RT_THREAD_PRIORITY_MAX / 2 + 1, 10);
    if (rst != RT_EOK)
    {
        return;
    }
    rt_thread_startup(&tc_mutex_thread4);

    rst = rt_mutex_take(&thread_mutex, RT_WAITING_FOREVER);
    if (rst != RT_EOK)
    {
        uassert_true(rst != RT_EOK);
        LOG_E("rt_mutex_init failed");
        return ;
    }

    rt_thread_mdelay(100);
    rst = rt_mutex_release(&thread_mutex);
    if (rst != RT_EOK)
    {
        LOG_E("rt_mutex_release failed");
        uassert_true(rst != RT_EOK);
        return ;
    }

    rst = rt_mutex_detach(&thread_mutex);
    uassert_true(rst == RT_EOK);

}

static void thread_entry_timeout_entry(void *param)
{
    rt_err_t rst;

    rst = rt_mutex_take(&thread_mutex, 100);
    uassert_true(rst == -RT_ETIMEOUT);
}

static void test_mutex_timeout(void)
{
    rt_err_t rst;

    rst = rt_mutex_init(&thread_mutex, "test_mutex", RT_IPC_FLAG_FIFO);
    if (rst != RT_EOK)
    {
        LOG_E("rt_mutex_init failed");
        uassert_true(rst != RT_EOK);
        return;
    }
    /* Take the mutex prevent from the other thread taking it. */
    rst = rt_mutex_take(&thread_mutex, 0);
    if (rst != RT_EOK)
    {
        uassert_true(rst != RT_EOK);
        LOG_E("rt_mutex_init failed");
        return ;
    }

    rst = rt_thread_init(&tc_mutex_thread4, "mutex",
                         thread_entry_timeout_entry,
                         RT_NULL,
                         tc_mutex_thread4_stack,
                         sizeof(tc_mutex_thread4_stack),
                         rt_thread_self()->current_priority - 1,
                         10);
    if (rst != RT_EOK)
    {
        return;
    }
    rt_thread_startup(&tc_mutex_thread4);

    /* waiting for test thread run, although it has run. */
    rt_thread_mdelay(1000);

    rst = rt_mutex_detach(&thread_mutex);
    uassert_true(rst == RT_EOK);
}


static void test_mutex_control(void)
{
    rt_err_t rst;

    rst = rt_mutex_init(&thread_mutex, "test_mutex", RT_IPC_FLAG_FIFO);
    if (rst != RT_EOK)
    {
        LOG_E("rt_mutex_init failed");
        uassert_true(rst != RT_EOK);
        return;
    }

    rst = rt_mutex_control(&thread_mutex, RT_IPC_CMD_RESET, RT_NULL);

    uassert_true(rst != RT_EOK);
    rt_mutex_detach(&thread_mutex);
}

static rt_err_t utest_tc_init(void)
{
    return RT_EOK;
}

static rt_err_t utest_tc_cleanup(void)
{
    return RT_EOK;
}

static void testcase(void)
{
    LOG_I("in testcase func...");

    UTEST_UNIT_RUN(test_mutex_threads);
    UTEST_UNIT_RUN(test_priority_inversion);
    // UTEST_UNIT_RUN(test_mutex_take_variable_time);
    // UTEST_UNIT_RUN(test_mutex_take_deep);
    // UTEST_UNIT_RUN(test_mutex_error_release);
    UTEST_UNIT_RUN(test_mutex_control);
    // UTEST_UNIT_RUN(test_mutex_timeout);
}
UTEST_TC_EXPORT(testcase, "src.ipc.mutex_static_tc", utest_tc_init, utest_tc_cleanup, 60);
