

set(CMAKE_C_COMPILER "g") #specify g++ compiler
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_LIST_DIR}/bin) # put compiled executable in bin/
add_compile_options(-g) # -g turns on debugging
add_compile_options(-O0 -Og) #

find_package( Threads )

add_executable("HAIRCUT-bin" src/main.cpp)
include_directories(${CMAKE_CURRENT_LIST_DIR}/inc/)
target_link_libraries("HAIRCUT-bin" "HAIRCUT")
target_link_libraries("HAIRCUT-bin" Threads::Threads LimeSuite fftw3f)
set_target_properties("HAIRCUT-bin" PROPERTIES OUTPUT_NAME "HAIRCUT")

if(DEFINED PRIMARY_EXECUTABLE_INSTALL_PATH)
    message(STATUS "Will install primary executables at ${PRIMARY_EXECUTABLE_INSTALL_PATH}")
    install(TARGETS HAIRCUT-bin DESTINATION ${PRIMARY_EXECUTABLE_INSTALL_PATH})
else()
    message(STATUS "No primary executable install path defined")
endif()
