cmake_minimum_required(VERSION 3.6)
project(crazy-sqlite-writer)

set(CMAKE_C_STANDARD 11)

LIST(APPEND CMAKE_MODULE_PATH "cmake")
FIND_PACKAGE(SQLite3 REQUIRED)
link_libraries(sqlite3)

include_directories(deps)
FILE(GLOB_RECURSE LIB_HEADER_FILES deps/*.h)
FILE(GLOB_RECURSE LIB_SOURCE_FILES deps/*.c)

set(HEADER_FILES crazy_sqlite_writer.h usage.h ${LIB_HEADER_FILES})
set(SOURCE_FILES main.c usage.c ${LIB_SOURCE_FILES})
add_executable(crazy-sqlite-writer ${SOURCE_FILES} ${HEADER_FILES})
