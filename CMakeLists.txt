cmake_minimum_required (VERSION 2.8)

INCLUDE(CMakeForceCompiler)

# this one is important
SET(CMAKE_SYSTEM_NAME Generic)
SET(CMAKE_FIND_ROOT_PATH /home/benoitgirard/mozilla/tools/android/ndk/android-ndk-r4c/build/platforms/android-5/arch-arm)
# search for programs in the build host directories
SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
# for libraries and headers in the target directories
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
SET(CMAKE_C_FLAGS "$CFLAGS -I$SYSROOT/usr/include -D__linux__ -mandroid -msoft-float -fno-short-enums -march=armv5te -mthumb-interwork -fno-strict-aliasing -march=armv7-a -mthumb -Wa, -march=armv7-a -Wa, -mthumb -mfpu=neon")

SET(OPENGL_INCLUDE_DIR "/home/benoitgirard/mozilla/tools/android/ndk/android-ndk-r4c/build/platforms/android-8/arch-x86/usr/include/GLES")
SET(OPENGL_gl_LIBRARY "/home/benoitgirard/mozilla/tools/android/ndk/android-ndk-r4c/build/platforms/android-8/arch-arm/usr/lib/libGLESv2.so")
SET(OPENGL_glu_LIBRARY "/home/benoitgirard/libEGL.so")

project (apitrace)

set (CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

# Set default built type
if (NOT CMAKE_BUILD_TYPE)
    set (CMAKE_BUILD_TYPE Debug
        CACHE
        STRING "Choose the build type, options are: None, Debug, Release, RelWithDebInfo, or MinSizeRel."
        FORCE)
endif (NOT CMAKE_BUILD_TYPE)

set (CMAKE_USE_PYTHON_VERSION 2.6)

find_package (PythonInterp REQUIRED)
find_package (OpenGL REQUIRED)
find_package (Qt4 COMPONENTS QtCore QtGui QtWebKit)
find_package (QJSON)

if (NOT WIN32)
    # Always use the bundled zlib and libpng sources on Windows to make it easy
    # to deploy the wrappers DLLs
    find_package (ZLIB)
    find_package (PNG)
    find_package (X11 REQUIRED)
else (NOT WIN32)
    find_package (DirectX)
endif (NOT WIN32)

if (WIN32)
    # MSVC & MinGW only define & use APIENTRY
    add_definitions (-DGLAPIENTRY=__stdcall)
endif (WIN32)

if (MSVC)
    # C99 includes for msvc
    include_directories (msvc)

    # Enable math constants defines
    add_definitions (-D_USE_MATH_DEFINES)

    # Adjust warnings
    add_definitions (-D_CRT_SECURE_NO_DEPRECATE -D_CRT_SECURE_NO_WARNINGS -D_CRT_NONSTDC_NO_WARNINGS)
    add_definitions (-D_SCL_SECURE_NO_DEPRECATE -D_SCL_SECURE_NO_WARNINGS)
    add_definitions (-W4)
    add_definitions (-wd4244) # conversion from 'type1' to 'type2', possible loss of data
    add_definitions (-wd4505) # unreferenced local function has been removed
    add_definitions (-wd4800) # forcing value to bool 'true' or 'false' (performance warning)
    # XXX: it's safer to use ssize_t everywhere instead of disabling warning
    add_definitions (-wd4018) # signed/unsigned mismatch
    
    # Use static runtime
    # http://www.cmake.org/Wiki/CMake_FAQ#How_can_I_build_my_MSVC_application_with_a_static_runtime.3F
    foreach (flag_var
        CMAKE_C_FLAGS CMAKE_C_FLAGS_DEBUG CMAKE_C_FLAGS_RELEASE CMAKE_C_FLAGS_MINSIZEREL CMAKE_C_FLAGS_RELWITHDEBINFO
        CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO
    )
        if (${flag_var} MATCHES "/MD")
            string (REGEX REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
        endif (${flag_var} MATCHES "/MD")
    endforeach (flag_var)
else ()
    # Adjust warnings
    add_definitions (-Wall)
    # XXX: it's safer to use ssize_t everywhere instead of disabling warning
    add_definitions (-Wno-sign-compare) # comparison between signed and unsigned integer expressions
endif ()

# Use bundled ZLIB if system one can't be found
if (ZLIB_FOUND)
    include_directories (${ZLIB_INCLUDE_DIRS})
    link_libraries (${ZLIB_LIBRARIES})
else (ZLIB_FOUND)
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
endif (ZLIB_FOUND)

# Use bundled LIBPNG if system one can't be found
if (PNG_FOUND)
    include_directories (${PNG_INCLUDE_DIR})
    add_definitions (${PNG_DEFINITIONS})
    link_libraries (${PNG_LIBRARIES})
else (PNG_FOUND)
    add_library (png STATIC
        libpng/png.c
        libpng/pngerror.c
        libpng/pngget.c
        libpng/pngmem.c
        libpng/pngpread.c
        libpng/pngread.c
        libpng/pngrio.c
        libpng/pngrtran.c
        libpng/pngrutil.c
        libpng/pngset.c
        libpng/pngtrans.c
        libpng/pngwio.c
        libpng/pngwrite.c
        libpng/pngwtran.c
        libpng/pngwutil.c
    )
    include_directories (libpng)
    link_libraries (png)
endif (PNG_FOUND)

include_directories (${CMAKE_CURRENT_SOURCE_DIR})

add_custom_command (
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/glproc.hpp
    COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/glproc.py > ${CMAKE_CURRENT_BINARY_DIR}/glproc.hpp
    DEPENDS glproc.py dispatch.py wglapi.py glxapi.py glapi.py glenum.py stdapi.py
)

if (WIN32)
    # Put wrappers in a separate directory
    set (LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/wrappers)

    # d3d8.dll
    #if (DirectX_D3D8_FOUND)
    #    include_directories (${DirectX_D3D8_INCLUDE_DIR})
    #    add_custom_command (
    #        OUTPUT d3d8.cpp
    #        COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/d3d8.py > ${CMAKE_CURRENT_BINARY_DIR}/d3d8.cpp
    #        DEPENDS d3d8.py d3d8types.py d3d8caps.py winapi.py stdapi.py
    #    )
    #    add_library (d3d8 SHARED d3d8.def d3d8.cpp trace_write.cpp os_win32.cpp)
    #    set_target_properties (d3d8 PROPERTIES PREFIX "")
    #endif (DirectX_D3D8_FOUND)

    # d3d9.dll
    if (DirectX_D3DX9_FOUND)
        include_directories (${DirectX_D3DX9_INCLUDE_DIR})
        add_custom_command (
            OUTPUT d3d9.cpp
            COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/d3d9.py > ${CMAKE_CURRENT_BINARY_DIR}/d3d9.cpp
            DEPENDS d3d9.py trace.py d3d9types.py d3d9caps.py d3dshader.py winapi.py stdapi.py
        )
        add_library (d3d9 SHARED d3d9.def d3d9.cpp trace_write.cpp os_win32.cpp)
        set_target_properties (d3d9 PROPERTIES PREFIX "")
    endif (DirectX_D3DX9_FOUND)

    # d3d10.dll
    #if (DirectX_D3D10_FOUND)
    #    include_directories (${DirectX_D3D10_INCLUDE_DIR})
    #    add_custom_command (
    #        OUTPUT d3d10.cpp
    #        COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/d3d10misc.py > ${CMAKE_CURRENT_BINARY_DIR}/d3d10.cpp
    #        DEPENDS d3d10misc.py winapi.py stdapi.py
    #    )
    #    add_library (d3d10 SHARED d3d10.def d3d10.cpp trace_write.cpp os_win32.cpp)
    #    set_target_properties (d3d10 PROPERTIES PREFIX "")
    #endif (DirectX_D3D10_FOUND)

    # opengl32.dll
    add_custom_command (
        OUTPUT wgltrace.cpp
        COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/wgltrace.py > ${CMAKE_CURRENT_BINARY_DIR}/wgltrace.cpp
        DEPENDS wgltrace.py gltrace.py trace.py wglapi.py wglenum.py glapi.py glenum.py winapi.py stdapi.py
    )
    add_library (opengl SHARED opengl32.def wgltrace.cpp trace_write.cpp os_win32.cpp ${CMAKE_CURRENT_BINARY_DIR}/glproc.hpp)
    set_target_properties (opengl PROPERTIES
        PREFIX ""
        OUTPUT_NAME opengl32)
    if (MINGW)
        set_target_properties(opengl PROPERTIES LINK_FLAGS "-Wl,--enable-stdcall-fixup ${CMAKE_CURRENT_SOURCE_DIR}/opengl32.def")
    endif (MINGW)

else ()
    include_directories (${X11_INCLUDE_DIR})

    # libGL.so
    add_custom_command (
        OUTPUT glxtrace.cpp
        COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/glxtrace.py > ${CMAKE_CURRENT_BINARY_DIR}/glxtrace.cpp
        DEPENDS glxtrace.py gltrace.py trace.py glxapi.py glapi.py glenum.py stdapi.py
    )

    add_library (glxtrace SHARED glxtrace.cpp trace_write.cpp os_posix.cpp ${CMAKE_CURRENT_BINARY_DIR}/glproc.hpp)

    set_target_properties (glxtrace PROPERTIES
        PREFIX ""
    )

    target_link_libraries (glxtrace dl)
endif ()

if (WIN32)
    set (os os_win32.cpp)
    set (glws glws_wgl.cpp)
else (WIN32)
    set (os os_posix.cpp)
    # set (glws glws_glx.cpp)
endif (WIN32)

add_library (trace trace_model.cpp trace_parser.cpp ${os})

add_executable (dump dump.cpp)
target_link_libraries (dump trace)

add_custom_command (
    OUTPUT glretrace_gl.cpp
    COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/glretrace.py > ${CMAKE_CURRENT_BINARY_DIR}/glretrace_gl.cpp
    DEPENDS glretrace.py retrace.py codegen.py glapi.py glenum.py stdapi.py
)

add_custom_command (
    OUTPUT glretrace_state.cpp
    COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/glstate.py > ${CMAKE_CURRENT_BINARY_DIR}/glretrace_state.cpp
    DEPENDS glstate.py glapi.py glenum.py stdapi.py
)

include_directories (
    ${CMAKE_CURRENT_BINARY_DIR}
    ${OPENGL_INCLUDE_PATH}
)

add_executable (glretrace
    glretrace_gl.cpp
    glretrace_main.cpp
    glretrace_state.cpp
    retrace.cpp
    ${glws}
    image.cpp 
    ${CMAKE_CURRENT_BINARY_DIR}/glproc.hpp
)

set_property (
    TARGET glretrace
    APPEND
    PROPERTY COMPILE_DEFINITIONS "RETRACE"
)

target_link_libraries (glretrace
    trace
    ${OPENGL_gl_LIBRARY}
    ${OPENGL_glu_LIBRARY}
    ${X11_LIBRARIES}
)

if (NOT WIN32)
    target_link_libraries (glretrace ${X11_LIBRARIES})
endif (NOT WIN32)

if (QT4_FOUND AND QJSON_FOUND)
    add_subdirectory(gui)
endif (QT4_FOUND AND QJSON_FOUND)
