cmake_minimum_required (VERSION 3.10)
project(Program_2)

set(CPP Program_2.cpp)

add_executable(${PROJECT_NAME}
   ${CPP}
)
source_group("" FILES ${CPP})

set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:CONSOLE  /ENTRY:mainCRTStartup")

add_definitions(-D_WINSOCK_DEPRECATED_NO_WARNINGS -D_CRT_SECURE_NO_WARNINGS)
target_link_libraries(${PROJECT_NAME} PRIVATE ws2_32.lib)
