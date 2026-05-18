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
#set(DIR ${CMAKE_CURRENT_SOURCE_DIR})
find_package(ur_rtde REQUIRED) #PATHS "${DIR}/../../ur_rtde/Build/ur_rtde" "${DIR}/../../ur_rtde/build/ur_rtde")

add_executable(pc
    src/app/main.cpp
    src/io/IoRunner.cpp
    src/io/Interface.cpp
    src/io/SerialPort.cpp
    src/io/Uart.cpp
    src/logging/Logger.cpp
    src/vision/VisionRunner.cpp
    src/vision/VisionController.cpp
    src/orchestrator/Orchestrator.cpp
    src/orchestrator/StorageManager.cpp
    src/web/WebServerRunner.cpp
    src/orchestrator/matrixoperations.cpp
    src/orchestrator/transformations.cpp
    src/orchestrator/orientation.cpp
    src/orchestrator/movement.cpp
    src/orchestrator/database.cpp
    ${SHARED_SOURCES}
)

target_link_libraries(pc PRIVATE ur_rtde::rtde)

set_target_properties(pc PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

target_include_directories(pc PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core
    ${CMAKE_CURRENT_SOURCE_DIR}/src/io
    ${CMAKE_CURRENT_SOURCE_DIR}/src/logging
    ${CMAKE_CURRENT_SOURCE_DIR}/src/vision
    ${CMAKE_CURRENT_SOURCE_DIR}/src/orchestrator
    ${CMAKE_CURRENT_SOURCE_DIR}/src/web
    ${SHARED_DIR}
)

if(MSVC)
    target_compile_options(pc PRIVATE /W4)
else()
    target_compile_options(pc PRIVATE -Wall -Wextra -Wpedantic)
endif()

if(NOT WIN32)
    target_link_libraries(pc PRIVATE pthread)
else()
    target_link_libraries(pc PRIVATE ws2_32)
endif()

target_link_libraries(pc PRIVATE ${OpenCV_LIBS})
