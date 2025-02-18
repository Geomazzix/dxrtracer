cmake_minimum_required(VERSION 3.27)
include_guard()
    
MESSAGE(STATUS "[Thirdparty] gtest")
MESSAGE(STATUS "[Thirdparty] gmock")

FetchContent_Declare(
	googletest
	GIT_REPOSITORY https://github.com/google/googletest.git
	GIT_TAG "e9fb5c7bacc4a25b030569c92ff9f6925288f1c3"
)
FetchContent_MakeAvailable(googletest)


mark_as_advanced(
    BUILD_GMOCK BUILD_GTEST BUILD_SHARED_LIBS
    gmock_build_tests gtest_build_samples gtest_build_tests
    gtest_disable_pthreads gtest_force_shared_crt gtest_hide_internal_symbols
)

set_target_properties(gtest PROPERTIES FOLDER "${THIRD_PARTY_FOLDER}/gtest")
set_target_properties(gtest_main PROPERTIES FOLDER "${THIRD_PARTY_FOLDER}/gtest")
set_target_properties(gmock PROPERTIES FOLDER "${THIRD_PARTY_FOLDER}/gtest")
set_target_properties(gmock_main PROPERTIES FOLDER "${THIRD_PARTY_FOLDER}/gtest")