include(FindPackageHandleStandardArgs)

set(NUKLEAR_FOUND TRUE)

get_filename_component(CMAKE_CURRENT_LIST_DIR "${CMAKE_CURRENT_LIST_FILE}" PATH)
get_filename_component(nuklear_INSTALL_PREFIX "${CMAKE_CURRENT_LIST_DIR}/../" ABSOLUTE)

set(nuklear_INCLUDE_DIRS ${nuklear_INSTALL_PREFIX})

add_library(nuklear INTERFACE IMPORTED)
set_target_properties(nuklear PROPERTIES
	INTERFACE_INCLUDE_DIRECTORIES "${nuklear_INCLUDE_DIRS}"
)

find_package_handle_standard_args(nuklear DEFAULT_MSG nuklear_INCLUDE_DIRS)
