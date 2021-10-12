/*
 * Copyright (c) 2006-2019, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021-09.01     luckyzjq     the first version
 */

#include <rtthread.h>
#include <stdlib.h>
#include "utest.h"

static struct rt_mutex static_mutex;

/* init test */
static void test_static_mutex_init(void)
{
    rt_err_t result = -RT_ERROR;

    result = rt_mutex_init(&static_mutex, "static_mutex", RT_IPC_FLAG_PRIO);
    if (RT_EOK != result)
    {
        uassert_true(RT_FALSE);
    }

    result = rt_mutex_detach(&static_mutex);
    if (RT_EOK != result)
    {
        uassert_true(RT_FALSE);
    }

    result = rt_mutex_init(&static_mutex, "static_mutex", RT_IPC_FLAG_FIFO);
    if (RT_EOK != result)
    {
        uassert_true(RT_FALSE);
    }

    result = rt_mutex_detach(&static_mutex);
    if (RT_EOK != result)
    {
        uassert_true(RT_FALSE);
    }

    uassert_true(RT_TRUE);
}

static void test_static_mutex_take(void)
{
    rt_err_t result;

    result = rt_mutex_init(&static_mutex, "static_mutex", RT_IPC_FLAG_PRIO);
    if (RT_EOK != result)
    {
        uassert_true(RT_FALSE);
        return;
    }

    /* take mutex and not release */
    result = rt_mutex_take(&static_mutex, RT_WAITING_FOREVER);
    uassert_true(RT_EOK == result);

    result = rt_mutex_release(&static_mutex);
    uassert_true(RT_EOK == result);

    result = rt_mutex_trytake(&static_mutex);
    uassert_true(RT_EOK == result);

    result = rt_mutex_release(&static_mutex);
    uassert_true(RT_EOK == result);

    result = rt_mutex_take(&static_mutex, 100);
    uassert_true(RT_EOK == result);

    result = rt_mutex_release(&static_mutex);
    uassert_true(RT_EOK == result);

    result = rt_mutex_detach(&static_mutex);
    if (RT_EOK != result)
        uassert_true(RT_FALSE);

    uassert_true(RT_TRUE);
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
    UTEST_UNIT_RUN(test_static_mutex_init);
    UTEST_UNIT_RUN(test_static_mutex_take);
}
UTEST_TC_EXPORT(testcase, "testcases.kernel.mutex_tc", utest_tc_init, utest_tc_cleanup, 1000);

/********************* end of file ************************/
