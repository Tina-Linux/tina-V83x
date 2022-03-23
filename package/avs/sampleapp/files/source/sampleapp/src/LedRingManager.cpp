/*
 * LedRingManager.cpp
 *
 * Copyright (c) 2017 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include <sstream>

#include "SampleApp/LedRingManager.h"

#include <AVSCommon/SDKInterfaces/DialogUXStateObserverInterface.h>

#include "SampleApp/ConsolePrinter.h"

namespace alexaClientSDK {
namespace sampleApp {

using namespace avsCommon::sdkInterfaces;

std::shared_ptr<LedRingManager> LedRingManager::create(std::shared_ptr<AW::IShowManager> show)
{
    return std::shared_ptr<LedRingManager>(new LedRingManager(show));
}

LedRingManager::LedRingManager(std::shared_ptr<AW::IShowManager> show) :
    m_show{show}
{}

void LedRingManager::onDialogUXStateChanged(DialogUXState state) {
    m_executor.submit([this, state]() {
        if (state == m_dialogState) {
            return;
        }
        m_dialogState = state;
        switch (m_dialogState) {
            case DialogUXState::IDLE:
                m_show->enableShow(AW::Profile::IDLE, AW::ProfileFlag::REPLACE);
                return;
            case DialogUXState::LISTENING:
                m_show->enableShow(AW::Profile::LISTENING, AW::ProfileFlag::REPLACE);
                return;
            case DialogUXState::THINKING:
                m_show->enableShow(AW::Profile::THINKING, AW::ProfileFlag::REPLACE);
                return;
                ;
            case DialogUXState::SPEAKING:
                m_show->enableShow(AW::Profile::SPEAKING, AW::ProfileFlag::REPLACE);
                return;
            /*
             * This is an intermediate state after a SPEAK directive is completed. In the case of a speech burst the
             * next SPEAK could kick in or if its the last SPEAK directive ALEXA moves to the IDLE state. So we do
             * nothing for this state.
             */
            case DialogUXState::FINISHED:
                return;
        }
    });
}

void LedRingManager::onConnectionStatusChanged(const Status status, const ChangedReason reason) {
    m_executor.submit([this, status]() {
        if (m_connectionStatus == status) {
            return;
        }
        m_connectionStatus = status;
        if (m_connectionStatus == avsCommon::sdkInterfaces::ConnectionStatusObserverInterface::Status::DISCONNECTED) {
            m_show->enableShow(AW::Profile::MUTE, AW::ProfileFlag::REPLACE);
        } else if (m_connectionStatus == avsCommon::sdkInterfaces::ConnectionStatusObserverInterface::Status::PENDING) {
            m_show->enableShow(AW::Profile::CONNECTING, AW::ProfileFlag::REPLACE);
        } else if (m_connectionStatus == avsCommon::sdkInterfaces::ConnectionStatusObserverInterface::Status::CONNECTED) {
            m_show->enableShow(AW::Profile::IDLE, AW::ProfileFlag::REPLACE);
        }
    });
}

void LedRingManager::onSettingChanged(const std::string& key, const std::string& value) {
    m_executor.submit([key, value]() {

    });
}

void LedRingManager::onSpeakerSettingsChanged(
    const SpeakerManagerObserverInterface::Source& source,
    const SpeakerInterface::Type& type,
    const SpeakerInterface::SpeakerSettings& settings) {
    m_executor.submit([source, type, settings]() {

    });
}


}  // namespace sampleApp
}  // namespace alexaClientSDK
