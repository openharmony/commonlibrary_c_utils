# Benchmark测试

基准测试（benchmarking）是一种测量和评估软件性能指标的活动，可以在某个时候通过基准测试建立一个已知的性能水平（称为基准线），
当系统的软硬件环境发生变化之后再进行一次基准测试以确定那些变化对性能的影响。这是基准测试最常见的用途，其他用途包括测定某种负载水平下的性能极限、
管理系统或环境的变化、发现可能导致性能问题的条件等。

## 目录结构

```
commonlibrary/c_utils/base/test
├─ benchmarktest
│   ├── ashemem_benchmark_test                      # 匿名共享内存接口性能测试
│   ├── datetime_benchmark_test                     # 日期与时间接口性能测试
│   ├── directory_benchmark_test                    # 文件与目录接口性能测试
│   ├── event_benchmark_test                        # 事件处理系统接口性能测试
│   ├── file_benchmark_test                         # 读写文件接口性能测试
│   ├── mapped_benchmark_test                       # 文件映射接口性能测试
│   ├── observer_benchmark_test                     # 观察者模式接口性能测试
│   ├── parcel_benchmark_test                       # 使用Parcel作为数据容器接口性能测试
│   ├── refbase_benchmark_test                      # 智能指针管理动态分配内存对象接口性能测试
│   ├── rwlock_benchmark_test                       # 读写锁接口性能测试
│   ├── safe_block_queue_benchmark_test             # 线程安全阻塞队列接口性能测试
│   ├── safe_block_queue_tracking_benchmark_test
│   ├── safe_map_benchmark_test                     # 线程安全Map接口性能测试
│   ├── safe_queue_benchmark_test                   # 线程安全栈与队列接口性能测试
│   ├── singleton_benchmark_test                    # 单例模式接口性能测试
│   ├── sorted_vector_benchmark_test                # 有序Vector接口性能测试
│   ├── string_benchmark_test                       # 字符串处理接口性能测试
│   ├── thread_benchmark_test                       # 强化线程能力接口性能测试
│   ├── thread_pool_benchmark_test                  # 线程池接口性能测试
│   ├── timer_benchmark_test                        # 定时器接口性能测试
│   ├── unique_fd_benchmark_test                    # 管理、传递文件描述符接口性能测试
│   ├── benchmark_assert.h
│   ├── BUILD.gn
│   └── benchmark_log.h 
```

## 编译构建
### 编译部件
```
./build.sh --product-name rk3568 --build-target c_utils
```

### 测试套编译
```
./build.sh --product-name rk3568 --build-target commonlibrary/c_utils/base/test/benchmarktest/
```

### 测试套生成路径
```
/out/rk3568/tests/benchmark
```

### 编译静态库
```
./build.sh --product-name rk3568 --build-target commonlibrary/c_utils/base:utilsbase
```

## 测试环境构建(windows)
### 测试目录构建

```

Test
├── developer_test                 # 开发者自测试框架 git clone https://gitee.com/openharmony/testfwk_developer_test.git
│   ├── aw
│   ├── config  # 测试框架配置
│   │   ├── framework_config.xml   #配置benchmark timeout字段为"900"
│   │   └── user_config.xml        #sn字段需要配置开发板序列号  test_cases字段需要配置编译生成的测试用例目录
│   ├── examples
│   ├── src
│   ├── third_party
│   ├── reports                    # 测试结果报告
│   ├── BUILD.gn
│   ├── start.bat                  # Windows
│   └── start.sh                   # Linux
├── testcase                       # cp -r /out/rk3568/tests/benchmark/ XXX/testcase/
└── xdevice                        # git clone https://gitee.com/openharmony/testfwk_xdevice.git

```
## 执行测试用例(windows)
### 启动测试框架
	执行start.bat

### 选择产品形态
	默认选择为rk3568


### 执行测试用例(以parcel为例)
	eg：run -t BENCHMARK -ts ParcelTest

  	执行命令参数说明：
    ```
    -t [TESTTYPE]: 指定测试用例类型，有UT，MST，ST，PERF，FUZZ，BENCHMARK等。（必选参数）
    -ts [TESTSUITE]: 指定测试套，可独立使用。
    ```

### 测试报告日志
当执行完测试指令，控制台会自动生成测试结果，若需要详细测试报告您可在相应的数据文档中进行查找。

#### 测试结果
测试结果输出根路径如下：
```
Test/developer_test/reports/xxxx_xx_xx_xx_xx_xx

