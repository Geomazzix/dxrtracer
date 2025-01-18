cmake_minimum_required(VERSION 3.27)

include_guard()
MESSAGE(STATUS "[Thirdparty] stb")

FetchContent_Declare(
	stb_external
	GIT_REPOSITORY https://github.com/Geomazzix/stb.git
	GIT_TAG "8796ff4b522556f8ec86ee409734ee995ab55034"
	GIT_PROGRESS TRUE
)

FetchContent_MakeAvailable(stb_external)

#a bit of an odd way of doing things, but the stb repository is bloated with the range of utility it provides, I just want the image utilities.
set(stb_headers
    "${stb_external_SOURCE_DIR}/stbImageLoad.h"
    "${stb_external_SOURCE_DIR}/stbImageWrite.h"
    "${stb_external_SOURCE_DIR}/stbImageResize.h"
)

target_set_ide_folders(
	NAME "stb"
	FILTER "thirdParty"
	HEADERS ${stb_headers}
)