/**
 *    Copyright 2015 MongoDB Inc.
 *
 *    This program is free software: you can redistribute it and/or  modify
 *    it under the terms of the GNU Affero General Public License, version 3,
 *    as published by the Free Software Foundation.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU Affero General Public License for more details.
 *
 *    You should have received a copy of the GNU Affero General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *    As a special exception, the copyright holders give permission to link the
 *    code of portions of this program with the OpenSSL library under certain
 *    conditions as described in each individual source file and distribute
 *    linked combinations including the program with the OpenSSL library. You
 *    must comply with the GNU Affero General Public License in all respects for
 *    all of the code used other than as permitted herein. If you modify file(s)
 *    with this exception, you may extend this exception to your version of the
 *    file(s), but you are not obligated to do so. If you do not wish to do so,
 *    delete this exception statement from your version. If you delete this
 *    exception statement from all source files in the program, then also delete
 *    it in the license file.
 */

#include "mongo/platform/basic.h"

#include "mongo/s/query/async_results_merger.h"

#include "mongo/db/json.h"
#include "mongo/db/query/getmore_response.h"
#include "mongo/db/query/lite_parsed_query.h"
#include "mongo/executor/thread_pool_task_executor_test_fixture.h"
#include "mongo/executor/network_interface_mock.h"
#include "mongo/executor/task_executor.h"
#include "mongo/stdx/memory.h"
#include "mongo/unittest/unittest.h"

namespace mongo {

namespace {

using executor::RemoteCommandRequest;
using executor::RemoteCommandResponse;

class AsyncResultsMergerTest : public executor::ThreadPoolExecutorTest {
public:
    AsyncResultsMergerTest()
        : _nss("testdb.testcoll"),
          _remotes({HostAndPort("localhost", -1),
                    HostAndPort("localhost", -2),
                    HostAndPort("localhost", -3)}) {}

    void setUp() final {
        ThreadPoolExecutorTest::setUp();
        launchExecutorThread();
        executor = &getExecutor();
    }

    void postExecutorThreadLaunch() final {}

protected:
    /**
     * Given a find command specification, 'findCmd', and a list of remote host:port pairs,
     * constructs the appropriate arm.
     */
    void makeCursorFromFindCmd(const BSONObj& findCmd, const std::vector<HostAndPort>& remotes) {
        const bool isExplain = true;
        lpq = unittest::assertGet(LiteParsedQuery::makeFromFindCommand(_nss, findCmd, isExplain));
        params = ClusterClientCursorParams(_nss);
        params.cmdObj = findCmd;
        params.sort = lpq->getSort();
        params.projection = lpq->getProj();
        params.limit = lpq->getLimit();
        params.batchSize = lpq->getBatchSize();
        if (lpq->getSkip()) {
            params.skip = lpq->getSkip();
        }

        arm = stdx::make_unique<AsyncResultsMerger>(executor, params, remotes);
    }

    /**
     * Schedules a list of getMore responses to be returned by the mock network.
     */
    void scheduleNetworkResponses(std::vector<GetMoreResponse> responses) {
        std::vector<BSONObj> objs;
        for (const auto& getMoreResponse : responses) {
            objs.push_back(getMoreResponse.toBSON());
        }
        scheduleNetworkResponseObjs(objs);
    }

    /**
     * Schedules a list of raw BSON command responses to be returned by the mock network.
     */
    void scheduleNetworkResponseObjs(std::vector<BSONObj> objs) {
        executor::NetworkInterfaceMock* net = getNet();
        net->enterNetwork();
        for (const auto& obj : objs) {
            ASSERT_TRUE(net->hasReadyRequests());
            Milliseconds millis(0);
            RemoteCommandResponse response(obj, BSONObj(), millis);
            executor::TaskExecutor::ResponseStatus responseStatus(response);
            net->scheduleResponse(net->getNextReadyRequest(), net->now(), responseStatus);
        }
        net->runReadyNetworkOperations();
        net->exitNetwork();
    }

    void scheduleErrorResponse(Status status) {
        invariant(!status.isOK());
        executor::NetworkInterfaceMock* net = getNet();
        net->enterNetwork();
        ASSERT_TRUE(net->hasReadyRequests());
        net->scheduleResponse(net->getNextReadyRequest(), net->now(), status);
        net->runReadyNetworkOperations();
        net->exitNetwork();
    }

    void blackHoleNextRequest() {
        executor::NetworkInterfaceMock* net = getNet();
        net->enterNetwork();
        ASSERT_TRUE(net->hasReadyRequests());
        net->blackHole(net->getNextReadyRequest());
        net->exitNetwork();
    }

    void runReadyNetworkOperations() {
        executor::NetworkInterfaceMock* net = getNet();
        net->enterNetwork();
        net->runReadyNetworkOperations();
        net->exitNetwork();
    }

    const NamespaceString _nss;
    const std::vector<HostAndPort> _remotes;

    executor::TaskExecutor* executor;
    std::unique_ptr<LiteParsedQuery> lpq;

    ClusterClientCursorParams params;
    std::unique_ptr<AsyncResultsMerger> arm;
};

TEST_F(AsyncResultsMergerTest, ClusterFind) {
    BSONObj findCmd = fromjson("{find: 'testcoll'}");
    makeCursorFromFindCmd(findCmd, _remotes);

    ASSERT_FALSE(arm->ready());
    auto readyEvent = unittest::assertGet(arm->nextEvent());
    ASSERT_FALSE(arm->ready());

    std::vector<GetMoreResponse> responses;
    std::vector<BSONObj> batch1 = {
        fromjson("{_id: 1}"), fromjson("{_id: 2}"), fromjson("{_id: 3}")};
    responses.emplace_back(_nss, CursorId(0), batch1);
    scheduleNetworkResponses(responses);
    executor->waitForEvent(readyEvent);

    ASSERT_TRUE(arm->ready());
    ASSERT_EQ(fromjson("{_id: 1}"), *unittest::assertGet(arm->nextReady()));
    ASSERT_TRUE(arm->ready());
    ASSERT_EQ(fromjson("{_id: 2}"), *unittest::assertGet(arm->nextReady()));
    ASSERT_TRUE(arm->ready());
    ASSERT_EQ(fromjson("{_id: 3}"), *unittest::assertGet(arm->nextReady()));
    ASSERT_FALSE(arm->ready());

    readyEvent = unittest::assertGet(arm->nextEvent());
    ASSERT_FALSE(arm->ready());

    responses.clear();
    std::vector<BSONObj> batch2 = {fromjson("{_id: 4}")};
    responses.emplace_back(_nss, CursorId(0), batch2);
    std::vector<BSONObj> batch3 = {fromjson("{_id: 5}"), fromjson("{_id: 6}")};
    responses.emplace_back(_nss, CursorId(0), batch3);
    scheduleNetworkResponses(responses);
    executor->waitForEvent(readyEvent);

    ASSERT_TRUE(arm->ready());
    ASSERT_EQ(fromjson("{_id: 4}"), *unittest::assertGet(arm->nextReady()));
    ASSERT_TRUE(arm->ready());
    ASSERT_EQ(fromjson("{_id: 5}"), *unittest::assertGet(arm->nextReady()));
    ASSERT_TRUE(arm->ready());
    ASSERT_EQ(fromjson("{_id: 6}"), *unittest::assertGet(arm->nextReady()));
    ASSERT_TRUE(arm->ready());
    ASSERT(!unittest::assertGet(arm->nextReady()));
}

TEST_F(AsyncResultsMergerTest, ClusterFindAndGetMore) {
    BSONObj findCmd = fromjson("{find: 'testcoll', batchSize: 2}");
    makeCursorFromFindCmd(findCmd, _remotes);

    ASSERT_FALSE(arm->ready());
    auto readyEvent = unittest::assertGet(arm->nextEvent());
    ASSERT_FALSE(arm->ready());

    std::vector<GetMoreResponse> responses;
    std::vector<BSONObj> batch1 = {fromjson("{_id: 1}"), fromjson("{_id: 2}")};
    responses.emplace_back(_nss, CursorId(10), batch1);
    std::vector<BSONObj> batch2 = {fromjson("{_id: 3}"), fromjson("{_id: 4}")};
    responses.emplace_back(_nss, CursorId(11), batch2);
    std::vector<BSONObj> batch3 = {fromjson("{_id: 5}"), fromjson("{_id: 6}")};
    responses.emplace_back(_nss, CursorId(12), batch3);
    scheduleNetworkResponses(responses);
    executor->waitForEvent(readyEvent);

    ASSERT_TRUE(arm->ready());
    ASSERT_EQ(fromjson("{_id: 1}"), *unittest::assertGet(arm->nextReady()));
    ASSERT_TRUE(arm->ready());
    ASSERT_EQ(fromjson("{_id: 2}"), *unittest::assertGet(arm->nextReady()));
    ASSERT_TRUE(arm->ready());
    ASSERT_EQ(fromjson("{_id: 3}"), *unittest::assertGet(arm->nextReady()));
    ASSERT_TRUE(arm->ready());
    ASSERT_EQ(fromjson("{_id: 4}"), *unittest::assertGet(arm->nextReady()));
    ASSERT_TRUE(arm->ready());
    ASSERT_EQ(fromjson("{_id: 5}"), *unittest::assertGet(arm->nextReady()));
    ASSERT_TRUE(arm->ready());
    ASSERT_EQ(fromjson("{_id: 6}"), *unittest::assertGet(arm->nextReady()));

    ASSERT_FALSE(arm->ready());
    readyEvent = unittest::assertGet(arm->nextEvent());
    ASSERT_FALSE(arm->ready());

    responses.clear();
    std::vector<BSONObj> batch4 = {fromjson("{_id: 7}"), fromjson("{_id: 8}")};
    responses.emplace_back(_nss, CursorId(10), batch4);
    std::vector<BSONObj> batch5 = {fromjson("{_id: 9}")};
    responses.emplace_back(_nss, CursorId(0), batch5);
    std::vector<BSONObj> batch6 = {fromjson("{_id: 10}")};
    responses.emplace_back(_nss, CursorId(0), batch6);
    scheduleNetworkResponses(responses);
    executor->waitForEvent(readyEvent);

    ASSERT_TRUE(arm->ready());
    ASSERT_EQ(fromjson("{_id: 10}"), *unittest::assertGet(arm->nextReady()));
    ASSERT_TRUE(arm->ready());
    ASSERT_EQ(fromjson("{_id: 7}"), *unittest::assertGet(arm->nextReady()));
    ASSERT_TRUE(arm->ready());
    ASSERT_EQ(fromjson("{_id: 8}"), *unittest::assertGet(arm->nextReady()));
    ASSERT_TRUE(arm->ready());
    ASSERT_EQ(fromjson("{_id: 9}"), *unittest::assertGet(arm->nextReady()));

    ASSERT_FALSE(arm->ready());
    readyEvent = unittest::assertGet(arm->nextEvent());
    ASSERT_FALSE(arm->ready());

    responses.clear();
    std::vector<BSONObj> batch7 = {fromjson("{_id: 11}")};
    responses.emplace_back(_nss, CursorId(0), batch7);
    scheduleNetworkResponses(responses);
    executor->waitForEvent(readyEvent);

    ASSERT_TRUE(arm->ready());
    ASSERT_EQ(fromjson("{_id: 11}"), *unittest::assertGet(arm->nextReady()));
    ASSERT_TRUE(arm->ready());
    ASSERT(!unittest::assertGet(arm->nextReady()));
}

TEST_F(AsyncResultsMergerTest, ClusterFindSorted) {
    BSONObj findCmd = fromjson("{find: 'testcoll', sort: {_id: 1}, batchSize: 2}");
    makeCursorFromFindCmd(findCmd, _remotes);

    ASSERT_FALSE(arm->ready());
    auto readyEvent = unittest::assertGet(arm->nextEvent());
    ASSERT_FALSE(arm->ready());

    std::vector<GetMoreResponse> responses;
    std::vector<BSONObj> batch1 = {fromjson("{_id: 5}"), fromjson("{_id: 6}")};
    responses.emplace_back(_nss, CursorId(0), batch1);
    std::vector<BSONObj> batch2 = {fromjson("{_id: 3}"), fromjson("{_id: 9}")};
    responses.emplace_back(_nss, CursorId(0), batch2);
    std::vector<BSONObj> batch3 = {fromjson("{_id: 4}"), fromjson("{_id: 8}")};
    responses.emplace_back(_nss, CursorId(0), batch3);
    scheduleNetworkResponses(responses);
    executor->waitForEvent(readyEvent);

    ASSERT_TRUE(arm->ready());
    ASSERT_EQ(fromjson("{_id: 3}"), *unittest::assertGet(arm->nextReady()));
    ASSERT_TRUE(arm->ready());
    ASSERT_EQ(fromjson("{_id: 4}"), *unittest::assertGet(arm->nextReady()));
    ASSERT_TRUE(arm->ready());
    ASSERT_EQ(fromjson("{_id: 5}"), *unittest::assertGet(arm->nextReady()));
    ASSERT_TRUE(arm->ready());
    ASSERT_EQ(fromjson("{_id: 6}"), *unittest::assertGet(arm->nextReady()));
    ASSERT_TRUE(arm->ready());
    ASSERT_EQ(fromjson("{_id: 8}"), *unittest::assertGet(arm->nextReady()));
    ASSERT_TRUE(arm->ready());
    ASSERT_EQ(fromjson("{_id: 9}"), *unittest::assertGet(arm->nextReady()));
    ASSERT_TRUE(arm->ready());
    ASSERT(!unittest::assertGet(arm->nextReady()));
}

TEST_F(AsyncResultsMergerTest, ClusterFindAndGetMoreSorted) {
    BSONObj findCmd = fromjson("{find: 'testcoll', sort: {_id: 1}, batchSize: 2}");
    makeCursorFromFindCmd(findCmd, _remotes);

    ASSERT_FALSE(arm->ready());
    auto readyEvent = unittest::assertGet(arm->nextEvent());
    ASSERT_FALSE(arm->ready());

    std::vector<GetMoreResponse> responses;
    std::vector<BSONObj> batch1 = {fromjson("{_id: 5}"), fromjson("{_id: 6}")};
    responses.emplace_back(_nss, CursorId(1), batch1);
    std::vector<BSONObj> batch2 = {fromjson("{_id: 3}"), fromjson("{_id: 4}")};
    responses.emplace_back(_nss, CursorId(0), batch2);
    std::vector<BSONObj> batch3 = {fromjson("{_id: 7}"), fromjson("{_id: 8}")};
    responses.emplace_back(_nss, CursorId(2), batch3);
    scheduleNetworkResponses(responses);
    executor->waitForEvent(readyEvent);

    ASSERT_TRUE(arm->ready());
    ASSERT_EQ(fromjson("{_id: 3}"), *unittest::assertGet(arm->nextReady()));
    ASSERT_TRUE(arm->ready());
    ASSERT_EQ(fromjson("{_id: 4}"), *unittest::assertGet(arm->nextReady()));
    ASSERT_TRUE(arm->ready());
    ASSERT_EQ(fromjson("{_id: 5}"), *unittest::assertGet(arm->nextReady()));
    ASSERT_TRUE(arm->ready());
    ASSERT_EQ(fromjson("{_id: 6}"), *unittest::assertGet(arm->nextReady()));

    ASSERT_FALSE(arm->ready());
    readyEvent = unittest::assertGet(arm->nextEvent());
    ASSERT_FALSE(arm->ready());

    responses.clear();
    std::vector<BSONObj> batch4 = {fromjson("{_id: 7}"), fromjson("{_id: 10}")};
    responses.emplace_back(_nss, CursorId(0), batch4);
    scheduleNetworkResponses(responses);
    executor->waitForEvent(readyEvent);

    ASSERT_TRUE(arm->ready());
    ASSERT_EQ(fromjson("{_id: 7}"), *unittest::assertGet(arm->nextReady()));
    ASSERT_TRUE(arm->ready());
    ASSERT_EQ(fromjson("{_id: 7}"), *unittest::assertGet(arm->nextReady()));
    ASSERT_TRUE(arm->ready());
    ASSERT_EQ(fromjson("{_id: 8}"), *unittest::assertGet(arm->nextReady()));

    ASSERT_FALSE(arm->ready());
    readyEvent = unittest::assertGet(arm->nextEvent());
    ASSERT_FALSE(arm->ready());

    responses.clear();
    std::vector<BSONObj> batch5 = {fromjson("{_id: 9}"), fromjson("{_id: 10}")};
    responses.emplace_back(_nss, CursorId(0), batch5);
    scheduleNetworkResponses(responses);
    executor->waitForEvent(readyEvent);

    ASSERT_TRUE(arm->ready());
    ASSERT_EQ(fromjson("{_id: 9}"), *unittest::assertGet(arm->nextReady()));
    ASSERT_TRUE(arm->ready());
    ASSERT_EQ(fromjson("{_id: 10}"), *unittest::assertGet(arm->nextReady()));
    ASSERT_TRUE(arm->ready());
    ASSERT_EQ(fromjson("{_id: 10}"), *unittest::assertGet(arm->nextReady()));
    ASSERT_TRUE(arm->ready());
    ASSERT(!unittest::assertGet(arm->nextReady()));
}

TEST_F(AsyncResultsMergerTest, StreamResultsFromOneShardIfOtherDoesntRespond) {
    BSONObj findCmd = fromjson("{find: 'testcoll', batchSize: 2}");
    makeCursorFromFindCmd(findCmd, {_remotes[0], _remotes[1]});

    ASSERT_FALSE(arm->ready());
    auto readyEvent = unittest::assertGet(arm->nextEvent());
    ASSERT_FALSE(arm->ready());

    // First shard responds, but second shard never responds.
    std::vector<GetMoreResponse> responses;
    std::vector<BSONObj> batch1 = {fromjson("{_id: 1}"), fromjson("{_id: 2}")};
    responses.emplace_back(_nss, CursorId(1), batch1);
    scheduleNetworkResponses(responses);
    blackHoleNextRequest();
    executor->waitForEvent(readyEvent);

    ASSERT_TRUE(arm->ready());
    ASSERT_EQ(fromjson("{_id: 1}"), *unittest::assertGet(arm->nextReady()));
    ASSERT_TRUE(arm->ready());
    ASSERT_EQ(fromjson("{_id: 2}"), *unittest::assertGet(arm->nextReady()));

    ASSERT_FALSE(arm->ready());
    readyEvent = unittest::assertGet(arm->nextEvent());
    ASSERT_FALSE(arm->ready());

    // We can continue to return results from first shard, while second shard remains unresponsive.
    responses.clear();
    std::vector<BSONObj> batch2 = {fromjson("{_id: 3}"), fromjson("{_id: 4}")};
    responses.emplace_back(_nss, CursorId(0), batch2);
    scheduleNetworkResponses(responses);
    executor->waitForEvent(readyEvent);

    ASSERT_TRUE(arm->ready());
    ASSERT_EQ(fromjson("{_id: 3}"), *unittest::assertGet(arm->nextReady()));
    ASSERT_TRUE(arm->ready());
    ASSERT_EQ(fromjson("{_id: 4}"), *unittest::assertGet(arm->nextReady()));

    // Kill cursor before deleting it, as the second remote cursor has not been exhausted. We don't
    // wait on 'killEvent' here, as the blackholed request's callback will only run on shutdown of
    // the network interface.
    auto killEvent = arm->kill();
    ASSERT_TRUE(killEvent.isValid());
}

TEST_F(AsyncResultsMergerTest, ErrorOnMismatchedCursorIds) {
    BSONObj findCmd = fromjson("{find: 'testcoll'}");
    makeCursorFromFindCmd(findCmd, {_remotes[0]});

    ASSERT_FALSE(arm->ready());
    auto readyEvent = unittest::assertGet(arm->nextEvent());
    ASSERT_FALSE(arm->ready());

    std::vector<GetMoreResponse> responses;
    std::vector<BSONObj> batch1 = {
        fromjson("{_id: 1}"), fromjson("{_id: 2}"), fromjson("{_id: 3}")};
    responses.emplace_back(_nss, CursorId(123), batch1);
    scheduleNetworkResponses(responses);
    executor->waitForEvent(readyEvent);

    ASSERT_TRUE(arm->ready());
    ASSERT_EQ(fromjson("{_id: 1}"), *unittest::assertGet(arm->nextReady()));
    ASSERT_TRUE(arm->ready());
    ASSERT_EQ(fromjson("{_id: 2}"), *unittest::assertGet(arm->nextReady()));
    ASSERT_TRUE(arm->ready());
    ASSERT_EQ(fromjson("{_id: 3}"), *unittest::assertGet(arm->nextReady()));
    ASSERT_FALSE(arm->ready());

    readyEvent = unittest::assertGet(arm->nextEvent());
    ASSERT_FALSE(arm->ready());

    responses.clear();
    std::vector<BSONObj> batch2 = {
        fromjson("{_id: 4}"), fromjson("{_id: 5}"), fromjson("{_id: 6}")};
    responses.emplace_back(_nss, CursorId(456), batch2);
    scheduleNetworkResponses(responses);
    executor->waitForEvent(readyEvent);

    ASSERT_TRUE(arm->ready());
    ASSERT(!arm->nextReady().isOK());

    // Required to kill the 'arm' on error before destruction.
    auto killEvent = arm->kill();
    executor->waitForEvent(killEvent);
}

TEST_F(AsyncResultsMergerTest, BadResponseReceivedFromShard) {
    BSONObj findCmd = fromjson("{find: 'testcoll'}");
    makeCursorFromFindCmd(findCmd, _remotes);

    ASSERT_FALSE(arm->ready());
    auto readyEvent = unittest::assertGet(arm->nextEvent());
    ASSERT_FALSE(arm->ready());

    std::vector<BSONObj> batch1 = {fromjson("{_id: 1}"), fromjson("{_id: 2}")};
    BSONObj response1 = GetMoreResponse(_nss, CursorId(123), batch1).toBSON();
    BSONObj response2 = fromjson("{foo: 'bar'}");
    std::vector<BSONObj> batch3 = {fromjson("{_id: 4}"), fromjson("{_id: 5}")};
    BSONObj response3 = GetMoreResponse(_nss, CursorId(456), batch3).toBSON();
    scheduleNetworkResponseObjs({response1, response2, response3});
    executor->waitForEvent(readyEvent);

    ASSERT_TRUE(arm->ready());
    auto statusWithNext = arm->nextReady();
    ASSERT(!statusWithNext.isOK());

    // Required to kill the 'arm' on error before destruction.
    auto killEvent = arm->kill();
    executor->waitForEvent(killEvent);
}

TEST_F(AsyncResultsMergerTest, ErrorReceivedFromShard) {
    BSONObj findCmd = fromjson("{find: 'testcoll'}");
    makeCursorFromFindCmd(findCmd, _remotes);

    ASSERT_FALSE(arm->ready());
    auto readyEvent = unittest::assertGet(arm->nextEvent());
    ASSERT_FALSE(arm->ready());

    std::vector<GetMoreResponse> responses;
    std::vector<BSONObj> batch1 = {fromjson("{_id: 1}"), fromjson("{_id: 2}")};
    responses.emplace_back(_nss, CursorId(1), batch1);
    std::vector<BSONObj> batch2 = {fromjson("{_id: 3}"), fromjson("{_id: 4}")};
    responses.emplace_back(_nss, CursorId(2), batch2);
    scheduleNetworkResponses(responses);

    scheduleErrorResponse({ErrorCodes::BadValue, "bad thing happened"});
    executor->waitForEvent(readyEvent);

    ASSERT_TRUE(arm->ready());
    auto statusWithNext = arm->nextReady();
    ASSERT(!statusWithNext.isOK());
    ASSERT_EQ(statusWithNext.getStatus().code(), ErrorCodes::BadValue);
    ASSERT_EQ(statusWithNext.getStatus().reason(), "bad thing happened");

    // Required to kill the 'arm' on error before destruction.
    auto killEvent = arm->kill();
    executor->waitForEvent(killEvent);
}

TEST_F(AsyncResultsMergerTest, ErrorCantScheduleEventBeforeLastSignaled) {
    BSONObj findCmd = fromjson("{find: 'testcoll'}");
    makeCursorFromFindCmd(findCmd, {_remotes[0]});

    ASSERT_FALSE(arm->ready());
    auto readyEvent = unittest::assertGet(arm->nextEvent());

    // Error to call nextEvent() before the previous event is signaled.
    ASSERT_NOT_OK(arm->nextEvent().getStatus());

    std::vector<GetMoreResponse> responses;
    std::vector<BSONObj> batch = {fromjson("{_id: 1}"), fromjson("{_id: 2}")};
    responses.emplace_back(_nss, CursorId(0), batch);
    scheduleNetworkResponses(responses);
    executor->waitForEvent(readyEvent);

    ASSERT_TRUE(arm->ready());
    ASSERT_EQ(fromjson("{_id: 1}"), *unittest::assertGet(arm->nextReady()));
    ASSERT_TRUE(arm->ready());
    ASSERT_EQ(fromjson("{_id: 2}"), *unittest::assertGet(arm->nextReady()));
    ASSERT_TRUE(arm->ready());
    ASSERT(!unittest::assertGet(arm->nextReady()));

    // Required to kill the 'arm' on error before destruction.
    auto killEvent = arm->kill();
    executor->waitForEvent(killEvent);
}

TEST_F(AsyncResultsMergerTest, NextEventAfterTaskExecutorShutdown) {
    BSONObj findCmd = fromjson("{find: 'testcoll'}");
    makeCursorFromFindCmd(findCmd, _remotes);
    executor->shutdown();
    ASSERT_NOT_OK(arm->nextEvent().getStatus());
    auto killEvent = arm->kill();
    ASSERT_FALSE(killEvent.isValid());
}

TEST_F(AsyncResultsMergerTest, KillAfterTaskExecutorShutdownWithOutstandingBatches) {
    BSONObj findCmd = fromjson("{find: 'testcoll'}");
    makeCursorFromFindCmd(findCmd, {_remotes[0]});

    // Make a request to the shard that will never get answered.
    ASSERT_FALSE(arm->ready());
    auto readyEvent = unittest::assertGet(arm->nextEvent());
    ASSERT_FALSE(arm->ready());
    blackHoleNextRequest();

    // Executor shuts down before a response is received.
    executor->shutdown();
    auto killEvent = arm->kill();
    ASSERT_FALSE(killEvent.isValid());
}

TEST_F(AsyncResultsMergerTest, KillNoBatchesRequested) {
    BSONObj findCmd = fromjson("{find: 'testcoll'}");
    makeCursorFromFindCmd(findCmd, _remotes);

    ASSERT_FALSE(arm->ready());
    auto killedEvent = arm->kill();

    // Killed cursors are considered ready, but return an error when you try to receive the next
    // doc.
    ASSERT_TRUE(arm->ready());
    ASSERT_NOT_OK(arm->nextReady().getStatus());

    executor->waitForEvent(killedEvent);
}

TEST_F(AsyncResultsMergerTest, KillAllBatchesReceived) {
    BSONObj findCmd = fromjson("{find: 'testcoll', batchSize: 2}");
    makeCursorFromFindCmd(findCmd, _remotes);

    ASSERT_FALSE(arm->ready());
    auto readyEvent = unittest::assertGet(arm->nextEvent());
    ASSERT_FALSE(arm->ready());

    std::vector<GetMoreResponse> responses;
    std::vector<BSONObj> batch1 = {fromjson("{_id: 1}"), fromjson("{_id: 2}")};
    responses.emplace_back(_nss, CursorId(0), batch1);
    std::vector<BSONObj> batch2 = {fromjson("{_id: 3}"), fromjson("{_id: 4}")};
    responses.emplace_back(_nss, CursorId(0), batch2);
    std::vector<BSONObj> batch3 = {fromjson("{_id: 3}"), fromjson("{_id: 4}")};
    responses.emplace_back(_nss, CursorId(123), batch3);
    scheduleNetworkResponses(responses);

    // Kill should be able to return right away if there are no pending batches.
    auto killedEvent = arm->kill();
    ASSERT_TRUE(arm->ready());
    ASSERT_NOT_OK(arm->nextReady().getStatus());
    executor->waitForEvent(killedEvent);
}

TEST_F(AsyncResultsMergerTest, KillTwoOutstandingBatches) {
    BSONObj findCmd = fromjson("{find: 'testcoll', batchSize: 2}");
    makeCursorFromFindCmd(findCmd, _remotes);

    ASSERT_FALSE(arm->ready());
    auto readyEvent = unittest::assertGet(arm->nextEvent());
    ASSERT_FALSE(arm->ready());

    std::vector<GetMoreResponse> responses;
    std::vector<BSONObj> batch1 = {fromjson("{_id: 1}"), fromjson("{_id: 2}")};
    responses.emplace_back(_nss, CursorId(0), batch1);
    scheduleNetworkResponses(responses);

    // Kill event will only be signalled once the pending batches are received.
    auto killedEvent = arm->kill();

    // Ensures that callbacks run with a cancelled status.
    runReadyNetworkOperations();

    // Ensure that we properly signal both those waiting for the kill, and those waiting for more
    // results to be ready.
    executor->waitForEvent(readyEvent);
    executor->waitForEvent(killedEvent);
}

TEST_F(AsyncResultsMergerTest, NextEventErrorsAfterKill) {
    BSONObj findCmd = fromjson("{find: 'testcoll', batchSize: 2}");
    makeCursorFromFindCmd(findCmd, {_remotes[0]});

    ASSERT_FALSE(arm->ready());
    auto readyEvent = unittest::assertGet(arm->nextEvent());
    ASSERT_FALSE(arm->ready());

    std::vector<GetMoreResponse> responses;
    std::vector<BSONObj> batch1 = {fromjson("{_id: 1}"), fromjson("{_id: 2}")};
    responses.emplace_back(_nss, CursorId(1), batch1);
    scheduleNetworkResponses(responses);

    auto killedEvent = arm->kill();

    // Attempting to schedule more network operations on a killed arm is an error.
    ASSERT_NOT_OK(arm->nextEvent().getStatus());

    executor->waitForEvent(killedEvent);
}

TEST_F(AsyncResultsMergerTest, KillCalledTwice) {
    BSONObj findCmd = fromjson("{find: 'testcoll', batchSize: 2}");
    makeCursorFromFindCmd(findCmd, {_remotes[0]});
    auto killedEvent1 = arm->kill();
    ASSERT(killedEvent1.isValid());
    auto killedEvent2 = arm->kill();
    ASSERT(killedEvent2.isValid());
    executor->waitForEvent(killedEvent1);
    executor->waitForEvent(killedEvent2);
}

}  // namespace

}  // namespace mongo
