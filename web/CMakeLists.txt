set(package_web_resources_gen_files
    ${BUILD_FOLDER}/zplayer.data
    ${BUILD_FOLDER}/zeditor.data
)
add_custom_command(
    OUTPUT ${package_web_resources_gen_files}
    COMMAND ${PYTHON} ${CMAKE_CURRENT_SOURCE_DIR}/../scripts/package.py --build_folder "${BUILD_FOLDER}" --cfg_os web
    DEPENDS
        ../scripts/package.py
    VERBATIM
)
add_custom_target(package_web_resources DEPENDS ${package_web_resources_gen_files})

set(main_bundle_command
    npm install --quiet && npx esbuild --bundle main.js --outfile=${BUILD_FOLDER}/main.js --sourcemap
    $<$<NOT:$<CONFIG:Debug>>:--minify>
)
add_custom_command(
    OUTPUT ${BUILD_FOLDER}/main.js
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMAND ${main_bundle_command}
    COMMAND_EXPAND_LISTS
    DEPENDS
        file_launch.js
        fsfs.js
        main.js
        package-lock.json
        package.json
        settings.js
        utils.js
        zasm.js
    VERBATIM
)
add_custom_target(main_bundle DEPENDS ${BUILD_FOLDER}/main.js)
add_dependencies(main_bundle zscript)

function (create_web_target target)
    add_custom_command(
        OUTPUT ${BUILD_FOLDER}/${target}.html
        COMMAND ${PYTHON} -Xutf8 ${CMAKE_CURRENT_SOURCE_DIR}/../scripts/create_html.py --build_folder "${BUILD_FOLDER}" --target ${target}
        DEPENDS
            ${CMAKE_CURRENT_SOURCE_DIR}/../scripts/create_html.py
            index.html
            styles.css
            package_web_resources
            ${package_web_resources_gen_files}
        VERBATIM
    )
    add_custom_target(html_${target} DEPENDS ${BUILD_FOLDER}/${target}.html)

    # For packaging just this target.
    add_custom_command(
        OUTPUT web_${target}.stamp
        COMMAND ${PYTHON} ${CMAKE_CURRENT_SOURCE_DIR}/../scripts/package_web.py --build_folder "${BUILD_FOLDER}" --target ${target}
        VERBATIM
    )
    add_custom_target(web_${target} DEPENDS web_${target}.stamp)
    add_dependencies(web_${target} html_${target} ${target} main_bundle)
    set_source_files_properties(web_${target}.stamp PROPERTIES SYMBOLIC 1)
endfunction()

create_web_target(zplayer)
create_web_target(zeditor)

# For packaging all web targets.
add_custom_command(
    OUTPUT web.stamp
    COMMAND ${PYTHON} ${CMAKE_CURRENT_SOURCE_DIR}/../scripts/package_web.py --build_folder "${BUILD_FOLDER}"
    VERBATIM
)
add_custom_target(web DEPENDS web.stamp)
add_dependencies(web zplayer html_zplayer zeditor html_zeditor zscript main_bundle)
set_source_files_properties(web.stamp PROPERTIES SYMBOLIC 1)
