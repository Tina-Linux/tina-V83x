#include "tconfigs/json/json_utils.h"

#include "tconfigs/log/logging.h"
#include <rapidjson/error/en.h>

#define GET_GENERIC_VALUE_BY_POINTER(type, root, token, result) \
do { \
    const rapidjson::Value* val_ptr = rapidjson::Pointer(token).Get(root); \
    if (!val_ptr || !val_ptr->Is##type()) {\
        return false; \
    } \
    *result = val_ptr->Get##type(); \
    return true; \
} while (0)

#define GET_OBJECT_OR_ARRAY_BY_POINTER(type, root, token, result) \
do { \
    const rapidjson::Value* val_ptr = rapidjson::Pointer(token).Get(root); \
    if (!val_ptr || !val_ptr->Is##type()) {\
        return false; \
    } \
    *result = val_ptr; \
    return true; \
} while (0)

namespace tconfigs {
namespace json {

bool ParseJsonToDom(const char* json_string, rapidjson::Document* doc)
{
    if (doc->Parse(json_string).HasParseError()) {
        TCLOG_ERROR("Parse JSON error (offset: %u characters): %s",
                (unsigned)doc->GetErrorOffset(),
                rapidjson::GetParseError_En(doc->GetParseError()));
        return false;
    }
    return true;
}

bool ParseJsonToDomRelaxed(const char* json_string, rapidjson::Document* doc)
{
    if (doc->Parse<
            rapidjson::kParseCommentsFlag
            | rapidjson::kParseTrailingCommasFlag
            | rapidjson::kParseNanAndInfFlag>(json_string).HasParseError()) {
        TCLOG_ERROR("Parse JSON error (offset: %u characters): %s",
                (unsigned)doc->GetErrorOffset(),
                rapidjson::GetParseError_En(doc->GetParseError()));
        return false;
    }
    return true;
}

namespace pointer {

bool GetValue(const rapidjson::Value& root, const char* token,
        const rapidjson::Value** result)
{
    const rapidjson::Value* val_ptr = rapidjson::Pointer(token).Get(root);
    if (!val_ptr) {
        return false;
    }
    *result = val_ptr;
    return true;
}

bool GetInt(const rapidjson::Value& root, const char* token, int* result)
{
    GET_GENERIC_VALUE_BY_POINTER(Int, root, token, result);
}

bool GetDouble(const rapidjson::Value& root, const char* token, double* result)
{
    GET_GENERIC_VALUE_BY_POINTER(Double, root, token, result);
}

bool GetString(const rapidjson::Value& root, const char* token, const char** result)
{
    GET_GENERIC_VALUE_BY_POINTER(String, root, token, result);
}

bool GetBool(const rapidjson::Value& root, const char* token, bool* result)
{
    GET_GENERIC_VALUE_BY_POINTER(Bool, root, token, result);
}

bool GetObject(const rapidjson::Value& root, const char* token,
        const rapidjson::Value** result)
{
    GET_OBJECT_OR_ARRAY_BY_POINTER(Object, root, token, result);
}

bool GetArray(const rapidjson::Value& root, const char* token,
        const rapidjson::Value** result)
{
    GET_OBJECT_OR_ARRAY_BY_POINTER(Array, root, token, result);
}

} // namespace pointer


std::shared_ptr<JsonStringFromFile> JsonStringFromFile::Create(const std::string& file_name)
{
    auto json_string = std::shared_ptr<JsonStringFromFile>(new JsonStringFromFile());
    if (!json_string || 0 != json_string->file_utils_.OpenRead(file_name)) {
        return nullptr;
    }
    int file_bytes = json_string->file_utils_.FileSizeInBytes();
    char* buf = new char[file_bytes + 1];
    if (file_bytes != json_string->file_utils_.Read(buf, file_bytes)) {
        return nullptr;
    }
    buf[file_bytes] = '\0';
    json_string->string_ = buf;
    return json_string;
}

JsonStringFromFile::~JsonStringFromFile()
{
    if (string_) {
        delete [] string_;
        string_ = nullptr;
    }
}

} // namespace json
} // namespace tconfigs
