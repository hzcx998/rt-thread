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
#include <rtconfig.h>
#include "utest.h"

static void test_delay_until(void)
{
    rt_tick_t check_tick = 0;
    rt_tick_t delta = 0;

    if (RT_TICK_PER_SECOND >= 50)
    {
        check_tick = rt_tick_get();
        rt_thread_delay_until(&check_tick, 100);
        delta = rt_tick_get() - check_tick;
        rt_kprintf("delta[100] -> %d\n", delta);
        uassert_in_range(delta, 100-1,100+1);

        check_tick = rt_tick_get();
        rt_thread_delay(2);
        rt_thread_delay_until(&check_tick, 200);
        delta = rt_tick_get() - check_tick;
        rt_kprintf("delta[200] -> %d\n", delta);
        uassert_in_range(delta, 200-1,200+1);

        check_tick = rt_tick_get();
        rt_thread_delay(2);
        rt_thread_delay_until(&check_tick, 300);
        delta = rt_tick_get() - check_tick;
        rt_kprintf("delta[300] -> %d\n", delta);
        uassert_in_range(delta, 300-1,300+1);

        check_tick = rt_tick_get();
        rt_thread_delay(2);
        rt_thread_delay_until(&check_tick, 100);
        delta = rt_tick_get() - check_tick;
        uassert_in_range(delta, 100-1,100+1);
    }

    check_tick = rt_tick_get();
    rt_thread_delay(2);
    rt_thread_delay_until(&check_tick, 50);
    delta = rt_tick_get() - check_tick;
    rt_kprintf("delta[50] -> %d\n", delta);
    uassert_in_range(delta, 50-1, 50+1);

    check_tick = rt_tick_get();
    rt_thread_delay(2);
    rt_thread_delay_until(&check_tick, 25);
    delta = rt_tick_get() - check_tick;
    rt_kprintf("delta[25] -> %d\n", delta);
    uassert_in_range(delta, 25-1, 25+1);

    check_tick = rt_tick_get();
    rt_thread_delay_until(&check_tick, 10);
    delta = rt_tick_get() - check_tick;
    rt_kprintf("delta[10] -> %d\n", delta);
    uassert_in_range(delta, 10-1, 10+1);

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

    UTEST_UNIT_RUN(test_delay_until);
}
UTEST_TC_EXPORT(testcase, "src.thread.delay_tc", utest_tc_init, utest_tc_cleanup, 60); 
