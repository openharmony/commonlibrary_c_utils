/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "io_event_epoll.h"

#include <gtest/gtest.h>
#include <sys/resource.h>
#include <unistd.h>
#include <utility>
#include <vector>
#include "common_event_sys_errors.h"
#include "io_event_common.h"

using namespace testing::ext;
using namespace OHOS::Utils;

namespace OHOS {
namespace {

class UtilsIOEventEpollTest : public testing::Test {
public:
    static void SetUpTestCase(void) {}
    static void TearDownTestCase(void) {}
    void SetUp() {}
    void TearDown() {}
};

/*
 * @tc.name: testEpollLifecycle001
 * @tc.desc: Test IOEventEpoll SetUp()/CleanUp() lifecycle.
 */
HWTEST_F(UtilsIOEventEpollTest, testEpollLifecycle001, TestSize.Level0)
{
    IOEventEpoll epoll;

    // 1. setup: the epoll fd is created and tagged
    ASSERT_EQ(epoll.SetUp(), EVENT_SYS_ERR_OK);
    // 2. repeated SetUp() is a no-op (no double tagging on a valid fd)
    ASSERT_EQ(epoll.SetUp(), EVENT_SYS_ERR_OK);

    // 3. cleanup: the epoll fd is closed with its tag and invalidated
    epoll.CleanUp();
    // 4. repeated CleanUp() is a no-op (no double close)
    epoll.CleanUp();

    // 5. setup again: a fresh epoll fd is created and tagged again
    ASSERT_EQ(epoll.SetUp(), EVENT_SYS_ERR_OK);
    epoll.CleanUp();
}

/*
 * @tc.name: testEpollSetUpFailed001
 * @tc.desc: Test IOEventEpoll construction and SetUp() when epoll fd creation fails.
 */
HWTEST_F(UtilsIOEventEpollTest, testEpollSetUpFailed001, TestSize.Level0)
{
    // 1. make epoll_create1() fail with EMFILE
    struct rlimit oldLimit = {0};
    ASSERT_EQ(getrlimit(RLIMIT_NOFILE, &oldLimit), 0);
    struct rlimit newLimit = {0};
    newLimit.rlim_cur = 1; // 1: one file descriptor at most.
    newLimit.rlim_max = oldLimit.rlim_max;
    ASSERT_EQ(setrlimit(RLIMIT_NOFILE, &newLimit), 0);

    // 2. construction and SetUp() fail, no tag is applied on an invalid fd
    IOEventEpoll epoll;
    ErrCode res = epoll.SetUp();

    // 3. restore the limit as early as possible
    ASSERT_EQ(setrlimit(RLIMIT_NOFILE, &oldLimit), 0);
    ASSERT_EQ(res, EVENT_SYS_ERR_BADF);

    // 4. SetUp() succeeds once fd creation is possible again
    ASSERT_EQ(epoll.SetUp(), EVENT_SYS_ERR_OK);
    epoll.CleanUp();
}

/*
 * @tc.name: testEpollModifyEventsAndPolling001
 * @tc.desc: Test IOEventEpoll ModifyEvents() and Polling() with a pipe.
 */
HWTEST_F(UtilsIOEventEpollTest, testEpollModifyEventsAndPolling001, TestSize.Level0)
{
    // 1. prepare a pipe
    int fds[2] = {-1, -1}; // 2: pipe ends.
    ASSERT_EQ(pipe(fds), 0);

    IOEventEpoll epoll;
    ASSERT_EQ(epoll.SetUp(), EVENT_SYS_ERR_OK);

    // 2. watch the read end
    ASSERT_EQ(epoll.ModifyEvents(fds[0], Events::EVENT_READ), EVENT_SYS_ERR_OK);

    // 3. no event before data arrives
    std::vector<std::pair<int, REventId>> res;
    ASSERT_EQ(epoll.Polling(10, res), EVENT_SYS_ERR_NOEVENT); // 10: timeout in ms.

    // 4. event arrives after writing to the write end
    char ch = 'a';
    ASSERT_EQ(write(fds[1], &ch, 1), 1); // 1: one byte.
    ASSERT_EQ(epoll.Polling(10, res), EVENT_SYS_ERR_OK);
    ASSERT_EQ(res.size(), 1u); // 1: one ready fd.
    EXPECT_EQ(res[0].first, fds[0]);
    EXPECT_EQ(res[0].second, Events::EVENT_READ);

    // 5. remove the interest: no event any more even if data remains unread
    ASSERT_EQ(epoll.ModifyEvents(fds[0], Events::EVENT_NONE), EVENT_SYS_ERR_OK);
    res.clear();
    ASSERT_EQ(epoll.Polling(10, res), EVENT_SYS_ERR_NOEVENT);

    epoll.CleanUp();
    close(fds[0]);
    close(fds[1]);
}

/*
 * @tc.name: testEpollModifyEventsFailed001
 * @tc.desc: Test IOEventEpoll ModifyEvents() with an invalid fd.
 */
HWTEST_F(UtilsIOEventEpollTest, testEpollModifyEventsFailed001, TestSize.Level0)
{
    IOEventEpoll epoll;
    ASSERT_EQ(epoll.SetUp(), EVENT_SYS_ERR_OK);

    EXPECT_EQ(epoll.ModifyEvents(-1, Events::EVENT_READ), EVENT_SYS_ERR_BADF);

    epoll.CleanUp();
}

}  // namespace
}  // namespace OHOS
