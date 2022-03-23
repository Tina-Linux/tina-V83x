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

#include "SampleApp/TconfigsPlatformAdapter.h"

#include <AVSCommon/Utils/Logger/Logger.h>

namespace alexaClientSDK {
namespace sampleApp {

static const std::string TAG("TconfigsPlatformAdapter");
#define LX(event) alexaClientSDK::avsCommon::utils::logger::LogEntry(TAG, event)

std::shared_ptr<TconfigsPlatformAdapter> TconfigsPlatformAdapter::create(
        const std::string& topConfigFile)
{
    auto adapter = std::shared_ptr<TconfigsPlatformAdapter>(new TconfigsPlatformAdapter());
    if (!adapter || !adapter->initialize(topConfigFile)) {
        ACSDK_CRITICAL(LX("Failed to create PlatformAdapter"));
        return nullptr;
    }
    return adapter;
}

bool TconfigsPlatformAdapter::initialize(const std::string& topConfigFile)
{
    auto topConfigJson= tconfigs::json::JsonStringFromFile::Create(topConfigFile);
    if (!topConfigJson) {
        ACSDK_CRITICAL(LX("Failed to get JSON").d("JSON file", topConfigFile));
        return false;
    }
    rapidjson::Document topDoc;
    if (!tconfigs::json::ParseJsonToDomRelaxed(topConfigJson->string(), &topDoc)) {
        ACSDK_CRITICAL(LX("Failed to parse JSON to DOM").d("JSON file", topConfigFile));
        return false;
    }

    const char* configFileString = nullptr;
    if (!tconfigs::json::pointer::GetString(topDoc, "/config_file", &configFileString)) {
        ACSDK_CRITICAL(LX("Failed to get config \"config_file\"").d(
                    "JSON file", topConfigFile));
        return false;
    }

    std::string configFile(configFileString);
    auto configJson= tconfigs::json::JsonStringFromFile::Create(configFile);
    if (!configJson) {
        ACSDK_CRITICAL(LX("Failed to get JSON").d("JSON file", configFile));
        return false;
    }
    if (!tconfigs::json::ParseJsonToDomRelaxed(configJson->string(), &m_doc)) {
        ACSDK_CRITICAL(LX("Failed to parse JSON to DOM").d("JSON file", configFile));
        return false;
    }

    const rapidjson::Value* config = nullptr;

    if (!tconfigs::json::pointer::GetObject(m_doc, "/record_pipeline", &config)) {
        ACSDK_CRITICAL(LX("Failed to get config \"record_pipeline\"").d(
                    "JSON file", configFile));
        return false;
    }
    m_recordPipeline = tconfigs::audio::Pipeline::Create("record_pipeline", *config);
    if (!m_recordPipeline) {
        ACSDK_CRITICAL(LX("Failed to create record pipeline"));
        return false;
    }

    if (!tconfigs::json::pointer::GetObject(m_doc, "/key_manager", &config)) {
        ACSDK_WARN(LX("Failed to get config \"key_manager\"").d(
                    "JSON file", configFile));
    } else {
        m_keyManager = tconfigs::input::KeyManager::Create(*config);
        if (!m_keyManager) {
            ACSDK_CRITICAL(LX("Failed to create key manager"));
            return false;
        }
    }

    return true;
}

}  // namespace sampleApp
}  // namespace alexaClientSDK
