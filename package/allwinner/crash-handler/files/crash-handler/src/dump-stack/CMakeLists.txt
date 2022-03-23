cmake_minimum_required(VERSION 2.8.11)

set(DUMP_STACK_BIN "dump-stack")

# Common source code files
set(DUMP_STACK_SRCS dump-stack.c dump-stack-libunw.c)

# Add architecture dependent source files
if (${CMAKE_SYSTEM_PROCESSOR} STREQUAL "aarch64")
  set(DUMP_STACK_SRCS ${DUMP_STACK_SRCS} dump-stack-aarch64.c)
else (${CMAKE_SYSTEM_PROCESSOR} STREQUAL "armv7l")
  set(DUMP_STACK_SRCS ${DUMP_STACK_SRCS} dump-stack-arm.c)
endif()

# Binary
add_executable(${DUMP_STACK_BIN} ${DUMP_STACK_SRCS})

include(FindPkgConfig)
pkg_check_modules(LIBUNWIND_PTRACE REQUIRED libunwind-ptrace)
set_property(TARGET ${DUMP_STACK_BIN} APPEND_STRING PROPERTY COMPILE_FLAGS ${LIBUNWIND_PTRACE_CFLAGS_OTHER})

# Linking
target_link_libraries(${DUMP_STACK_BIN} ${LIBUNWIND_PTRACE_LIBRARIES} ${EBL_LIBRARY} unwind-ptrace dw elf dl stdc++ awbase)
if (${CMAKE_SYSTEM_PROCESSOR} STREQUAL "aarch64")
  target_link_libraries(${DUMP_STACK_BIN} unwind-aarch64)
else (${CMAKE_SYSTEM_PROCESSOR} STREQUAL "armv7l")
  target_link_libraries(${DUMP_STACK_BIN} unwind-arm)
endif()

# Installing
install(TARGETS ${DUMP_STACK_BIN} DESTINATION libexec)

