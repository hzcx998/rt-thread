/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021-08-28     Sherman      the first version
 */

#include <rtthread.h>
#include "utest.h"

/**
 * case1: 创建一个消息队列，2个线程，一个接收，一个发送。
 * case2: 创建一个消息队列，2个线程，一个发送并等待，一个一只等待，收到消息后回复，并结束。
 * case3: 创建一个消息队列，2个线程，一个非阻塞发送，一个非阻塞接收。
 */

#define UTEST_THREAD_STACK_SIZE 4096

static struct rt_thread mq_send_thread;
static struct rt_thread mq_recv_thread;

/* 消息队列是异步的，用于保障信息打印顺序 */
static struct rt_mutex mq_mutex;

static rt_uint8_t mq_send_stack[UTEST_THREAD_STACK_SIZE];
static rt_uint8_t mq_recv_stack[UTEST_THREAD_STACK_SIZE];

static struct rt_event finish_e;
#define MQSEND_FINISH   0x01
#define MQRECV_FINIHS   0x02

#define MSG_SIZE    4
#define MAX_MSGS    5
static struct rt_messagequeue static_mq;
static rt_uint8_t mq_buf[(MSG_SIZE + 4) * MAX_MSGS];

static void test_mq_init(void)
{
    rt_err_t ret;
    ret = rt_mq_init(&static_mq,"testmq1", mq_buf, MSG_SIZE, sizeof(mq_buf), RT_IPC_FLAG_FIFO);
    uassert_true(ret == RT_EOK);

    ret = rt_mutex_init(&mq_mutex,"sync", RT_IPC_FLAG_PRIO);
    uassert_true(ret == RT_EOK);
}

static void test_mq_detach(void)
{
    rt_err_t ret = rt_mq_detach(&static_mq);
    uassert_true(ret == RT_EOK);

    ret = rt_mutex_detach(&mq_mutex);
    uassert_true(ret == RT_EOK);
}

static void mq_send_case(rt_mq_t testmq)
{
    /* 同步发送消息 */
    rt_uint32_t send_buf[MAX_MSGS+1] = {0};
    rt_err_t ret = RT_EOK;

    for (int var = 0; var < MAX_MSGS; ++var)
    {
        send_buf[var] = var + 1;
        ret = rt_mq_send_wait(testmq, &send_buf[var], sizeof(send_buf[0]), RT_WAITING_FOREVER);
        rt_mutex_take(&mq_mutex, RT_WAITING_FOREVER);
        uassert_true(ret == RT_EOK);
        rt_mutex_release(&mq_mutex);
    }

    send_buf[MAX_MSGS] = MAX_MSGS + 1;

    ret = rt_mq_send_wait(testmq, &send_buf[MAX_MSGS], sizeof(send_buf[0]), RT_WAITING_FOREVER);
    rt_mutex_take(&mq_mutex, RT_WAITING_FOREVER);
    uassert_true(ret == RT_EOK);
    rt_mutex_release(&mq_mutex);
}

static void mq_recv_case(rt_mq_t testmq)
{
    /* 同步接收消息 */
    rt_uint32_t recv_buf[MAX_MSGS+1] = {0};
    rt_err_t ret = RT_EOK;

    for (int var = 0; var < MAX_MSGS + 1; ++var)
    {
        ret = rt_mq_recv(testmq, &recv_buf[var], sizeof(recv_buf[0]), RT_WAITING_FOREVER);
        rt_mutex_take(&mq_mutex, RT_WAITING_FOREVER);
        uassert_true(ret == RT_EOK);
        uassert_true(recv_buf[var] == (var + 1));
        rt_mutex_release(&mq_mutex);
    }
}

static void mq_send_entry(void *param)
{
    mq_send_case(&static_mq);
    
    rt_event_send(&finish_e, MQSEND_FINISH);
}

static void mq_recv_entry(void *param)
{
    /* recv msg */
    mq_recv_case(&static_mq);

    rt_event_send(&finish_e, MQRECV_FINIHS);
}

static void test_mq_send_recv(void)
{
    rt_err_t ret ;
    ret = rt_thread_init(&mq_send_thread, "mq_send", mq_send_entry, RT_NULL, mq_send_stack, sizeof(mq_send_stack), 22, 10);
    if(ret != RT_EOK)
        uassert_true(RT_FALSE);

    ret = rt_thread_init(&mq_recv_thread, "mq_recv", mq_recv_entry, RT_NULL, mq_recv_stack, sizeof(mq_recv_stack), 23, 10);
    if(ret != RT_EOK)
        uassert_true(RT_FALSE);

    ret = rt_event_init(&finish_e, "finish", RT_IPC_FLAG_FIFO);
    if(ret != RT_EOK)
        uassert_true(RT_FALSE);

    rt_thread_startup(&mq_send_thread);
    rt_thread_startup(&mq_recv_thread);

    rt_event_recv(&finish_e, MQSEND_FINISH | MQRECV_FINIHS, RT_EVENT_FLAG_AND, RT_WAITING_FOREVER, RT_NULL);

    rt_event_detach(&finish_e);
}

static void mq_send_noblock_case(rt_mq_t testmq)
{
    /* 异步发送消息 */
    rt_uint32_t send_buf[MAX_MSGS+1] = {0};
    rt_err_t ret = RT_EOK;

    for (int var = 0; var < MAX_MSGS; ++var)
    {
        send_buf[var] = var + 1;
        rt_mq_send(testmq, &send_buf[var], sizeof(send_buf[0]));
    }

    send_buf[MAX_MSGS] = MAX_MSGS + 1;
    rt_mq_send(testmq, &send_buf[MAX_MSGS], sizeof(send_buf[0]));
}

static void mq_recv_noblock_case(rt_mq_t testmq)
{
    /* 异步接收消息 */
    rt_uint32_t recv_buf[MAX_MSGS+1] = {0};
    rt_err_t ret = RT_EOK;
    int var = 0;
    int timeout = 1000;
    while (var < MAX_MSGS + 1 && timeout > 0)
    {
        /* recv no wait */
        ret = rt_mq_recv(testmq, &recv_buf[var], sizeof(recv_buf[0]), 0);
        if (ret == RT_EOK)
        {
            uassert_true(recv_buf[var] == (var + 1));
            var++;
        }
        timeout--;
    }
    uassert_true(timeout > 0);
}

static void mq_send_noblock_entry(void *param)
{
    mq_send_noblock_case(&static_mq);
    
    rt_event_send(&finish_e, MQSEND_FINISH);
}

static void mq_recv_noblock_entry(void *param)
{
    /* recv msg */
    mq_recv_noblock_case(&static_mq);

    rt_event_send(&finish_e, MQRECV_FINIHS);
}

static void test_mq_send_recv_noblock(void)
{
    rt_err_t ret ;
    ret = rt_thread_init(&mq_send_thread, "mq_send", mq_send_noblock_entry, RT_NULL, mq_send_stack, sizeof(mq_send_stack), 22, 10);
    if(ret != RT_EOK)
        uassert_true(RT_FALSE);

    ret = rt_thread_init(&mq_recv_thread, "mq_recv", mq_recv_noblock_entry, RT_NULL, mq_recv_stack, sizeof(mq_recv_stack), 23, 10);
    if(ret != RT_EOK)
        uassert_true(RT_FALSE);

    ret = rt_event_init(&finish_e, "finish", RT_IPC_FLAG_FIFO);
    if(ret != RT_EOK)
        uassert_true(RT_FALSE);

    rt_thread_startup(&mq_send_thread);
    rt_thread_startup(&mq_recv_thread);

    rt_event_recv(&finish_e, MQSEND_FINISH | MQRECV_FINIHS, RT_EVENT_FLAG_AND, RT_WAITING_FOREVER, RT_NULL);

    rt_event_detach(&finish_e);
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
    UTEST_UNIT_RUN(test_mq_init);
    UTEST_UNIT_RUN(test_mq_send_recv);
    UTEST_UNIT_RUN(test_mq_send_recv_noblock);
    UTEST_UNIT_RUN(test_mq_detach);
}
UTEST_TC_EXPORT(testcase, "testcases.kernel.messagequeue_tc", utest_tc_init, utest_tc_cleanup, 1000);
