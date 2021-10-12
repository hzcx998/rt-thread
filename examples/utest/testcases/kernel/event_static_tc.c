/*
 * Copyright (c) 2006-2019, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author          Notes
 * 2019-03-15     WillianChan     the first version
 * 2020-05-27     ErnestChen      update the test and add more test
 */

#include <rtthread.h>
#include "utest.h"

#define THREAD_PRIORITY   (RT_THREAD_PRIORITY_MAX / 2)
#define THREAD_STACK_SIZE 4096
#define THREAD_TIMESLICE  5
#define EVENT_FLAG3       (1 << 3)
#define EVENT_FLAG5       (1 << 5)
#define EVENT_FLAG7       (1 << 7)
#define EVENT_FLAG9       (1 << 9)

static struct rt_event event_static;
static rt_uint32_t recv_event = 0;
static rt_uint32_t recv_event_flag1 = 0, recv_event_flag2 = 0;
static rt_uint32_t thread_finish_flag3 = 0, thread_finish_flag4 = 0;

static struct rt_thread tc_event_thread3;
static char tc_event_thread3_stack[THREAD_STACK_SIZE];
static struct rt_thread tc_event_thread4;
static char tc_event_thread4_stack[THREAD_STACK_SIZE];

static void thread3_entry(void *parameter)
{
    rt_err_t rst1 = RT_ERROR, rst2 = RT_ERROR;

    rst1 = rt_event_recv(&event_static, (EVENT_FLAG3 | EVENT_FLAG5),
                         RT_EVENT_FLAG_OR | RT_EVENT_FLAG_CLEAR,
                         RT_WAITING_FOREVER, &recv_event);
    if (rst1 != RT_EOK)
    {
        LOG_E("event_receive failed!");
        return;
    }
    else
    {
        recv_event_flag1 = recv_event;
    }

    rt_thread_mdelay(50);

    rst2 = rt_event_recv(&event_static, (EVENT_FLAG3 | EVENT_FLAG5),
                         RT_EVENT_FLAG_AND | RT_EVENT_FLAG_CLEAR,
                         RT_WAITING_FOREVER, &recv_event);
    if (rst2 != RT_EOK)
    {
        LOG_E("event_receive failed!");
        return;
    }
    else
    {
        recv_event_flag2 = recv_event;
    }

    thread_finish_flag3 = 1;
}

static void thread4_entry(void *parameter)
{
    rt_event_send(&event_static, EVENT_FLAG3);
    rt_thread_mdelay(10);
    rt_event_send(&event_static, EVENT_FLAG5);
    rt_thread_mdelay(10);
    rt_event_send(&event_static, EVENT_FLAG3);

    thread_finish_flag4 = 1;
}

static void test_event_static(void)
{
    rt_err_t rst_event_init = -RT_ERROR;
    rt_err_t rst_event_detach = -RT_ERROR;
    rt_err_t result = RT_EOK;

    /* initialize a event object with RT_IPC_FLAG_PRIO flag */
    rst_event_init = rt_event_init(&event_static, "event_static", RT_IPC_FLAG_PRIO);
    if (rst_event_init != RT_EOK)
    {
        LOG_E("rt_event_init failed!");
        uassert_false(1);
        goto __exit;
    }

    result = rt_thread_init(&tc_event_thread3, "thread3",
                            thread3_entry,
                            RT_NULL,
                            tc_event_thread3_stack,
                            sizeof(tc_event_thread3_stack),
                            THREAD_PRIORITY,
                            THREAD_TIMESLICE);
    if (result == RT_EOK)
    {
        if (rt_thread_startup(&tc_event_thread3) != RT_EOK)
        {
            LOG_E("rt_thread_startup failed!");
            uassert_false(1);
            goto __exit;
        }
    }
    else
    {
        LOG_E("rt_thread_create failed!");
        uassert_false(1);
        goto __exit;
    }

    result = rt_thread_init(&tc_event_thread4, "thread4",
                            thread4_entry,
                            RT_NULL,
                            tc_event_thread4_stack,
                            sizeof(tc_event_thread4_stack),
                            THREAD_PRIORITY,
                            THREAD_TIMESLICE);
    if (result == RT_EOK)
    {
        if (rt_thread_startup(&tc_event_thread4) != RT_EOK)
        {

            LOG_E("rt_thread_startup failed!");
            uassert_false(1);
            goto __exit;
        }
    }
    else
    {
        LOG_E("rt_thread_create failed!");
        uassert_false(1);
        goto __exit;
    }

    /* wait for thread3 and thread4 exit  */
    while (thread_finish_flag3 != 1 || thread_finish_flag4 != 1)
    {
        rt_thread_delay(1);
    }

    /* if the first flag is (EVENT_FLAG3) and the second flag is (EVENT_FLAG3 | EVENT_FLAG5),
     * then detach the event object */
    if (recv_event_flag1 == EVENT_FLAG3 && (recv_event_flag2 == (EVENT_FLAG3 | EVENT_FLAG5)))
    {
        rst_event_detach = rt_event_detach(&event_static);
        if (rst_event_detach != RT_EOK)
        {
            LOG_E("rt_event_detach failed!");
            uassert_false(1);
            goto __exit;
        }
        uassert_true(1);
    }
    else
    {
        rst_event_detach = rt_event_detach(&event_static);
        if (rst_event_detach != RT_EOK)
        {
            LOG_E("rt_event_detach failed!");
            uassert_false(1);
            goto __exit;
        }
        LOG_E("rt_event_send_receive failed!");
        uassert_false(1);
    }

__exit:
    thread_finish_flag3 = 0;
    thread_finish_flag4 = 0;
    if (rst_event_detach != RT_EOK)
    {
        rt_event_detach(&event_static);
    }
    return;
}

static struct rt_thread tc_event_thread1;
static char tc_event_thread1_stack[THREAD_STACK_SIZE];

static void thread1_entry(void *param)
{
    rt_err_t result = RT_EOK;

    /* time is not negative and bigger than 0 */
    result = rt_event_recv(&event_static,
                           EVENT_FLAG3 | EVENT_FLAG5,
                           RT_EVENT_FLAG_AND | RT_EVENT_FLAG_CLEAR,
                           100000, 0);

    uassert_true(result == RT_EOK);
}

static void test_event_special(void)
{
    rt_err_t result = RT_EOK;

    result = rt_event_init(&event_static, "event_special", RT_IPC_FLAG_FIFO);
    if (result != RT_EOK)
    {
        LOG_E("rt_event init failed!");
        uassert_false(1);
        return;
    }

    result = rt_thread_init(&tc_event_thread1, "vnt_thr",
                            thread1_entry,
                            RT_NULL,
                            tc_event_thread1_stack,
                            sizeof(tc_event_thread1_stack),
                            THREAD_PRIORITY,
                            THREAD_TIMESLICE);
    if (result != RT_EOK)
    {
        LOG_E("rt_thread init failed!");
        uassert_false(1);
        return;
    }

    if (rt_thread_startup(&tc_event_thread1) != RT_EOK)
    {
        LOG_E("rt_thread_startup failed!");
        uassert_false(1);
        return;
    }

    /* send and receive: normal */
    result = rt_event_send(&event_static, EVENT_FLAG3 | EVENT_FLAG5);
    uassert_true(result == RT_EOK);
    rt_thread_mdelay(10);

    /* set event set to 0 */
    result = rt_event_send(&event_static, 0);
    uassert_true(result != RT_EOK);

#ifdef RT_DEBUG
    /* test when receive event set is 0, the result must not be RT_EOK */
    result = rt_event_recv(&event_static,
                           0,
                           RT_EVENT_FLAG_AND | RT_EVENT_FLAG_CLEAR,
                           100000, 0);
    uassert_true(result != RT_EOK);
#endif

    /* test when set wait time is set to 0, the result must not be RT_EOK */
    result = rt_event_recv(&event_static,
                           EVENT_FLAG3 | EVENT_FLAG5,
                           RT_EVENT_FLAG_AND | RT_EVENT_FLAG_CLEAR,
                           0, 0);
    uassert_true(result != RT_EOK);

    /* event control: reset */
    result = rt_event_control(&event_static, RT_IPC_CMD_RESET, 0);
    uassert_true(result == RT_EOK);

    /* event control: unknown */
    result = rt_event_control(&event_static, RT_IPC_CMD_UNKNOWN, 0);
    uassert_true(result != RT_EOK);

    while (rt_thread_find("vnt_thr") != RT_NULL)
    {
        rt_thread_mdelay(100);
    }
    result = rt_event_detach(&event_static);
    uassert_true(result == RT_EOK);
}

static void thread1_entry_or_recv(void *param)
{
    rt_err_t result = RT_EOK;

    /* wait the event from other thread for a while */
    result = rt_event_recv(&event_static,
                           EVENT_FLAG3 | EVENT_FLAG5,
                           RT_EVENT_FLAG_OR | RT_EVENT_FLAG_CLEAR,
                           1000, 0);

    /* if the result is not RT_EOK means test fails */
    if (result != RT_EOK)
    {
        uassert_false(1);
    }

    /* waiting for another thread send event */
    rt_thread_mdelay(100);

    /* recieve after other thread has send event */
    result = rt_event_recv(&event_static,
                           EVENT_FLAG7 | EVENT_FLAG9,
                           RT_EVENT_FLAG_AND | RT_EVENT_FLAG_CLEAR,
                           1000, 0);
    uassert_true(result == RT_EOK);
}

static void test_event_send_before_recv(void)
{
    rt_err_t result = RT_EOK;

    result = rt_event_init(&event_static, "event_special", RT_IPC_FLAG_FIFO);
    if (result != RT_EOK)
    {
        LOG_E("rt_event init failed!");
        uassert_false(1);
        return;
    }

    result = rt_event_send(&event_static, EVENT_FLAG3 | EVENT_FLAG5);
    uassert_true(result == RT_EOK);

    result = rt_thread_init(&tc_event_thread1, "vnt_thr",
                            thread1_entry_or_recv,
                            RT_NULL,
                            tc_event_thread1_stack,
                            sizeof(tc_event_thread1_stack),
                            THREAD_PRIORITY,
                            THREAD_TIMESLICE);
    if (result != RT_EOK)
    {
        LOG_E("rt_thread init failed!");
        uassert_false(1);
        return;
    }

    if (rt_thread_startup(&tc_event_thread1) != RT_EOK)
    {
        LOG_E("rt_thread_startup failed!");
        uassert_false(1);
        return;
    }

    /* waiting for another thread receive event */
    rt_thread_mdelay(10);

    /* send event to another thread */
    result = rt_event_send(&event_static, EVENT_FLAG7 | EVENT_FLAG9);

    while (rt_thread_find("vnt_thr") != RT_NULL)
    {
        rt_thread_mdelay(100);
    }
    result = rt_event_detach(&event_static);
    uassert_true(result == RT_EOK);
}

static void thread_entry_and(void *param)
{
    rt_err_t result = RT_EOK;
    /* recieve after other thread has send event */
    result = rt_event_recv(&event_static,
                           EVENT_FLAG7 | EVENT_FLAG9,
                           RT_EVENT_FLAG_AND,
                           1000, 0);
    if (result != RT_EOK)
    {
        LOG_E("rt_event receive failed!");
        uassert_false(result != RT_EOK);
        return;
    }
}

static void test_event_receive_and(void)
{
    rt_err_t result = RT_EOK;

    result = rt_event_init(&event_static, "timeout", RT_IPC_FLAG_FIFO);
    if (result != RT_EOK)
    {
        LOG_E("rt_event init failed!");
        uassert_false(result != RT_EOK);
        return;
    }

    result = rt_event_send(&event_static, EVENT_FLAG7);
    if (result != RT_EOK)
    {
        LOG_E("rt_event send failed!");
        uassert_false(result != RT_EOK);
        return;
    }

    result = rt_thread_init(&tc_event_thread1, "vnt_thr",
                            thread_entry_and,
                            RT_NULL,
                            tc_event_thread1_stack,
                            sizeof(tc_event_thread1_stack),
                            rt_thread_self()->current_priority - 1,
                            THREAD_TIMESLICE);
    if (result != RT_EOK)
    {
        LOG_E("rt_thread init failed!");
        uassert_false(1);
        return;
    }

    if (rt_thread_startup(&tc_event_thread1) != RT_EOK)
    {
        LOG_E("rt_thread_startup failed!");
        uassert_false(1);
        return;
    }

    /* Send event to another thread.
     * The other thread will satisfy to receive the event.
     */
    rt_event_send(&event_static, EVENT_FLAG9);

    while (rt_thread_find("vnt_thr") != RT_NULL)
    {
        rt_thread_mdelay(100);
    }
    result = rt_event_detach(&event_static);
    uassert_true(result == RT_EOK);
}

static void test_event_receive_timeout(void)
{
    rt_err_t result = RT_EOK;

    result = rt_event_init(&event_static, "timeout", RT_IPC_FLAG_FIFO);
    if (result != RT_EOK)
    {
        LOG_E("rt_event init failed!");
        uassert_false(result != RT_EOK);
        return;
    }

    /* There is not the event. */
    result = rt_event_recv(&event_static,
                           EVENT_FLAG7 | EVENT_FLAG9,
                           RT_EVENT_FLAG_AND,
                           10, 0);
    uassert_true(result == -RT_ETIMEOUT);

    rt_event_detach(&event_static);
}

static rt_err_t utest_tc_init(void)
{
    thread_finish_flag3 = 0;
    thread_finish_flag4 = 0;
    return RT_EOK;
}

static rt_err_t utest_tc_cleanup(void)
{
    return RT_EOK;
}

static void testcase(void)
{
    LOG_I("in testcase func...");

    UTEST_UNIT_RUN(test_event_static);
    // UTEST_UNIT_RUN(test_event_special);
    UTEST_UNIT_RUN(test_event_send_before_recv);
    UTEST_UNIT_RUN(test_event_receive_and);
    UTEST_UNIT_RUN(test_event_receive_timeout);
}
UTEST_TC_EXPORT(testcase, "src.ipc.event_static_tc", utest_tc_init, utest_tc_cleanup, 60);
