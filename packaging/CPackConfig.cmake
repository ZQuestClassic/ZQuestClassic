set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY OFF)
set(CPACK_PACKAGE_NAME "ZQuestClassic")
set(VERSION_STRING "unknown")

if(CPACK_ZC_VERSION)
	message(STATUS "CPack: Using explicit version ${CPACK_ZC_VERSION}")
	set(VERSION_STRING "${CPACK_ZC_VERSION}")
else()
	find_package(Git QUIET)
	if(GIT_EXECUTABLE)
		if(DEFINED ENV{GITHUB_REF})
			execute_process(
				COMMAND ${GIT_EXECUTABLE} describe $ENV{GITHUB_REF} --tags --abbrev=0 --match "*.*.*"
				WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
				OUTPUT_VARIABLE GIT_LATEST_VERSION_TAG
				OUTPUT_STRIP_TRAILING_WHITESPACE
				ERROR_QUIET
			)
		else()
			execute_process(
				COMMAND ${GIT_EXECUTABLE} describe --tags --abbrev=0 --match "*.*.*"
				WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
				OUTPUT_VARIABLE GIT_LATEST_VERSION_TAG
				OUTPUT_STRIP_TRAILING_WHITESPACE
				ERROR_QUIET
			)
		endif()
	endif()

	if(GIT_LATEST_VERSION_TAG)
		set(VERSION_STRING "${GIT_LATEST_VERSION_TAG}")
		message(STATUS "CPack: Detected git tag ${VERSION_STRING}")
	else()
		message(WARNING "CPack: Could not determine version from git or ZC_VERSION")
	endif()
endif()

if(NOT CPACK_ZC_VERSION)
	string(APPEND VERSION_STRING "-dev")
endif()

set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${VERSION_STRING}")
