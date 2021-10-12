/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021-08-15     supperthomas add irq_test
 */

#include <rtthread.h>
#include "utest.h"
#include "rthw.h"

#define UTEST_NAME "irq_tc"

#define WAIT_MS 5

static void test_ticks(void)
{
    rt_tick_t old = rt_tick_get();
    rt_tick_set(old + 1);
    uassert_int_equal(rt_tick_get(), old + 1);
}

static int thread_exit_flags;

static void test_thread(void *arg)
{
    int i;
    for (i = 0; i < WAIT_MS; i++)
    {
        rt_thread_mdelay(1000);
        rt_kprintf(".");
    }
    rt_kprintf("delay end.\n");
    thread_exit_flags = 1;
}

static void test_run_time(void)
{
    rt_err_t err = -RT_ERROR;
    rt_tick_t ticks_start;
    rt_tick_t ticks_end;

    rt_thread_t thread = rt_thread_create("test1", test_thread, RT_NULL, 4096, RT_THREAD_PRIORITY_MAX / 2 - 1, 10);
    if (thread == RT_NULL)
    {
        LOG_E("rt_thread_create failed!");
        uassert_false(thread == RT_NULL);
        goto __exit;
    }
    else
    {
        ticks_start = rt_tick_get_millisecond();
        err = rt_thread_startup(thread);
        if (err != RT_EOK)
        {
            LOG_E("rt_thread_startup failed!");
            uassert_false(err != RT_EOK);
            goto __exit;
        }
    }

    while (!thread_exit_flags)
    {
    }
    ticks_end = rt_tick_get_millisecond();

    /* calc run time */
    uassert_in_range(ticks_end - ticks_start, WAIT_MS * 1000, (WAIT_MS + 1) * 1000);
    return;
__exit:
    if (thread != RT_NULL && err != RT_EOK)
    {
        rt_thread_delete(thread);
    }
    return;
}

static rt_err_t utest_tc_init(void)
{
    thread_exit_flags = 0;
    return RT_EOK;
}

static rt_err_t utest_tc_cleanup(void)
{
    return RT_EOK;
}

static void testcase(void)
{
    UTEST_UNIT_RUN(test_ticks);
    UTEST_UNIT_RUN(test_run_time);
}
UTEST_TC_EXPORT(testcase, "testcases.kernel.clock_tc", utest_tc_init, utest_tc_cleanup, 10);
