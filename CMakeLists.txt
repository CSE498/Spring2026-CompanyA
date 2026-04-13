cmake_minimum_required(VERSION 3.24)

if(NOT DEFINED CMAKE_C_COMPILER)
    set(CMAKE_C_COMPILER cc)
endif()

if(NOT DEFINED CMAKE_CXX_COMPILER)
    set(CMAKE_CXX_COMPILER c++)
endif()

project(Spring2026CompanyA LANGUAGES C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(COMPANYA_PROFILE "default" CACHE STRING "Build profile used by compatibility Makefiles")
set_property(CACHE COMPANYA_PROFILE PROPERTY STRINGS default debug opt quick grumpy)


# ===========================================
# DIRECTORY VARIABLES
# ===========================================
set(COMPANYA_SOURCE_DIR "${PROJECT_SOURCE_DIR}/source")
set(COMPANYA_TEST_DIR "${PROJECT_SOURCE_DIR}/tests")
set(COMPANYA_DEMOS_DIR "${PROJECT_SOURCE_DIR}/demos")
set(COMPANYA_THIRD_PARTY_DIR "${PROJECT_SOURCE_DIR}/third-party")

# Font directory for GUI only
set(COMPANYA_FONT_DIR
    "${COMPANYA_SOURCE_DIR}/Interfaces/GUI/fonts"
)
# Images directory for GUI tests only
set(COMPANYA_IMAGE_DIR
    "${COMPANYA_TEST_DIR}/Interfaces/GUI/images"
)
set(COMPANYA_ASSETS_DIR
        "${COMPANYA_SOURCE_DIR}/Assets/"
)

# ===========================================
# HELPER FUNCTIONS
# ===========================================
function(companya_read_brew_prefix formula out_var)
    set(${out_var} "" PARENT_SCOPE)
    find_program(COMPANYA_BREW_EXECUTABLE brew)
    if(NOT COMPANYA_BREW_EXECUTABLE)
        return()
    endif()

    execute_process(
        COMMAND "${COMPANYA_BREW_EXECUTABLE}" --prefix "${formula}"
        RESULT_VARIABLE brew_result
        OUTPUT_VARIABLE brew_prefix
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )

    if(brew_result EQUAL 0 AND EXISTS "${brew_prefix}")
        set(${out_var} "${brew_prefix}" PARENT_SCOPE)
    endif()
endfunction()

function(companya_collect_native_sources out_var)
    file(GLOB_RECURSE native_sources CONFIGURE_DEPENDS "${COMPANYA_SOURCE_DIR}/*.cpp")
    list(FILTER native_sources EXCLUDE REGEX "/Interfaces/WebUI/")
    list(REMOVE_ITEM native_sources "${COMPANYA_SOURCE_DIR}/simple_main.cpp")
    list(SORT native_sources)
    set(${out_var} "${native_sources}" PARENT_SCOPE)
endfunction()

function(companya_collect_native_test_sources out_var)
    file(GLOB_RECURSE native_headers CONFIGURE_DEPENDS "${COMPANYA_SOURCE_DIR}/*.hpp")
    list(FILTER native_headers EXCLUDE REGEX "/Interfaces/WebUI/")

    set(native_test_sources "")
    foreach(header IN LISTS native_headers)
        string(REGEX REPLACE "\\.hpp$" ".cpp" source_file "${header}")
        if(EXISTS "${source_file}")
            list(APPEND native_test_sources "${source_file}")
        endif()
    endforeach()

    list(REMOVE_DUPLICATES native_test_sources)
    list(SORT native_test_sources)
    set(${out_var} "${native_test_sources}" PARENT_SCOPE)
endfunction()

function(companya_collect_native_demo_sources out_var)
    file(GLOB demo_mains CONFIGURE_DEPENDS "${COMPANYA_DEMOS_DIR}/*/Group*_main.cpp")
    list(SORT demo_mains)
    set(${out_var} "${demo_mains}" PARENT_SCOPE)
endfunction()

function(companya_apply_native_profile target_name)
    target_compile_options(${target_name} PRIVATE
        -Wall
        -Wextra
        -Wcast-align
        -Wnon-virtual-dtor
        -Woverloaded-virtual
        -pedantic
    )

    if(COMPANYA_PROFILE STREQUAL "debug")
        target_compile_options(${target_name} PRIVATE -g)
    elseif(COMPANYA_PROFILE STREQUAL "opt")
        target_compile_options(${target_name} PRIVATE -O3 -DNDEBUG)
    elseif(COMPANYA_PROFILE STREQUAL "quick")
        target_compile_options(${target_name} PRIVATE -DNDEBUG)
    elseif(COMPANYA_PROFILE STREQUAL "grumpy")
        target_compile_options(${target_name} PRIVATE -Wconversion -Weffc++)
    elseif(NOT COMPANYA_PROFILE STREQUAL "default")
        message(FATAL_ERROR "Unknown COMPANYA_PROFILE='${COMPANYA_PROFILE}'")
    endif()
endfunction()


# ===========================================
# SDL2 DISCOVERY
# ===========================================
set(COMPANYA_LOCAL_SDL_PREFIX "${PROJECT_SOURCE_DIR}/third-party/SDL2")
set(COMPANYA_SDL_INCLUDE_DIR "")
if(EXISTS "${COMPANYA_LOCAL_SDL_PREFIX}/include")
    set(COMPANYA_SDL_INCLUDE_DIR "${COMPANYA_LOCAL_SDL_PREFIX}/include")
else()
    find_path(COMPANYA_SDL_INCLUDE_DIR SDL2/SDL.h)
endif()

set(COMPANYA_SDL_LIBRARY_HINTS "")
list(APPEND COMPANYA_SDL_LIBRARY_HINTS "${COMPANYA_LOCAL_SDL_PREFIX}/lib")

companya_read_brew_prefix("sdl2" COMPANYA_BREW_SDL2_PREFIX)
companya_read_brew_prefix("sdl2_image" COMPANYA_BREW_SDL2_IMAGE_PREFIX)
companya_read_brew_prefix("sdl2_ttf" COMPANYA_BREW_SDL2_TTF_PREFIX)

foreach(prefix IN ITEMS
    "${COMPANYA_BREW_SDL2_PREFIX}"
    "${COMPANYA_BREW_SDL2_IMAGE_PREFIX}"
    "${COMPANYA_BREW_SDL2_TTF_PREFIX}"
)
    if(prefix)
        list(APPEND COMPANYA_SDL_LIBRARY_HINTS "${prefix}/lib")
    endif()
endforeach()

find_library(COMPANYA_SDL2_LIBRARY NAMES SDL2 HINTS ${COMPANYA_SDL_LIBRARY_HINTS})
find_library(COMPANYA_SDL2_IMAGE_LIBRARY NAMES SDL2_image HINTS ${COMPANYA_SDL_LIBRARY_HINTS})
find_library(COMPANYA_SDL2_TTF_LIBRARY NAMES SDL2_ttf HINTS ${COMPANYA_SDL_LIBRARY_HINTS})

if(NOT COMPANYA_SDL_INCLUDE_DIR)
    message(FATAL_ERROR
        "Could not find SDL headers. Run `make install` for the local dependency bundle "
        "or install SDL2/SDL2_image/SDL2_ttf on the host system."
    )
endif()

set(COMPANYA_SDL_LIBRARIES "")
set(COMPANYA_MISSING_SDL_LIBRARIES "")

if(COMPANYA_SDL2_LIBRARY)
    list(APPEND COMPANYA_SDL_LIBRARIES "${COMPANYA_SDL2_LIBRARY}")
else()
    list(APPEND COMPANYA_SDL_LIBRARIES "SDL2")
    list(APPEND COMPANYA_MISSING_SDL_LIBRARIES "SDL2")
endif()

if(COMPANYA_SDL2_IMAGE_LIBRARY)
    list(APPEND COMPANYA_SDL_LIBRARIES "${COMPANYA_SDL2_IMAGE_LIBRARY}")
else()
    list(APPEND COMPANYA_SDL_LIBRARIES "SDL2_image")
    list(APPEND COMPANYA_MISSING_SDL_LIBRARIES "SDL2_image")
endif()

if(COMPANYA_SDL2_TTF_LIBRARY)
    list(APPEND COMPANYA_SDL_LIBRARIES "${COMPANYA_SDL2_TTF_LIBRARY}")
else()
    list(APPEND COMPANYA_SDL_LIBRARIES "SDL2_ttf")
    list(APPEND COMPANYA_MISSING_SDL_LIBRARIES "SDL2_ttf")
endif()

if(COMPANYA_MISSING_SDL_LIBRARIES)
    list(JOIN COMPANYA_MISSING_SDL_LIBRARIES ", " COMPANYA_MISSING_SDL_MESSAGE)
    message(WARNING
        "Could not resolve these SDL libraries during CMake configure: ${COMPANYA_MISSING_SDL_MESSAGE}. "
        "Targets that need them will rely on the host linker path. Run `make install` or install the missing "
        "packages on the host system if linking fails."
    )
endif()

set(COMPANYA_SDL_LIBRARY_DIRS "")
foreach(sdl_library IN LISTS COMPANYA_SDL_LIBRARIES)
    get_filename_component(sdl_library_dir "${sdl_library}" DIRECTORY)
    list(APPEND COMPANYA_SDL_LIBRARY_DIRS "${sdl_library_dir}")
endforeach()
list(REMOVE_DUPLICATES COMPANYA_SDL_LIBRARY_DIRS)

# ===========================================
# CORE LIBRARY
# ===========================================
companya_collect_native_sources(COMPANYA_NATIVE_SOURCES)
companya_collect_native_test_sources(COMPANYA_NATIVE_TEST_SOURCES) 
companya_collect_native_demo_sources(COMPANYA_NATIVE_DEMOS)

add_library(companya_native STATIC ${COMPANYA_NATIVE_SOURCES})
target_compile_features(companya_native PUBLIC cxx_std_23)
target_include_directories(companya_native
    PUBLIC
        "${COMPANYA_SOURCE_DIR}"
    PRIVATE
        "${COMPANYA_SDL_INCLUDE_DIR}"
)
target_compile_definitions(companya_native
    PUBLIC
        DEFAULT_FONT_PATH="${COMPANYA_FONT_DIR}/OpenSans-Regular.ttf"
        TEST_IMAGE_DIR="${COMPANYA_IMAGE_DIR}"
        ASSETS_DIR="${COMPANYA_ASSETS_DIR}"
        DUNGEON_ROOMS_DIR="${COMPANYA_SOURCE_DIR}/Worlds/Dungeon/rooms"
)
companya_apply_native_profile(companya_native)

# ===========================================
# EXECUTABLES
# ===========================================
# `simple` executable
add_executable(simple "${COMPANYA_SOURCE_DIR}/simple_main.cpp")
target_link_libraries(simple PRIVATE companya_native)
set_target_properties(simple PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${COMPANYA_SOURCE_DIR}"
)

# Collect all test files (excluding WebUI's) that should be compiled into `run_tests`
file(GLOB_RECURSE COMPANYA_NATIVE_TEST_FILES CONFIGURE_DEPENDS "${COMPANYA_TEST_DIR}/*.cpp")
list(FILTER COMPANYA_NATIVE_TEST_FILES EXCLUDE REGEX "/Interfaces/WebUI/")
list(SORT COMPANYA_NATIVE_TEST_FILES)

# `run_tests` executable, excludes WebUI
add_executable(run_tests
    ${COMPANYA_NATIVE_TEST_FILES}
    ${COMPANYA_NATIVE_TEST_SOURCES}
    "${COMPANYA_THIRD_PARTY_DIR}/Tinyexpr/tinyexpr.c"
)
target_compile_features(run_tests PRIVATE cxx_std_23)
target_include_directories(run_tests PRIVATE
    "${COMPANYA_SOURCE_DIR}"
    "${COMPANYA_THIRD_PARTY_DIR}/Catch/single_include"
    "${COMPANYA_SDL_INCLUDE_DIR}"
)
target_compile_definitions(run_tests
    PRIVATE
        DEFAULT_FONT_PATH="${COMPANYA_FONT_DIR}/OpenSans-Regular.ttf"
        TEST_IMAGE_DIR="${COMPANYA_IMAGE_DIR}"
        ASSETS_DIR="${COMPANYA_ASSETS_DIR}"
        DUNGEON_ROOMS_DIR="${COMPANYA_SOURCE_DIR}/Worlds/Dungeon/rooms"
)
target_compile_options(run_tests PRIVATE
    $<$<COMPILE_LANGUAGE:CXX>:-O2 -g -Wall -Wextra -Wpedantic -Werror>
    $<$<COMPILE_LANGUAGE:C>:-O2 -g>
)
target_link_libraries(run_tests PRIVATE ${COMPANYA_SDL_LIBRARIES})
set_target_properties(run_tests PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${COMPANYA_TEST_DIR}/build"
    BUILD_RPATH "${COMPANYA_SDL_LIBRARY_DIRS}"
)

# CTest integration (run `ctest`)
enable_testing()
add_test(
    NAME native_tests
    COMMAND run_tests
)
set_tests_properties(native_tests PROPERTIES WORKING_DIRECTORY "${COMPANYA_TEST_DIR}")
if(EXISTS "${COMPANYA_LOCAL_SDL_PREFIX}/lib")
    if(APPLE)
        set_property(
            TEST native_tests
            APPEND PROPERTY ENVIRONMENT_MODIFICATION
            "DYLD_LIBRARY_PATH=path_list_prepend:${COMPANYA_LOCAL_SDL_PREFIX}/lib"
        )
    elseif(UNIX)
        set_property(
            TEST native_tests
            APPEND PROPERTY ENVIRONMENT_MODIFICATION
            "LD_LIBRARY_PATH=path_list_prepend:${COMPANYA_LOCAL_SDL_PREFIX}/lib"
        )
    endif()
endif()


# Create one target PER demo
foreach(demo_main IN LISTS COMPANYA_NATIVE_DEMOS)
    # Strips extension -> "Group18_main"
    get_filename_component(demo_filename "${demo_main}" NAME_WE)
    # Strips "_main" -> "Group18"
    string(REGEX REPLACE "_main$" "" demo_name "${demo_filename}")
    # Lowercase -> "group18"
    string(TOLOWER "${demo_name}" demo_name)

    # Group 18 uses Emscripten, handled separately below
    if(demo_name STREQUAL "group18")
        continue()
    endif()
 
    # Group 17 uses SDL, has two demos
    if(demo_name STREQUAL "group17")
        add_executable(group17_demo "${demo_main}")
        target_link_libraries(group17_demo PRIVATE companya_native ${COMPANYA_SDL_LIBRARIES})
        target_include_directories(group17_demo PRIVATE "${COMPANYA_SDL_INCLUDE_DIR}")
        set_target_properties(group17_demo PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY "${COMPANYA_DEMOS_DIR}"
        )
        

        add_executable(group17_demo2 "${demo_main}")
        target_link_libraries(group17_demo2 PRIVATE companya_native ${COMPANYA_SDL_LIBRARIES})
        target_include_directories(group17_demo2 PRIVATE "${COMPANYA_SDL_INCLUDE_DIR}")
        set_target_properties(group17_demo2 PROPERTIES
                RUNTIME_OUTPUT_DIRECTORY "${COMPANYA_DEMOS_DIR}"
        )

        # Only built when explicitly requested with --target groupXX_demo
        set_target_properties("group17_demo" PROPERTIES EXCLUDE_FROM_ALL TRUE)
        set_target_properties("group17_demo2" PROPERTIES EXCLUDE_FROM_ALL TRUE)
        continue()
    endif()

    add_executable("${demo_name}_demo" "${demo_main}")
    target_link_libraries("${demo_name}_demo" PRIVATE companya_native)
    target_compile_features("${demo_name}_demo" PRIVATE cxx_std_23)
    companya_apply_native_profile("${demo_name}_demo")
    set_target_properties("${demo_name}_demo" PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${COMPANYA_DEMOS_DIR}"
    )
    # Only built when explicitly requested with --target groupXX_demo
    set_target_properties("${demo_name}_demo" PROPERTIES EXCLUDE_FROM_ALL TRUE)
endforeach()



# Emscripten targets
find_program(COMPANYA_EMPP_EXECUTABLE em++)
set(COMPANYA_WEB_DEMO_OUTPUT_DIR "${COMPANYA_DEMOS_DIR}/WebUI")
set(COMPANYA_WEB_SOURCE "${COMPANYA_SOURCE_DIR}/Interfaces/WebUI")
set(COMPANYA_WEB_DEMO_COMMON_ARGS
    --bind
    -std=c++23
    -sWASM=1
    -sASSERTIONS=1
    -sASYNCIFY=1
    -sNO_EXIT_RUNTIME=1
    -sALLOW_MEMORY_GROWTH=1
    -gsource-map
)
set(COMPANYA_WEB_DEMO_INCLUDE_ARGS
    "-I${COMPANYA_SOURCE_DIR}"
)

set(COMPANYA_WEB_TEST_OUTPUT_DIR "${COMPANYA_TEST_DIR}/build/web_tests")
set(COMPANYA_WEB_TEST_INCLUDE_ARGS
    "-I${COMPANYA_SOURCE_DIR}"
    "-I${COMPANYA_THIRD_PARTY_DIR}/Catch/single_include"
)
set(COMPANYA_WEB_TEST_COMMON_ARGS
    --bind
    -std=c++23
    -s
    WASM=1
    -s
    ASSERTIONS=1
    -gsource-map
    -DCATCH_CONFIG_MAIN
)

if(COMPANYA_EMPP_EXECUTABLE)
    add_custom_command(
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
        OUTPUT "${COMPANYA_WEB_DEMO_OUTPUT_DIR}/Group18_main.js"
        COMMAND "${COMPANYA_EMPP_EXECUTABLE}"
            ${COMPANYA_WEB_DEMO_COMMON_ARGS}
            --preload-file "assets"
            --js-library "${COMPANYA_WEB_SOURCE}/WebCanvas/WebCanvas.js"
            "${COMPANYA_SOURCE_DIR}/tools/BehaviorTree.cpp"
            "${COMPANYA_WEB_SOURCE}/WebTextbox/WebTextbox.cpp"
            "${COMPANYA_WEB_SOURCE}/WebLayout/WebLayout.cpp"
            "${COMPANYA_WEB_SOURCE}/WebImage/WebImage.cpp"
            "${COMPANYA_WEB_SOURCE}/WebButton/WebButton.cpp"
            "${COMPANYA_WEB_SOURCE}/WebCanvas/WebCanvas.cpp"
            "${COMPANYA_WEB_SOURCE}/interface/WebInterface.cpp"
            "${COMPANYA_WEB_SOURCE}/InputManager.cpp"
            "${COMPANYA_WEB_DEMO_OUTPUT_DIR}/Group18_main.cpp"
            -o "${COMPANYA_WEB_DEMO_OUTPUT_DIR}/Group18_main.js"
        DEPENDS
            "${COMPANYA_WEB_SOURCE}/WebTextbox/WebTextbox.cpp"
            "${COMPANYA_WEB_SOURCE}/WebLayout/WebLayout.cpp"
            "${COMPANYA_WEB_SOURCE}/WebImage/WebImage.cpp"
            "${COMPANYA_WEB_SOURCE}/WebButton/WebButton.cpp"
            "${COMPANYA_WEB_SOURCE}/WebCanvas/WebCanvas.cpp"
            "${COMPANYA_WEB_SOURCE}/interface/WebInterface.cpp"
            "${COMPANYA_WEB_SOURCE}/InputManager.cpp"
            "${COMPANYA_WEB_DEMO_OUTPUT_DIR}/Group18_main.cpp"
            "${COMPANYA_WEB_SOURCE}/WebCanvas/WebCanvas.js"
    )

    add_custom_command(
        OUTPUT "${COMPANYA_WEB_TEST_OUTPUT_DIR}/WebTextboxTest.html"
        COMMAND "${CMAKE_COMMAND}" -E make_directory "${COMPANYA_WEB_TEST_OUTPUT_DIR}"
        COMMAND "${COMPANYA_EMPP_EXECUTABLE}"
            ${COMPANYA_WEB_TEST_COMMON_ARGS}
            ${COMPANYA_WEB_TEST_INCLUDE_ARGS}
            "${COMPANYA_TEST_DIR}/Interfaces/WebUI/WebTextboxTest.cpp"
            "${COMPANYA_SOURCE_DIR}/Interfaces/WebUI/WebTextbox/WebTextbox.cpp"
            "${COMPANYA_SOURCE_DIR}/Interfaces/WebUI/WebLayout/WebLayout.cpp"
            "${COMPANYA_SOURCE_DIR}/Interfaces/WebUI/WebCanvas/WebCanvas.cpp"
            --js-library "${COMPANYA_SOURCE_DIR}/Interfaces/WebUI/WebCanvas/WebCanvas.js"
            -o "${COMPANYA_WEB_TEST_OUTPUT_DIR}/WebTextboxTest.html"
        DEPENDS
            "${COMPANYA_TEST_DIR}/Interfaces/WebUI/WebTextboxTest.cpp"
            "${COMPANYA_SOURCE_DIR}/Interfaces/WebUI/WebTextbox/WebTextbox.cpp"
            "${COMPANYA_SOURCE_DIR}/Interfaces/WebUI/WebLayout/WebLayout.cpp"
            "${COMPANYA_SOURCE_DIR}/Interfaces/WebUI/WebCanvas/WebCanvas.cpp"
            "${COMPANYA_SOURCE_DIR}/Interfaces/WebUI/WebCanvas/WebCanvas.js"
        VERBATIM
    )

    add_custom_command(
        OUTPUT "${COMPANYA_WEB_TEST_OUTPUT_DIR}/WebImageTest.html"
        COMMAND "${CMAKE_COMMAND}" -E make_directory "${COMPANYA_WEB_TEST_OUTPUT_DIR}"
        COMMAND "${COMPANYA_EMPP_EXECUTABLE}"
            ${COMPANYA_WEB_TEST_COMMON_ARGS}
            ${COMPANYA_WEB_TEST_INCLUDE_ARGS}
            "${COMPANYA_TEST_DIR}/Interfaces/WebUI/WebImageTest.cpp"
            "${COMPANYA_SOURCE_DIR}/Interfaces/WebUI/WebImage/WebImage.cpp"
            "${COMPANYA_SOURCE_DIR}/Interfaces/WebUI/WebLayout/WebLayout.cpp"
            "${COMPANYA_SOURCE_DIR}/Interfaces/WebUI/WebCanvas/WebCanvas.cpp"
            --js-library "${COMPANYA_SOURCE_DIR}/Interfaces/WebUI/WebCanvas/WebCanvas.js"
            -o "${COMPANYA_WEB_TEST_OUTPUT_DIR}/WebImageTest.html"
        DEPENDS
            "${COMPANYA_TEST_DIR}/Interfaces/WebUI/WebImageTest.cpp"
            "${COMPANYA_SOURCE_DIR}/Interfaces/WebUI/WebImage/WebImage.cpp"
            "${COMPANYA_SOURCE_DIR}/Interfaces/WebUI/WebLayout/WebLayout.cpp"
            "${COMPANYA_SOURCE_DIR}/Interfaces/WebUI/WebCanvas/WebCanvas.cpp"
            "${COMPANYA_SOURCE_DIR}/Interfaces/WebUI/WebCanvas/WebCanvas.js"
        VERBATIM
    )   

    add_custom_command(
        OUTPUT "${COMPANYA_WEB_TEST_OUTPUT_DIR}/WebButtonTest.html"
        COMMAND "${CMAKE_COMMAND}" -E make_directory "${COMPANYA_WEB_TEST_OUTPUT_DIR}"
        COMMAND "${COMPANYA_EMPP_EXECUTABLE}"
            ${COMPANYA_WEB_TEST_COMMON_ARGS}
            ${COMPANYA_WEB_TEST_INCLUDE_ARGS}
            "${COMPANYA_TEST_DIR}/Interfaces/WebUI/WebButtonTest.cpp"
            "${COMPANYA_SOURCE_DIR}/Interfaces/WebUI/WebButton/WebButton.cpp"
            "${COMPANYA_SOURCE_DIR}/Interfaces/WebUI/WebLayout/WebLayout.cpp"
            "${COMPANYA_SOURCE_DIR}/Interfaces/WebUI/WebCanvas/WebCanvas.cpp"
            --js-library "${COMPANYA_SOURCE_DIR}/Interfaces/WebUI/WebCanvas/WebCanvas.js"
            -o "${COMPANYA_WEB_TEST_OUTPUT_DIR}/WebButtonTest.html"
        DEPENDS
            "${COMPANYA_TEST_DIR}/Interfaces/WebUI/WebButtonTest.cpp"
            "${COMPANYA_SOURCE_DIR}/Interfaces/WebUI/WebButton/WebButton.cpp"
            "${COMPANYA_SOURCE_DIR}/Interfaces/WebUI/WebLayout/WebLayout.cpp"
            "${COMPANYA_SOURCE_DIR}/Interfaces/WebUI/WebCanvas/WebCanvas.cpp"
            "${COMPANYA_SOURCE_DIR}/Interfaces/WebUI/WebCanvas/WebCanvas.js"
        VERBATIM
    )

    add_custom_command(
        OUTPUT "${COMPANYA_WEB_TEST_OUTPUT_DIR}/WebCanvasTest.html"
        COMMAND "${CMAKE_COMMAND}" -E make_directory "${COMPANYA_WEB_TEST_OUTPUT_DIR}"
        COMMAND "${COMPANYA_EMPP_EXECUTABLE}"
            ${COMPANYA_WEB_TEST_COMMON_ARGS}
            ${COMPANYA_WEB_TEST_INCLUDE_ARGS}
            "${COMPANYA_TEST_DIR}/Interfaces/WebUI/WebCanvasTest.cpp"
            "${COMPANYA_SOURCE_DIR}/Interfaces/WebUI/WebLayout/WebLayout.cpp"
            "${COMPANYA_SOURCE_DIR}/Interfaces/WebUI/WebCanvas/WebCanvas.cpp"
            --js-library "${COMPANYA_SOURCE_DIR}/Interfaces/WebUI/WebCanvas/WebCanvas.js"
            -o "${COMPANYA_WEB_TEST_OUTPUT_DIR}/WebCanvasTest.html"
        DEPENDS
            "${COMPANYA_TEST_DIR}/Interfaces/WebUI/WebCanvasTest.cpp"
            "${COMPANYA_SOURCE_DIR}/Interfaces/WebUI/WebLayout/WebLayout.cpp"
            "${COMPANYA_SOURCE_DIR}/Interfaces/WebUI/WebCanvas/WebCanvas.cpp"
            "${COMPANYA_SOURCE_DIR}/Interfaces/WebUI/WebCanvas/WebCanvas.js"
        VERBATIM
    )

    add_custom_command(
        OUTPUT "${COMPANYA_WEB_TEST_OUTPUT_DIR}/WebLayoutTest.html"
        COMMAND "${CMAKE_COMMAND}" -E make_directory "${COMPANYA_WEB_TEST_OUTPUT_DIR}"
        COMMAND "${COMPANYA_EMPP_EXECUTABLE}"
            ${COMPANYA_WEB_TEST_COMMON_ARGS}
            ${COMPANYA_WEB_TEST_INCLUDE_ARGS}
            "${COMPANYA_TEST_DIR}/Interfaces/WebUI/WebLayoutTest.cpp"
            "${COMPANYA_SOURCE_DIR}/Interfaces/WebUI/WebLayout/WebLayout.cpp"
            "${COMPANYA_SOURCE_DIR}/Interfaces/WebUI/WebCanvas/WebCanvas.cpp"
            --js-library "${COMPANYA_SOURCE_DIR}/Interfaces/WebUI/WebCanvas/WebCanvas.js"
            -o "${COMPANYA_WEB_TEST_OUTPUT_DIR}/WebLayoutTest.html"
        DEPENDS
            "${COMPANYA_TEST_DIR}/Interfaces/WebUI/WebLayoutTest.cpp"
            "${COMPANYA_SOURCE_DIR}/Interfaces/WebUI/WebLayout/WebLayout.cpp"
            "${COMPANYA_SOURCE_DIR}/Interfaces/WebUI/WebCanvas/WebCanvas.cpp"
            "${COMPANYA_SOURCE_DIR}/Interfaces/WebUI/WebCanvas/WebCanvas.js"
        VERBATIM
    )

    add_custom_command(
      WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
      OUTPUT "${COMPANYA_WEB_TEST_OUTPUT_DIR}/WebInterfaceTest.js"
      COMMAND "${CMAKE_COMMAND}" -E make_directory "${COMPANYA_WEB_TEST_OUTPUT_DIR}"
      COMMAND "${CMAKE_COMMAND}" -E copy
            "${COMPANYA_TEST_DIR}/Interfaces/WebUI/WebInterfaceTest.html"
            "${COMPANYA_WEB_TEST_OUTPUT_DIR}/WebInterfaceTest.html"
      COMMAND "${COMPANYA_EMPP_EXECUTABLE}"
          ${COMPANYA_WEB_TEST_COMMON_ARGS}
          ${COMPANYA_WEB_TEST_INCLUDE_ARGS}
          -sASYNCIFY=1
          -sNO_EXIT_RUNTIME=1
          -sALLOW_MEMORY_GROWTH=1
          --preload-file "assets"
          --js-library "${COMPANYA_WEB_SOURCE}/WebCanvas/WebCanvas.js"
          "${COMPANYA_SOURCE_DIR}/tools/BehaviorTree.cpp"
          "${COMPANYA_WEB_SOURCE}/WebTextbox/WebTextbox.cpp"
          "${COMPANYA_WEB_SOURCE}/WebLayout/WebLayout.cpp"
          "${COMPANYA_WEB_SOURCE}/WebImage/WebImage.cpp"
          "${COMPANYA_WEB_SOURCE}/WebButton/WebButton.cpp"
          "${COMPANYA_WEB_SOURCE}/WebCanvas/WebCanvas.cpp"
          "${COMPANYA_WEB_SOURCE}/interface/WebInterface.cpp"
          "${COMPANYA_WEB_SOURCE}/InputManager.cpp"
          "${COMPANYA_TEST_DIR}/Interfaces/WebUI/WebInterfaceTest.cpp"
          -o "${COMPANYA_WEB_TEST_OUTPUT_DIR}/WebInterfaceTest.js"
      DEPENDS
          "${COMPANYA_WEB_SOURCE}/WebTextbox/WebTextbox.cpp"
          "${COMPANYA_WEB_SOURCE}/WebLayout/WebLayout.cpp"
          "${COMPANYA_WEB_SOURCE}/WebImage/WebImage.cpp"
          "${COMPANYA_WEB_SOURCE}/WebButton/WebButton.cpp"
          "${COMPANYA_WEB_SOURCE}/WebCanvas/WebCanvas.cpp"
          "${COMPANYA_WEB_SOURCE}/interface/WebInterface.cpp"
          "${COMPANYA_WEB_SOURCE}/InputManager.cpp"
          "${COMPANYA_WEB_SOURCE}/WebCanvas/WebCanvas.js"
    )

    add_custom_target(group18_demo
        DEPENDS
            "${COMPANYA_WEB_DEMO_OUTPUT_DIR}/Group18_main.js"
    )

    add_custom_target(web_test
        DEPENDS
            "${COMPANYA_WEB_TEST_OUTPUT_DIR}/WebTextboxTest.html"
            "${COMPANYA_WEB_TEST_OUTPUT_DIR}/WebImageTest.html"
            "${COMPANYA_WEB_TEST_OUTPUT_DIR}/WebButtonTest.html"
            "${COMPANYA_WEB_TEST_OUTPUT_DIR}/WebCanvasTest.html"
            "${COMPANYA_WEB_TEST_OUTPUT_DIR}/WebLayoutTest.html"
            "${COMPANYA_WEB_TEST_OUTPUT_DIR}/WebInterfaceTest.js"
    )
else()
    add_custom_target(group18_demo
        COMMAND "${CMAKE_COMMAND}" -E echo "group18_demo requires em++ in PATH with the Emscripten environment active."
        COMMAND "${CMAKE_COMMAND}" -E false
        VERBATIM
    )
    add_custom_target(web_test
        COMMAND "${CMAKE_COMMAND}" -E echo "web_test requires em++ in PATH with the Emscripten environment active."
        COMMAND "${CMAKE_COMMAND}" -E false
        VERBATIM
    )
endif()