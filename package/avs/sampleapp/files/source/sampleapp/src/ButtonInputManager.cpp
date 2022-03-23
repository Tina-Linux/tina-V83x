/*
 * ButtonInputManager.cpp
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

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <errno.h>
#include <linux/input.h>
#include <ugpio/ugpio.h>

#include <cctype>

#include <AVSCommon/SDKInterfaces/SpeakerInterface.h>
#include "SampleApp/ButtonInputManager.h"
#include "SampleApp/ConsolePrinter.h"
/*
static int gpio_output_setting(int pin, int value) {
    int rq, rv, al;
    rq = gpio_is_requested(pin);
    if(rq < 0) printf("gpio: %d is requested!\n", pin);
    rv = gpio_request(pin, NULL);
    if(rv < 0) printf("gpio: %d requested failed!\n", pin);
    rv = gpio_direction_output(pin, GPIOF_INIT_LOW);
    if(rv < 0) printf("gpio: %d direction output failed!\n", pin);
    al = gpio_get_activelow(pin);
    if(al < 0) printf("gpio: %d set active low failed!\n", pin);
    gpio_set_value(pin, value);

    return 0;
}

static int mozart_do_mute(bool is_mute) {
    gpio_output_setting(100, is_mute?1:0);
    gpio_output_setting(362, is_mute?1:0);

    return 0;
}

static int hena_do_mute(bool is_mute) {
    gpio_output_setting(38, is_mute?1:0);
    return 0;
}
*/
namespace alexaClientSDK {
namespace sampleApp {

using namespace avsCommon::sdkInterfaces;

static const int8_t INCREASE_VOLUME = 10;
static const int8_t DECREASE_VOLUME = -10;

std::shared_ptr<ButtonInputManager> ButtonInputManager::create(std::shared_ptr<InteractionManager> interactionManager,
                                                               std::shared_ptr<AW::PlatformAdapter> platformadapter) {
    if (!interactionManager) {
        ConsolePrinter::simplePrint("Invalid InteractionManager passed to ButtonInputManager");
        return nullptr;
    }
    auto button_manger = std::shared_ptr<ButtonInputManager>(new ButtonInputManager(interactionManager, platformadapter));
    platformadapter->getButtonManager()->addButtonObserver(button_manger);

    return button_manger;
}

ButtonInputManager::ButtonInputManager(std::shared_ptr<InteractionManager> interactionManager,
                                       std::shared_ptr<AW::PlatformAdapter> platformadapter) :
        m_interactionManager{interactionManager}, m_platformadapter{platformadapter} {
            m_platformadapter->getMuteManager()->privacyMute(false);
            m_status = Status::PRIVATE_UNMUTE;
}

void ButtonInputManager::onVolumeUp()
{
    m_interactionManager->adjustVolume(SpeakerInterface::Type::AVS_SYNCED, INCREASE_VOLUME);
}

void ButtonInputManager::onVolumeDown()
{
    m_interactionManager->adjustVolume(SpeakerInterface::Type::AVS_SYNCED, DECREASE_VOLUME);
}

void ButtonInputManager::onMute()
{
    if(m_status == Status::PRIVATE_UNMUTE){
        m_status = Status::PRIVATE_MUTE;
        m_platformadapter->getShowManager()->enableShow(AW::Profile::MUTE, AW::ProfileFlag::REPLACE);
        m_platformadapter->getMuteManager()->privacyMute(true);
    }else{
        m_status = Status::PRIVATE_UNMUTE;
        m_platformadapter->getShowManager()->enableShow(AW::Profile::UNMUTE, AW::ProfileFlag::REPLACE);
        m_platformadapter->getMuteManager()->privacyMute(false);
    }
}

void ButtonInputManager::onAudioJackPlugIn()
{
    m_platformadapter->getAudioJackManager()->doAudioJackPlugIn();
}

void ButtonInputManager::onAudioJackPlugOut()
{
    m_platformadapter->getAudioJackManager()->doAudioJackPlugOut();
}

/*
void ButtonInputManager::onKeyEventDown(int keycode)
{
    switch(keycode) {
        case KEY_VOLUMEDOWN :
            m_interactionManager->adjustVolume(SpeakerInterface::Type::AVS_SYNCED, DECREASE_VOLUME);
            break;
        case KEY_VOLUMEUP :
            m_interactionManager->adjustVolume(SpeakerInterface::Type::AVS_SYNCED, INCREASE_VOLUME);
            break;
        case KEY_MUTE:
            if(m_status == Status::PRIVATE_UNMUTE){
                m_status = Status::PRIVATE_MUTE;
                hena_do_mute(true);
                m_show->enableShow(AW::Profile::MUTE, AW::ProfileFlag::REPLACE);
            }else{
                m_status = Status::PRIVATE_UNMUTE;
                hena_do_mute(false);
                m_show->enableShow(AW::Profile::IDLE, AW::ProfileFlag::REPLACE);
            }
            break;
        case KEY_PLAYPAUSE:
            //m_interactionManager->playbackPlay();
            //m_interactionManager->playbackPause();
            break;
		case 0x02:
			printf("hp jack plug in\n");
			gpio_output_setting(96, 0);
    }
}

void ButtonInputManager::onKeyEventUp(int keycode)
{
    switch(keycode) {
		case 0x02:
			printf("hp jack plug out\n");
			gpio_output_setting(96, 1);
    }

}

void ButtonInputManager::onkeyEventLongClick(int keycode, float longclicktime)
{

}
*/
int ButtonInputManager::run() {

    return 0;
}

void ButtonInputManager::stop() {
    m_platformadapter->getButtonManager()->removeButtonObserver(shared_from_this());
}

}  // namespace sampleApp
}  // namespace alexaClientSDK
