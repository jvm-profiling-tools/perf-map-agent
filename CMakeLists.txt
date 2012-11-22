cmake_minimum_required (VERSION 2.6)
project (perf-map-agent)

find_package(JNI)
if (JNI_FOUND)
    message (STATUS "JNI_INCLUDE_DIRS=${JNI_INCLUDE_DIRS}")
    message (STATUS "JNI_LIBRARIES=${JNI_LIBRARIES}")
endif()
INCLUDE_DIRECTORIES(${JAVA_INCLUDE_PATH})

add_library(perfmap SHARED perf-map-agent.c)


