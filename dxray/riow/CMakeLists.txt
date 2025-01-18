cmake_minimum_required(VERSION 3.23)

set(HEADERS
	"${CMAKE_CURRENT_SOURCE_DIR}/include/riow/riow.h"
)

set(SOURCE
	"${CMAKE_CURRENT_SOURCE_DIR}/src/riow.cpp"
)

set(DEPS
	"core"
)

project_add_target(
	NAME riow
	TYPE EXECUTABLE
	HEADERS ${HEADERS}
	SOURCE ${SOURCE}
	LINK_DEPS ${DEPS}
)