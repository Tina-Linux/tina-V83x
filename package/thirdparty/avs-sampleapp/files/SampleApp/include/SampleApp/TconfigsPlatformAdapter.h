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

#ifndef ALEXA_CLIENT_SDK_SAMPLEAPP_INCLUDE_SAMPLEAPP_TCONFIGSPLATFORMADAPTER_H_
#define ALEXA_CLIENT_SDK_SAMPLEAPP_INCLUDE_SAMPLEAPP_TCONFIGSPLATFORMADAPTER_H_

#include <tconfigs/json/json_utils.h>
#include <tconfigs/audio/common/pipeline.h>
#include <tconfigs/input/key_manager.h>

namespace alexaClientSDK {
namespace sampleApp {

class TconfigsPlatformAdapter {
public:
    static std::shared_ptr<TconfigsPlatformAdapter> create(const std::string& topConfigFile);

    ~TconfigsPlatformAdapter() = default;

    std::shared_ptr<tconfigs::audio::Pipeline> getRecordPipeline() { return m_recordPipeline; }
    std::shared_ptr<tconfigs::input::KeyManager> getKeyManager() { return m_keyManager; }

private:
    TconfigsPlatformAdapter() = default;

    bool initialize(const std::string& topConfigFile);

    rapidjson::Document m_doc;

    std::shared_ptr<tconfigs::audio::Pipeline> m_recordPipeline = nullptr;
    std::shared_ptr<tconfigs::input::KeyManager> m_keyManager = nullptr;
};

} // namespace sampleApp
} // namespace alexaClientSDK

#endif  // ALEXA_CLIENT_SDK_SAMPLEAPP_INCLUDE_SAMPLEAPP_TCONFIGSPLATFORMADAPTER_H_
