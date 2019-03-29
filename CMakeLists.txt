cmake_minimum_required(VERSION 3.4)

if(NOT WIN32)
	message(FATAL_ERROR "This project only supports Windows")
	return()
endif()

project(sautracks CXX)
add_executable(sautracks WIN32 src/UserTracks.cc src/Main.cc sautracks.rc)

if(MSVC)
	target_compile_definitions(sautracks PRIVATE _CRT_SECURE_NO_WARNINGS _CRT_SECURE_NO_DEPRECATE)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4996")
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /MANIFEST:NO")
endif()

install(TARGETS sautracks DESTINATION bin)
