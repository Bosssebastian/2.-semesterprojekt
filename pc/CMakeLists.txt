cmake_minimum_required(VERSION 3.10)

project(pc LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(SHARED_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../shared")

file(GLOB_RECURSE SHARED_SOURCES CONFIGURE_DEPENDS
    "${SHARED_DIR}/*.c"
    "${SHARED_DIR}/*.cc"
    "${SHARED_DIR}/*.cpp"
    "${SHARED_DIR}/*.cxx"
)
list(FILTER SHARED_SOURCES EXCLUDE REGEX ".*/[Uu]artClass\\.(c|cc|cpp|cxx)$")
list(FILTER SHARED_SOURCES EXCLUDE REGEX ".*/serialPort\\.(c|cc|cpp|cxx)$")

find_package(OpenCV REQUIRED)

add_executable(pc
    src/app/main.cpp
    src/io/IoRunner.cpp
    src/io/Interface.cpp
    src/io/SerialPort.cpp
    src/vision/VisionRunner.cpp
    src/vision/VisionController.cpp
    src/orchestrator/Orchestrator.cpp
    ${SHARED_SOURCES}
)

target_include_directories(pc PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core
    ${CMAKE_CURRENT_SOURCE_DIR}/src/io
    ${CMAKE_CURRENT_SOURCE_DIR}/src/vision
    ${CMAKE_CURRENT_SOURCE_DIR}/src/orchestrator
    ${SHARED_DIR}
)

target_link_libraries(pc PRIVATE ${OpenCV_LIBS})

if(MSVC)
    target_compile_options(pc PRIVATE /W4)
else()
    target_compile_options(pc PRIVATE -Wall -Wextra -Wpedantic)
endif()

target_link_libraries(pc PRIVATE pthread)

target_link_libraries(pc PRIVATE ${OpenCV_LIBS})