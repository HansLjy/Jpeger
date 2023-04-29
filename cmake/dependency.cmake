
include(FetchContent)
message("-- Fetching CPM")
FetchContent_Declare(
    get_cpm
    URL https://github.com/cpm-cmake/CPM.cmake/releases/latest/download/get_cpm.cmake
    DOWNLOAD_NO_EXTRACT ON
)
FetchContent_MakeAvailable(
    get_cpm
)

FetchContent_GetProperties(
	get_cpm
	SOURCE_DIR get_cpm_DIR
)

message("-- Done fetching CPM")

include(${get_cpm_DIR}/get_cpm.cmake)

# spdlog

find_package(spdlog 1.10.0 EXACT)

if (NOT spdlog_FOUND)
	message("-- spdlog not found, download instead")
	CPMAddPackage(
		NAME spdlog
		GIT_REPOSITORY https://github.com/gabime/spdlog.git
		GIT_TAG 76fb40d95455f249bd70824ecfcae7a8f0930fa3
	)
	message("-- Finish downloading spdlog")
endif()

# json

find_package(jsoncpp 1.9.5 EXACT)

if (NOT jsoncpp_FOUND)
	message("-- JsonCpp not found, download instead")
	CPMAddPackage(
		NAME jsoncpp
		GIT_REPOSITORY https://github.com/open-source-parsers/jsoncpp.git
		GIT_PROGRESS TRUE
		GIT_SHALLOW TRUE
		GIT_TAG v1.9.5
	)
	message("-- Finish downloading JsonCpp")
endif()

find_package(Eigen3 3.4.0 EXACT)

if(NOT Eigen3_FOUND)
	message("-- Eigen3 not found, download instead")
	CPMAddPackage(
		NAME eigen
		GIT_REPOSITORY https://gitlab.com/libeigen/eigen.git
        GIT_TAG 3147391d946bb4b6c68edd901f2add6ac1f31f8c
	)
	message("-- Finish downloading Eigen3")
endif()

find_package(CLI11 2.3.2 EXACT)

if (NOT CLI11_FOUND)
	message("-- cli11 not found, download instead")
	CPMAddPackage(
		NAME cli11
		GIT_REPOSITORY https://github.com/CLIUtils/CLI11
		GIT_TAG        v2.3.2
	)
	message("-- Finish downloading cli11")
endif()