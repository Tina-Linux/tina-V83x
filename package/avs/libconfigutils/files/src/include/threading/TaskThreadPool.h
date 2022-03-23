/*
 * TaskThread.h
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

#ifndef AW_ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_THREADING_TASKTHREAD_POOL_H_
#define AW_ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_THREADING_TASKTHREAD_POOL_H_

#include <atomic>
#include <unordered_set>

#include "threading/TaskThread.h"

namespace AW {
/**
 * A TaskThreadPool is threads which reads from a TaskQueue and executes those tasks.
 */
class TaskThreadPool {
public:
    /**
     * Constructs a TaskThreadPool to read from the given TaskQueue. This does not start the thread.
     *
     * @params taskQueue A TaskQueue to take tasks from to execute.
     */
    TaskThreadPool(std::shared_ptr<TaskQueue> taskQueue, int numThreads);

    /**
     * Destructs the TaskThreadPool.
     */
    ~TaskThreadPool();

    /**
     * Starts executing tasks from the queue on the TaskThreadPool.
     */
    void start();

    /**
     * Returns whether or not the TaskThreadPool has been shutdown.
     *
     * @returns whether or not the TaskThreadPool has been shutdown.
     */
    bool isShutdown();

private:

    /// The threads to run tasks on.
    std::unordered_set<std::shared_ptr<TaskThread>> m_pool;
};

}
#endif  // AW_ALEXA_CLIENT_SDK_AVSCOMMON_UTILS_INCLUDE_AVSCOMMON_UTILS_THREADING_TASKTHREAD_POOL_H_
