    Date         Author        Email                       Notes
    2019-04-03   WillianChan   chentingwei@rt-thread.com   the first version
    2019-06-14   flybreak      guozhanxin@rt-thread.com    add Priority preemption and time slice rotation test.

## 1、介绍

该目录下是线程的测试用例，测试用例在 `testcases/src/thread` 目录结构里。

该测试用例用于测试线程相关功能是否正常。

## 2、文件说明

| 文件名     | 说明           |
| :--------- | :------------- |
| thread_tc.c | 线程测试用例 |

## 3、软硬件环境

运行该测试用例需要在 `env` 工具的 `menuconfig` 中打开 `thread` 测试用例接口，硬件需要支持 RT-Thread 操作系统，软件上支持线程相关内核接口。

## 4、测试项

### 4.1 测试说明

| 测试项             | 说明                                               |
| :----------------- | :------------------------------------------------- |
| 动态线程相关功能 | 测试线程的创建，启动，删除功能是否正常。   |
| 静态线程相关功能 | 测试线程的初始化，启动，脱离功能是否正常。 |
| 线程睡眠功能     | 测试线程睡眠功能是否正常，即测试线程从睡眠到唤醒所经历时长与预设值是否一致。 |
| 线程空闲钩子功能     | 测试线程钩子函数功能是否正常。 |
| 线程让出处理器资源功能     | 测试使线程让出处理器资源功能是否正常。 |
| 线程控制功能     | 测试控制线程功能是否正常，即通过设置 `rt_thread_control` 这个 API 的指示控制命令判断对应功能是否正常。 |
| 线程优先级抢占 | 测试高优先级的任务能否抢占低优先级的任务 |
| 时间片轮转 | 测试两个相同优先级的任务之间，能否以时间片轮转的方式并行运行 |

### 4.2 测试顺序

```c
static void testcase(void)
{
    LOG_I("in testcase func...");

    UTEST_UNIT_RUN(test_thread_dynamic);
    UTEST_UNIT_RUN(test_thread_static);
    UTEST_UNIT_RUN(test_thread_delay);
    UTEST_UNIT_RUN(test_idle_hook);
    UTEST_UNIT_RUN(test_thread_yield);
    UTEST_UNIT_RUN(test_thread_control);
    UTEST_UNIT_RUN(test_thread_priority);
    UTEST_UNIT_RUN(test_thread_time_slice);
}
```

### 4.3 测试思路

#### 4.3.1 动态线程相关功能 `UTEST_UNIT_RUN(test_thread_dynamic)` 测试思路

这是对 `rt_thread_create`、`rt_thread_startup` 和 `rt_thread_delete` 的一个组合测试，即三者组合成一个 UNIT 来测，当且仅当这个 UNIT 通过的时候，才能说明这三者功能完好。

具体的：

- 先使用 `rt_thread_create` 函数创建一个动态线程，此动态线程的线程名为 `thread1`，入口函数是 `thread1_entry`，创建成功则进入下一步，失败则进入异常处理。

- 再使用 `rt_thread_startup` 函数启动该线程，启动成功则进入下一步，失败则进入异常处理。

- 最后使用 `rt_thread_delete` 函数删除正处于运行态的 `thread1`，删除失败则进入异常处理。

- 以上均正常，此 UNIT 通过。

#### 4.3.2 静态线程相关功能 `UTEST_UNIT_RUN(test_thread_static)` 测试思路

这是对 `rt_thread_init`、`rt_thread_startup` 和 `rt_thread_detach` 的一个组合测试，即三者组合成一个 UNIT 来测，当且仅当这个 UNIT 通过的时候，才能说明这三者功能完好。

参考动态线程相关功能 `UTEST_UNIT_RUN(test_timer_dynamic)` 的测试思路。

#### 4.3.3 线程睡眠功能 `UTEST_UNIT_RUN(test_thread_delay)` 测试思路

- 先创建一个动态线程，名为 `thread3`，入口函数是 `thread3_entry`。

- 在入口函数 `thread3_entry` 中，先使用 `rt_tick_get` 函数获取当前的 OS Tick，并暂存下来，使用 `rt_thread_delay` 函数使 `thread3` 睡眠10个OS Tick，在线程唤醒后再次通过 `rt_tick_get` 获取唤醒后的当前 OS Tick，比对现在的 OS Tick 与之前暂存的 OS Tick 对比，判断两者之差是否与预设值相匹配。

- 若匹配，此 UNIT 通过。

#### 4.3.4 线程空闲钩子功能 `UTEST_UNIT_RUN(test_idle_hook)` 测试思路

- 先使用 `rt_thread_idle_sethook` 函数设置一个空闲线程的钩子函数。

- 再创建一个动态线程，名为 `thread4`，入口函数是 `thread4_entry`。

- 在 `thread4_entry` 中利用 `rt_thread_delay` 函数使当前线程挂起并进入钩子函数，钩子函数中对变量 `entry_idle_hook_times` 自加，需要注意的是，变量 `entry_idle_hook_times` 初始值是0；在调用 `rt_thread_delay` 5五次后，利用 `rt_thread_idle_delhook` 函数删除钩子函数，并退出当前线程。

- 若变量 `entry_idle_hook_times` 的值确实是在增加的，此 UNIT 通过。

#### 4.3.5 线程让出处理器资源功能 `UTEST_UNIT_RUN(test_thread_yield)` 测试思路

- 先创建两个个动态线程，先创建的线程名为 `thread5`，入口函数是 `thread5_entry`；后创建的线程名为 `thread6`，入口函数是 `thread6_entry`。

- `thread5`先对变量资源 `thread5_source` 进行占用，对变量资源 `thread5_source` 进行自加，当自加到5时，使用 `rt_thread_yield` 函数使 `thread5` 让出当前变量资源 `thread5_source`；此时线程 `thread6` 可对此变量资源进行操作了，对 `thread5_source` 自加至10后退出线程 `thread6`。

- 判断 `thread5_source` 的值是否是10。

- 若是且其他操作一切正常，该 UNIT 通过。

#### 4.3.6 线程控制功能 `UTEST_UNIT_RUN(test_thread_control)` 测试思路

- 先创建一个动态线程，名为 `thread7`，入口函数是 `thread7_entry`。
- 使用 `rt_thread_control`，通过设置该 API 的指示控制命令为 `RT_THREAD_CTRL_STARTUP` 来实现启动线程 `thread7`，启动成功则进入下一步，否则进行异常处理。
- 再次使用 `rt_thread_control`，通过设置该 API 的指示控制命令为 `RT_THREAD_CTRL_CHANGE_PRIORITY` 来实现动态更改线程 `thread7` 的优先级，此时读取线程 `thread7` 的优先级，判断与设定值是否一致。
- 再次使用 `rt_thread_control`，通过设置该 API 的指示控制命令为 `RT_THREAD_CTRL_CLOSE` 来实现删除线程 `thread7`。
- 以上均正常，此 UNIT 通过。

#### 4.3.7 优先级抢占 `UTEST_UNIT_RUN(test_thread_priority)` 测试思路

- 创建一个比当前运行的线程优先级高的任务，如果新创建的高优先级的任务可以打断当前的任务运行，说明优先级抢占的功能正常。

#### 4.3.8 时间片轮转 `UTEST_UNIT_RUN(test_thread_time_slice)` 测试思路

- 创建两个**优先级相同**且**都比当前测试线程优先级低**的线程，启动两个线程之后，由于两个线程的优先级较低，并不会立即执行。
- 令当前测试线程延时两个时间片的时间，此时当前线程让出了CPU，两个线程得到运行机会。
- 两个优先级相同得线程，运行时会利用 while 循环抢占CPU资源，并完成自身运行次数（time_slice1、time_slice2）的累加操作，并根据两个线程的运行次数检测两个优先级相同的线程运行时间是否相近。若多次运行之后，两个线程运行的时间差均小于1%，则认为测试通过。

## 5、配置

使用该测试用例需要在 `env` 工具的 `menuconfig` 中做相关配置，配置如下所示：

```
Privated Packages of RealThread  --->
	[*] testcase: Enable Test  --->
        kernel test  --->
            thread  --->
                [*] Enable thread test
```

配置完成后，使用 `pkgs --update` 更新软件包。

## 6、使用

- 编译下载。
- 在 MSH 中输入 `utest_run src.thread.thread_tc` 运行该测试用例。

## 7、注意事项

- 需配置正确的测试用例。

- 需在 MSH 中输入正确的命令行。