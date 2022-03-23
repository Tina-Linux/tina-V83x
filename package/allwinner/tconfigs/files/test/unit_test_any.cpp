#include <iostream>
#include <vector>
#include <memory>
#include <gtest/gtest.h>

#include "tconfigs/common/any.h"

TEST(AnyTest, Constructors) {
    int tmp1 = 11;
    tconfigs::common::Any foo1(tmp1);
    tconfigs::common::Any foo2(22);
    const int tmp3 = 33;
    tconfigs::common::Any foo3(tmp3);
    tconfigs::common::Any foo4(foo1);
    foo4 = foo2;
    tconfigs::common::Any foo5("hello");
    std::cout << "foo1: " << foo1.Cast<int>() << std::endl;
    std::cout << "foo2: " << foo2.Cast<int>() << std::endl;
    std::cout << "foo3: " << foo3.Cast<int>() << std::endl;
    std::cout << "foo4: " << foo4.Cast<int>() << std::endl;
    std::cout << "foo5: " << foo5.Cast<const char*>() << std::endl;
}

TEST(AnyTest, Vector) {
    std::vector<tconfigs::common::Any> anys;

    anys.push_back(42);
    std::cout << "int: " << anys.back().Cast<int>() << std::endl;

    anys.push_back("hello");
    std::cout << "const char*: " << anys.back().Cast<const char*>() << std::endl;

    anys.push_back(std::string("world"));
    std::cout << "std::string: " << anys.back().Cast<std::string>() << std::endl;
}

TEST(AnyTest, SharedPtr1) {
    std::vector<std::shared_ptr<tconfigs::common::Any>> anys;

    auto foo1 = std::shared_ptr<tconfigs::common::Any>(new tconfigs::common::Any(42));
    anys.push_back(foo1);
    std::cout << "int: " << anys.back()->Cast<int>() << std::endl;

    auto foo2 = std::shared_ptr<tconfigs::common::Any>(new tconfigs::common::Any("hello"));
    anys.push_back(foo2);
    std::cout << "const char*: " << anys.back()->Cast<const char*>() << std::endl;

    auto foo3 = std::shared_ptr<tconfigs::common::Any>(
            new tconfigs::common::Any(std::string("world")));
    anys.push_back(foo3);
    std::cout << "std::string: " << anys.back()->Cast<std::string>() << std::endl;
}

TEST(AnyTest, SharedPtr2) {
    std::vector<tconfigs::common::Any> anys;

    auto foo1 = std::shared_ptr<int>(new int(42));
    anys.push_back(foo1);
    std::cout << "int: " << *(anys.back().Cast<std::shared_ptr<int>>()) << std::endl;

    auto foo2 = std::shared_ptr<const char*>(new const char*("hello"));
    anys.push_back(foo2);
    std::cout << "const char*: " <<
        *(anys.back().Cast<std::shared_ptr<const char*>>()) << std::endl;

    auto foo3 = std::shared_ptr<std::string>(new std::string("world"));
    anys.push_back(foo3);
    std::cout << "std::string: " <<
        *(anys.back().Cast<std::shared_ptr<std::string>>()) << std::endl;
}

int main(int argc, char *argv[])
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
