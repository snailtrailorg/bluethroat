# This file was automatically generated for projects
# without default 'CMakeLists.txt' file.

FILE(GLOB APP_SOURCES ${CMAKE_CURRENT_LIST_DIR} "*.c" "*.cpp")

FILE(GLOB src_subdirs RELATIVE ${CMAKE_CURRENT_LIST_DIR} ${CMAKE_CURRENT_LIST_DIR}/*)
FOREACH(src_subdir ${src_subdirs})
    IF(IS_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/${src_subdir})
        set(CURRENT_LIST_DIR_BACKUP_SRC ${CMAKE_CURRENT_LIST_DIR})
        set(CMAKE_CURRENT_LIST_DIR ${CMAKE_CURRENT_LIST_DIR}/${src_subdir})
        include(${CMAKE_CURRENT_LIST_DIR}/CMakeLists.txt)
        set(CMAKE_CURRENT_LIST_DIR ${CURRENT_LIST_DIR_BACKUP_SRC})
    ENDIF()
ENDFOREACH()

idf_component_register(SRCS ${APP_SOURCES})
