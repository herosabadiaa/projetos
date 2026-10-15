cmake_minimum_required(VERSION 3.15)
project(jogo)

find_package(raylib REQUIRED)


add_executable(${PROJECT_NAME} main.c cards.c )
target_link_libraries(${PROJECT_NAME} PRIVATE raylib)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/resources"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/resources"
    COMMAND ${CMAKE_COMMAND} -E copy
        "${CMAKE_CURRENT_SOURCE_DIR}/cards.csv"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/cards.csv"
)
