set(TEST_NAME injectorpp_test)
set(TEST_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/${CMAKE_BUILD_TYPE})

add_subdirectory(thirdparty/gtest)
include_directories(thirdparty/gtest/googletest)
include_directories(thirdparty/gtest/googletest/include)
include_directories(thirdparty/gtest/googlemock)
include_directories(thirdparty/gtest/googlemock/include)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} TEST_MAIN_SOURCE)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/fixtures TEST_FIXTURE_SOURCE)
add_executable(${TEST_NAME} ${TEST_MAIN_SOURCE} ${TEST_FIXTURE_SOURCE})
target_link_libraries(${TEST_NAME} gmock injectorpp)