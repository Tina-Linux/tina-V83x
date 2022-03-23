cmake_minimum_required(VERSION 2.8.11)

set(CMAKE_C_FLAGS_DEFAULT ${CMAKE_C_FLAGS})
set(CMAKE_C_FLAGS_CUSTOM "-g -O0 -fno-omit-frame-pointer -funwind-tables")

set(TEST_DUMP_STACK_BIN "test-dump-stack")

# Common source code files
set(TEST_DUMP_STACK_SRCS test-dump-stack.c )


#set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++")

# Binary
add_executable(${TEST_DUMP_STACK_BIN} ${TEST_DUMP_STACK_SRCS})


# Linking
target_link_libraries(${TEST_DUMP_STACK_BIN}  dl stdc++)

# Installing
install(TARGETS ${TEST_DUMP_STACK_BIN} DESTINATION libexec)

