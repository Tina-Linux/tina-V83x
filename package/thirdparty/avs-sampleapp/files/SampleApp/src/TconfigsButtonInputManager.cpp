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

#include "SampleApp/TconfigsButtonInputManager.h"

#include <AVSCommon/SDKInterfaces/SpeakerInterface.h>
#include <AVSCommon/Utils/Logger/Logger.h>

namespace alexaClientSDK {
namespace sampleApp {

static const std::string TAG("TconfigsButtonInputManager");
#define LX(event) alexaClientSDK::avsCommon::utils::logger::LogEntry(TAG, event)

static const int8_t INCREASE_VOLUME = 10;
static const int8_t DECREASE_VOLUME = -10;

std::unique_ptr<TconfigsButtonInputManager> TconfigsButtonInputManager::create(
        std::shared_ptr<InteractionManager> interactionManager,
        std::shared_ptr<TconfigsPlatformAdapter> adapter)
{
    if (!interactionManager) {
        ACSDK_CRITICAL(LX("Invalid parameter interactionManager"));
        return nullptr;
    }
    if (!adapter) {
        ACSDK_CRITICAL(LX("Invalid parameter adapter"));
        return nullptr;
    }

    std::unique_ptr<TconfigsButtonInputManager> buttonInputManager(
            new TconfigsButtonInputManager(interactionManager));
    if (!buttonInputManager || !buttonInputManager->initialize(adapter)) {
        ACSDK_WARN(LX("Failed to create TconfigsButtonInputManager"));
        return nullptr;
    }
    return buttonInputManager;
}

TconfigsButtonInputManager::TconfigsButtonInputManager(
        std::shared_ptr<InteractionManager> interactionManager)
    : m_interactionManager(interactionManager)
{
}

bool TconfigsButtonInputManager::initialize(std::shared_ptr<TconfigsPlatformAdapter> adapter)
{
    m_keyManager = adapter->getKeyManager();
    if (!m_keyManager) {
        ACSDK_WARN(LX("Invalid key manager"));
        return false;
    }

    bool ret = true;
    ret = m_keyManager->SetBehaviorCallback("VolumeUp", [&] {
            m_interactionManager->adjustVolume(
                    avsCommon::sdkInterfaces::SpeakerInterface::Type::AVS_SPEAKER_VOLUME,
                    INCREASE_VOLUME);
        });
    if (!ret) {
        ACSDK_WARN(LX("Failed to set callback for behavior \"VolumeUp\""));
    }

    ret = m_keyManager->SetBehaviorCallback("VolumeDown", [&] {
            m_interactionManager->adjustVolume(
                    avsCommon::sdkInterfaces::SpeakerInterface::Type::AVS_SPEAKER_VOLUME,
                    DECREASE_VOLUME);
        });
    if (!ret) {
        ACSDK_WARN(LX("Failed to set callback for behavior \"VolumeDown\""));
    }

    ret = m_keyManager->SetBehaviorCallback("Mute", [&] {
            m_interactionManager->setMute(
                    avsCommon::sdkInterfaces::SpeakerInterface::Type::AVS_SPEAKER_VOLUME,
                    !m_isMute);
            m_isMute = !m_isMute;
        });
    if (!ret) {
        ACSDK_WARN(LX("Failed to set callback for behavior \"Mute\""));
    }

    if (!m_keyManager->StartBuiltInEventLoop()) {
        ACSDK_CRITICAL(LX("Failed to start built-in event loop"));
        return false;
    }
    return true;
}

} // namespace sampleApp
} // namespace alexaClientSDK
