cmake_minimum_required(VERSION 3.20)

project(QRScanner LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(OpenCV REQUIRED)

add_executable(QRScanner
    Project3/main.cpp
    Project3/QRScanner.cpp
    Project3/FileDialog.cpp
)

target_include_directories(QRScanner PRIVATE
    Project3
    ${OpenCV_INCLUDE_DIRS}
)

target_link_libraries(QRScanner PRIVATE
    ${OpenCV_LIBS}
)