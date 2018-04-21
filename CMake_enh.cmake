
option(OPTION_BUILD_DOCS OFF)
option(OPTION_BUILD_EXAMPLES OFF)
option(OPTION_BUILD_GPU_TESTS OFF)
option(OPTION_BUILD_TESTS OFF)
option(OPTION_BUILD_TOOLS OFF)

# this should be removed as soon as glbinding is updated.
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_SILENCE_CXX17_ADAPTOR_TYPEDEFS_DEPRECATION_WARNING")

add_subdirectory(${PROJECT_SOURCE_DIR}/extern/fwenh/extern/glbinding)

set(VISCOM_DO_PROFILING ON CACHE BOOL "Turn on profiling.")

file(GLOB_RECURSE SHADER_FILES_ENH ${PROJECT_SOURCE_DIR}/extern/fwenh/resources/shader/*.*)
file(GLOB_RECURSE SRC_FILES_ENH
    ${PROJECT_SOURCE_DIR}/extern/fwenh/src/enh/*.h
    ${PROJECT_SOURCE_DIR}/extern/fwenh/src/enh/*.cpp
    ${PROJECT_SOURCE_DIR}/extern/fwenh/src/enh/*.inl)

foreach(f ${SRC_FILES_ENH})
    file(RELATIVE_PATH SRCGR ${PROJECT_SOURCE_DIR}/extern/fwenh ${f})
    string(REGEX REPLACE "(.*)(/[^/]*)$" "\\1" SRCGR ${SRCGR})
    string(REPLACE / \\ SRCGR ${SRCGR})
    source_group("${SRCGR}" FILES ${f})
endforeach()

foreach(f ${SHADER_FILES_ENH})
    file(RELATIVE_PATH SRCGR ${PROJECT_SOURCE_DIR}/extern/fwenh/resources ${f})
    string(REGEX REPLACE "(shader.*)(/[^/]*\\.[^/]+)$" "\\1" SRCGR ${SRCGR})
    string(LENGTH ${SRCGR} SRCGRLEN)
    if (${SRCGRLEN} GREATER 6)
        string(REPLACE / \\ SRCGR ${SRCGR})
        string(SUBSTRING ${SRCGR} 7 -1 SRCGR)
        source_group("shader\\enhanced\\${SRCGR}" FILES ${f})
    else()
        source_group("shader\\enhanced" FILES ${f})
    endif()
endforeach()

list(APPEND ENH_INCLUDE_DIRS
    extern/fwenh/src
    extern/fwenh/extern/glbinding/source/glbinding/include
    ${CMAKE_CURRENT_BINARY_DIR}/extern/fwenh/extern/glbinding/source/include
    ${CMAKE_CURRENT_BINARY_DIR}/extern/fwenh/extern/glbinding/source/glbinding/include
    extern/fwenh/extern/glbinding/source/glbinding-aux/include
    ${CMAKE_CURRENT_BINARY_DIR}/extern/fwenh/extern/glbinding/source/glbinding-aux/include
    extern/fwenh/extern/cereal/include)

list(APPEND ENH_LIBS glbinding glbinding-aux)
list(APPEND COMPILE_TIME_DEFS $<$<CONFIG:DebugOpenGLCalls>:VISCOM_OGL_DEBUG_MSGS> GLFW_INCLUDE_NONE _SILENCE_CXX17_ADAPTOR_TYPEDEFS_DEPRECATION_WARNING)

if (VISCOM_DO_PROFILING)
    list(APPEND COMPILE_TIME_DEFS ENABLE_PROFILING)
endif()
