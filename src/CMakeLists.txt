add_subdirectory(Jpeger)
add_subdirectory(DataStructure)
add_subdirectory(LosslessCompress)
add_subdirectory(Transformation)
add_subdirectory(Quantization)
add_subdirectory(IO)

add_executable(jpeger main.cc)
add_executable(benchmark Benchmark.cc)
target_link_libraries(jpeger PRIVATE Jpeger)
target_link_libraries(jpeger PRIVATE IO)
target_link_libraries(benchmark PRIVATE Jpeger)
target_link_libraries(benchmark PRIVATE IO)

