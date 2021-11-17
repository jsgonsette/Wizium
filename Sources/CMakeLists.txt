cmake_minimum_required(VERSION 3.0)

#set (CMAKE_BUILD_TYPE Release)
set (BUILD_SHARED_LIBS TRUE)
set (CMAKE_INSTALL_PREFIX .)

project(Wizium)

include_directories (.)

add_library (libWizium
	SHARED
	library.cpp
	library.h
	library.Module.cpp
	library.Module.h
	libWizium.cpp
	libWizium.h
	Dictionary/Dictionary.cpp
	Dictionary/Dictionary.h
	Grid/Box.cpp
	Grid/Box.h
	Grid/Grid.cpp
	Grid/Grid.h
	Solvers/SolverDynamic.cpp
	Solvers/SolverDynamic.h
	Solvers/SolverDynamic.DynamicItem.cpp
	Solvers/SolverDynamic.DynamicItem.h
	Solvers/SolverStatic.cpp
	Solvers/SolverStatic.h
	Solvers/SolverStatic.StaticItem.cpp
	Solvers/SolverStatic.StaticItem.h
	Solvers/ISolver.h
	)

set_property(TARGET libWizium PROPERTY CXX_STANDARD 17)


if (APPLE)
	message (STATUS "Darwin configuration")
	target_compile_options(libWizium PRIVATE -Wall -Wextra)
	set (PATH ./../Binaries/Darwin/)

elseif (UNIX)
	message (STATUS "Unix configuration")
	target_compile_options(libWizium PRIVATE -Wall -Wextra)
	set (PATH ./../Binaries/Linux/)
	set_target_properties(libWizium PROPERTIES PREFIX "")

elseif (MSVC)
	message (STATUS "Microsoft Visual Studio configuration")
	target_compile_options(libWizium PRIVATE /W3 /O2 /Oi /GL /Gy)
	target_compile_definitions(libWizium PRIVATE LIBWIZIUM_EXPORTS)
	if (${CMAKE_GENERATOR_PLATFORM} STREQUAL x64)
		set (PATH ./../Binaries/Windows/x64/)
		target_link_options(libWizium PRIVATE /MACHINE:X64)
	else ()
		set (PATH ./../Binaries/Windows/x86/)
	endif ()

else ()
	message (STATUS "Default configuration")
endif ()

install (TARGETS libWizium
	DESTINATION ${PATH})

