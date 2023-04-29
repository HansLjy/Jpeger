file(GLOB QUANTIZATION_SRC *.cc)
add_library(Quantization ${QUANTIZATION_SRC})
target_include_directories(Quantization PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(Quantization PUBLIC DataStructure)