# Generate the list of source files
# using GLOB does not RECURSE

file(
	GLOB
	headers_files
	*.hpp
)

# Make sure the compiler can find include files from our library.
include_directories (${CMAKE_CURRENT_SOURCE_DIR})

install (FILES ${headers_files}
	 DESTINATION ${INCLUDE_INSTALL_DIR}
	 COMPONENT devel)
