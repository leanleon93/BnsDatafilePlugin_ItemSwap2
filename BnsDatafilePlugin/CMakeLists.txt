# SET YOUR PLUGIN NAME AND VERSION HERE
set(PLUGIN_NAME "Template")
project ("${PLUGIN_NAME}Plugin" VERSION 1.0.0)

configure_file(pluginversion.h.in pluginversion.h @ONLY)
configure_file(${CMAKE_SOURCE_DIR}/version.rc.in version.rc @ONLY)

add_library(${PROJECT_NAME} SHARED "BnsDatafilePlugin.cpp" "UiPanel.cpp" "plugin_config.cpp" "datafile_detours.cpp")
# Trim whitespace
string(STRIP "${PLUGIN_NAME}" PLUGIN_NAME)

# Replace spaces with underscores
string(REPLACE " " "_" PLUGIN_NAME "${PLUGIN_NAME}")

# Remove special characters
string(REGEX REPLACE "[^a-zA-Z0-9_]" "" PLUGIN_NAME "${PLUGIN_NAME}")

# Convert to lowercase
string(TOLOWER "${PLUGIN_NAME}" PLUGIN_NAME)

target_sources(${PROJECT_NAME} PRIVATE ${PROJECT_BINARY_DIR}/version.rc)
set(CLIENT_SUFFIX "")
set(REGION_SUFFIX "")

if(BNSKR)
    set(REGION_SUFFIX "_KR")
endif()
if(BNSLIVE)
    set(CLIENT_SUFFIX "_LIVE")
else()
    set(CLIENT_SUFFIX "_NEO")
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME ${PLUGIN_NAME}${CLIENT_SUFFIX}${REGION_SUFFIX})
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${GLOBAL_DLL_OUTPUT_DIR}"
)

if(BNSLIVE)
    set(PLUGIN_TABLES_DIR "${CMAKE_SOURCE_DIR}/external/BnsPluginTables/LIVE/Generated/include")
else()
    set(PLUGIN_TABLES_DIR "${CMAKE_SOURCE_DIR}/external/BnsPluginTables/NEO/Generated/include")
endif()

target_include_directories(${PROJECT_NAME} PRIVATE
    ${PLUGIN_TABLES_DIR}
    ${CMAKE_SOURCE_DIR}/external/PluginSdk/BnsPlugin
    ${CMAKE_SOURCE_DIR}/external/include
    ${PROJECT_BINARY_DIR})

target_link_libraries(${PROJECT_NAME} PRIVATE glaze::glaze)

if(MSVC)
    # basic compile options
    target_compile_options(${PROJECT_NAME} PRIVATE /Zc:preprocessor /MP)
    target_compile_options(${PROJECT_NAME} PRIVATE /W3 /permissive- /sdl)
    target_compile_options(${PROJECT_NAME} PRIVATE $<$<CONFIG:Debug>:/Zi> $<$<CONFIG:Release>:/O2>)
    add_compile_options("/utf-8")
    # Match __RuntimeTypeInfo false from the vcxproj (disables RTTI)
    target_compile_options(${PROJECT_NAME} PRIVATE /GR-)

    # IMPORTANT: make Debug use the same CRT as Release/host to avoid ABI mismatch.
    # If host expects /MD (MultiThreadedDLL), use that. If host uses /MDd, change accordingly.
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL" CACHE STRING "" FORCE)

    # Disable iterator / secure CRT checks in Debug to match Release ABI (only if you need byte-for-byte ABI parity).
    # This makes Debug STL layout compatible with Release builds of the host.
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Debug>:_ITERATOR_DEBUG_LEVEL=0>
        $<$<CONFIG:Debug>:_SECURE_SCL=0>
    )
endif()