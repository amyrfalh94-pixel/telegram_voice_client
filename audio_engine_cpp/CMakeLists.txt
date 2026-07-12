cmake_minimum_required(VERSION 3.18)
project(audio_engine_cpp CXX)
set(CMAKE_CXX_STANDARD 17)
include(FetchContent)
# تثبيت Oboe 1.7.0 (ثابت ومتوافق مع NDK r25b)
FetchContent_Declare(oboe GIT_REPOSITORY https://github.com/google/oboe.git GIT_TAG 1.7.0)
FetchContent_MakeAvailable(oboe)
# بناء المكتبة المشتركة (تشمل Oboe بشكل ثابت)
add_library(audio_engine_cpp SHARED
    audio_engine.cpp
    ring_buffer.cpp
    jni_bridge.cpp
    tgvoip_adapter.cpp
)
target_include_directories(audio_engine_cpp PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(audio_engine_cpp PRIVATE oboe android log)
