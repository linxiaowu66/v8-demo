cmake_minimum_required(VERSION 3.2)
project(V8Demo)

include_directories(/Users/linxiaowu/Github/v8/include)
include_directories(/Users/linxiaowu/Github/v8)

link_directories(
        /Users/linxiaowu/Github/v8/out.gn/x64.release.sample/obj
)
link_libraries(
        v8_monolith
)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -pthread")

set(SOURCE_FILES
        ./helloworld.cc)
set(CALC_SOURCE
        ./shell/shell.cpp
        ./shell/exposeToJsFuncs.cpp
        ./shell/exposeToJsVar.cpp
        ./shell/shell_util.cpp)
add_executable(HelloWorld ${SOURCE_FILES})
add_executable(Shell ${CALC_SOURCE})