/*
 * Copyright (c) 2006-2019, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2019-01-26     MurphyZhao   the first version
 * 2020-05-10     ErnestChen   update the test and add more test
 */

#include <rtthread.h>
#include "utest.h"

#define UTEST_THREAD_STACK_SIZE 4096

static struct rt_semaphore test_sem;
static struct rt_timer tc_semaphore_timer;

static struct rt_semaphore tc_test_sem_standby;
static struct rt_semaphore tc_test_sem_resume_all;

static rt_bool_t sem_take_finish;
static rt_bool_t sem_release_finish;
static rt_bool_t sem_isr_release_finish;
static rt_bool_t sem_thread_inverse;

static struct rt_thread tc_semaphore_thread1;
static char tc_semaphore_thread1_stack[UTEST_THREAD_STACK_SIZE];
static struct rt_thread tc_semaphore_thread2;
static char tc_semaphore_thread2_stack[UTEST_THREAD_STACK_SIZE];
static struct rt_thread tc_semaphore_thread3;
static char tc_semaphore_thread3_stack[UTEST_THREAD_STACK_SIZE];
static struct rt_thread tc_semaphore_thread4;
static char tc_semaphore_thread4_stack[UTEST_THREAD_STACK_SIZE];
static struct rt_thread tc_semaphore_thread5;
static char tc_semaphore_thread5_stack[UTEST_THREAD_STACK_SIZE];
static struct rt_thread tc_semaphore_thread6;
static char tc_semaphore_thread6_stack[UTEST_THREAD_STACK_SIZE];
static struct rt_thread tc_semaphore_thread7;
static char tc_semaphore_thread7_stack[UTEST_THREAD_STACK_SIZE];

static void sem_take_thr(void *arg)
{
    rt_err_t rst = RT_EOK;
    rst = rt_sem_take(&test_sem, RT_WAITING_FOREVER);
    uassert_true(rst == RT_EOK);
    sem_take_finish = RT_TRUE;
}

static void sem_release_thr(void *arg)
{
    rt_err_t rst = RT_EOK;
    rt_thread_mdelay(200);
    rst = rt_sem_release(&test_sem);
    uassert_true(rst == RT_EOK);
    sem_release_finish = RT_TRUE;
}

static rt_err_t sync_between_threads(void)
{
    sem_take_finish = RT_FALSE;
    sem_release_finish = RT_FALSE;
    rt_err_t result = RT_EOK;

    if (rt_sem_init(&test_sem, "test_sem", 0, RT_IPC_FLAG_PRIO) != RT_EOK)
    {
        LOG_E("rt_sem_init failed");
        return -RT_ERROR;
    }

    result = rt_thread_init(&tc_semaphore_thread1,
                            "thr_take", sem_take_thr, RT_NULL,
                            tc_semaphore_thread1_stack,
                            sizeof(tc_semaphore_thread1_stack),
                            RT_THREAD_PRIORITY_MAX / 2 - 1,
                            5);

    if (result == RT_EOK)
    {
        rt_thread_startup(&tc_semaphore_thread1);
    }
    else
    {
        LOG_E("create thr_take failed");
        return -RT_ERROR;
    }

    result = rt_thread_init(&tc_semaphore_thread2,
                            "thr_release", sem_release_thr, RT_NULL,
                            tc_semaphore_thread2_stack,
                            sizeof(tc_semaphore_thread2_stack),
                            RT_THREAD_PRIORITY_MAX / 2 - 1,
                            5);

    if (result == RT_EOK)
    {
        rt_thread_startup(&tc_semaphore_thread2);
    }
    else
    {
        LOG_E("create thr_release failed");
        return -RT_ERROR;
    }

    while (!sem_take_finish || !sem_release_finish)
    {
        rt_thread_mdelay(1000);
    }

    /* add wait for 0 period, return timeout */
    result = rt_sem_trytake(&test_sem);
    uassert_true(result != RT_EOK);

    if (rt_sem_detach(&test_sem) != RT_EOK)
    {
        LOG_E("rt_sem_detach failed");
        return -RT_ERROR;
    }

    return RT_EOK;
}

static void test_sem_threads(void)
{
    uassert_true(sync_between_threads() == RT_EOK);
}

static void timer_isr_handle(void *arg)
{
    rt_err_t rst = RT_EOK;
    rst = rt_sem_release(&test_sem);
    uassert_true(rst == RT_EOK);
    rst = rt_sem_release(&test_sem);
    uassert_true(rst == RT_EOK);

    sem_isr_release_finish = RT_TRUE;
}

static rt_err_t sync_between_thread_isr(void)
{
    sem_take_finish = RT_FALSE;
    sem_isr_release_finish = RT_FALSE;
    rt_err_t result = RT_EOK;

    if (rt_sem_init(&test_sem, "test_sem", 0, RT_IPC_FLAG_FIFO) != RT_EOK)
    {
        LOG_E("rt_sem_init failed");
        return -RT_ERROR;
    }

    result = rt_thread_init(&tc_semaphore_thread1,
                            "thr_take", sem_take_thr, RT_NULL,
                            tc_semaphore_thread1_stack,
                            sizeof(tc_semaphore_thread1_stack),
                            RT_THREAD_PRIORITY_MAX / 2 - 1,
                            5);
    if (result != RT_EOK)
    {
        LOG_E("restart tc_semaphore_thread1 failed");
        return -RT_ERROR;
    }
    else
    {
        result = rt_thread_startup(&tc_semaphore_thread1);
        if (result != RT_EOK)
        {
            LOG_E("restart tc_semaphore_thread1 failed");
            return -RT_ERROR;
        }
    }

    /* RT_USING_TIMER_SOFT */
    rt_timer_init(&tc_semaphore_timer, "timer2", timer_isr_handle, RT_NULL, 100,
                  RT_TIMER_FLAG_ONE_SHOT | RT_TIMER_FLAG_HARD_TIMER);

    rt_timer_start(&tc_semaphore_timer);


    while (!sem_take_finish || !sem_isr_release_finish)
    {
        rt_thread_mdelay(1000);
    }

    if (rt_sem_detach(&test_sem) != RT_EOK)
    {
        LOG_E("rt_sem_detach failed");
        return -RT_ERROR;
    }

    return RT_EOK;
}


static void test_sem_isr_threads(void)
{
    uassert_true(sync_between_thread_isr() == RT_EOK);
}

void semaphore_prio_duplicate_release(void *parameter)
{
    rt_err_t result;

    rt_enter_critical();
    result = rt_sem_release(&tc_test_sem_standby);
    uassert_true(result == RT_EOK);
    result = rt_sem_release(&tc_test_sem_standby);
    uassert_true(result == RT_EOK);
    rt_exit_critical();
}

void semaphore_prio_duplicate_take(void *parameter)
{
    rt_err_t rst = RT_EOK;
    rst = rt_sem_take(&tc_test_sem_standby, RT_WAITING_FOREVER);
    uassert_true(rst == RT_EOK);
    rst = rt_sem_take(&tc_test_sem_standby, RT_WAITING_FOREVER);
    uassert_true(rst == RT_EOK);
}

static void test_sem_prio_flag(void)
{
    rt_err_t result = RT_EOK;

    result = rt_sem_init(&tc_test_sem_standby, "standby_sem", 0, RT_IPC_FLAG_PRIO);
    uassert_true(result == RT_EOK);

    result = rt_thread_init(&tc_semaphore_thread3,
                            "1prio_sem", semaphore_prio_duplicate_release, RT_NULL,
                            tc_semaphore_thread3_stack,
                            sizeof(tc_semaphore_thread3_stack),
                            RT_THREAD_PRIORITY_MAX / 2 - 2,
                            5);

    if (result == RT_EOK)
    {
        rt_thread_startup(&tc_semaphore_thread3);
    }
    else
    {
        LOG_E("create thr_take failed");
        uassert_false(1);
    }

    result = rt_thread_init(&tc_semaphore_thread4,
                            "2prio_sem", semaphore_prio_duplicate_take, RT_NULL,
                            tc_semaphore_thread4_stack,
                            sizeof(tc_semaphore_thread4_stack),
                            RT_THREAD_PRIORITY_MAX / 2 - 1,
                            5);

    if (result == RT_EOK)
    {
        rt_thread_startup(&tc_semaphore_thread4);
    }
    else
    {
        LOG_E("create thr_release failed");
        uassert_false(1);
    }

    while (rt_thread_find("2prio_sem") && rt_thread_find("1prio_sem"))
    {
        rt_thread_mdelay(100);
    }

    result = rt_sem_detach(&tc_test_sem_standby);
    uassert_true(result == RT_EOK);

}

static void semaphore_thread_resume(void *param)
{
    rt_err_t result = RT_EOK;

    result = rt_sem_take(&tc_test_sem_resume_all, RT_WAITING_FOREVER);
    uassert_true(result != RT_EOK);
    sem_take_finish = RT_TRUE;
}

static void test_sem_resume_all(void)
{
    rt_err_t result = RT_EOK;

    if (rt_sem_init(&tc_test_sem_resume_all, "resume_sem", 0, RT_IPC_FLAG_PRIO) != RT_EOK)
    {
        LOG_E("resume thread sem init failed");
        uassert_false(1);
        return;
    }
    else
    {
        uassert_true(1);
    }

    result = rt_thread_init(&tc_semaphore_thread5,
                            "rsm_sem", semaphore_thread_resume, RT_NULL,
                            tc_semaphore_thread5_stack,
                            sizeof(tc_semaphore_thread5_stack),
                            RT_THREAD_PRIORITY_MAX / 2 - 1,
                            5);

    if (result == RT_EOK)
    {
        rt_thread_startup(&tc_semaphore_thread5);
    }
    else
    {
        LOG_E("create thr_take failed");
        uassert_false(1);
    }
    sem_take_finish = RT_FALSE;

    /* wait thread start */
    rt_thread_mdelay(100);
    result = rt_sem_control(&tc_test_sem_resume_all, RT_IPC_CMD_RESET, 0);
    uassert_true(result == RT_EOK);
    result = rt_sem_control(&tc_test_sem_resume_all, RT_IPC_CMD_UNKNOWN, 0);
    uassert_true(result != RT_EOK);

    /* wait thread resume */
    rt_thread_mdelay(100);
    uassert_true(sem_take_finish == RT_TRUE);

    result = rt_sem_detach(&tc_test_sem_resume_all);
    uassert_true(result == RT_EOK);
}

static void semaphore_thread_inverse1(void *arg)
{
    rt_err_t rst = RT_EOK;
    rst = rt_sem_take(&tc_test_sem_standby, RT_WAITING_FOREVER);
    uassert_true(rst == RT_EOK);

    sem_thread_inverse = RT_TRUE;
}

static void semaphore_thread_inverse2(void *arg)
{
    rt_err_t rst = RT_EOK;
    rst = rt_sem_take(&tc_test_sem_standby, RT_WAITING_FOREVER);
    uassert_true(rst == RT_EOK);
    uassert_true(sem_thread_inverse == RT_TRUE);
}

static void test_sem_thread_inverse(void)
{
    rt_err_t result = RT_EOK;

    result = rt_sem_init(&tc_test_sem_standby, "tc_sem", 0, RT_IPC_FLAG_PRIO) ;
    uassert_true(result == RT_EOK);

    result = rt_thread_init(&tc_semaphore_thread6,
                            "rsm_sem", semaphore_thread_inverse1, RT_NULL,
                            tc_semaphore_thread6_stack,
                            sizeof(tc_semaphore_thread6_stack),
                            RT_THREAD_PRIORITY_MAX / 2,
                            5);

    if (result == RT_EOK)
    {
        rt_thread_startup(&tc_semaphore_thread6);
        uassert_true(result == RT_EOK);
    }
    else
    {
        LOG_E("create thr_take failed");
        uassert_false(1);
    }

    result = rt_thread_init(&tc_semaphore_thread7,
                            "rsm_sem", semaphore_thread_inverse2, RT_NULL,
                            tc_semaphore_thread7_stack,
                            sizeof(tc_semaphore_thread7_stack),
                            RT_THREAD_PRIORITY_MAX / 2 + 1,
                            5);

    if (result == RT_EOK)
    {
        rt_thread_startup(&tc_semaphore_thread7);
        uassert_true(result == RT_EOK);
    }
    else
    {
        LOG_E("create thr_take failed");
        uassert_false(1);
    }

    sem_thread_inverse = RT_FALSE;

    /* wait thread start */
    rt_thread_mdelay(100);

    rt_sem_release(&tc_test_sem_standby);
    rt_sem_release(&tc_test_sem_standby);

    rt_thread_mdelay(100);
    result = rt_sem_detach(&tc_test_sem_standby);
    uassert_true(result == RT_EOK);
}

static void sem_release_thread_entry(void *parameter)
{
    sem_release_finish = RT_TRUE;

    rt_sem_release(&test_sem);
}

static void test_sem_positive_time(void)
{
    rt_err_t result = RT_EOK;

    sem_release_finish = RT_FALSE;

    result = rt_sem_init(&test_sem, "tc_sem", 0, RT_IPC_FLAG_PRIO) ;
    uassert_true(result == RT_EOK);

    result = rt_thread_init(&tc_semaphore_thread1,
                            "sem_test_thread",
                            sem_release_thread_entry,
                            RT_NULL,
                            tc_semaphore_thread1_stack,
                            sizeof(tc_semaphore_thread1_stack),
                            rt_thread_self()->init_priority + 1,
                            5);

    if (result == RT_EOK)
    {
        rt_thread_startup(&tc_semaphore_thread1);
        uassert_true(result == RT_EOK);
    }
    else
    {
        LOG_E("create thr_take failed");
        uassert_false(1);
    }

    /* Wait for the semaphore positive time and the test thread will run. */
    result = rt_sem_take(&test_sem, 1000);
    uassert_true(result == RT_EOK);

    rt_sem_detach(&test_sem);

    uassert_true(sem_release_finish == RT_TRUE);
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
    UTEST_UNIT_RUN(test_sem_threads);
    UTEST_UNIT_RUN(test_sem_isr_threads);
    UTEST_UNIT_RUN(test_sem_prio_flag);
    UTEST_UNIT_RUN(test_sem_resume_all);
    UTEST_UNIT_RUN(test_sem_thread_inverse);
    UTEST_UNIT_RUN(test_sem_positive_time);
}
UTEST_TC_EXPORT(testcase, "src.ipc.semaphore_static_tc", utest_tc_init, utest_tc_cleanup, 60);
