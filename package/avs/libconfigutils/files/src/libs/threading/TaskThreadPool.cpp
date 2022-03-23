/*
 * TaskThreadPool.cpp
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

#include "threading/TaskThreadPool.h"

namespace AW {

TaskThreadPool::TaskThreadPool(std::shared_ptr<TaskQueue> taskQueue, int numThreads) {
    for(int i = 0; i < numThreads; i++) {
        m_pool.insert(std::shared_ptr<TaskThread>(new TaskThread(taskQueue)));
    }
}

TaskThreadPool::~TaskThreadPool() {
}

void TaskThreadPool::start() {
    for(auto taskthread : m_pool) {
        taskthread->start();
    }
}

bool TaskThreadPool::isShutdown() {
    bool is_shutdown = true;
    for(auto taskthread : m_pool) {
        if(!taskthread->isShutdown()) return false;
    }
    return is_shutdown;
}
}
