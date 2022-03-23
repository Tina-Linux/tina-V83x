/*
 * KeyPadInputManager.h
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

#ifndef ALEXA_CLIENT_SDK_SAMPLEAPP_INCLUDE_SAMPLEAPP_KEYPADINPUTMANAGER_H_
#define ALEXA_CLIENT_SDK_SAMPLEAPP_INCLUDE_SAMPLEAPP_KEYPADINPUTMANAGER_H_

#include <memory>
#include <thread>

#include <ev++.h>

#include "SampleApp/InteractionManager.h"

#include <button/IButtonManager.h>
#include <platformadapter/PlatformAdapter.h>
#include <ledshow/Show.h>

namespace alexaClientSDK {
namespace sampleApp {

/// Observes user input from the console and notifies the interaction manager of the user's intentions.
class ButtonInputManager : public AW::IButtonManager::Observer,
                           public std::enable_shared_from_this<ButtonInputManager>
{
public:
    /**
     * Create a ButtonInputManager.
     *
     * @param interactionManager An instance of the @c InteractionManager used to manage user input.
     * @return Returns a new @c ButtonInputManager, or @c nullptr if the operation failed.
     */
    static std::shared_ptr<ButtonInputManager> create(std::shared_ptr<InteractionManager> interactionManager,
                                                      std::shared_ptr<AW::PlatformAdapter> platformadapter);

    /**
     * Processes user input forever. Returns upon a quit command.
     */
    int run();
    void stop();
private:
    /**
     * Constructor.
     */
    ButtonInputManager(std::shared_ptr<InteractionManager> interactionManager,
                       std::shared_ptr<AW::PlatformAdapter> platformadapter);

    void onVolumeUp();
    void onVolumeDown();
    void onMute();
    void onAudioJackPlugIn();
    void onAudioJackPlugOut();

private:
    /// The main interaction manager that interfaces with the SDK.
    std::shared_ptr<InteractionManager> m_interactionManager;
    std::shared_ptr<AW::PlatformAdapter> m_platformadapter;

    enum class Status
    {
        PRIVATE_MUTE,
        PRIVATE_UNMUTE
    };
    Status m_status{Status::PRIVATE_UNMUTE};
};

}  // namespace sampleApp
}  // namespace alexaClientSDK

#endif  // ALEXA_CLIENT_SDK_SAMPLEAPP_INCLUDE_SAMPLEAPP_KEYPADINPUTMANAGER_H_
