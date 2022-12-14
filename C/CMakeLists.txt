cmake_minimum_required(VERSION 3.22)
project(server_protocol C)
set(CMAKE_C_STANDARD 99)

set_target_properties(${TARGET} PROPERTIES
        COMPILE_FLAGS "-pthread"
        LINK_FLAGS "-pthread")

set(
        SRC
        ${CMAKE_SOURCE_DIR}/source/base/socket_base.c
        ${CMAKE_SOURCE_DIR}/source/base/event_loop.c
        ${CMAKE_SOURCE_DIR}/source/base/signal_handle.c
        ${CMAKE_SOURCE_DIR}/source/base/initialize.c
        ${CMAKE_SOURCE_DIR}/source/base/thpool.c

        ${CMAKE_SOURCE_DIR}/source/global.c
        ${CMAKE_SOURCE_DIR}/source/protocol.c

        ${CMAKE_SOURCE_DIR}/source/utils/timer.c
        ${CMAKE_SOURCE_DIR}/source/utils/log.c
        ${CMAKE_SOURCE_DIR}/source/utils/io_utils.c
        ${CMAKE_SOURCE_DIR}/source/utils/str_utils.c
)

set(
    INCLUDES
    ${CMAKE_SOURCE_DIR}/include/base/
    ${CMAKE_SOURCE_DIR}/include/
    ${CMAKE_SOURCE_DIR}/include/utils/
    ${CMAKE_SOURCE_DIR}/include/redis/
    ${CMAKE_SOURCE_DIR}/include/cJSON
)

set(
        STATIC_LIBS
        ${CMAKE_SOURCE_DIR}/lib/libhiredis.a
)




# compiling server_protocol
add_executable(server_protocol main.c ${SRC})

target_include_directories(server_protocol PUBLIC ${INCLUDES})

target_link_directories(server_protocol PUBLIC ./lib)

target_link_libraries(server_protocol pthread cJSON hiredis)






# compiling redis_test
add_executable(redis_test test/hiredis_test.c ${SRC} )

target_include_directories(redis_test PUBLIC ${INCLUDES})

target_link_libraries(redis_test PUBLIC pthread ${CMAKE_SOURCE_DIR}/lib/libhiredis.a  ${CMAKE_SOURCE_DIR}/lib/libcJSON.a)
