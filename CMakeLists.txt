cmake_minimum_required ( VERSION 2.6 )
set ( CMAKE_CXX_FLAGS "-std=c++11" )

project ( Solver )

add_subdirectory( client )
add_subdirectory( server )
add_subdirectory( utils )
