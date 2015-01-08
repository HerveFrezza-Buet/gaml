

# Loop over the files to copy to share/${CMAKE_PROJECT_NAME}
file(
	GLOB 
	share_files
	*
)
# Be carefull when using the GLOB pattern *, it may also include
# .svn files. It is much better to use a glob pattern
# filtering the elements you want to install rather than selecting everything
# and removing some items from the list
list(REMOVE_ITEM share_files "${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt")

foreach(f ${share_files})
	  get_filename_component(fname ${f} NAME) 
	  install (FILES ${fname}    
	  	   DESTINATION share/${CMAKE_PROJECT_NAME}
		   COMPONENT binary)
endforeach(f)
