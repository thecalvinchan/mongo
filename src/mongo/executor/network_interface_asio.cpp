/**
 *    Copyright (C) 2015 MongoDB Inc.
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

#define MONGO_LOG_DEFAULT_COMPONENT ::mongo::logger::LogComponent::kExecutor

#include "mongo/platform/basic.h"

#include "mongo/executor/network_interface_asio.h"

#include <utility>

#include "mongo/config.h"
#include "mongo/stdx/chrono.h"
#include "mongo/stdx/memory.h"
#include "mongo/util/log.h"
#include "mongo/util/net/sock.h"

#include "mongo/util/net/ssl_manager.h"

namespace mongo {
namespace executor {

NetworkInterfaceASIO::NetworkInterfaceASIO()
    : _io_service(), _resolver(_io_service), _state(State::kReady), _isExecutorRunnable(false) {
    _connPool = stdx::make_unique<ConnectionPool>(kMessagingPortKeepOpen);

#ifdef MONGO_CONFIG_SSL
    if (getSSLManager()) {
        // We use sslv23, which corresponds to OpenSSLs SSLv23_method, for compatibility with older
        // versions of OpenSSL. This mirrors the call to SSL_CTX_new in ssl_manager.cpp. In
        // initAsyncSSLContext we explicitly disable all protocols other than TLSv1, TLSv1.1,
        // and TLSv1.2.
        _sslContext.emplace(asio::ssl::context::sslv23);
        uassertStatusOK(
            getSSLManager()->initSSLContext(_sslContext->native_handle(), getSSLGlobalParams()));
    }
#endif
}

std::string NetworkInterfaceASIO::getDiagnosticString() {
    str::stream output;
    output << "NetworkInterfaceASIO";
    output << " inShutdown: " << inShutdown();
    return output;
}

std::string NetworkInterfaceASIO::getHostName() {
    return getHostNameCached();
}

void NetworkInterfaceASIO::startup() {
    _serviceRunner = stdx::thread([this]() {
        asio::io_service::work work(_io_service);
        _io_service.run();
    });
    _state.store(State::kRunning);
}

void NetworkInterfaceASIO::shutdown() {
    _state.store(State::kShutdown);
    _io_service.stop();
    _serviceRunner.join();
}

void NetworkInterfaceASIO::waitForWork() {
    stdx::unique_lock<stdx::mutex> lk(_executorMutex);
    // TODO: This can be restructured with a lambda.
    while (!_isExecutorRunnable) {
        _isExecutorRunnableCondition.wait(lk);
    }
    _isExecutorRunnable = false;
}

void NetworkInterfaceASIO::waitForWorkUntil(Date_t when) {
    stdx::unique_lock<stdx::mutex> lk(_executorMutex);
    // TODO: This can be restructured with a lambda.
    while (!_isExecutorRunnable) {
        const Milliseconds waitTime(when - now());
        if (waitTime <= Milliseconds(0)) {
            break;
        }
        _isExecutorRunnableCondition.wait_for(lk, waitTime);
    }
    _isExecutorRunnable = false;
}

void NetworkInterfaceASIO::setConnectionHook(std::unique_ptr<ConnectionHook> hook) {
    MONGO_UNREACHABLE;
}

void NetworkInterfaceASIO::signalWorkAvailable() {
    stdx::unique_lock<stdx::mutex> lk(_executorMutex);
    _signalWorkAvailable_inlock();
}

void NetworkInterfaceASIO::_signalWorkAvailable_inlock() {
    if (!_isExecutorRunnable) {
        _isExecutorRunnable = true;
        _isExecutorRunnableCondition.notify_one();
    }
}

Date_t NetworkInterfaceASIO::now() {
    return Date_t::now();
}

void NetworkInterfaceASIO::startCommand(const TaskExecutor::CallbackHandle& cbHandle,
                                        const RemoteCommandRequest& request,
                                        const RemoteCommandCompletionFn& onFinish) {
    auto ownedOp = stdx::make_unique<AsyncOp>(cbHandle, request, onFinish, now());

    AsyncOp* op = ownedOp.get();

    {
        stdx::lock_guard<stdx::mutex> lk(_inProgressMutex);
        _inProgress.emplace(op, std::move(ownedOp));
    }

    asio::post(_io_service, [this, op]() { _startCommand(op); });
}

void NetworkInterfaceASIO::cancelCommand(const TaskExecutor::CallbackHandle& cbHandle) {
    stdx::lock_guard<stdx::mutex> lk(_inProgressMutex);
    for (auto iter = _inProgress.begin(); iter != _inProgress.end(); ++iter) {
        if (iter->first->cbHandle() == cbHandle) {
            iter->first->cancel();
            break;
        }
    }
}

void NetworkInterfaceASIO::setAlarm(Date_t when, const stdx::function<void()>& action) {
    MONGO_UNREACHABLE;
};

bool NetworkInterfaceASIO::inShutdown() const {
    return (_state.load() == State::kShutdown);
}

}  // namespace executor
}  // namespace mongo
