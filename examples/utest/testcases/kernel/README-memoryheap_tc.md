
    Date         Author        Email                       Notes
    2019-03-25   WillianChan   chentingwei@rt-thread.com   the first version

## 1、介绍

该目录下是内存堆的测试用例，测试用例在 `testcases/src/memory` 目录结构里。

该测试用例用于测试内存堆相关功能是否正常。

## 2、文件说明

| 文件名 | 说明 |
| :--- | :----- |
| memoryheap_tc.c   | 内存堆测试用例 |

## 3、软硬件环境

运行该测试用例需要在 `env` 工具的 `menuconfig` 中打开 `memoryheap` 测试用例接口，硬件需要支持 RT-Thread 操作系统，软件上支持相关动态内存堆内核接口。

## 4、测试项

### 4.1 测试说明

| 测试项 | 说明 |
| :--- | :----- |
| 分配释放内存块功能 | 测试分配内存块功能和释放内存块功能是否正常，同时需测试非字节对齐的情况。 |
| 重新分配内存块功能1 | 测试重新分配比原来更多内存块的情况是否正常，同时需测试非字节对齐的情况。 |
| 重新分配内存块功能2 | 测试重新分配比原来更少内存块的情况是否正常，同时需测试非字节对齐的情况。 |
| 分配多内存块功能  | 测试分配多内存块功能是否正常，同时需测试非字节对齐的情况。 |

### 4.2 测试顺序

```
static void testcase(void)
{
    LOG_I("in testcase func...");
    
    UTEST_UNIT_RUN(test_rt_malloc_free);
    UTEST_UNIT_RUN(test_rt_realloc_more);
    UTEST_UNIT_RUN(test_rt_realloc_less);
    UTEST_UNIT_RUN(test_rt_calloc);
}
```

### 4.3 测试思路

#### 4.3.1 分配释放内存块功能 `UTEST_UNIT_RUN(test_rt_malloc_free)` 测试思路

这是对 `rt_malloc` 和 `rt_free` 的一个组合测试，即两者组合成一个 UNIT 来测，当且仅当这个 UNIT 通过的时候，才能说明这两者功能完好。

具体的：

- 先使用 `rt_malloc` 函数申请16个字节的内存块。

- 再往这16字节的内存块中写入数据，再顺序读出，判断读出数据与写入数据是否一致。

- 最后利用 `rt_free` 函数释放这16字节的内存。

- 同时，需要测试内存 size 为非对齐的情况，使用 `rt_malloc` 函数申请17个字节，后续测试流程同上。

- 若写入和读出的数据一致，此 UNIT 通过。


#### 4.3.2 重新分配内存块功能1 `UTEST_UNIT_RUN(test_rt_realloc_more)` 测试思路

- 先使用 `rt_malloc` 函数申请16个字节的内存块，往这16字节的内存块中写入数据。

- 再使用 `rt_realloc` 函数重新申请32字节的内存块，读出前16字节的数据，判断与之前写入的数据是否一致。

- 再往后16个字节中写入数据，读出后判断与写入的是否一致。

- 同时，需要测试内存 size 为非对齐的情况，使用 `rt_realloc` 函数重新申请37个字节，后续测试流程同上。

- 若都一致，此 UNIT 通过。

#### 4.3.3 重新分配内存块功能2 `UTEST_UNIT_RUN(test_rt_realloc_less)` 测试思路

- 先使用 `rt_malloc` 函数申请16个字节的内存块，往这16字节的内存块中写入数据。

- 再使用 `er_realloc` 函数重新申请8字节的内存块，读出这8个字节的数据，判断与之前写入的数据是否一致。

- 同时，需要测试内存 size 为非对齐的情况，使用 `rt_realloc` 函数重新申请5个字节，后续测试流程同上。

- 若一致，此 UNIT 通过。

#### 4.3.4 重新分配内存块功能2 `UTEST_UNIT_RUN(test_rt_calloc)` 测试思路

- 先用 `rt_calloc` 函数申请4×16字节的内存块，往这4×16个字节的内存块里面写入数据。

- 顺序读出4×16字节内的数据，判断与之前写入的数据是否一致。

- 同时，需要测试内存 size 为非对齐的情况，使用 `rt_calloc` 函数重新申请3×5个字节，后续测试流程同上。

- 若一致，此 UNIT 通过。

## 5、配置

使用该测试用例需要在 `env` 工具的 `menuconfig` 中做相关配置，配置如下所示：

```
Privated Packages of RealThread  --->
	[*] testcase: Enable Test  --->
        kernel test  --->
            memory  --->
                [*] Enable memoryheap test
```

配置完成后，使用 `pkgs --update` 更新软件包。

## 6、使用

- 编译下载。
- 在 MSH 中输入 `utest_run src.memory.memoryheap_tc` 运行该测试用例。

## 7、注意事项

- 需配置正确的测试用例。

- 需在 MSH 中输入正确的命令行。
