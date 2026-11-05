cmake_minimum_required(VERSION 3.22)
project(PocketCrankVoice246 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
include(CTest)
find_package(Python3 3.11 COMPONENTS Interpreter REQUIRED)
add_executable(crank246_voice_reference_tests ../../Tests/Crank246VoiceReferenceTests.cpp)
target_include_directories(crank246_voice_reference_tests PRIVATE ../../Source)
add_test(NAME crank246_voice_reference COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/../../Tests/RunCrank246VoiceTests.py" "$<TARGET_FILE:crank246_voice_reference_tests>" "${CMAKE_CURRENT_SOURCE_DIR}/../../Tests/ReferenceVectors")


add_executable(crank246_voice_integration_tests ../../Tests/Crank246VoiceIntegrationTests.cpp)
target_include_directories(crank246_voice_integration_tests PRIVATE ../../Source)
add_test(NAME crank246_voice_integration COMMAND crank246_voice_integration_tests)
