cmake_minimum_required(VERSION 3.12)
project(avakar_corecursive)

add_library(avakar_corecursive INTERFACE
#    include/avakar/corecursive.h
    )
add_library(avakar::corecursive ALIAS avakar_corecursive)

target_include_directories(avakar_corecursive INTERFACE include)
target_compile_features(avakar_corecursive INTERFACE cxx_std_20)

if(MSVC)
    target_compile_options(avakar_corecursive INTERFACE /await)
else()
    target_compile_options(avakar_corecursive INTERFACE -stdlib=libc++)
    target_link_options(avakar_corecursive INTERFACE -stdlib=libc++)
endif()

include(CTest)
if (BUILD_TESTING)
	include(FetchContent)

	FetchContent_Declare(
		Catch2
		GIT_REPOSITORY https://github.com/catchorg/Catch2.git
		GIT_TAG 255aa5f2afe1a622c97422f65ace6ca915be0d8d
		GIT_SHALLOW YES
		)

	FetchContent_GetProperties(Catch2)
	if (NOT catch2_POPULATED)
		FetchContent_Populate(Catch2)
		add_subdirectory("${catch2_SOURCE_DIR}" "${catch2_BINARY_DIR}")
	endif()

	include(${catch2_SOURCE_DIR}/contrib/Catch.cmake)

	add_executable(avakar_corecursive_test
		test/main.cpp
		test/test.cpp
		)
	target_link_libraries(avakar_corecursive_test PUBLIC avakar::corecursive Catch2::Catch2)

	catch_discover_tests(avakar_corecursive_test)
endif()
