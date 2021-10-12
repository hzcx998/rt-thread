#include <rtthread.h>
#include <utest.h>

static void test_object_find(void)
{
    /* find tidle0 thread */
    rt_thread_t tid;
    rt_object_t obj;

    tid = (rt_thread_t)rt_object_find("tidle0", RT_Object_Class_Thread);
    if (tid == RT_NULL)
    {
        LOG_E("object_find: find tidle0 failed!");
        uassert_false(tid == RT_NULL);
    }

    obj = rt_object_find("#123tidle0", RT_Object_Class_Timer);
    if (obj)
    {
        LOG_E("object_find: Failed to find an object that does not exist.");
        uassert_false(obj != RT_NULL);
    }

    obj = rt_object_find("tidle0", 255);
    if (obj)
    {
        LOG_E("object_find: Failed to find not exist object kind.");
        uassert_false(obj != RT_NULL);
    }

    return;
}

static void test_object_length(void)
{
    int length;

    length = rt_object_get_length(RT_Object_Class_Thread);
    if (length < 0)
    {
        LOG_E("object_get_length: Failed to get length for thread list.");
        uassert_false(length > 0);
    }

    length = rt_object_get_length(255);
    if (length != 0)
    {
        LOG_E("object_get_length: Failed to get length for not exist object kind.");
        uassert_false(length != 0);
    }
}

static void test_object_get(void)
{
    int length;
    rt_object_t* objs;

    length = rt_object_get_length(RT_Object_Class_Thread);
    if (length < 0 || length > 100)
    {
        LOG_E("object_get: Failed to get length for thread list.");
        uassert_false(length > 0 || length > 100);
    }
    else
    {
        objs = rt_malloc(sizeof(rt_object_t) * length);
        if (objs == RT_NULL)
        {
            LOG_E("object_get: out of memory.");
            uassert_false(objs == RT_NULL);
        }
        else
        {
            int ret_len = rt_object_get_pointers(RT_Object_Class_Thread, objs, length);
            if (ret_len == 0)
            {
                LOG_E("object_get: Failed to get objects.");
                uassert_false(ret_len == 0);
            }

            rt_free(objs);
        }
    }
}

static void test_object_get_len(void)
{
    int length;
    rt_object_t* objs;

    length = rt_object_get_length(RT_Object_Class_Thread);
    if (length < 0 || length > 100)
    {
        LOG_E("object_get: Failed to get length for thread list.");
        uassert_false(length > 0 || length > 100);
    }
    else
    {
        length = length / 2;
        objs = rt_malloc(sizeof(rt_object_t) * length);
        if (objs == RT_NULL)
        {
            LOG_E("object_get: out of memory.");
            uassert_false(objs == RT_NULL);
        }
        else
        {
            int ret_len = rt_object_get_pointers(RT_Object_Class_Thread, objs, length);
            if (ret_len == 0)
            {
                LOG_E("object_get: Failed to get objects.");
                uassert_false(ret_len == 0);
            }
            if (ret_len > length)
            {
                LOG_E("object_get: Failed to get objects with length.");
                uassert_false(ret_len > length);
            }

            rt_free(objs);
        }
    }
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
    UTEST_UNIT_RUN(test_object_find);
    UTEST_UNIT_RUN(test_object_length);
    UTEST_UNIT_RUN(test_object_get);
    UTEST_UNIT_RUN(test_object_get_len);
}
UTEST_TC_EXPORT(testcase, "src.object", utest_tc_init, utest_tc_cleanup, 60);
