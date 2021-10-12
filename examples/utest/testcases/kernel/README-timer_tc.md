    Date         Author        Email                       Notes
    2019-03-28   WillianChan   chentingwei@rt-thread.com   the first version

## 1、介绍

该目录下是定时器的测试用例，测试用例在 `testcases/src/timer` 目录结构里。

该测试用例用于测试定时器相关功能是否正常。

## 2、文件说明

| 文件名 | 说明 |
| :--- | :----- |
| timer_tc.c   | 定时器测试用例 |

## 3、软硬件环境

运行该测试用例需要在 `env` 工具的 `menuconfig` 中打开 `timer` 测试用例接口，硬件需要支持 RT-Thread 操作系统，软件上支持相关定时器内核接口。

## 4、测试项

### 4.1 测试说明

| 测试项 | 说明 |
| :--- | :----- |
| 动态定时器相关功能 | 测试定时器的创建，启动，停止，删除功能是否正常。 |
| 静态定时器相关功能 | 测试定时器的初始化，启动，停止，脱离功能是否正常。 |
| 定时器控制功能 | 测试设置和获取定时器的超时时间功能是否正常。 |

### 4.2 测试顺序

```
static void testcase(void)
{
    LOG_I("in testcase func...");

    UTEST_UNIT_RUN(test_timer_dynamic);
    UTEST_UNIT_RUN(test_timer_static);
    UTEST_UNIT_RUN(test_timer_control);
}
```

### 4.3 测试思路

#### 4.3.1 动态定时器相关功能 `UTEST_UNIT_RUN(test_timer_dynamic)` 测试思路

这是对 `rt_timer_create`、`rt_timer_start`、`rt_timer_stop` 和 `rt_timer_delete` 的一个组合测试，即四者组合成一个 UNIT 来测，当且仅当这个 UNIT 通过的时候，才能说明这四者功能完好。

具体的：

- 先使用 `rt_timer_create` 函数创建两个动态定时器，一个定时器设置为单次定时，另一个设置为周期定时。

- 再分别用 `rt_timer_start` 函数启动这两个定时器。

- 单次定时器的超时函数对变量 `timeout_flag1` 赋值为1，周期定时器的超时函数对变量 `timeout_flag2` 自加，当变量 `timeout_flag2` 自加到5时，使用 `rt_timer_stop` 函数停止周期定时器。

- 判断变量 `timeout_flag1` 的值和变量 `timeout_flag2` 是否为1和5，同时使用 `rt_timer_delete` 函数删除这两个定时器。

- 以上均正常，此UNIT通过。

#### 4.3.2 静态定时器相关功能 `UTEST_UNIT_RUN(test_timer_static)` 测试思路

这是对 `rt_timer_init`、`rt_timer_start`、`rt_timer_stop` 和 `rt_timer_detach` 的一个组合测试，即四者组合成一个 UNIT 来测，当且仅当这个 UNIT 通过的时候，才能说明这四者功能完好。

参考动态定时器相关功能 `UTEST_UNIT_RUN(test_timer_dynamic)` 的测试思路。

#### 4.3.3 定时器控制功能 `UTEST_UNIT_RUN(test_timer_control)` 测试思路

- 创建一个动态定时器，设置该定时器的定时长度为10个 OS Tick，使用 `rt_timer_control` 函数获取该定时器的定时长度，判断是否为10。

- 使用 `rt_timer_control` 函数设置该定时器的定时长度为50个 OS Tick，设置后，重新启动定时器；等待定时器结束之后，再次使用 `rt_timer_control` 函数获取该定时器的定时长度，获取后，重新启动定时器；等待定时器结束之后，判断获取得到的定时长度和使用 `rt_timer_control` 前后的定时器的 `timeout_tick` 之差是否均为50。

- 以上均正常，此UNIT通过。

## 5、配置

使用该测试用例需要在 `env` 工具的 `menuconfig` 中做相关配置，配置如下所示：

```
Privated Packages of RealThread  --->
	[*] testcase: Enable Test  --->
        kernel test  --->
            timer  --->
                [*] Enable timer test
```

配置完成后，使用 `pkgs --update` 更新软件包。

## 6、使用

- 编译下载。
- 在 MSH 中输入 `utest_run src.timer.timer_tc` 运行该测试用例。

## 7、注意事项

- 需配置正确的测试用例。

- 需在 MSH 中输入正确的命令行。
