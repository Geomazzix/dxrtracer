cmake_minimum_required(VERSION 3.27)

set(SOURCE
	"vath/matrix2x2_testSuite.cpp"
	"vath/matrix3x3_testSuite.cpp"
	"vath/matrix4x4_testSuite.cpp"
	"vath/vector4_testSuite.cpp"
	"vath/vector3_testSuite.cpp"
	"vath/vector2_testSuite.cpp"
	"vath/quaternion_testSuite.cpp"

	"unit_test_suite.cpp"
)

set(DEPS
	"gtest"
	"gmock"
	"core"
)

project_add_target(
	NAME UnitTestSuite
	VERSION ${TEST_SUITE_VERSION}
	TYPE EXECUTABLE
	HEADERS ""
	SOURCE ${SOURCE}
	LINK_DEPS ${DEPS}
)