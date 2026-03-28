cmake_minimum_required (VERSION 3.10)

if(NOT WIN32)
    message(FATAL_ERROR "This project only supports Windows.")
endif()

project("BnsDatafilePluginMonorepo")

string(TIMESTAMP CURRENT_YEAR "%Y")
set(COMPANY_NAME "")
set(COPYRIGHT "© ${CURRENT_YEAR} ${COMPANY_NAME}")

option(ENABLE_PERSISTENCE "Enable persistence for the plugin configuration" OFF)
option(BNSKR, "Build for BNSKR version" OFF)
option(BNSLIVE, "Build for BNSLIVE version" OFF)

# Output build variables for debugging
message(STATUS "CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")
message(STATUS "ENABLE_PERSISTENCE: ${ENABLE_PERSISTENCE}")
message(STATUS "BNSKR: ${BNSKR}")
message(STATUS "BNSLIVE: ${BNSLIVE}")

if(ENABLE_PERSISTENCE)
    add_compile_definitions(PLUGIN_ENABLE_PERSISTENCE)
else()
    add_compile_definitions(PLUGIN_DISABLE_PERSISTENCE)
endif()
if(BNSKR)
    add_compile_definitions(BNSKR)
endif()

# Ensure Git is available
find_package(Git REQUIRED)

set(BNSPLUGIN_TABLES_FILE_NEO "${PROJECT_SOURCE_DIR}/external/BnsPluginTables/NEO/Generated/include/DrEl.h")
set(BNSPLUGIN_TABLES_FILE_LIVE "${PROJECT_SOURCE_DIR}/external/BnsPluginTables/LIVE/Generated/include/DrEl.h")
set(PLUGINSKD_FILE "${PROJECT_SOURCE_DIR}/external/PluginSdk/BnsPlugin/DrEl.h")

if(NOT EXISTS "${BNSPLUGIN_TABLES_FILE_NEO}" OR NOT EXISTS "${BNSPLUGIN_TABLES_FILE_LIVE}" OR NOT EXISTS "${PLUGINSKD_FILE}")
    message(STATUS "Required submodule files missing. Fetching submodules...")
    # fetch the latest submodules
    if(GIT_FOUND AND EXISTS "${PROJECT_SOURCE_DIR}/.git")
        message(STATUS "Fetching the latest submodules")
        execute_process(
            COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive --remote
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            RESULT_VARIABLE GIT_SUBMOD_RESULT
            OUTPUT_VARIABLE GIT_SUBMOD_OUTPUT
            ERROR_VARIABLE GIT_SUBMOD_ERROR
        )
        if(NOT GIT_SUBMOD_RESULT EQUAL "0")
            message(STATUS "Git Submodule Output: ${GIT_SUBMOD_OUTPUT}")
            message(STATUS "Git Submodule Error: ${GIT_SUBMOD_ERROR}")
            message(FATAL_ERROR "Failed to fetch the latest submodules. Error code: ${GIT_SUBMOD_RESULT}")
        endif()
    endif()
endif()

# Verify submodules are present
if(NOT EXISTS "${PROJECT_SOURCE_DIR}/external/BnsPluginTables/NEO/Generated/include/DrEl.h")
    message(FATAL_ERROR "The BnsPluginTables submodule was not downloaded or is missing required files!")
endif()
if(NOT EXISTS "${PROJECT_SOURCE_DIR}/external/BnsPluginTables/LIVE/Generated/include/DrEl.h")
    message(FATAL_ERROR "The BnsPluginTables submodule was not downloaded or is missing required files!")
endif()
if(NOT EXISTS "${PROJECT_SOURCE_DIR}/external/PluginSdk/BnsPlugin/DrEl.h")
    message(FATAL_ERROR "The PluginSdk submodule was not downloaded or is missing required files!")
endif()

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|AMD64")
    set(glaze_ENABLE_AVX2 ON)
endif()

set(FETCHCONTENT_BASE_DIR "${CMAKE_SOURCE_DIR}/.dependencies")
include(FetchContent)
FetchContent_Declare(
  glaze
  GIT_REPOSITORY https://github.com/stephenberry/glaze.git
  GIT_TAG main
  GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(glaze)

set(GLOBAL_DLL_OUTPUT_DIR "${CMAKE_BINARY_DIR}/bin")
# Include sub-projects.
add_subdirectory ("BnsDatafilePlugin")