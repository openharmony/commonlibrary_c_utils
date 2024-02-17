# Benchmark Test

Benchmarking is an activity of measuring and evaluating software performance indicators, which can establish a known
performance level (called a baseline) at a certain time through benchmarking. 
When the software and hardware environment of the system changes, another benchmarking is conducted to determine the
impact of those changes on performance. This is the most common use of benchmark testing, with other uses including
determining performance limits at certain load levels, managing changes in systems or environments,
and identifying conditions that may cause performance issues.

## 目录结构

```
commonlibrary/c_utils/base/test
├─ benchmarktest
│   ├── ashemem_benchmark_test                      # Anonymous Shared Memory API test
│   ├── datetime_benchmark_test                     # datetime API test
│   ├── directory_benchmark_test                    # directory API test
│   ├── event_benchmark_test                        # event API test
│   ├── file_benchmark_test                         # file API test
│   ├── mapped_benchmark_test                       # mapped API test
│   ├── observer_benchmark_test                     # observer API test
│   ├── parcel_benchmark_test                       # parcel API test
│   ├── refbase_benchmark_test                      # refbase API test
│   ├── rwlock_benchmark_test                       # rwlock API test
│   ├── safe_block_queue_benchmark_test             # safe blocke queue API test
│   ├── safe_block_queue_tracking_benchmark_test
│   ├── safe_map_benchmark_test                     # safe map API test
│   ├── safe_queue_benchmark_test                   # safe queue API test
│   ├── singleton_benchmark_test                    # singleton API test
│   ├── sorted_vector_benchmark_test                # sortedvector API test
│   ├── string_benchmark_test                       # string API test
│   ├── thread_benchmark_test                       # thread API test
│   ├── thread_pool_benchmark_test                  # threadpool API test
│   ├── timer_benchmark_test                        # timer API test
│   ├── unique_fd_benchmark_test                    # uniquefd API test
│   ├── benchmark_assert.h
│   ├── BUILD.gn
│   └── benchmark_log.h 
```

## Build
### Build Component
```
./build.sh --product-name rk3568 --build-target c_utils
```

### Build testsuits
```
./build.sh --product-name rk3568 --build-target commonlibrary/c_utils/base/test/benchmarktest/
```

### output
```
/out/rk3568/tests/benchmark
```

## Test environment setup(windows)
### Test dir

```

Test
├── developer_test                # test framework(git clone https://gitee.com/openharmony/testfwk_developer_test.git)
│   ├── aw
│   ├── config
│   │   ├── framework_config.xml  #config the benchmark timeout field to "900"
│   │   └── user_config.xml       # The SN field needs to be configured with the development board serial numbe
                                    test_cases field needs to be configured with the compiled test case directory
│   ├── examples
│   ├── src
│   ├── third_party
│   ├── reports                   #test result
│   ├── BUILD.gn 
│   ├── start.bat                 # Windwos 
│   └── start.sh                  # Linux
├── testcase                      # cp -r /out/rk3568/tests/benchmark/ XXX/testcase/
└── xdevice                       # (git clone https://gitee.com/openharmony/testfwk_xdevice.git)

```
## Execute test cases(windows)
### start test framework
	start.bat

### Select product type
	default : rk3568


### Execute(eg:parcel)
	eg：run -t BENCHMARK -ts ParcelTest

  	Instructions for executing command parameters：
    ```
    -t [TESTTYPE]: Specify test case type，such as UT，MST，ST，PERF，FUZZ，BENCHMARK。（Required parameters）
    -ts [TESTSUITE]: Specify test suilts，can be used independently。
    ```

### Test report log
After executing the test instructions, the console will automatically generate test results.
If you need a detailed test report, you can search in the corresponding data document.

#### Test results
The root path of the test result output is as follows：
```
Test/developer_test/reports/xxxx_xx_xx_xx_xx_xx

