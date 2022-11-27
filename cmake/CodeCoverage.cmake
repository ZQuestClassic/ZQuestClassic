find_program(CODE_COVERAGE_GCOVR gcovr)
if (NOT CODE_COVERAGE_GCOVR)
  message(FATAL_ERROR "Cannot find gcovr...")
endif()

set(CMAKE_CXX_FLAGS_COVERAGE "-g -O0 --coverage")

function(setup_coverage_test_target target_name output_dir test_runner)
  file(MAKE_DIRECTORY ${output_dir})
  add_custom_target(${target_name}
        COMMAND ${test_runner} ${ARGN}
        COMMAND ${CODE_COVERAGE_GCOVR} -d --html-details ${output_dir}/index.html -r ${CMAKE_SOURCE_DIR} -f ${CMAKE_SOURCE_DIR}/src --html-theme blue --html-title "Zelda Classic Coverage Report"
        COMMAND ${CMAKE_COMMAND} -E echo "Open ${output_dir}/index.html in your browser to view the coverage report."
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Generating coverage results")
endfunction()
