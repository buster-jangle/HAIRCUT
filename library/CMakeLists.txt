
set(LIBRARY_OUTPUT_PATH ${CMAKE_CURRENT_LIST_DIR}/lib) # put compiled executable in bin/
add_library("HAIRCUT" STATIC "src/HAIRCUT.cpp")
include_directories(${CMAKE_CURRENT_LIST_DIR}/inc/)
set_target_properties("HAIRCUT" PROPERTIES PUBLIC_HEADER "inc/HAIRCUT.h")

if(DEFINED LIBRARY_INSTALL_PATH AND DEFINED HEADER_INSTALL_PATH)
	message(STATUS "Will install libraries at  ${LIBRARY_INSTALL_PATH}")
	message(STATUS "Will install headers at  ${HEADER_INSTALL_PATH}")
	install(TARGETS "HAIRCUT"
		 LIBRARY DESTINATION ${LIBRARY_INSTALL_PATH}
		PUBLIC_HEADER DESTINATION ${HEADER_INSTALL_PATH}
	)
else()
	message(STATUS "No libbrary and/or header install path defined")
endif()


