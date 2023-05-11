add_library(DataStructure Color.cc)
target_include_directories(DataStructure PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(DataStructure PUBLIC Eigen3::Eigen)

get_target_property(JSON_INC_PATH jsoncpp_lib INTERFACE_INCLUDE_DIRECTORIES)
include_directories(${JSON_INC_PATH})
target_link_libraries(DataStructure PUBLIC jsoncpp_lib)

if(BUILD_TEST)
    enable_testing()
    add_executable(ColorTest ColorTest.cc)
    target_link_libraries(ColorTest PRIVATE Jpeger)
    target_link_libraries(ColorTest PRIVATE GTest::gtest_main)

    include(GoogleTest)
    gtest_discover_tests(ColorTest)
endif()