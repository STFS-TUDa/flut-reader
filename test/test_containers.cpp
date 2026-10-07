// Including Catch2
#include <catch2/catch_all.hpp>

// own headers
#include "containers.hpp"

#include <iostream>

auto table_dir = std::string(DATA_DIR) + "03_testFLUT.h5";

// Test cases for InputDict
TEST_CASE("InputDict Constructor with numVars", "[InputDict]") {
    // This constructor initalizes a inputDict with a specific number of variables,
    // i.e. it sets the size of the vectors holding information on the input-layer to numVars
    size_t numVars = 5;
    FLUT::InputDict input_dict(numVars);

    REQUIRE(input_dict.names.size() == numVars);
    REQUIRE(input_dict.values.size() == numVars);
    REQUIRE(input_dict.normalizedDims.size() == numVars);
    REQUIRE(input_dict.normalizationSettings.empty());
}

TEST_CASE("InputDict Constructor from hdf5 pyflut file", "[InputDict]") {
    // Create an InputDict using the file-based constructor
    FLUT::InputDict input_dict(table_dir, "pyflut");

    // Check name read in.
    const std::vector<std::string> expected_names({"x", "y", "znew"});
    REQUIRE(input_dict.names == expected_names);
    // Check value read in.
    const std::vector<std::vector<double>> expected_values = {
        {0, 0.5, 1},
        {0, 1},
        {0, 0.25, 0.5, 0.75, 1}
    };
    for (int i = 0; i < input_dict.values.size(); i++)
        REQUIRE(input_dict.values[i] == expected_values[i]);

    // Check normalization.
    const std::vector<bool> expected_normalizedDims({true, false, true});
    REQUIRE(input_dict.normalizedDims == expected_normalizedDims);

    const std::unordered_map<std::string, std::unordered_map<std::string, std::string>>
        expected_normalizedSettings({
            {   "x",
             {{"Normalization_type", "default"},
             {"Normalized_name", "xnorm"},
             {"Physical_name", "x"},
             {"Max_name_normalization", "x_max"},
             {"Min_name_normalization", "x_min"}}   },
            {"znew",
             {{"Normalization_type", "default"},
             {"Normalized_name", "znewnorm"},
             {"Max_name_normalization", "znew_max"},
             {"Min_name_normalization", "znew_min"}}}
    });
    for (auto &v : expected_normalizedSettings) {
        std::string variableName = v.first;
        auto variableMap = v.second;
        for (auto &entry : variableMap)
            REQUIRE(input_dict.normalizationSettings[variableName][entry.first] == entry.second);
    }

    // Test of the read-in of the normalization values.
    auto &normSpec0 = std::get<FLUT::DefaultNormSpec>(input_dict.normSpec[0]);
    auto &normSpec2 = std::get<FLUT::DefaultNormSpec>(input_dict.normSpec[2]);

    std::vector<double> expectedNormalizationValues({0.0, 0.1});
    REQUIRE(normSpec0.normValues.values == expectedNormalizationValues);
    expectedNormalizationValues = {0.0, 4.0, 1.0, 5.0, 0.05, 4.05, 1.05, 5.05, 0.1, 4.1, 1.1, 5.1};
    REQUIRE(normSpec2.normValues.values == expectedNormalizationValues);

    std::vector<uint32_t> expectedOffset = {1};
    REQUIRE(normSpec0.normValues.dimensionOffsets == expectedOffset);
    expectedOffset = {4, 2, 1};
    REQUIRE(normSpec2.normValues.dimensionOffsets == expectedOffset);

    std::vector<size_t> expectedSize = {2};
    REQUIRE(normSpec0.normValues.dimensionSizes == expectedSize);
    expectedSize = {3, 2, 2};
    REQUIRE(normSpec2.normValues.dimensionSizes == expectedSize);
}

TEST_CASE("InputDict constructor invalid file type", "[InputDict]") {
    REQUIRE_THROWS_AS(FLUT::InputDict(table_dir, "unknownFileType"), std::invalid_argument);
}

TEST_CASE("InputDict methods", "[InputDict]") {
    FLUT::InputDict input_dict(table_dir, "pyflut");

    // Check class methods
    REQUIRE(input_dict.size() == 3);

    const std::vector<bool> expected_normalizedDims({true, false, true});
    for (int i = 0; i < input_dict.size(); i++)
        REQUIRE(input_dict.isNormalized(i) == expected_normalizedDims[i]);
}

//
// Test cases for OutputDict
//
TEST_CASE("OutputDict constructor from map", "[OutputDict]") {
    std::unordered_map<std::string, size_t> testMap = {
        {"var0", 0},
        {"var1", 1},
        {"var2", 3},
        {"var3", 2}
    };
    FLUT::OutputDict output_dict(testMap);

    REQUIRE(output_dict.outputDict == testMap);
}

TEST_CASE("OutputDict constructor from pyflut hdf5-file", "[OutputDict]") {
    std::unordered_map<std::string, size_t> expected_output_dict = {
        {   "out_1",  0},
        {   "out_2",  1},
        {       "x",  2},
        {       "y",  3},
        {       "z",  4},
        {    "znew",  5},
        {   "xnorm",  6},
        {   "x_max",  7},
        {   "x_min",  8},
        {"znewnorm",  9},
        {"znew_max", 10},
        {"znew_min", 11}
    };
    FLUT::OutputDict output_dict(table_dir, "pyflut");

    REQUIRE(output_dict.outputDict == expected_output_dict);
}

TEST_CASE("OutputDict constructor with invalid file format", "[OutputDict]") {
    REQUIRE_THROWS_AS(FLUT::OutputDict(table_dir, "unknownFileType"), std::invalid_argument);
}

TEST_CASE("OutputDict methods", "[OutputDict]") {
    std::unordered_map<std::string, size_t> testMap = {
        {"var0", 0},
        {"var1", 1},
        {"var2", 3},
        {"var3", 2}
    };
    FLUT::OutputDict output_dict(testMap);

    REQUIRE(output_dict.outputDict == testMap);

    const std::vector<std::string> expected_names = {"var0", "var1", "var3", "var2"};
    REQUIRE(output_dict.getNames() == expected_names);

    REQUIRE(output_dict.getIndex("var1") == 1);
    REQUIRE(output_dict.getIndex("var2") == 3);
    REQUIRE(output_dict.getIndex("var3") == 2);

    REQUIRE_NOTHROW(output_dict.containsVariables({"var1", "var2", "var3"}));
    REQUIRE_NOTHROW(output_dict.containsVariables({"var1", "var3"}));
    REQUIRE_THROWS_AS(output_dict.containsVariables({"var1", "dummy"}), std::invalid_argument);
}

//
// DataSharedMemory tests.
//
TEST_CASE("DataSharedMemory Constructor from pyflut hdf5-file; no outputVariables given",
          "[DataSharedMemory]") {
    std::vector<std::string> outputVariableNames = {};

    // Construct with all input variables
    FLUT::DataSharedMemory data_object(table_dir, {}, "pyflut");

    std::vector<std::string> expected_variableNames = {"out_1", "out_2",    "x",        "y",
                                                       "z",     "znew",     "xnorm",    "x_max",
                                                       "x_min", "znewnorm", "znew_max", "znew_min"};
    REQUIRE(data_object.variableNames == expected_variableNames);
    REQUIRE(data_object.nVariables() == expected_variableNames.size());

    // The dimension offset is the number of entries that needs to be skipped
    // The dimension of the test data is: (3, 2, 5, 11).
    size_t nVars = data_object.nVariables();
    std::vector<size_t> expected_dimensionOffsets({nVars * 5 * 2, nVars * 5, nVars, 1});
    for (int i = 0; i < expected_dimensionOffsets.size(); i++)
        REQUIRE(data_object.dimensionOffset(i) == expected_dimensionOffsets[i]);

    std::vector<double> expected_data = {
        0.0,  0.0,  0.0,  0.0,  0.0,  0.0,  0.0,  0.1,  0.0,  0.0,  4.0,  0.0,  0.0,  1.0,  0.0,
        0.0,  1.0,  1.0,  0.0,  0.1,  0.0,  0.25, 4.0,  0.0,  0.0,  2.0,  0.0,  0.0,  2.0,  2.0,
        0.0,  0.1,  0.0,  0.5,  4.0,  0.0,  0.0,  3.0,  0.0,  0.0,  3.0,  3.0,  0.0,  0.1,  0.0,
        0.75, 4.0,  0.0,  0.0,  4.0,  0.0,  0.0,  4.0,  4.0,  0.0,  0.1,  0.0,  1.0,  4.0,  0.0,
        1.0,  0.0,  0.0,  1.0,  0.0,  1.0,  0.0,  0.1,  0.0,  0.0,  5.0,  1.0,  1.0,  1.0,  0.0,
        1.0,  1.0,  2.0,  0.0,  0.1,  0.0,  0.25, 5.0,  1.0,  1.0,  2.0,  0.0,  1.0,  2.0,  3.0,
        0.0,  0.1,  0.0,  0.5,  5.0,  1.0,  1.0,  3.0,  0.0,  1.0,  3.0,  4.0,  0.0,  0.1,  0.0,
        0.75, 5.0,  1.0,  1.0,  4.0,  0.0,  1.0,  4.0,  5.0,  0.0,  0.1,  0.0,  1.0,  5.0,  1.0,
        0.05, 0.0,  0.05, 0.0,  0.0,  0.05, 0.5,  0.1,  0.0,  0.0,  4.05, 0.05, 0.05, 1.0,  0.05,
        0.0,  1.0,  1.05, 0.5,  0.1,  0.0,  0.25, 4.05, 0.05, 0.05, 2.0,  0.05, 0.0,  2.0,  2.05,
        0.5,  0.1,  0.0,  0.5,  4.05, 0.05, 0.05, 3.0,  0.05, 0.0,  3.0,  3.05, 0.5,  0.1,  0.0,
        0.75, 4.05, 0.05, 0.05, 4.0,  0.05, 0.0,  4.0,  4.05, 0.5,  0.1,  0.0,  1.0,  4.05, 0.05,
        1.05, 0.05, 0.05, 1.0,  0.0,  1.05, 0.5,  0.1,  0.0,  0.0,  5.05, 1.05, 1.05, 1.05, 0.05,
        1.0,  1.0,  2.05, 0.5,  0.1,  0.0,  0.25, 5.05, 1.05, 1.05, 2.05, 0.05, 1.0,  2.0,  3.05,
        0.5,  0.1,  0.0,  0.5,  5.05, 1.05, 1.05, 3.05, 0.05, 1.0,  3.0,  4.05, 0.5,  0.1,  0.0,
        0.75, 5.05, 1.05, 1.05, 4.05, 0.05, 1.0,  4.0,  5.05, 0.5,  0.1,  0.0,  1.0,  5.05, 1.05,
        0.1,  0.0,  0.1,  0.0,  0.0,  0.1,  1.0,  0.1,  0.0,  0.0,  4.1,  0.1,  0.1,  1.0,  0.1,
        0.0,  1.0,  1.1,  1.0,  0.1,  0.0,  0.25, 4.1,  0.1,  0.1,  2.0,  0.1,  0.0,  2.0,  2.1,
        1.0,  0.1,  0.0,  0.5,  4.1,  0.1,  0.1,  3.0,  0.1,  0.0,  3.0,  3.1,  1.0,  0.1,  0.0,
        0.75, 4.1,  0.1,  0.1,  4.0,  0.1,  0.0,  4.0,  4.1,  1.0,  0.1,  0.0,  1.0,  4.1,  0.1,
        1.1,  0.1,  0.1,  1.0,  0.0,  1.1,  1.0,  0.1,  0.0,  0.0,  5.1,  1.1,  1.1,  1.1,  0.1,
        1.0,  1.0,  2.1,  1.0,  0.1,  0.0,  0.25, 5.1,  1.1,  1.1,  2.1,  0.1,  1.0,  2.0,  3.1,
        1.0,  0.1,  0.0,  0.5,  5.1,  1.1,  1.1,  3.1,  0.1,  1.0,  3.0,  4.1,  1.0,  0.1,  0.0,
        0.75, 5.1,  1.1,  1.1,  4.1,  0.1,  1.0,  4.0,  5.1,  1.0,  0.1,  0.0,  1.0,  5.1,  1.1};
    for (int i = 0; i < expected_data.size(); i++)
        REQUIRE_THAT(data_object.buffer[i], Catch::Matchers::WithinAbs(expected_data[i], 1e-8));
}

TEST_CASE("DataSharedMemory Constructor from pyflut hdf5-file; with outputVariables given",
          "[DataSharedMemory]") {
    // Note: The order of the variables is also changed in order to
    // test if the reodering of variables is working properly.

    std::vector<std::string> expected_variableNames = {"y", "out_2", "x", "out_1"};
    FLUT::DataSharedMemory data_object(table_dir, expected_variableNames, "pyflut");
    REQUIRE(data_object.variableNames == expected_variableNames);
    REQUIRE(data_object.nVariables() == expected_variableNames.size());

    // The dimension offset is the number of entries that needs to be skipped
    // The dimension of the test data is: (3, 2, 5, 11).
    size_t nVars = data_object.nVariables();
    std::vector<size_t> expected_dimensionOffsets = {nVars * 5 * 2, nVars * 5, nVars, 1};
    for (int i = 0; i < expected_dimensionOffsets.size(); i++)
        REQUIRE(data_object.dimensionOffset(i) == expected_dimensionOffsets[i]);

    // Test the actual data read-in.
    std::vector<double> expected_data = {
        0.0,  0.0,  0.0,  0.0,  0.0,  1.0,  0.0,  0.0,  0.0, 2.0,  0.0,  0.0,  0.0,  3.0,  0.0,
        0.0,  0.0,  4.0,  0.0,  0.0,  1.0,  0.0,  0.0,  1.0, 1.0,  1.0,  0.0,  1.0,  1.0,  2.0,
        0.0,  1.0,  1.0,  3.0,  0.0,  1.0,  1.0,  4.0,  0.0, 1.0,  0.0,  0.0,  0.05, 0.05, 0.0,
        1.0,  0.05, 0.05, 0.0,  2.0,  0.05, 0.05, 0.0,  3.0, 0.05, 0.05, 0.0,  4.0,  0.05, 0.05,
        1.0,  0.05, 0.05, 1.05, 1.0,  1.05, 0.05, 1.05, 1.0, 2.05, 0.05, 1.05, 1.0,  3.05, 0.05,
        1.05, 1.0,  4.05, 0.05, 1.05, 0.0,  0.0,  0.1,  0.1, 0.0,  1.0,  0.1,  0.1,  0.0,  2.0,
        0.1,  0.1,  0.0,  3.0,  0.1,  0.1,  0.0,  4.0,  0.1, 0.1,  1.0,  0.1,  0.1,  1.1,  1.0,
        1.1,  0.1,  1.1,  1.0,  2.1,  0.1,  1.1,  1.0,  3.1, 0.1,  1.1,  1.0,  4.1,  0.1,  1.1};
    for (int i = 0; i < expected_data.size(); i++)
        REQUIRE_THAT(data_object.buffer[i], Catch::Matchers::WithinAbs(expected_data[i], 1e-8));
}

TEST_CASE("DataSharedMemory constructor with invalid file format", "[DataSharedMemory]") {
    REQUIRE_THROWS_AS(FLUT::DataSharedMemory(table_dir, {}, "unknownFileType"),
                      std::invalid_argument);
}
