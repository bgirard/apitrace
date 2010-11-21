cmake_minimum_required (VERSION 2.6)

project (apitrace)

set (CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

find_package (PythonInterp REQUIRED)
find_package (OpenGL REQUIRED)
find_package (ZLIB)

find_package (DirectX)

find_package (GLUT)

find_library (GLEW_glew_LIBRARY GLEW
	/usr/lib
)

find_path (GLEW_INCLUDE_DIR GL/glew.h
      /usr/include/GL
)

if (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
	# Nobody likes to include windows.h:
	# - Microsoft's GL/gl.h header depends on windows.h but doesn't include it;
	# - GLEW temporarily defines the necessary defines but undefines them later
	# - certain GLUT distributions don't include it;
	# - most of our programs are meant to be portable so don't include it.
	#
	# We could try to replicate the windows.h definitions required by
	# GL/gl.h, but the build time savings don't compensate the constant
	# headaches that brings, so instead we force windows.h to be included
	# on every file.
	if (MSVC)
		add_definitions (-FIwindows.h)
	else (MSVC)
		add_definitions (--include windows.h)
	endif (MSVC)

	# MSVC & MinGW only define & use APIENTRY
	add_definitions (-DGLAPIENTRY=__stdcall)

	link_libraries (winmm)
endif (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")

if (MSVC)
	# Enable math constants defines
	add_definitions (-D_USE_MATH_DEFINES)

	# Adjust warnings
	add_definitions (-D_CRT_SECURE_NO_DEPRECATE -D_CRT_SECURE_NO_WARNINGS -D_CRT_NONSTDC_NO_WARNINGS)
	add_definitions (-D_SCL_SECURE_NO_DEPRECATE -D_SCL_SECURE_NO_WARNINGS)
	add_definitions (-W4)
	add_definitions (-wd4244) # conversion from 'type1' to 'type2', possible loss of data
	add_definitions (-wd4505) # unreferenced local function has been removed
	add_definitions (-wd4800) # forcing value to bool 'true' or 'false' (performance warning)
else ()
	add_definitions (-Wall)
endif ()

# Use bundled ZLIB if system one can't be found
if (NOT ZLIB_FOUND)
	add_library (zlib STATIC
		zlib/adler32.c
		zlib/compress.c
		zlib/crc32.c
		zlib/gzio.c
		zlib/uncompr.c
		zlib/deflate.c
		zlib/trees.c
		zlib/zutil.c
		zlib/inflate.c
		zlib/infback.c
		zlib/inftrees.c
		zlib/inffast.c
	)

	include_directories (zlib)
	link_libraries (zlib)
else (NOT ZLIB_FOUND)
	include_directories (${ZLIB_INCLUDE_DIRS})
	link_libraries (${ZLIB_LIBRARIES})
endif (NOT ZLIB_FOUND)

include_directories (${CMAKE_CURRENT_SOURCE_DIR})

if (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
	# d3d8.dll
	if (D3D8_FOUND)
		include_directories (${D3D8_INCLUDE_DIR})
		add_custom_command (
			OUTPUT d3d8.cpp
			COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/d3d8.py > ${CMAKE_CURRENT_BINARY_DIR}/d3d8.cpp
			DEPENDS d3d8.py d3d8types.py d3d8caps.py windows.py base.py
		)
		add_library (d3d8 SHARED d3d8.def d3d8.cpp log.cpp os_win32.cpp)
		set_target_properties (d3d8 PROPERTIES PREFIX "")
	endif (D3D8_FOUND)

	# d3d9.dll
	if (D3DX9_FOUND)
		include_directories (${D3DX9_INCLUDE_DIR})
		add_custom_command (
			OUTPUT d3d9.cpp
			COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/d3d9.py > ${CMAKE_CURRENT_BINARY_DIR}/d3d9.cpp
			DEPENDS d3d9.py d3d9types.py d3d9caps.py d3dshader.py windows.py base.py
		)
		add_library (d3d9 SHARED d3d9.def d3d9.cpp log.cpp os_win32.cpp)
		set_target_properties (d3d9 PROPERTIES PREFIX "")
	endif (D3DX9_FOUND)

	# d3d10.dll
	if (D3D10_FOUND)
		include_directories (${D3D10_INCLUDE_DIR})
		add_custom_command (
			OUTPUT d3d10.cpp
			COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/d3d10misc.py > ${CMAKE_CURRENT_BINARY_DIR}/d3d10.cpp
			DEPENDS d3d10misc.py windows.py base.py
		)
		add_library (d3d10 SHARED d3d10.def d3d10.cpp log.cpp os_win32.cpp)
		set_target_properties (d3d10 PROPERTIES PREFIX "")
	endif (D3D10_FOUND)

	# opengl32.dll
	add_custom_command (
		OUTPUT opengl32.cpp
		COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/opengl32.py > ${CMAKE_CURRENT_BINARY_DIR}/opengl32.cpp
		DEPENDS opengl32.py gl.py windows.py base.py
	)
	add_library (opengl32 SHARED opengl32.def opengl32.cpp log.cpp os_win32.cpp)
	set_target_properties (opengl32 PROPERTIES PREFIX "")

else ()

	# libGL.so
	add_custom_command (
		OUTPUT glx.cpp
		COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/glx.py > ${CMAKE_CURRENT_BINARY_DIR}/glx.cpp
		DEPENDS glx.py gl.py dl.py base.py
	)
	add_library (glxtrace SHARED glx.cpp log.cpp os_posix.cpp)
	set_target_properties (glxtrace PROPERTIES PREFIX "")
	target_link_libraries (glxtrace dl)
endif ()

add_executable (dump dump.cpp trace_model.cpp)


if (GLEW_INCLUDE_DIR)
	add_custom_command (
		OUTPUT glretrace.cpp
		COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/glretrace.py > ${CMAKE_CURRENT_BINARY_DIR}/glretrace.cpp
		DEPENDS glretrace.py glx.py gl.py dl.py base.py
	)

	include_directories (
		${OPENGL_INCLUDE_PATH}
		${GLUT_INCLUDE_DIR}
		${GLEW_INCLUDE_DIR}
	)

	add_executable (glretrace glretrace.cpp trace_model.cpp)

	target_link_libraries (glretrace
		${OPENGL_gl_LIBRARY}
		${OPENGL_glu_LIBRARY}
		${GLUT_glut_LIBRARY}
		${GLEW_glew_LIBRARY}
	)
endif (GLEW_INCLUDE_DIR)

