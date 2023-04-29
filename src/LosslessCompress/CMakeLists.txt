file(GLOB LOSSLESS_SRC BitStream.cc)
add_library(LosslessCompression ${LOSSLESS_SRC})
target_include_directories(LosslessCompression PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(LosslessCompression PUBLIC DataStructure)

if (BUILD_TEST)
    enable_testing()
    add_executable(LosslessCompressTest LosslessCompressTest.cc)
    target_link_libraries(LosslessCompressTest PRIVATE LosslessCompression)
    target_link_libraries(LosslessCompressTest PRIVATE GTest::gtest_main)

    include(GoogleTest)
    gtest_discover_tests(LosslessCompressTest)
endif()