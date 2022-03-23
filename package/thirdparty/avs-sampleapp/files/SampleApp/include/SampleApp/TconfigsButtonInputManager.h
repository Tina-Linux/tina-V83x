/*
 * Copyright 2017-2018 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * Copyright 2019 Allwinner Technology Co.,Ltd
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
#ifndef ALEXA_CLIENT_SDK_SAMPLEAPP_INCLUDE_SAMPLEAPP_TCONFIGSBUTTONINPUTMANAGER_H_
#define ALEXA_CLIENT_SDK_SAMPLEAPP_INCLUDE_SAMPLEAPP_TCONFIGSBUTTONINPUTMANAGER_H_

#include <tconfigs/input/key_manager.h>

#include "SampleApp/InteractionManager.h"
#include "SampleApp/TconfigsPlatformAdapter.h"

namespace alexaClientSDK {
namespace sampleApp {

class TconfigsButtonInputManager {
public:
    static std::unique_ptr<TconfigsButtonInputManager> create(
            std::shared_ptr<InteractionManager> interactionManager,
            std::shared_ptr<TconfigsPlatformAdapter> adapter);

    ~TconfigsButtonInputManager() = default;

private:
    TconfigsButtonInputManager() = delete;
    TconfigsButtonInputManager(std::shared_ptr<InteractionManager> interactionManager);

    bool initialize(std::shared_ptr<TconfigsPlatformAdapter> adapter);

    std::shared_ptr<InteractionManager> m_interactionManager = nullptr;
    std::shared_ptr<tconfigs::input::KeyManager> m_keyManager = nullptr;

    bool m_isMute = false;
};

} // namespace sampleApp
} // namespace alexaClientSDK

#endif  // ALEXA_CLIENT_SDK_SAMPLEAPP_INCLUDE_SAMPLEAPP_TCONFIGSBUTTONINPUTMANAGER_H_
