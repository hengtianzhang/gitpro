# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)

project(lib${librtochius_VAR}librtochius VERSION 0.0.1)

add_library(${librtochius_VAR}librtochius STATIC EXCLUDE_FROM_ALL)

add_subdirectory(src)
add_subdirectory(arch/${ARCH}/src)

target_include_directories(${librtochius_VAR}librtochius PUBLIC
	include
	arch/${ARCH}/include
	../include/uapi
	../arch/${ARCH}/include/uapi
)

find_package(base REQUIRED)
base_user_import_libraries()

target_link_libraries(${librtochius_VAR}librtochius
	PUBLIC
	${librtochius_VAR}base
)
