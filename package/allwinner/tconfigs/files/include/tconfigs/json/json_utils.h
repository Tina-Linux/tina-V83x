#ifndef __TCONFIGS_JSON_JSON_UTILS_H__
#define __TCONFIGS_JSON_JSON_UTILS_H__

#include <string>
#include <memory>
#include <rapidjson/pointer.h>

#include "tconfigs/file/file_utils.h"

namespace tconfigs {
namespace json {

/**
 * @brief Parse JSON to a DOM
 *
 * @param json_string The JSON string to be parsed
 * @param[out] doc The output rapidjson @c Document, representing the DOM
 * @return @c true if has no parse error, @c false otherwise
 */
bool ParseJsonToDom(const char* json_string, rapidjson::Document* doc);

/**
 * Similar to @c ParseJsonToDom, but allow the following relaxed JSON syntax:
 *  1. one-line and multi-line comments
 *  2. trailing commas at the end of objects and arrays
 *  3. parsing @c NaN, @c Inf, @c Infinity, @c -Inf and @c -Infinity as @c double value
 */
bool ParseJsonToDomRelaxed(const char* json_string, rapidjson::Document* doc);

/**
 * @brief Include functions using JSON Pointer in rapidjson
 *
 * @see http://rapidjson.org/zh-cn/md_doc_pointer_8zh-cn.html (Chinese)
 * @see http://rapidjson.org/md_doc_pointer.html (English)
 */
namespace pointer {

/**
 * @brief Get value by JSON Pointer
 *
 * @param root The root node of JSON, can be a rapidjson @c Document or @c Value
 * @param token The JSON Pointer token
 * @param[out] result The pointer to the value got
 * @return @c true if the value exists and has no error, @c false otherwise
 *
 * This function doesn't check the value type, which can be int, string, object,
 * array, etc.
 */
bool GetValue(const rapidjson::Value& root, const char* token,
        const rapidjson::Value** result);

/**
 * @brief Get int/double/string value by JSON Pointer
 *
 * @param root The root node of JSON, can be a rapidjson @c Document or @c Value
 * @param token The JSON Pointer token
 * @param[out] result The int/double/string value got
 * @return @c true if the value exists and has no error, @c false otherwise
 */
bool GetInt(const rapidjson::Value& root, const char* token, int* result);
bool GetDouble(const rapidjson::Value& root, const char* token, double* result);
bool GetString(const rapidjson::Value& root, const char* token, const char** result);
bool GetBool(const rapidjson::Value& root, const char* token, bool* result);

/**
 * @brief Get object/array by JSON Pointer
 *
 * @param root The root node of JSON, can be a rapidjson @c Document or @c Value
 * @param token The JSON Pointer token
 * @param[out] result The pointer to the object/array got
 * @return @c true if the object/array exists and has no error, @c false otherwise
 */
bool GetObject(const rapidjson::Value& root, const char* token,
        const rapidjson::Value** result);
bool GetArray(const rapidjson::Value& root, const char* token,
        const rapidjson::Value** result);

} // namespace pointer

class JsonStringFromFile {
public:
    static std::shared_ptr<JsonStringFromFile> Create(const std::string& file_name);
    ~JsonStringFromFile();

    const char* string(void) const { return string_; }

private:
    JsonStringFromFile() = default;

    file::FileUtils file_utils_;
    const char* string_ = nullptr;
};

} // namespace json
} // namespace tconfigs

#endif /* ifndef __TCONFIGS_JSON_JSON_UTILS_H__ */
