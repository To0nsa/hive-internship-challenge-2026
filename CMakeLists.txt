cmake_minimum_required(VERSION 3.22.1)
project(Survive LANGUAGES CXX)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# --- Sanitizers (always on) ---------------------------------------------------
option(ENABLE_SANITIZERS "Force-enable sanitizers for this target" ON)

function(apply_sanitizers_always tgt)
  if(NOT ENABLE_SANITIZERS)
    return()
  endif()

  if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    # Address + Undefined; keep frame pointers
    target_compile_options(${tgt} PRIVATE -fsanitize=address,undefined -fno-omit-frame-pointer)
    target_link_options(${tgt}    PRIVATE -fsanitize=address,undefined)

  elseif(MSVC)
    # MSVC supports only AddressSanitizer (x64). Requires /MD runtime.
    target_compile_options(${tgt} PRIVATE /fsanitize=address /Zi)
    # Linker picks up ASan automatically with the compile flag.
  endif()
endfunction()

# --- SFML via FetchContent ----------------------------------------------------
include(FetchContent)
FetchContent_Declare(SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG 3.0.x
    GIT_SHALLOW ON)
FetchContent_MakeAvailable(SFML)

# --- Sources ------------------------------------------------------------------
file(GLOB_RECURSE SOURCES src/*.cpp)

# --- Target -------------------------------------------------------------------
add_executable(runner ${SOURCES})
target_link_libraries(runner PRIVATE sfml-graphics sfml-audio sfml-network)
target_compile_features(runner PRIVATE cxx_std_17)
target_compile_options(runner PRIVATE -Wall -Wextra -Werror -Wpedantic)
target_include_directories(runner PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

# Apply sanitizers unconditionally (can be disabled with -DENABLE_SANITIZERS=OFF)
apply_sanitizers_always(runner)

# --- LeakSanitizer suppressions ----------------------------------------------
# Create a suppression file at configure time and deploy it next to the binary.
set(LSAN_SUPPRESSIONS_CONTENT
"leak:libGLX_mesa.so
leak:glad_glx_has_extension
leak:gladLoaderLoadGLX
")
set(LSAN_SUPPRESSIONS_PATH ${CMAKE_CURRENT_BINARY_DIR}/lsan.supp)
file(WRITE ${LSAN_SUPPRESSIONS_PATH} "${LSAN_SUPPRESSIONS_CONTENT}")

# Copy lsan.supp beside the executable on build.
add_custom_command(
    TARGET runner
    POST_BUILD
    COMMENT "Deploy lsan.supp next to the executable"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${LSAN_SUPPRESSIONS_PATH}
            $<TARGET_FILE_DIR:runner>/lsan.supp
    VERBATIM)

if(ENABLE_SANITIZERS)
    # Convenient 'run' target that sets LSAN_OPTIONS to use the suppression file.
    add_custom_target(run
        COMMAND ${CMAKE_COMMAND} -E env
                LSAN_OPTIONS=suppressions=$<TARGET_FILE_DIR:runner>/lsan.supp
                ASAN_OPTIONS=halt_on_error=1
                $<TARGET_FILE:runner>
        WORKING_DIRECTORY $<TARGET_FILE_DIR:runner>
        DEPENDS runner
        USES_TERMINAL
        COMMENT "Running runner with LeakSanitizer suppressions"
    )
endif()

# --- Post-build: copy assets --------------------------------------------------
add_custom_command(
    TARGET runner
    COMMENT "Copy assets directory"
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_CURRENT_SOURCE_DIR}/assets
            $<TARGET_FILE_DIR:runner>/assets
    VERBATIM)

# --- Windows: copy OpenAL DLL -------------------------------------------------
if(WIN32)
    add_custom_command(
        TARGET runner
        COMMENT "Copy OpenAL DLL"
        PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
                ${SFML_SOURCE_DIR}/extlibs/bin/$<IF:$<EQUAL:${CMAKE_SIZEOF_VOID_P},8>,x64,x86>/openal32.dll
                $<TARGET_FILE_DIR:runner>
        VERBATIM)
endif()
