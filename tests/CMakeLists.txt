find_package(GTest REQUIRED)

include_directories(${PROJECT_SOURCE_DIR}/include)

add_library(common_lib STATIC
    ${PROJECT_SOURCE_DIR}/src/common/elias_fano.cpp
    ${PROJECT_SOURCE_DIR}/src/common/golomb_rice.cpp
)
target_include_directories(common_lib PUBLIC ${PROJECT_SOURCE_DIR}/include)

add_executable(test_elias_fano common/elias_fano.test.cpp)
target_link_libraries(test_elias_fano PRIVATE common_lib GTest::GTest GTest::Main)
add_test(NAME TestEliasFano COMMAND test_elias_fano)
add_custom_target(run_test_elias_fano COMMAND test_elias_fano)

add_executable(test_golomb_rice common/golomb_rice.test.cpp)
target_link_libraries(test_golomb_rice PRIVATE common_lib GTest::GTest GTest::Main)
add_test(NAME TestGolombRice COMMAND test_golomb_rice)
add_custom_target(run_golomb_rice COMMAND test_golomb_rice)

add_custom_target(run_all_tests COMMAND ctest)