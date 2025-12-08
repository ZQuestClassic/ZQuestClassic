if(ZC_VERSION)
	set(CPACK_ZC_VERSION ${ZC_VERSION})
endif()
set(CPACK_ZC_OFFICIAL ${ZC_OFFICIAL})
set(CPACK_PROJECT_CONFIG_FILE "${CMAKE_CURRENT_SOURCE_DIR}/CPackConfig.cmake")

if(APPLE)
	# Note: ideally this would be "MacOS", which is where binaries go in an .app bundle.
	# But for some reason when the launcher opens zplayer/zeditor, the new window appears
	# behind the launcher window. That doesn't happen when the executables are all in Resources.
	# So for now, keep them in the "wrong" place.
	# If this changes to MacOS: update "locate_zapp_file" and "create_binary_paths". And mac_entry.sh
	# could be deleted.
	set(INSTALL_BIN_DIR "ZQuest Classic.app/Contents/Resources")
	set(INSTALL_LIB_DIR "ZQuest Classic.app/Contents/Frameworks")
	set(INSTALL_DATA_DIR "ZQuest Classic.app/Contents/Resources")
elseif(WIN32)
	set(INSTALL_BIN_DIR ".")
	set(INSTALL_LIB_DIR ".")
	set(INSTALL_DATA_DIR ".")
else()
	set(INSTALL_BIN_DIR "bin")
	set(INSTALL_LIB_DIR "lib")
	set(INSTALL_DATA_DIR "share/zquestclassic")
endif()

function(get_all_linked_targets input_targets output_var)
	set(targets_to_process ${input_targets})
	set(processed_targets "")
	set(found_dependencies "")

	while(targets_to_process)
		list(GET targets_to_process 0 current_target)
		list(REMOVE_AT targets_to_process 0)

		if(current_target IN_LIST processed_targets)
			continue()
		endif()
		list(APPEND processed_targets "${current_target}")

		get_target_property(direct_libs "${current_target}" LINK_LIBRARIES)
		get_target_property(interface_libs "${current_target}" INTERFACE_LINK_LIBRARIES)
		
		set(all_links "")
		if(direct_libs)
			list(APPEND all_links ${direct_libs})
		endif()
		if(interface_libs)
			list(APPEND all_links ${interface_libs})
		endif()

		foreach(lib IN LISTS all_links)
			if(TARGET "${lib}")
				get_target_property(aliased_target "${lib}" ALIASED_TARGET)
				if(aliased_target)
					set(real_target "${aliased_target}")
				else()
					set(real_target "${lib}")
				endif()

				if(NOT real_target IN_LIST processed_targets)
					list(APPEND found_dependencies "${real_target}")
					list(APPEND targets_to_process "${real_target}")
				endif()
			endif()
		endforeach()
	endwhile()

	if(found_dependencies)
		list(REMOVE_DUPLICATES found_dependencies)
	endif()

	set(${output_var} ${found_dependencies} PARENT_SCOPE)
endfunction()

function(zc_install_targets)
	if(APPLE)
		set_target_properties(${ARGV} PROPERTIES INSTALL_RPATH "@loader_path/../Frameworks")
	elseif(LINUX)
		set_target_properties(${ARGV} PROPERTIES INSTALL_RPATH "$ORIGIN/../lib")
		set_target_properties(${ARGV} PROPERTIES INSTALL_RPATH_USE_LINK_PATH TRUE)
	endif()

	install(TARGETS ${ARGV}
		RUNTIME DESTINATION "${INSTALL_BIN_DIR}"
		LIBRARY DESTINATION "${INSTALL_LIB_DIR}"
		BUNDLE DESTINATION .
	)
endfunction()

function(zc_install_targets_windows)
	foreach(app ${ARGV})
		get_all_linked_targets("${app}" ALL_DEPS)

		# For debugging.
		# message(STATUS "---------------------------------------------------")
		# message(STATUS "DEBUG: Analyzing dependencies for app: ${app}")
		# foreach(dep ${ALL_DEPS})
		# 	get_target_property(type ${dep} TYPE)
		# 	# Attempt to get the directory, but don't crash if it fails
		# 	set(dir "NOT_FOUND")
		# 	if(TARGET ${dep})
		# 		# Check if we can get the file location
		# 		get_property(is_imported TARGET ${dep} PROPERTY IMPORTED)
		# 		if(is_imported OR type STREQUAL "SHARED_LIBRARY" OR type STREQUAL "STATIC_LIBRARY")
		# 			# This gen-ex prints the path at configure time for us to see
		# 			set(dir "$<TARGET_FILE_DIR:${dep}>") 
		# 		endif()
		# 	endif()
		# 	message(STATUS "   Dependency: ${dep} [Type: ${type}] -> Dir: ${dir}")
		# endforeach()
		# message(STATUS "---------------------------------------------------")

		set(SEARCH_DIRS "")
		foreach(dep ${ALL_DEPS})
			# Only look at targets that are actual libraries/executables (skip Headers/Interface)
			get_target_property(type ${dep} TYPE)
			if(NOT type STREQUAL "INTERFACE_LIBRARY" AND NOT type STREQUAL "UTILITY")
				list(APPEND SEARCH_DIRS "$<TARGET_FILE_DIR:${dep}>")
			endif()
		endforeach()

		install(TARGETS ${app} 
			RUNTIME_DEPENDENCIES
				DIRECTORIES "${SEARCH_DIRS}"
				PRE_EXCLUDE_REGEXES "api-ms-" "ext-ms-" 
				POST_EXCLUDE_REGEXES ".*system32/.*\\.dll"

			RUNTIME DESTINATION "${INSTALL_BIN_DIR}"
			LIBRARY DESTINATION "${INSTALL_LIB_DIR}"
			FRAMEWORK DESTINATION "${INSTALL_LIB_DIR}"
			BUNDLE DESTINATION .
		)
	endforeach()
endfunction()

if(NOT EMSCRIPTEN)
	list(APPEND ZC_INSTALL_TARGETS zlauncher zplayer zeditor zscript zcsound zupdater)
	zc_install_targets("${ZC_INSTALL_TARGETS}")
	if(WIN32)
		# This copies additional dlls that for some reason the above install command ignores:
		# zlib1.dll vorbisfile.dll vorbis.dll ogg.dll libcurl.dll
		zc_install_targets_windows("${ZC_INSTALL_TARGETS}")
	endif()
endif()

install(DIRECTORY "${CMAKE_SOURCE_DIR}/resources/"
	DESTINATION "${INSTALL_DATA_DIR}"
	PATTERN ".gitkeep" EXCLUDE
	PATTERN ".DS_Store" EXCLUDE)
install(DIRECTORY "${CMAKE_SOURCE_DIR}/changelogs" DESTINATION "${INSTALL_DATA_DIR}")

install(CODE "
	execute_process(
		COMMAND ${PYTHON} \"${CMAKE_SOURCE_DIR}/scripts/package.py\"
			--new_packager
			--build_folder \"${BUILD_FOLDER}\"
			--package_folder \"$<INSTALL_PREFIX>/${INSTALL_DATA_DIR}\"
			$<$<BOOL:${ZC_VERSION}>:--version;${ZC_VERSION}>
		COMMAND_ERROR_IS_FATAL ANY)
")

if(APPLE)
	if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
		set_property(CACHE CMAKE_INSTALL_PREFIX PROPERTY VALUE "/Applications")
	endif()

	set(CPACK_GENERATOR "DragNDrop")
	set(CPACK_DMG_VOLUME_NAME "ZQuest Classic")
	set(CPACK_DMG_DS_STORE_SETUP_SCRIPT "${CMAKE_SOURCE_DIR}/packaging/MacDMGSetup.scpt")
	set(CPACK_DMG_BACKGROUND_IMAGE "${CMAKE_SOURCE_DIR}/packaging/MacDMGBackground.png")

	install(FILES "${CMAKE_SOURCE_DIR}/packaging/Info.plist" DESTINATION "ZQuest Classic.app/Contents")
	install(PROGRAMS "${CMAKE_SOURCE_DIR}/packaging/mac_entry.sh" DESTINATION "ZQuest Classic.app/Contents/MacOS")

	# Icons.
	set(ICON_SCRIPT "${CMAKE_SOURCE_DIR}/packaging/generate_mac_bundle_icons.sh")
	install(CODE "
		execute_process(
			COMMAND bash \"${ICON_SCRIPT}\" \"$<INSTALL_PREFIX>/ZQuest Classic.app/Contents/Resources/icons.icns\"
			COMMAND_ERROR_IS_FATAL ANY
		)
	")

	# Generate debug info bundle (ZQuest Classic.app.dSYM):
	#   cpack -D CPACK_CUSTOM_INSTALL_VARIABLES=MAC_CREATE_DEBUG_SYMBOLS_BUNDLE=TRUE
	# This is only used for testing in CI. For releases, the debug symbols are stripped and sent to Sentry directly.
	install(CODE "
		if(MAC_CREATE_DEBUG_SYMBOLS_BUNDLE)
			set(APP_BUNDLE \"$<INSTALL_PREFIX>/ZQuest Classic.app\")
			set(DSYM_BUNDLE \"$<INSTALL_PREFIX>/ZQuest Classic.app.dSYM\")

			file(GLOB_RECURSE CANDIDATES \"\${APP_BUNDLE}/Contents/Resources/*\")
			set(EXECUTABLES \"\")
			foreach(ITEM \${CANDIDATES})
				if(IS_DIRECTORY \"\${ITEM}\")
					continue()
				endif()

				execute_process(
					COMMAND file -b \"\${ITEM}\"
					OUTPUT_VARIABLE FILE_TYPE
					OUTPUT_STRIP_TRAILING_WHITESPACE
				)

				if(FILE_TYPE MATCHES \"Mach-O.*executable\")
					message(STATUS \"Found executable: \${ITEM}\")
					list(APPEND EXECUTABLES \"\${ITEM}\")
				endif()
			endforeach()

			file(GLOB_RECURSE DYLIBS \"\${APP_BUNDLE}/Contents/libs/*.dylib\")

			set(ALL_BINARIES \${EXECUTABLES} \${DYLIBS})

			message(STATUS \"Generating debug symbols: \${DSYM_BUNDLE}\")
			execute_process(
				COMMAND xcrun dsymutil
				-o \"\${DSYM_BUNDLE}\"
				\${ALL_BINARIES}
				RESULT_VARIABLE DSYM_RESULT
				OUTPUT_VARIABLE DSYM_OUT
				ERROR_VARIABLE DSYM_ERR
			)

			if(NOT DSYM_RESULT EQUAL 0)
				message(FATAL_ERROR \"Failed to generate dSYM: \${DSYM_ERR}\")
			endif()
		endif()
	")

	# Make the binaries portable.
	install(CODE "
		include(BundleUtilities)

		set(APP_ROOT \"$<INSTALL_PREFIX>/ZQuest Classic.app\")
		set(BINARIES_TO_FIX
			\"\${APP_ROOT}/Contents/Resources/zlauncher\"
			\"\${APP_ROOT}/Contents/Resources/zplayer\"
			\"\${APP_ROOT}/Contents/Resources/zeditor\"
			\"\${APP_ROOT}/Contents/Resources/zscript\"
			\"\${APP_ROOT}/Contents/Resources/zupdater\"
		)
		fixup_bundle(\"\${APP_ROOT}\" \"\${BINARIES_TO_FIX}\" \"\${APP_ROOT}/Contents/Frameworks\")
	")
elseif(LINUX)
	if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
		set_property(CACHE CMAKE_INSTALL_PREFIX PROPERTY VALUE "/opt/zquestclassic")
	endif()

	set(CPACK_GENERATOR "TGZ")
elseif(WIN32)
	set(CPACK_GENERATOR "ZIP")

	# Note: on Windows the allegro 5 CMakeLists.txt totally effs up CMAKE_INSTALL_PREFIX.
	# So `--prefix` is always required to be set by the user on Windows.
endif()

get_cmake_property(CPACK_COMPONENTS_ALL COMPONENTS)
list(REMOVE_ITEM CPACK_COMPONENTS_ALL BuildOnly)
include(CPack)

# Add custom target package_zc for convenience, so the packaging command can be shown in Visual Studio.

add_custom_command(
	COMMAND "${CMAKE_CPACK_COMMAND}" -C "$<CONFIG>" -D CPACK_PACKAGE_DIRECTORY="${BUILD_FOLDER}/packages"
	WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/.."
	OUTPUT package_zc.stamp
	DEPENDS ${zc_targets}
)
add_custom_target(package_zc DEPENDS package_zc.stamp)
set_source_files_properties(package_zc.stamp PROPERTIES SYMBOLIC 1)

# These "copy_resources" commands move data files next to the generated binaries.
# TODO: delete these, and instead change ZC to look for the actual resources folder (../resources or ../../resources)
# for local development.

add_custom_command(
	COMMAND "${PYTHON}" "${CMAKE_SOURCE_DIR}/scripts/package.py" --build_folder "${BUILD_FOLDER}" --keep_existing_files
	OUTPUT copy_resources_if_not_exist.stamp
)
add_custom_target(copy_resources_if_not_exist DEPENDS copy_resources_if_not_exist.stamp)
set_source_files_properties(copy_resources_if_not_exist.stamp PROPERTIES SYMBOLIC 1)

add_custom_command(
	COMMAND "${PYTHON}" "${CMAKE_SOURCE_DIR}/scripts/package.py" --build_folder "${BUILD_FOLDER}"
	OUTPUT copy_resources.stamp
)
add_custom_target(copy_resources DEPENDS copy_resources.stamp)
set_source_files_properties(copy_resources.stamp PROPERTIES SYMBOLIC 1)

if(COPY_RESOURCES AND NOT EMSCRIPTEN)
	foreach(target IN LISTS zc_targets)
		add_dependencies(${target} copy_resources_if_not_exist)
	endforeach()
endif()
