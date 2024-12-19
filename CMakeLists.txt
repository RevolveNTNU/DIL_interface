cmake_minimum_required(VERSION 3.8)
project(DIL_interface)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")  # High optimization level

find_package(PkgConfig REQUIRED)               
pkg_check_modules(LIBEVDEV REQUIRED libevdev)  

set(PUBLIC_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/include/DIL_interface)
set(PRIVATE_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/src)

add_library(DIL_interface SHARED
    src/shared_state.cpp
    src/input_event_handler.cpp
    src/output_event_handler.cpp
)

target_include_directories(DIL_interface
    PUBLIC ${PUBLIC_INCLUDE_DIR}
    PRIVATE ${PRIVATE_INCLUDE_DIR} ${LIBEVDEV_INCLUDE_DIRS}
)

target_link_libraries(DIL_interface PRIVATE ${LIBEVDEV_LIBRARIES})

install(TARGETS DIL_interface
    EXPORT DIL_interfaceTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
)

install(DIRECTORY ${PUBLIC_INCLUDE_DIR}/ DESTINATION include)