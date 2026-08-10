cmake_minimum_required(VERSION 3.14)

project(my_server C)

find_package(PostgreSQL REQUIRED)

add_executable(server
    server.c
    DBConnection.c
)

target_link_libraries(server
    PRIVATE
    PostgreSQL::PostgreSQL
)