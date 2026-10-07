# Fetch Catch2 for testing
# FetchContent module for Catch2
include(FetchContent)

# Fetch Catch2
FetchContent_Declare(
    catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG        v3.12.0 # You can specify a version tag or branch if needed
)

# Make Catch2 available for your project
FetchContent_MakeAvailable(catch2)

# Include source directories of build
include_directories(${PROJECT_SOURCE_DIR}/src)

# Add a static library or object file for the shared utility code.
add_library(test_utility STATIC test_utility.cpp)

foreach(
    test
    test_containers
    test_interpolation
    test_lookup_table
    test_lookup_table_csv
)
    # Set the name of your test executable. You can change the name as needed.
    set(TEST_NAME ${test})
    add_executable(${TEST_NAME} ${TEST_NAME}.cpp)
    target_link_libraries(${TEST_NAME}
        ${PROJECT_NAME}
        Catch2::Catch2WithMain
    )

    # If the test requires the utility functions, link with test_utility.
    if(${TEST_NAME} STREQUAL "test_lookup_table_csv")
        target_link_libraries(${TEST_NAME} test_utility)
    endif()

    # Register the test with CTest.
    # The COMMAND can either run the test executable directly or call a script that sets up the test environment.
    # If you are using a script, replace `${CMAKE_CURRENT_SOURCE_DIR}/Alltest` with the path to your script.
    add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME} --reporter junit -o test-results-${TEST_NAME}.xml)
endforeach()

foreach(
    benchmark
    benchmark_lookup_table
    benchmark_interpolate
)
    # Add benchmark for the lookup table
    add_executable(${benchmark}
            ${benchmark}.cpp
            ../src/interpolator.cpp
            ../src/lookup_table.cpp
            ../src/containers.cpp
            test_utility.cpp
    )
    target_include_directories(${benchmark} PRIVATE ${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/test)
    # librt (POSIX realtime extensions) is Linux-only; on macOS those symbols are part of libc
    if(APPLE)
        target_link_libraries(${benchmark} Catch2::Catch2WithMain HighFive)
    else()
        target_link_libraries(${benchmark} Catch2::Catch2WithMain HighFive -lrt)
    endif()
    target_compile_options(${benchmark} PRIVATE -O3)
endforeach()
