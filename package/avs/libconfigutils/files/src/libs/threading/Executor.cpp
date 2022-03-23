/*
 * Executor.cpp
 *
 * Copyright 2017 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License").
 * You may not use this file except in compliance with the License.
 * A copy of the License is located at
 *
 *     http://aws.amazon.com/apache2.0/
 *
 * or in the "license" file accompanying this file. This file is distributed
 * on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 */

#include "threading/Executor.h"
/*
template <typename T, typename... Args>
std::unique_ptr<T> make_unique(Args&&... args) {
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}
m_taskThread{make_unique<TaskThread>(m_taskQueue)}
*/
namespace AW {

std::shared_ptr<Executor> Executor::create(int numThreads)
{
    return std::shared_ptr<Executor>(new Executor(numThreads));
}

Executor::Executor(int numThreads) :
        m_taskQueue{std::make_shared<TaskQueue>()},
        m_taskThreadPool{std::unique_ptr<TaskThreadPool>(new TaskThreadPool(m_taskQueue, numThreads))} {
    m_taskThreadPool->start();
}

Executor::~Executor() {
    shutdown();
}

void Executor::waitForSubmittedTasks() {
    std::promise<void> flushedPromise;
    auto flushedFuture = flushedPromise.get_future();
    auto task = [&flushedPromise]() { flushedPromise.set_value(); };
    submit(task);
    flushedFuture.get();
}

void Executor::shutdown() {
    m_taskQueue->shutdown();
    m_taskThreadPool.reset();
}

bool Executor::isShutdown() {
    return m_taskQueue->isShutdown();
}

}
