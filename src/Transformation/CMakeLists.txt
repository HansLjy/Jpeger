add_library(Transformation Transformation.cc)
target_include_directories(Transformation PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(Transformation PUBLIC DataStructure)

if(BUILD_TEST)
    enable_testing()
    add_executable(TransformationTest TransformationTest.cc)
    target_link_libraries(TransformationTest PRIVATE Transformation)
    target_link_libraries(TransformationTest PRIVATE GTest::gtest_main)

    include(GoogleTest)
    gtest_discover_tests(TransformationTest)
endif()