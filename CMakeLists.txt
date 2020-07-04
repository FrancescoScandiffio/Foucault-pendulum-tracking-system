cmake_minimum_required(VERSION 3.8)
project(Foucault)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES main.cpp ourFunctions.h perspective.cpp utilityFunctions.h calibrationGui.cpp guiFunctions.h)

add_executable(Foucault ${SOURCE_FILES})
set(CMAKE_THREAD_LIBS_INIT "-lpthread")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
set(CMAKE_HAVE_THREADS_LIBRARY 1)
set(CMAKE_USE_WIN32_THREADS_INIT 0)
set(CMAKE_USE_PTHREADS_INIT 1)
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package( OpenCV REQUIRED )
target_link_libraries( Foucault ${OpenCV_LIBS})

