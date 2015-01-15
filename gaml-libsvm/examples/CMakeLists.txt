# Make sure the compiler can find include files from our library.
include_directories (${CMAKE_SOURCE_DIR}/src)

file(
	GLOB
	USAGE_EXAMPLES
	*.cpp
)

# loop over the list of examples
foreach(f ${USAGE_EXAMPLES})
    get_filename_component(exampleName ${f} NAME_WE) 
    add_executable (${exampleName} ${f}) 
    set_target_properties(${exampleName} PROPERTIES LINKER_LANGUAGE CXX)
    set_target_properties(${exampleName} PROPERTIES COMPILE_FLAGS "${PROJECT_ALL_CFLAGS}" LINK_FLAGS "${PROJECT_ALL_LDFLAGS}")

    install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/${exampleName} 
            DESTINATION bin
            RENAME ${CMAKE_PROJECT_NAME}-${exampleName}
	    COMPONENT binary)
endforeach(f)

