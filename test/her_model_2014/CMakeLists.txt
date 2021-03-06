include_directories("${CMAKE_CURRENT_SOURCE_DIR}")

add_subdirectory("${DENSE_SOURCE_DIR}/source" "${CMAKE_CURRENT_BINARY_DIR}/source")


DETERMINISTIC_SIMULATION(her2014_simulation ${CMAKE_CURRENT_SOURCE_DIR})


set(test_cmd ${CMAKE_CURRENT_BINARY_DIR}/her2014_simulation)
set(test_args "--param-sets ${CMAKE_CURRENT_SOURCE_DIR}/param_sets.csv --cell-total 10 --tissue-width 5 --time-total 3000 --anlys-intvl .01 --step-size .01 --analysis ${CMAKE_CURRENT_SOURCE_DIR}/analyses.xml")
add_test( "her_test"
 ${CMAKE_COMMAND}
 -D test_cmd=${test_cmd}
 -D test_args:string=${test_args}
 -D output_blessed=${CMAKE_CURRENT_SOURCE_DIR}/anlys_basic_a.csv
 -D output_test=${CMAKE_CURRENT_BINARY_DIR}/anlys_basic_a.csv
 -P ${CMAKE_SOURCE_DIR}/test/her_model_2014/run_test.cmake
)

set(cell_width_test_args "--param-sets ${CMAKE_CURRENT_SOURCE_DIR}/param_sets.csv --cell-total 10 --tissue-width 1 --time-total 3000 --anlys-intvl .01 --step-size .01 --analysis ${CMAKE_CURRENT_SOURCE_DIR}/analyses.xml")
add_test( "cell_width_test"
 ${CMAKE_COMMAND}
 -D test_cmd=${test_cmd}
 -D test_args:string=${cell_width_test_args}
 -D output_blessed=${CMAKE_CURRENT_SOURCE_DIR}/anlys_basic_a.csv
 -D output_test=${CMAKE_CURRENT_BINARY_DIR}/anlys_basic_a.csv
 -P ${CMAKE_SOURCE_DIR}/test/her_model_2014/run_test.cmake
 )

set(cell_width_test_01_args "--param-sets ${CMAKE_CURRENT_SOURCE_DIR}/param_sets.csv --cell-total 50 --tissue-width 10 --time-total 3000 --anlys-intvl .01 --step-size .01 --analysis ${CMAKE_CURRENT_SOURCE_DIR}/analyses.xml")
add_test( "cell_width_test_01"
 ${CMAKE_COMMAND}
 -D test_cmd=${test_cmd}
 -D test_args:string=${cell_width_test_01_args}
 -D output_blessed=${CMAKE_CURRENT_SOURCE_DIR}/cell_width_test_anlys.csv
 -D output_test=${CMAKE_CURRENT_BINARY_DIR}/anlys_basic_a.csv
 -P ${CMAKE_SOURCE_DIR}/test/her_model_2014/run_test.cmake
 )


set(cell_width_test_02_args "--param-sets ${CMAKE_CURRENT_SOURCE_DIR}/param_sets.csv --cell-total 100 --tissue-width 10 --time-total 3000 --anlys-intvl .01 --step-size .01 --analysis ${CMAKE_CURRENT_SOURCE_DIR}/analyses.xml")
add_test( "cell_width_test_02"
 ${CMAKE_COMMAND}
 -D test_cmd=${test_cmd}
 -D test_args:string=${cell_width_test_02_args}
 -D output_blessed=${CMAKE_CURRENT_SOURCE_DIR}/cell_width_test_anlys.csv
 -D output_test=${CMAKE_CURRENT_BINARY_DIR}/anlys_basic_a.csv
 -P ${CMAKE_SOURCE_DIR}/test/her_model_2014/run_test.cmake
 )


set(cell_number_test_args "--param-sets ${CMAKE_CURRENT_SOURCE_DIR}/param_sets.csv --cell-total 50 --tissue-width 5 --time-total 3000 --anlys-intvl .01 --step-size .01 --analysis ${CMAKE_CURRENT_SOURCE_DIR}/analyses.xml")
add_test( "cell_number_test"
 ${CMAKE_COMMAND}
 -D test_cmd=${test_cmd}
 -D test_args:string=${cell_number_test_args}
 -D output_blessed=${CMAKE_CURRENT_SOURCE_DIR}/anlys_basic_a.csv
 -D output_test=${CMAKE_CURRENT_BINARY_DIR}/anlys_basic_a.csv
 -P ${CMAKE_SOURCE_DIR}/test/her_model_2014/run_test.cmake
 )

set(cell_number_test_01_args "--param-sets ${CMAKE_CURRENT_SOURCE_DIR}/param_sets.csv --cell-total 200 --tissue-width 5 --time-total 3000 --anlys-intvl .01 --step-size .01 --analysis ${CMAKE_CURRENT_SOURCE_DIR}/analyses.xml")
add_test( "cell_number_test_01"
 ${CMAKE_COMMAND}
 -D test_cmd=${test_cmd}
 -D test_args:string=${cell_number_test_01_args}
 -D output_blessed=${CMAKE_CURRENT_SOURCE_DIR}/anlys_basic_a.csv
 -D output_test=${CMAKE_CURRENT_BINARY_DIR}/anlys_basic_a.csv
 -P ${CMAKE_SOURCE_DIR}/test/her_model_2014/run_test.cmake
 )


set(time_total_test_args "--param-sets ${CMAKE_CURRENT_SOURCE_DIR}/param_sets.csv --cell-total 10 --tissue-width 5 --time-total 300 --anlys-intvl .01 --step-size .01 --analysis ${CMAKE_CURRENT_SOURCE_DIR}/analyses.xml")
add_test( "time_total_test"
 ${CMAKE_COMMAND}
 -D test_cmd=${test_cmd}
 -D test_args:string=${time_total_test_args}
 -D output_blessed=${CMAKE_CURRENT_SOURCE_DIR}/total_time_test_anlys.csv
 -D output_test=${CMAKE_CURRENT_BINARY_DIR}/anlys_basic_a.csv
 -P ${CMAKE_SOURCE_DIR}/test/her_model_2014/run_test.cmake
 )

set(time_total_test_01_args "--param-sets ${CMAKE_CURRENT_SOURCE_DIR}/param_sets.csv --cell-total 10 --tissue-width 5 --time-total 5000 --anlys-intvl .01 --step-size .01 --analysis ${CMAKE_CURRENT_SOURCE_DIR}/analyses.xml")
add_test( "time_total_test_01"
 ${CMAKE_COMMAND}
 -D test_cmd=${test_cmd}
 -D test_args:string=${time_total_test_01_args}
 -D output_blessed=${CMAKE_CURRENT_SOURCE_DIR}/total_time_test_01_anlys.csv
 -D output_test=${CMAKE_CURRENT_BINARY_DIR}/anlys_basic_a.csv
 -P ${CMAKE_SOURCE_DIR}/test/her_model_2014/run_test.cmake
 )



if (${CUDA_FOUND})
  CUDA_SIMULATION(her2014_cuda_simulation ${CMAKE_CURRENT_SOURCE_DIR})
endif (${CUDA_FOUND})

