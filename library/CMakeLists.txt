cmake_minimum_required (VERSION 3.0.0)

add_definitions(-DLIBRARY_VERSION="${CMAKE_PROJECT_VERSION}") #Make project version available to the library as a definition
add_definitions(-DLIBRARY_RELEASE="${RELEASE}") #Make project release string available to the library as a definition

# By default, get all source files in src/ directory and compile them into HAIRCUT library
file(GLOB LIB_SOURCES "${CMAKE_CURRENT_LIST_DIR}/src/*.cpp" " ${CMAKE_CURRENT_LIST_DIR}/src/*.c")

set(LIBRARY_OUTPUT_PATH ${CMAKE_CURRENT_LIST_DIR}/lib) # put compiled library in lib/
set(LIBRARY_HEADERS ${CMAKE_CURRENT_LIST_DIR}/inc/HAIRCUT.h) #

#includes for libs
include_directories(${CMAKE_CURRENT_LIST_DIR}/inc/)

#Static library always builds
add_library(HAIRCUT-static STATIC ${LIB_SOURCES}) #Library is not declared static or shared. This is defined in the top-level cmake options
message(STATUS "Linking project library libs:")
foreach(lib ${LINK_PROJECT_LIBRARY_LIST})
	message(STATUS "\t${lib}")
endforeach()
target_link_libraries(HAIRCUT-static ${LINK_PROJECT_LIBRARY_LIST}) #Link to a list of libraries declared in top-level cmake
set_target_properties(HAIRCUT-static PROPERTIES OUTPUT_NAME "HAIRCUT")
set_target_properties(HAIRCUT-static PROPERTIES PUBLIC_HEADER ${LIBRARY_HEADERS})

if(BUILD_SHARED_LIBS OR LINK_SHARED_LIBS) 	#Optionally build shared library
	add_library(HAIRCUT SHARED ${LIB_SOURCES})
	target_link_libraries(HAIRCUT ${LINK_PROJECT_LIBRARY_LIBS}) #Link to a list of libraries declared in top-level cmake
	set_target_properties(HAIRCUT PROPERTIES VERSION ${CMAKE_PROJECT_VERSION} SOVERSION 0 )
	set_target_properties(HAIRCUT PROPERTIES POSITION_INDEPENDENT_CODE 1)
	set_target_properties(HAIRCUT PROPERTIES PUBLIC_HEADER ${LIBRARY_HEADERS})
endif()

if(DEFINED HEADER_INSTALL_PATH AND INSTALL_HEADERS)
	#	message(STATUS "Will install headers at  ${HEADER_INSTALL_PATH}")
	#	install(FILES ${HEADERS}
	#			DESTINATION ${HEADER_INSTALL_PATH})
endif()

if(DEFINED LIBRARY_INSTALL_PATH AND INSTALL_STATIC_LIBS)
	message(STATUS "Will install static libraries at  ${LIBRARY_INSTALL_PATH}")
	install(TARGETS HAIRCUT-static
			ARCHIVE DESTINATION ${LIBRARY_INSTALL_PATH}
			PUBLIC_HEADER DESTINATION ${HEADER_INSTALL_PATH})
endif()
if(DEFINED LIBRARY_INSTALL_PATH AND INSTALL_SHARED_LIBS AND BUILD_SHARED_LIBS)
	message(STATUS "Will install shared libraries at  ${LIBRARY_INSTALL_PATH}")
	install(TARGETS HAIRCUT
			LIBRARY DESTINATION ${LIBRARY_INSTALL_PATH}
			PUBLIC_HEADER DESTINATION ${HEADER_INSTALL_PATH})
endif()


