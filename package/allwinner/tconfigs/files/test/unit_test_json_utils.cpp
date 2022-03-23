#include <string>
#include <gtest/gtest.h>
#include "tconfigs/json/json_utils.h"

std::string json_string =
    "{"
        "\"Int\" : 1,"
        "\"Double\" : 1.2,"
        "\"String\" : \"This is a string\","
        "\"Object\" : {\"name\" : \"foo\", \"content\" : \"bar\"},"
        "\"Array\" : [ 11, 13, 13 ],"
        "\"ObjectNest\" : {"
            "\"foo1\" : { \"bar1\" : \"foobar1\"},"
            "\"foo2\" : { \"bar2\" : \"foobar2\"}"
        "}"
    "}";

TEST(DomTest, ParseJsonToDom) {
    rapidjson::Document doc;
    ASSERT_TRUE(tconfigs::json::ParseJsonToDom(json_string.c_str(), &doc))
        << "The JSON string is:" << std::endl << json_string;
}

TEST(PointerTest, GetInt) {
    rapidjson::Document doc;
    tconfigs::json::ParseJsonToDom(json_string.c_str(), &doc);
    int result = 0;
    ASSERT_TRUE(tconfigs::json::pointer::GetInt(doc, "/Int", &result));
    std::cout << "/Int: " << result << std::endl;
}

TEST(PointerTest, GetDouble) {
    rapidjson::Document doc;
    tconfigs::json::ParseJsonToDom(json_string.c_str(), &doc);
    double result = 0;
    ASSERT_TRUE(tconfigs::json::pointer::GetDouble(doc, "/Double", &result));
    std::cout << "/Double: " << result << std::endl;
}

TEST(PointerTest, GetString) {
    rapidjson::Document doc;
    tconfigs::json::ParseJsonToDom(json_string.c_str(), &doc);
    const char* result;
    ASSERT_TRUE(tconfigs::json::pointer::GetString(doc, "/String", &result));
    std::cout << "/String: " << result << std::endl;
    ASSERT_TRUE(tconfigs::json::pointer::GetString(doc, "/Object/name", &result));
    std::cout << "/Object/name: " << result << std::endl;
}

TEST(PointerTest, GetObject) {
    rapidjson::Document doc;
    tconfigs::json::ParseJsonToDom(json_string.c_str(), &doc);
    const rapidjson::Value* object;
    ASSERT_TRUE(tconfigs::json::pointer::GetObject(doc, "/Object", &object));
    const char* result;
    ASSERT_TRUE(tconfigs::json::pointer::GetString(*object, "/name", &result));
    std::cout << "/Object/name: " << result << std::endl;
    ASSERT_TRUE(tconfigs::json::pointer::GetString(*object, "/content", &result));
    std::cout << "/Object/content: " << result << std::endl;
}

TEST(PointerTest, GetArray) {
    rapidjson::Document doc;
    tconfigs::json::ParseJsonToDom(json_string.c_str(), &doc);
    const rapidjson::Value* array;
    ASSERT_TRUE(tconfigs::json::pointer::GetArray(doc, "/Array", &array));
    for (rapidjson::SizeType i = 0; i < array->Size(); ++i) {
        ASSERT_TRUE((*array)[i].IsInt());
        std::cout << "Array[" << i << "]: " << (*array)[i].GetInt() << std::endl;
    }
}

TEST(PointerTest, GetArrayMember) {
    rapidjson::Document doc;
    tconfigs::json::ParseJsonToDom(json_string.c_str(), &doc);

    // Get the array first, then get its member by pointer or index
    const rapidjson::Value* array;
    ASSERT_TRUE(tconfigs::json::pointer::GetArray(doc, "/Array", &array));
    int member = 0;
    // Get the first member by pointer
    ASSERT_TRUE(tconfigs::json::pointer::GetInt(*array, "/0", &member));
    ASSERT_EQ(member, 11);
    std::cout << "/Array/0: " << member << std::endl;
    member = 0;
    // Get the first member by index
    ASSERT_TRUE((*array)[0].IsInt());
    member = (*array)[0].GetInt();
    ASSERT_EQ(member, 11);
    std::cout << "Array[0]: " << member << std::endl;

    // Alternatively, we can get the array member directly by pointer
    member = 0;
    ASSERT_TRUE(tconfigs::json::pointer::GetInt(doc, "/Array/0", &member));
    ASSERT_EQ(member, 11);
    std::cout << "/Array/0: " << member << std::endl;
}

TEST(PointerTest, GetObjectNestMember) {
    rapidjson::Document doc;
    tconfigs::json::ParseJsonToDom(json_string.c_str(), &doc);

    const rapidjson::Value* object;
    ASSERT_TRUE(tconfigs::json::pointer::GetObject(doc, "/ObjectNest", &object));
    EXPECT_EQ(object->Size(), 2);
    std::cout << "ObjectNest size: " << object->Size() << std::endl;
    const char* member = nullptr;
    ASSERT_TRUE(tconfigs::json::pointer::GetString(*object, "/foo1/bar1", &member));
    ASSERT_NE(member, nullptr);
    std::cout << "/ObjectNest/foo1/bar1: " << member << std::endl;
}

TEST(PointerTest, GetObjectMemberName) {
    rapidjson::Document doc;
    tconfigs::json::ParseJsonToDom(json_string.c_str(), &doc);

    const rapidjson::Value* object;
    ASSERT_TRUE(tconfigs::json::pointer::GetObject(doc, "/ObjectNest", &object));
    EXPECT_EQ(object->Size(), 2);
    std::cout << "ObjectNest size: " << object->Size() << std::endl;

    std::cout << "ObjectNest's members: ";
    for (rapidjson::Value::ConstMemberIterator itr = object->MemberBegin();
            itr != object->MemberEnd(); ++itr) {
        std::cout << itr->name.GetString() << " ";
    }
    std::cout << std::endl;
}

int main(int argc, char *argv[])
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
