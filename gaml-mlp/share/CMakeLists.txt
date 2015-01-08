

# Loop over the files to copy to share/${CMAKE_PROJECT_NAME}
file(
	GLOB 
	share_files
	*.gplot
)

foreach(f ${share_files})
	  get_filename_component(fname ${f} NAME) 
	  install (FILES ${fname}    
	  	   DESTINATION share/${CMAKE_PROJECT_NAME}
		   COMPONENT binary)
endforeach(f)
