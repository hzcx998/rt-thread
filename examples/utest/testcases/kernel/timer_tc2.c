/*
 * Copyright (c) 2006-2019, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2019-03-19     WillianChan  the first version
 */
#include <rthw.h>
#include <rtthread.h>
#include "utest.h"

static rt_timer_t timer1 = RT_NULL, timer2 = RT_NULL;
static struct rt_timer timer3, timer4;
static int timeout_flag1 = 0, timeout_flag2 = 0;

static void timeout1(void *parameter)
{
    if (timeout_flag2 ++ >= 4)
    {
        rt_timer_stop(timer1);
    }
}

static void timeout2(void *parameter)
{
    timeout_flag1 = 1;
}

static void test_timer_dynamic(void)
{
    timer1 = rt_timer_create("timer1", timeout1,
                             RT_NULL, 1,
                             RT_TIMER_FLAG_PERIODIC);
    if (timer1 != RT_NULL)
    {
        if (rt_timer_start(timer1) != RT_EOK)
        {
            LOG_E("rt_timer_start failed");
            uassert_false(1);
            goto __exit;
        }
    }
    else
    {
        LOG_E("rt_timer_create failed");
        uassert_false(timer1 == RT_NULL);
        goto __exit;
    }
    timer2 = rt_timer_create("timer2", timeout2,
                             RT_NULL,  1,
                             RT_TIMER_FLAG_ONE_SHOT);
    if (timer2 != RT_NULL)
    {
        if (rt_timer_start(timer2) != RT_EOK)
        {
            LOG_E("rt_timer_start failed");
            uassert_false(1);
            goto __exit;
        }
    }
    else
    {
        LOG_E("rt_timer_create failed");
        uassert_false(timer2 == RT_NULL);
        goto __exit;
    }

    while (timeout_flag1 != 1);
    while (timeout_flag2 != 5);
    if (rt_timer_delete(timer1) != RT_EOK)
    {
        LOG_E("rt_timer_delete failed!");
        uassert_false(1);
        goto __exit;
    }
    timer1 = RT_NULL;
    if (rt_timer_delete(timer2) != RT_EOK)
    {
        LOG_E("rt_timer_delete failed!");
        uassert_false(1);
        goto __exit;
    }
    timer2 = RT_NULL;
    uassert_true(1);

__exit:
    if (timer1 != RT_NULL)
    {
        rt_timer_delete(timer1);
    }
    if (timer2 != RT_NULL)
    {
        rt_timer_delete(timer2);
    }
    timeout_flag1 = 0;
    timeout_flag2 = 0;
    return;
}

static void timeout3(void *parameter)
{
    if (timeout_flag2 ++ >= 4)
    {
        rt_timer_stop(&timer3);
    }
}

static void timeout4(void *parameter)
{
    timeout_flag1 = 1;
}

static void test_timer_static(void)
{
    rt_err_t rst_start3 = -RT_ERROR;
    rt_err_t rst_start4 = -RT_ERROR;

     rt_timer_init(&timer3, "timer3",
                  timeout3, RT_NULL, 1,
                  RT_TIMER_FLAG_PERIODIC);
    rst_start3 = rt_timer_start(&timer3);
    if (rst_start3 != RT_EOK)
    {
        LOG_E("rt_timer_start failed");
        goto __exit;
    }
    rt_timer_init(&timer4, "timer4",
                  timeout4, RT_NULL, 1,
                  RT_TIMER_FLAG_ONE_SHOT);
    rst_start4 = rt_timer_start(&timer4);
    if (rst_start4 != RT_EOK)
    {
        LOG_E("rt_timer_start failed");
        goto __exit;
    }

    while (timeout_flag1 != 1);
    while (timeout_flag2 != 5);
    if (rt_timer_detach(&timer3) != RT_EOK)
    {
        LOG_E("rt_timer_detach failed!");
        uassert_false(1);
        goto __exit;
    }
    if (rt_timer_detach(&timer4) != RT_EOK)
    {
        LOG_E("rt_timer_detach failed!");
        uassert_false(1);
        goto __exit;
    }
    uassert_true(1);


__exit:
    if (rst_start3 != RT_EOK)
    {
        rt_timer_detach(&timer3);
    }
    if (rst_start4 != RT_EOK)
    {
        rt_timer_detach(&timer4);
    }
    timeout_flag1 = 0;
    timeout_flag1 = 0;
    return;
}

static void test_timer_control(void)
{
    rt_tick_t timeout_get = 0;
    rt_tick_t timeout_set = 50;
    rt_err_t rst = -RT_ERROR;
    rt_uint32_t timeout_tick_before = 0, timeout_tick_after = 0;

    timer2 = rt_timer_create("timer2", timeout2,
                             RT_NULL, 10,
                             RT_TIMER_FLAG_ONE_SHOT);
    if (rt_timer_start(timer2) != RT_EOK)
    {
        LOG_E("rt_timer_start failed");
        uassert_false(1);
        goto __exit;
    }
    rst = rt_timer_control(timer2, RT_TIMER_CTRL_GET_TIME, (void *)&timeout_get);
    if (rst != RT_EOK)
    {
        LOG_E("rt_timer_control failed");
        uassert_false(1);
        goto __exit;
    }
    if (timeout_get != 10 || rst != RT_EOK)
    {
        LOG_E("rt_timer_start failed");
        uassert_false(1);
        goto __exit;
    }
    while (timeout_flag1 != 1);
    timeout_flag1 = 0;
    timeout_tick_before = timer2->timeout_tick;
    rt_timer_control(timer2, RT_TIMER_CTRL_SET_TIME, (void *)&timeout_set);
    rt_timer_start(timer2);
    while (timeout_flag1 != 1);
    timeout_flag1 = 0;
    timeout_tick_after = timer2->timeout_tick;
    rt_timer_control(timer2, RT_TIMER_CTRL_GET_TIME, (void *)&timeout_get);
    rt_timer_start(timer2);
    while (timeout_flag1 != 1);
    uassert_true(timeout_get == 50);
    uassert_true((timeout_tick_after - timeout_tick_before) >= 50);

__exit:
    if (timer2 != RT_NULL)
    {
        rt_timer_delete(timer2);
    }
    return;
}

/**
 * create timer1, timer2
 * timeout in timer1 -> start timer2
 * timeout in timer2 -> start timer1 < if stopped >
 */
static int start_stop_count = 0;
static void timer1_timeout(void *parameter)
{
    register rt_base_t level;

    level = rt_hw_interrupt_disable();
    if (timer2 &&
            start_stop_count &&
            !(timer2->parent.flag & RT_TIMER_FLAG_ACTIVATED))
    {
        start_stop_count--;
        rt_hw_interrupt_enable(level);

        rt_kprintf("+");
        rt_timer_start(timer2);
    }
    else
    {
        rt_hw_interrupt_enable(level);
    }
}

static void timer2_timeout(void *parameter)
{
    register rt_base_t level;

    level = rt_hw_interrupt_disable();
    if (timer1 &&
            start_stop_count &&
            !(timer1->parent.flag & RT_TIMER_FLAG_ACTIVATED))
    {
        start_stop_count --;
        rt_hw_interrupt_enable(level);

        rt_kprintf("*");
        rt_timer_start(timer1);
    }
    else
    {
        rt_hw_interrupt_enable(level);
    }
}

static void test_timer_stop_start(void)
{
    start_stop_count = 20; /* repeat for 20/2 time */

    /* create timer2 firstly */
    timer2 = rt_timer_create("timer2", timer2_timeout,
            RT_NULL, 20,
            RT_TIMER_FLAG_ONE_SHOT | RT_TIMER_FLAG_SOFT_TIMER);

    timer1 = rt_timer_create("timer1", timer1_timeout,
            RT_NULL, 10,
            RT_TIMER_FLAG_ONE_SHOT | RT_TIMER_FLAG_SOFT_TIMER);
    if (rt_timer_start(timer1) != RT_EOK)
    {
        LOG_E("rt_timer_start failed");
        uassert_false(1);
        goto __exit;
    }

    while (1)
    {
        if (start_stop_count == 0)
        {
            rt_thread_mdelay(100);
            break;
        }

        rt_kprintf(".");
        rt_thread_mdelay(500);
    }
    rt_kprintf("\n");
    uassert_true(1);

__exit:
    if (timer2 != RT_NULL)
    {
        rt_timer_delete(timer2);
        timer2 = RT_NULL;
    }

    if (timer1 != RT_NULL)
    {
        rt_timer_delete(timer1);
        timer1 = RT_NULL;
    }

    return;
}

/*
 * timeout in timer1 -> start timer1 self
 */
static void self_timeout(void *parameter)
{
    register rt_base_t level;

    level = rt_hw_interrupt_disable();

    if (start_stop_count)
    {
        start_stop_count --;
        rt_kprintf(".");
        rt_timer_start(timer1);
    }

    rt_hw_interrupt_enable(level);
}

static void test_timer_self_start(void)
{
    start_stop_count = 20; /* repeat for 20 time */

    timer1 = rt_timer_create("timer", self_timeout, RT_NULL,
            30, RT_TIMER_FLAG_ONE_SHOT | RT_TIMER_FLAG_SOFT_TIMER);
    rt_timer_start(timer1);

    while (1)
    {
        if (start_stop_count == 0)
        {
            rt_thread_mdelay(100);
            break;
        }

        rt_thread_mdelay(500);
    }
    rt_kprintf("\n");
    uassert_true(1);

    rt_timer_delete(timer1);

    return ;
}

/*
 * stop a periodic timer when it's timeout.
 */
void periodic_timeout(void* parameter)
{
    if (start_stop_count == 0)
    {
        rt_timer_stop(timer1);
        return;
    }

    start_stop_count --;
}

static void timer_periodic_stop(void)
{
    start_stop_count = 10;

    timer1 = rt_timer_create("timer1", periodic_timeout, RT_NULL,
            10, RT_TIMER_FLAG_PERIODIC | RT_TIMER_FLAG_SOFT_TIMER);
    rt_timer_start(timer1);

    while (1)
    {
        if (start_stop_count == 0)
        {
            rt_thread_mdelay(500);
            break;
        }

        rt_thread_mdelay(500);
    }
    rt_kprintf("\n");
    if (timer1)
    {
        if (timer1->parent.flag & RT_TIMER_FLAG_ACTIVATED)
            uassert_true(0);
        else
            uassert_true(1);

        rt_timer_delete(timer1);
    }

    return ;
}

/*
 * soft timer
 *
 * timer1 -> timeout, then pending on semaphore
 * timer2 timeout, still pending until timer1 finished.
 * when timer1 finished, system timer thread to handle timer2 timeout
 */
static rt_sem_t timeout_sem = RT_NULL;
static void pending_timeout(void* parameter)
{
    rt_err_t ret = rt_sem_take(timeout_sem, 100);
    if (ret == -RT_ETIMEOUT)
    {
        rt_kprintf("pending timeout\n");
        timeout_flag2 = 1;
    }

    return ;
}
static void timer2_pending_timeout(void* parameter)
{
    if (timeout_flag2 == 1) timeout_flag1 = 1;
    else timeout_flag1 = 2;

    return ;
}

static void soft_timer_pending(void)
{
    timeout_flag1 = 0;
    timeout_flag2 = 0;

    timeout_sem = rt_sem_create("pending", 0, RT_IPC_FLAG_FIFO);

    timer1 = rt_timer_create("timer1", pending_timeout, RT_NULL,
            10, RT_TIMER_FLAG_ONE_SHOT | RT_TIMER_FLAG_SOFT_TIMER);
    if (timer1) rt_timer_start(timer1);

    timer2 = rt_timer_create("timer2", timer2_pending_timeout, RT_NULL,
            15, RT_TIMER_FLAG_ONE_SHOT | RT_TIMER_FLAG_SOFT_TIMER);
    if (timer2) rt_timer_start(timer2);

    while (1)
    {
        if (timeout_flag2 == 1)
        {
            rt_thread_mdelay(200);
            break;
        }

        rt_thread_mdelay(500);
    }

    uassert_true(timeout_flag1 && (timeout_flag2 == 1));

    if (timer1)
    {
        rt_timer_delete(timer1);
        timer1 = RT_NULL;
    }

    if (timer2)
    {
        rt_timer_delete(timer2);
        timer2 = RT_NULL;
    }

    if (timeout_sem)
    {
        rt_sem_delete(timeout_sem);
        timeout_sem = RT_NULL;
    }

    return;
}

/*
 * delete self
 */
static void timer1_delete_timeout(void* parameter)
{
    if (timer1)
    {
        timeout_flag1 = 0;
        rt_timer_delete(timer1);
        timer1 = RT_NULL;
    }
}

static void timer2_delete_timeout(void* parameter)
{
    timeout_flag2 --;

    if (timer2 && (timeout_flag2 == 0))
    {
        rt_timer_delete(timer2);
        timer2 = RT_NULL;
    }
}

static void timer_delete_self(void)
{
    timeout_flag1 = 5;
    timeout_flag2 = 5;

    timer1 = rt_timer_create("timer1", timer1_delete_timeout, RT_NULL,
            10, RT_TIMER_FLAG_SOFT_TIMER | RT_TIMER_FLAG_ONE_SHOT);
    if (timer1) rt_timer_start(timer1);

    timer2 = rt_timer_create("timer2", timer2_delete_timeout, RT_NULL,
            10, RT_TIMER_FLAG_SOFT_TIMER | RT_TIMER_FLAG_PERIODIC);
    if (timer2) rt_timer_start(timer2);

    while (1)
    {
        if (timeout_flag1 != 0 && timeout_flag2 != 0)
            rt_thread_mdelay(200);
        else
        {
            rt_thread_mdelay(100);
            break;
        }
    }

    uassert_true((timeout_flag1 == 0) && (timeout_flag2 == 0));
    return;
}

static rt_err_t utest_tc_init(void)
{
    timeout_flag1 = 0;
    timeout_flag2 = 0;
    return RT_EOK;
}

static rt_err_t utest_tc_cleanup(void)
{
    return RT_EOK;
}

static void testcase(void)
{
    LOG_I("in testcase func...");

    UTEST_UNIT_RUN(test_timer_dynamic);
    UTEST_UNIT_RUN(test_timer_static);
    UTEST_UNIT_RUN(test_timer_control);
    UTEST_UNIT_RUN(test_timer_stop_start);
    UTEST_UNIT_RUN(test_timer_self_start);
    UTEST_UNIT_RUN(timer_periodic_stop);
    // UTEST_UNIT_RUN(soft_timer_pending);
    // UTEST_UNIT_RUN(timer_delete_self);
}
UTEST_TC_EXPORT(testcase, "src.timer.timer_tc", utest_tc_init, utest_tc_cleanup, 60); 
