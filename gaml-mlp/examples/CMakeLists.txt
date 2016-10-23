# Make sure the compiler can find include files from our library.
include_directories (${CMAKE_SOURCE_DIR}/src)

# Define our examples to compile
file(
	GLOB 
	usage_examples
	*.cpp
)

# Flags for the data files, plot scripts, ..
SET(SHARE_PATH_CFLAGS "-DML_MLP_SHAREDIR=\"\\\"${CMAKE_INSTALL_PREFIX}/share/${CMAKE_PROJECT_NAME}\\\"\"")

# loop over the list to compile them
foreach(f ${usage_examples})
    get_filename_component(exampleName ${f} NAME_WE) 
    add_executable (${exampleName} ${f}) 
    set_target_properties(${exampleName} PROPERTIES LINKER_LANGUAGE CXX)
    set_target_properties(${exampleName} PROPERTIES COMPILE_FLAGS "${PROJECT_ALL_CFLAGS} ${SHARE_PATH_CFLAGS} -std=c++14" LINK_FLAGS "${PROJECT_ALL_LDFLAGS}")

    install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/${exampleName} 
            DESTINATION bin
            RENAME ${CMAKE_PROJECT_NAME}-${exampleName}
	    COMPONENT binary)
endforeach(f)
