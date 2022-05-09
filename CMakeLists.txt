cmake_minimum_required(VERSION 3.20)
project(crc_prefixer 
    VERSION 1.0.0
    LANGUAGES CXX
    DESCRIPTION "Calculate a message CRC prefix from the message"
)
add_subdirectory(src)
add_subdirectory(doc)
SET(CPACK_SOURCE_IGNORE_FILES "/build/;.swp;.git")
INCLUDE(InstallRequiredSystemLibraries)
include (CPack)
