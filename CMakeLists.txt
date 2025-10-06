cmake_minimum_required(VERSION 3.16)

project (neostow
  VERSION 0.1.0
  DESCRIPTION "The Declarative GNU Stow"
  LANGUAGES C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include(CheckCCompilerFlag)
function(enable_strict_warnings target)
    if (CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic -Werror)
    elseif (CMAKE_C_COMPILER_ID STREQUAL "MSVC")
        target_compile_options(${target} PRIVATE /W4 /WX)
    endif()
endfunction()

add_library(vendor STATIC)

target_sources(vendor PRIVATE
    vendor/bashgen.c
    vendor/zshgen.c
    vendor/printfc.c
    vendor/printh.c
)

set(CMAKE_C_FLAGS_RELEASE "-O2")

target_include_directories(vendor PUBLIC vendor)
target_compile_features(vendor PUBLIC c_std_99)

add_executable(neostow neostow.c)

target_include_directories(neostow PRIVATE include)
target_link_libraries(neostow PRIVATE vendor)

enable_strict_warnings(neostow)
enable_strict_warnings(vendor)

install(TARGETS neostow RUNTIME DESTINATION bin)
# install(DIRECTORY include/ DESTINATION include)
# install(FILES docs/neostow.1 DESTINATION share/man/man1)
# install(FILES docs/neostow.5 DESTINATION share/man/man5)
