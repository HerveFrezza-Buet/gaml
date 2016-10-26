# Make sure the compiler can find include files from our library.
include_directories (${CMAKE_SOURCE_DIR}/src)

# Define our examples to compile
file(
	GLOB 
	USAGE_EXAMPLES
	*.cpp
)

# loop over the list to compile them
foreach(f ${USAGE_EXAMPLES})
#foreach(f example-003-002-multiclass.cpp)
    get_filename_component(exampleName ${f} NAME_WE) 
    add_executable (${exampleName} ${f})
    set_target_properties(${exampleName} PROPERTIES LINKER_LANGUAGE CXX)
    set_target_properties(${exampleName} PROPERTIES COMPILE_FLAGS "${PROJECT_ALL_CFLAGS} -I${CMAKE_SOURCE_DIR}/examples" LINK_FLAGS "${PROJECT_ALL_LDFLAGS}")

    install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/${exampleName} 
            DESTINATION bin
            RENAME gaml-${exampleName}
	    COMPONENT binary)
endforeach(f) 
