include(FetchContent)

FetchContent_Declare(
    BmpReader
    GIT_REPOSITORY https://github.com/marc-q/libbmp.git
    GIT_TAG        66bec6d7daf254e6dc07d55c9383fd68276a6a39
)

FetchContent_MakeAvailable(BmpReader)

FetchContent_GetProperties(
    BmpReader
    SOURCE_DIR BMPREADER_SRC
)

add_library(IO ImageIO.cc ${BMPREADER_SRC}/CPP/libbmp.cpp)
target_include_directories(IO PRIVATE ${BMPREADER_SRC}/CPP)
target_include_directories(IO PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(IO PUBLIC DataStructure)