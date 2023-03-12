cmake_minimum_required(VERSION 3.10)

set(CMAKE_PREFIX_PATH ${CMAKE_CURRENT_SOURCE_DIR}/conan)
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/conan)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(
    artichoke-sub
    VERSION 0.0.1
    DESCRIPTION ""
    LANGUAGES CXX
)

set(PROJECT_AUTHOR "Erick Saúl")
set(PROJECT_AUTHOR_GITHUB "@Erick3900")

add_subdirectory(configured_files)

find_package(fmt CONFIG REQUIRED)
find_package(ctre CONFIG REQUIRED)
find_package(Boost CONFIG REQUIRED)
find_package(tl-expected CONFIG REQUIRED)

add_executable(
    ${PROJECT_NAME} 
        main.cpp

        src/options_parser.cpp
)

target_link_libraries(
    ${PROJECT_NAME} PRIVATE 
        fmt::fmt
        ctre::ctre
        tl::expected
        Boost::program_options
)

target_include_directories(
    ${PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_BINARY_DIR}/configured_files/include
)

set_target_properties(
    ${PROJECT_NAME} PROPERTIES
        RUNTIME_OUTPUT_NAME arti-sub
)

install(
    TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION sub/bin
)
