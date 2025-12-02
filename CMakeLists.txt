cmake_minimum_required(VERSION 3.10)
project(AIV_1945 LANGUAGES C)

include_directories(include)

set(RAYLIB ${CMAKE_SOURCE_DIR}/lib/raylib.lib)
set(AIV_VECTOR ${CMAKE_SOURCE_DIR}/lib/aiv_vector.lib)


add_executable(GameApp src/main.c)
target_include_directories(GameApp PRIVATE include)
target_link_libraries(GameApp PUBLIC ${AIV_VECTOR})
target_link_libraries(GameApp PRIVATE ${RAYLIB}      
    winmm
    gdi32
    user32
    shell32)

add_custom_command(TARGET GameApp POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
                ${CMAKE_SOURCE_DIR}/assets
                $<TARGET_FILE_DIR:GameApp>/assets

        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                ${CMAKE_SOURCE_DIR}/lib/aiv_vector.dll
                $<TARGET_FILE_DIR:GameApp>
)