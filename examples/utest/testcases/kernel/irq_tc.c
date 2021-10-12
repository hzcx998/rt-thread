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
#include <interrupt.h>
#include <board.h>

#define UTEST_NAME "irq_tc"
static uint32_t irq_count = 0;
static uint32_t max_get_nest_count = 0;
static uint32_t irq_handler_ok = 0;
static uint32_t irq_handler_end = 0;

#ifndef SOC_BM3883
#define SOC_BM3883
#endif

static void irq_callback()
{
    if(rt_interrupt_get_nest() >  max_get_nest_count)
    {
        max_get_nest_count = rt_interrupt_get_nest();
    }
    irq_count ++;
}

static void irq_test(void)
{
    irq_count = 0;
    rt_interrupt_enter_sethook(irq_callback);
    rt_interrupt_leave_sethook(irq_callback);
    rt_thread_mdelay(10);
    LOG_D("%s test irq_test! irq_count %d  max_get_nest_count %d\n", UTEST_NAME, irq_count, max_get_nest_count);
    uassert_int_not_equal(0, irq_count);
    uassert_int_not_equal(0, max_get_nest_count);
    rt_interrupt_enter_sethook(RT_NULL);
    rt_interrupt_leave_sethook(RT_NULL);
    LOG_D("irq_test OK!\n");
}

static void interrupt_test(void)
{
    rt_base_t level;
    uint32_t i = 1000;

    rt_interrupt_enter_sethook(irq_callback);
    rt_interrupt_leave_sethook(irq_callback);
    irq_count = 0;
    level = rt_hw_interrupt_disable();
    while(i)
    {
        i --;
    }
    uassert_int_equal(0, irq_count);
    rt_hw_interrupt_enable(level);
    rt_interrupt_enter_sethook(RT_NULL);
    rt_interrupt_leave_sethook(RT_NULL);
}

#ifdef SOC_BM3883
#include <bm3883_timer.h>
static void test_handler(int vector, void *arg)
{
    bm3883_timer_update_counter(RT_HW_SYSTEM_TIMERID);
    irq_handler_ok = 1;
}

static void test_interrupt_handler(void)
{
    rt_isr_handler_t old_handler;
    irq_handler_ok = 0;

    /* mask timer irq, no irq handler occur */
    rt_hw_interrupt_mask(IRQ_TIMER1);
    old_handler = rt_hw_interrupt_install(IRQ_TIMER1, test_handler, RT_NULL, "timer");
    rt_hw_interrupt_umask(IRQ_TIMER1);
    
    while (!irq_handler_ok)
    {
    }

    /* handler occur! */
    uassert_true(irq_handler_ok == 1);

    /* mask timer irq, no irq handler occur */
    rt_hw_interrupt_mask(IRQ_TIMER1);
    old_handler = rt_hw_interrupt_install(IRQ_TIMER1, old_handler, RT_NULL, "timer");
    rt_hw_interrupt_umask(IRQ_TIMER1);
    uassert_true(old_handler == test_handler);
}

static int check_timeout = 0;

static void test_priority_handler1(int vector, void *arg)
{
    if (!irq_handler_end)
    {
        irq_handler_ok = 0;
        rt_kprintf("#");
    }
    bm3883_timer_update_counter(1);
}

static void test_priority_handler2(int vector, void *arg)
{
    if (!irq_handler_end)
    {
        check_timeout++;
        if (check_timeout > 1000)
        {
            irq_handler_ok = 1;
            irq_handler_end = 1;
            
            /* stop timer */
            bm3883_timer_mask(2);
            bm3883_timer_stop(2);
        }
        if (check_timeout % 200)
        {
            rt_kprintf(".");
        }
    }
    bm3883_timer_update_counter(2);
}

static void test_interrupt_priority(void)
{
    rt_isr_handler_t old_timer1_handler;
    rt_isr_handler_t old_timer2_handler;

    irq_handler_end = 0;
    irq_handler_ok = 0;
    check_timeout = 0;

    /* timer 2 priority is higher than timer 1 */
    old_timer1_handler = bm3883_timer_init(1, test_priority_handler1, RT_NULL);
    bm3883_timer_set_reload_value(1, TIMER_CNT_VAL_DEFAULT);
    bm3883_timer_start(1);
    bm3883_timer_umask(1);

    old_timer2_handler = bm3883_timer_init(2, test_priority_handler2, RT_NULL);
    bm3883_timer_set_reload_value(2, TIMER_CNT_VAL_DEFAULT / 50);
    bm3883_timer_start(2);
    bm3883_timer_umask(2);

    while (!irq_handler_end)
    {
    }
    rt_kprintf("\n");
    uassert_true(irq_handler_ok == 1);
    
    /* stop timer */
    bm3883_timer_mask(1);
    bm3883_timer_stop(1);
    /* uninstall timer */
    uassert_true(bm3883_timer_init(1, old_timer1_handler, RT_NULL) == test_priority_handler1);
    
    /* uninstall timer */
    uassert_true(bm3883_timer_init(2, old_timer2_handler, RT_NULL) == test_priority_handler2);
}
#endif

static rt_err_t utest_tc_init(void)
{
    irq_count = 0;
    max_get_nest_count = 0;
    irq_handler_ok = 0;
    irq_handler_end = 0;
    check_timeout = 0;
    
    return RT_EOK;
}

static rt_err_t utest_tc_cleanup(void)
{
    return RT_EOK;
}

static void testcase(void)
{
    UTEST_UNIT_RUN(irq_test);
    UTEST_UNIT_RUN(interrupt_test);

#ifdef SOC_BM3883
    UTEST_UNIT_RUN(test_interrupt_handler);
    UTEST_UNIT_RUN(test_interrupt_priority);
#endif

}
UTEST_TC_EXPORT(testcase, "testcases.kernel.irq_tc", utest_tc_init, utest_tc_cleanup, 10);
