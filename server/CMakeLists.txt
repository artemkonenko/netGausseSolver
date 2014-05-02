cmake_minimum_required ( VERSION 2.6 )
set ( CMAKE_CXX_FLAGS "-std=c++11" )

project ( Server )
set ( APP_NAME server )

set( CMAKE_BUILD_TYPE RELEASE )

include_directories( ../utils/include )

set ( EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/../bin )

add_executable ( ${APP_NAME} src/server.cpp )

target_link_libraries( ${APP_NAME} Utils )
