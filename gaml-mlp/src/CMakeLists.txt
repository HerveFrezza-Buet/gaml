file(
	GLOB
	headers_files
	*.hpp
)

install (FILES ${headers_files}
	 DESTINATION ${INCLUDE_INSTALL_DIR}
	 COMPONENT devel)
