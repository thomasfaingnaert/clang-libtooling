cmake_minimum_required(VERSION 3.18.4)

project(LoopConvert)

find_package(Clang REQUIRED CONFIG)

add_executable(LoopConvert src/LoopConvert.cpp)

target_include_directories(LoopConvert PRIVATE ${CLANG_INCLUDE_DIRS})
target_link_libraries(LoopConvert PRIVATE clangTooling)

if (NOT ${LLVM_ENABLE_RTTI})
    target_compile_options(LoopConvert PRIVATE -fno-rtti)
endif()
