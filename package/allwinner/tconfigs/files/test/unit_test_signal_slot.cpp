#include <iostream>
#include <string>
#include <gtest/gtest.h>

#include "tconfigs/common/signal_slot.h"

TEST(SignalSlotTest, Int) {
    auto sig = tconfigs::common::SignalCreate<int, int>("sig");
    auto slot = tconfigs::common::SlotCreate<int, int>("slot",
            [](int a, int b) { std::cout << a << "," << b << std::endl; });

    sig->Connect(slot);

    int foo = 3;
    int bar = 4;
    sig->Emit(foo, bar);
    sig->Emit(5, 6);
}

TEST(SignalSlotTest, String) {
    auto sig = tconfigs::common::SignalCreate<int, const std::string&>("sig");
    auto slot = tconfigs::common::SlotCreate<int, const std::string&>( "slot",
            [](int a, const std::string& b) { std::cout << a << ", " << b << std::endl; });

    sig->Connect(slot);

    int foo = 3;
    std::string bar("bar");
    sig->Emit(foo, "bar");
    sig->Emit(32, "foobar");
}

TEST(SignalSlotTest, Pointer) {
    auto sig = tconfigs::common::SignalCreate<int*, std::string*>("sig");
    auto slot = tconfigs::common::SlotCreate<int*, std::string*>("slot",
            [](int* a, std::string* b) { *a = 4, *b = "answer"; });

    sig->Connect(slot);

    int foo = 3;
    std::string bar("question");
    std::cout << foo << ", " << bar << std::endl;
    sig->Emit(&foo, &bar);
    std::cout << foo << ", " << bar << std::endl;
}

TEST(SignalSlotTest, MultiSlot) {
    auto sig = tconfigs::common::SignalCreate<int, int>("sig");
    auto slot_1 = tconfigs::common::SlotCreate<int, int>("slot_1",
            [](int a, int b) { std::cout << "slot_1: " << a << "," << b << std::endl; });
    auto slot_2 = tconfigs::common::SlotCreate<int, int>("slot_2",
            [](int a, int b) { std::cout << "slot_2: " << a << "," << b << std::endl; });

    std::cout << "Connect slot_1 twice, slot_2 once:" << std::endl;
    sig->Connect(slot_1);
    sig->Connect(slot_1);
    sig->Connect(slot_2);
    sig->Emit(4, 2);
    std::cout << "Disconnect slot_1 by name:" << std::endl;
    sig->Disconnect("slot_1");
    sig->Emit(4, 2);
    std::cout << "Disconnect slot_2 directly: " << std::endl;
    sig->Disconnect(slot_2);
    sig->Emit(4, 2);
}

int main(int argc, char *argv[])
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
