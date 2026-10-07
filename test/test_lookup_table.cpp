// Including Catch2
#include <catch2/catch_all.hpp>

// own headers
#include "lookup_table.hpp"

#include <iostream>

auto table_dir = std::string(DATA_DIR) + "03_testFLUT.h5";

TEST_CASE("normalizeMinMax", "[FLUT::Normalization]") {
    // Normal range
    REQUIRE(FLUT::Normalization::normalizeMinMax(0, -0.1, 0.1) == 0.5);
    REQUIRE(FLUT::Normalization::normalizeMinMax(0.7, 0, 2) == 0.35);

    // Exeptions
    // value = min
    REQUIRE(FLUT::Normalization::normalizeMinMax(-0.1, -0.1, 2) == 0);
    // value < min
    REQUIRE(FLUT::Normalization::normalizeMinMax(-0.7, 0, 2) == 0);
    // value = max
    REQUIRE(FLUT::Normalization::normalizeMinMax(2, 0, 2) == 1);
    // value > max
    REQUIRE(FLUT::Normalization::normalizeMinMax(3, 0, 2) == 1);
    // min == max
    REQUIRE(FLUT::Normalization::normalizeMinMax(1, 1, 1) == 0.0);
}

TEST_CASE("LookupTable Constructor", "[LookupTable]") {
    REQUIRE_NOTHROW(FLUT::LookupTable(table_dir, "pyflut"));

    std::vector<std::string> outputVariableNames = {};
    REQUIRE_NOTHROW(FLUT::LookupTable(table_dir, outputVariableNames, "pyflut"));

    REQUIRE_NOTHROW(FLUT::LookupTable(table_dir, {"out_1", "out_2"}, "pyflut"));

    REQUIRE_THROWS_AS(FLUT::LookupTable(table_dir, outputVariableNames, "unknownFileType"),
                      std::invalid_argument);

    REQUIRE_THROWS_AS(FLUT::LookupTable(table_dir, {"invalidArgument", "out_2"}, "pyflut"),
                      std::invalid_argument);
}

// Create an object of type LookupTable for the following tests.
struct LookupTableFixture {
    std::vector<std::string> outputVariableNames{};
    FLUT::LookupTable table{table_dir, outputVariableNames, "pyflut"};
};

TEST_CASE_METHOD(LookupTableFixture, "LookupTable.getInputVariables()", "[LookupTable]") {
    const std::vector<std::string> expected_inputVariables({"x", "y", "znew"});
    REQUIRE(table.getInputVariables() == expected_inputVariables);
}

TEST_CASE_METHOD(LookupTableFixture, "LookupTable.getAvailableOutputVariables()", "[LookupTable]") {
    const std::vector<std::string> expected_outputVariables({"out_1", "out_2", "x", "y", "z",
                                                             "znew", "xnorm", "x_max", "x_min",
                                                             "znewnorm", "znew_max", "znew_min"});
    REQUIRE(table.getAvailableOutputVariables() == expected_outputVariables);
}

TEST_CASE_METHOD(LookupTableFixture, "LookupTable.getLoadedOutputVariables()", "[LookupTable]") {
    // Load all variables
    FLUT::LookupTable table(table_dir, {}, "pyflut");
    const std::vector<std::string> expected_outputVariables({"out_1", "out_2", "x", "y", "z",
                                                             "znew", "xnorm", "x_max", "x_min",
                                                             "znewnorm", "znew_max", "znew_min"});
    REQUIRE(table.getLoadedOutputVariables() == expected_outputVariables);

    std::vector<std::string> expected_outputVariables_red = {"out_2", "out_1"};
    FLUT::LookupTable table_red_variables(table_dir, expected_outputVariables_red, "pyflut");
    REQUIRE(table_red_variables.getLoadedOutputVariables() == expected_outputVariables_red);
}

TEST_CASE_METHOD(LookupTableFixture, "LookupTable.containsOutputVariable(std::string& name)",
                 "[LookupTable]") {
    REQUIRE_NOTHROW(table.containsOutputVariable("out_1"));
    REQUIRE_THROWS_AS(table.containsOutputVariable("dummy"), std::invalid_argument);
}

TEST_CASE_METHOD(LookupTableFixture, "LookupTable.hasInitializedOutputData()", "[LookupTable]") {
    FLUT::LookupTable table(table_dir, "pyflut");
    REQUIRE(table.hasInitializedOutputData() == false);

    std::vector<std::string> redOutputVariableNames = {"out_2", "out_1"};
    REQUIRE_NOTHROW(table.readOutputVariableData(redOutputVariableNames));

    REQUIRE(table.hasInitializedOutputData() == true);
}

// Testing of the lookup inside the table
TEST_CASE_METHOD(LookupTableFixture, "LookupTable.findIndexInGrid()", "[LookupTable]") {
    // Input values in the first dimension are: (0, 0.5, 1.0)
    // Lower than lowest value
    std::vector<double> grid = {0, 0.5, 1.0};

    FLUT::DimensionBounds indexForDimension = FLUT::LookupTable::findIndexInGrid(grid, -0.1);
    REQUIRE(indexForDimension == FLUT::DimensionBounds({0, 0, 0, 0, 0}));
    // Lowest value
    indexForDimension = FLUT::LookupTable::findIndexInGrid(grid, 0.0);
    REQUIRE(indexForDimension == FLUT::DimensionBounds({0, 1, 0, 0.5, 0}));
    // Between lowest and next value
    indexForDimension = FLUT::LookupTable::findIndexInGrid(grid, 0.25);
    REQUIRE(indexForDimension == FLUT::DimensionBounds({0, 1, 0, 0.5, 0.25}));
    // Exact value
    indexForDimension = FLUT::LookupTable::findIndexInGrid(grid, 0.5);
    REQUIRE(indexForDimension == FLUT::DimensionBounds({1, 2, 0.5, 1.0, 0.5}));
    // Inbetween value
    indexForDimension = FLUT::LookupTable::findIndexInGrid(grid, 0.75);
    REQUIRE(indexForDimension == FLUT::DimensionBounds({1, 2, 0.5, 1.0, 0.75}));
    // Last index
    indexForDimension = FLUT::LookupTable::findIndexInGrid(grid, 1.0);
    REQUIRE(indexForDimension == FLUT::DimensionBounds({2, 2, 1.0, 1.0, 1.0}));
    // Higher than last index
    indexForDimension = FLUT::LookupTable::findIndexInGrid(grid, 2.0);
    REQUIRE(indexForDimension == FLUT::DimensionBounds({2, 2, 1.0, 1.0, 1.0}));

    // Input values in the 2nd dimension are: (0, 0.25, 0.5, 0.75, 1.0)
    // Lower than lowest value
    grid = {0, 0.25, 0.5, 0.75, 1.0};
    indexForDimension = FLUT::LookupTable::findIndexInGrid(grid, -0.2);
    REQUIRE(indexForDimension == FLUT::DimensionBounds({0, 0, 0.0, 0.0, 0.0}));
    // Lowest value
    indexForDimension = FLUT::LookupTable::findIndexInGrid(grid, 0.0);
    REQUIRE(indexForDimension == FLUT::DimensionBounds({0, 1, 0.0, 0.25, 0.0}));
    // Lower than value
    indexForDimension = FLUT::LookupTable::findIndexInGrid(grid, 0.3);
    REQUIRE(indexForDimension == FLUT::DimensionBounds({1, 2, 0.25, 0.5, 0.3}));
    // Exact value
    indexForDimension = FLUT::LookupTable::findIndexInGrid(grid, 0.5);
    REQUIRE(indexForDimension == FLUT::DimensionBounds({2, 3, 0.5, 0.75, 0.5}));
    // Inbetween value
    indexForDimension = FLUT::LookupTable::findIndexInGrid(grid, 0.675);
    REQUIRE(indexForDimension == FLUT::DimensionBounds({2, 3, 0.5, 0.75, 0.675}));
    // Last index
    indexForDimension = FLUT::LookupTable::findIndexInGrid(grid, 1.0);
    REQUIRE(indexForDimension == FLUT::DimensionBounds({4, 4, 1.0, 1.0, 1.0}));
    // Higher than last index
    indexForDimension = FLUT::LookupTable::findIndexInGrid(grid, 2.0);
    REQUIRE(indexForDimension == FLUT::DimensionBounds({4, 4, 1.0, 1.0, 1.0}));
}

TEST_CASE_METHOD(LookupTableFixture, "LookupTable.extractIndex()", "[LookupTable]") {
    FLUT::Hypercube hypercube;
    hypercube.bounds = {
        {2, 3, 1, 8, 5.5},
        {1, 2, 1, 8, 5.5},
        {3, 4, 1, 8, 5.5}
    };
    hypercube.indexInBuffer.resize(8);

    const uint32_t nVars = 5;
    // Arbitary dimension offset for a 2D table with dimensions: (3, 2, 5, nVars).
    const std::vector<uint32_t> dimensionOffset({nVars * 5 * 2, nVars * 5, nVars, 1});

    std::vector<uint32_t> result({140, 190, 165, 215, 145, 195, 170, 220});
    for (uint32_t i = 0; i < result.size(); i++) {
        FLUT::LookupTable::extractIndex(i, hypercube, dimensionOffset);
        REQUIRE(hypercube.indexInBuffer[i] == result[i]);
    }
}

TEST_CASE_METHOD(LookupTableFixture, "LookupTable.extractHypercubeIndices()", "[LookupTable]") {
    FLUT::Hypercube hypercube;
    hypercube.bounds = {
        {2, 3, 1, 8, 5.5},
        {1, 2, 1, 8, 5.5},
        {3, 4, 1, 8, 5.5}
    };
    hypercube.indexInBuffer.resize(8);

    const uint32_t nVars = 5;
    // Arbitary dimension offset for a 2D table with dimensions: (3, 2, 5, nVars).
    const std::vector<uint32_t> dimensionOffset({nVars * 5 * 2, nVars * 5, nVars, 1});

    std::vector<uint32_t> result({140, 190, 165, 215, 145, 195, 170, 220});
    FLUT::LookupTable::extractHypercubeIndices(hypercube, dimensionOffset);
    REQUIRE(hypercube.indexInBuffer == result);

    hypercube.bounds = {
        {2, 3, 1, 8, 5.5},
        {0, 1, 1, 8, 5.5},
        {5, 5, 1, 8, 5.5}
    };
    result = {125, 175, 150, 200, 125, 175, 150, 200};
    FLUT::LookupTable::extractHypercubeIndices(hypercube, dimensionOffset);
    REQUIRE(hypercube.indexInBuffer == result);
}

// Testing of the lookup inside the table
TEST_CASE_METHOD(LookupTableFixture, "LookupTable.normalize()", "[LookupTable]") {
    std::vector<double> inputVector({0.05, 0.4, 0.6});
    table.normalize(inputVector);
    std::vector<FLUT::DimensionBounds> normalizedInputVector = table.getBounds();
    std::vector<double> expected_result = {
        inputVector[0] / 0.1, inputVector[1],
        (inputVector[2] - (inputVector[0] + inputVector[1])) /
            ((4 + inputVector[0] + inputVector[1]) - (inputVector[0] + inputVector[1]))};

    // Extract normalized value
    std::vector<double> result(normalizedInputVector.size());
    for (int i = 0; i < result.size(); i++)
        result[i] = normalizedInputVector[i].value;

    // Check the result
    REQUIRE_THAT(result, Catch::Matchers::Approx(expected_result));
}

// Testing of the lookup inside the table
TEST_CASE_METHOD(LookupTableFixture, "LookupTable.lookup() inside the domain", "[LookupTable]") {
    std::vector<double> expected_result = {0.45, 0.17, 0.05, 0.4,    0.15, 0.6,
                                           0.5,  0.1,  0.0,  0.0375, 4.45, 0.45};
    INFO("Check lookup inside the table");
    REQUIRE_THAT(table.lookup({0.05, 0.4, 0.6}), Catch::Matchers::Approx(expected_result));

    // Reduce number of output variables
    std::vector<std::string> redOutputVariableNames = {"out_1", "out_2", "x", "y", "z"};
    table.readOutputVariableData(redOutputVariableNames);
    // Different lookup values inside the domain
    std::vector<std::vector<double>> inputVectors({
        { 0.0, 0.0,  0.0},
        { 0.0, 0.0,  2.0},
        { 0.0, 0.0,  4.0},
        { 0.0, 0.5,  0.5},
        { 0.0, 0.5,  2.5},
        { 0.0, 0.5,  4.5},
        { 0.0, 1.0,  1.0},
        { 0.0, 1.0,  3.0},
        { 0.0, 1.0,  5.0},
        {0.05, 0.0, 0.05},
        {0.05, 0.0, 2.05},
        {0.05, 0.0, 4.05},
        {0.05, 0.5, 0.55},
        {0.05, 0.5, 2.55},
        {0.05, 0.5, 4.55},
        {0.05, 1.0, 1.05},
        {0.05, 1.0, 3.05},
        {0.05, 1.0, 5.05},
        { 0.1, 0.0,  0.1},
        { 0.1, 0.0,  2.1},
        { 0.1, 0.0,  4.1},
        { 0.1, 0.5,  0.6},
        { 0.1, 0.5,  2.6},
        { 0.1, 0.5,  4.6},
        { 0.1, 1.0,  1.1},
        { 0.1, 1.0,  3.1},
        { 0.1, 1.0,  5.1}
    });
    for (auto inputVector : inputVectors) {
        // Calculation formulars see write_test_table.ipynb.
        std::vector<double> expected_result = {
            inputVector[0] + inputVector[1], // out_1 = x + y
            inputVector[0] * inputVector[1] +
                (inputVector[2] - inputVector[0] -
                 inputVector[1]), // out_2 = x * y + z = x * y + (z_new - x - y)
            inputVector[0],       // x
            inputVector[1],       // y
            inputVector[2] - inputVector[0] - inputVector[1] // znew
        };
        REQUIRE_THAT(table.lookup(inputVector), Catch::Matchers::Approx(expected_result));
    }
}

// Testing of edge lookup of the table in different variations
TEST_CASE_METHOD(LookupTableFixture, "LookupTable.lookup() at edges of table", "[LookupTable]") {
    std::vector<double> epsilons({0.0000001, 0.001, 0.1, 1.0, 100});
    for (auto epsilon : epsilons) {
        // Lower than edge of x
        REQUIRE_THAT(table.lookup({0.0, 0.4, 0.6}),
                     Catch::Matchers::Approx(table.lookup({0.0 - epsilon, 0.4, 0.6})));
        // Higher than edge of x
        REQUIRE_THAT(table.lookup({0.1, 0.4, 0.6}),
                     Catch::Matchers::Approx(table.lookup({0.1 + epsilon, 0.4, 0.6})));
        // Lower than edge of y
        REQUIRE_THAT(table.lookup({0.05, 0.0, 2.0}),
                     Catch::Matchers::Approx(table.lookup({0.05, 0.0 - epsilon, 2.0})));
        // Higher than edge of y
        REQUIRE_THAT(table.lookup({0.05, 1.0, 2.0}),
                     Catch::Matchers::Approx(table.lookup({0.05, 1.0 + epsilon, 2.0})));
        // Lower than edge of z
        REQUIRE_THAT(table.lookup({0.05, 0.4, 0.45}),
                     Catch::Matchers::Approx(table.lookup({0.05, 0.4, 0.45 - epsilon})));
        // Higher than edge of z
        REQUIRE_THAT(table.lookup({0.05, 0.4, 4.45}),
                     Catch::Matchers::Approx(table.lookup({0.05, 0.4, 4.45 + epsilon})));
        // Higher than edge of x and lower than edge of z
        REQUIRE_THAT(table.lookup({0.1, 0.4, 0.5}),
                     Catch::Matchers::Approx(table.lookup({0.1 + epsilon, 0.4, 0.5 - epsilon})));
        // Lower than edge of x and higher than edge of z
        REQUIRE_THAT(table.lookup({0.0, 0.4, 4.4}),
                     Catch::Matchers::Approx(table.lookup({0.0 - epsilon, 0.4, 4.4 + epsilon})));
    }
}

TEST_CASE_METHOD(LookupTableFixture, "LookupTable.lookup() with empty SharedMemoryData object",
                 "[LookupTable]") {
    FLUT::LookupTable table_no_output_data(table_dir, "pyflut");
    REQUIRE_THROWS_AS(table_no_output_data.lookup({0.0, 0.0, 0.0}), std::runtime_error);
}

TEST_CASE_METHOD(LookupTableFixture, "LookupTable.readOutputVariableData()", "[LookupTable]") {
    std::vector<std::string> redOutputVariableNames = {"out_2", "out_1"};
    REQUIRE_NOTHROW(table.readOutputVariableData(redOutputVariableNames));
    REQUIRE(table.getLoadedOutputVariables() == redOutputVariableNames);

    std::vector<double> expected_result = {0.14, 0.5};
    REQUIRE_THAT(table.lookup({0.3, 0.4, 0.6}), Catch::Matchers::Approx(expected_result));

    REQUIRE_THROWS_AS(table.readOutputVariableData({"invalidArgument", "out_2"}),
                      std::invalid_argument);
}
