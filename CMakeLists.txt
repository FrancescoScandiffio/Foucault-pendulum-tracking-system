cmake_minimum_required(VERSION 3.8)
project(Foucault)

set(CMAKE_CXX_STANDARD 11)
set(SOURCE_FILES main.cpp utilityFunctions.cpp utilityFunctions.h)
set(THREADS_PREFER_PTHREAD_FLAG ON)

add_executable(Foucault ${SOURCE_FILES})

find_package( OpenCV REQUIRED )
target_link_libraries( Foucault ${OpenCV_LIBS})

find_package(Threads REQUIRED)
target_link_libraries(Foucault ${CMAKE_THREAD_LIBS_INIT})