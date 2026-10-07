// Including Catch2
#include "interpolator.hpp"
#include "lookup_table.hpp"
#include "test_utility.hpp"

#include <catch2/catch_all.hpp>
#include <fstream>
#include <random>
#include <vector>

auto one_dim_table_cc = std::string(DATA_DIR) + "00_1D-FLUT-cc.h5";
auto two_dim_table_cc = std::string(DATA_DIR) + "01_2D-FLUT-cc.h5";
auto three_dim_table_cc = std::string(DATA_DIR) + "02_3D-FLUT-cc.h5";
auto three_dim_csv_cc = std::string(DATA_DIR) + "results_3D-FLUT.csv";

std::vector<std::vector<double>> getInputVectorFromCSV(FLUT::LookupTable &table) {
    // Define input
    std::fstream csvFile(three_dim_csv_cc);
    std::string line;

    // Create a map with indexes
    std::map<std::string, int> indexes = getIndexMap(csvFile);

    // Read rows
    std::vector<std::vector<double>> inputVectors;
    size_t rowIndex = 0;
    while (std::getline(csvFile, line)) {
        rowIndex++;
        std::string value;
        std::stringstream ss(line);
        std::vector<double> row;
        // Add individual columns of a row to a vector
        while (std::getline(ss, value, ',')) {
            row.push_back(std::stod(value));
        }

        // Add input values from row vector to inputValues vector
        std::vector<double> vec;
        for (const auto &name : table.getInputVariables()) {
            vec.push_back(row[indexes[name]]);
        }
        inputVectors.push_back(vec);
    }
    return inputVectors;
}

void BenchmarkTable(std::string &tableName, const bool testRedVariables = true) {
    FLUT::LookupTable table(tableName, {}, "pyflut");
    auto inputVectors = getInputVectorFromCSV(table);

    auto &inputVector = inputVectors[0];
    BENCHMARK("normalize " + std::to_string(inputVectors.size()) + "x with single inputs") {
        for (int i = 0; i < inputVectors.size(); i++)
            table.normalize(inputVector);
    };

    table.normalize(inputVector);
    auto &dimensionOffset = table.getDimensionOffset();
    auto &hypercube = table.getHypercube();
    BENCHMARK("extractHypercubeIndices " + std::to_string(inputVectors.size()) +
              "x with single inputs") {
        for (int i = 0; i < inputVectors.size(); i++)
            table.extractHypercubeIndices(hypercube, dimensionOffset);
    };

    table.normalize(inputVector);
    table.extractHypercubeIndices(hypercube, dimensionOffset);
    std::vector<double> result(table.getLoadedOutputVariables().size());
    BENCHMARK("interpolate " + std::to_string(inputVectors.size()) + "x with single inputs") {
        for (int i = 0; i < inputVectors.size(); i++)
            FLUT::Interpolator::interpolate(hypercube.bounds.size(), hypercube, table.getBuffer(),
                                            result);
    };

    BENCHMARK("Lookup " + std::to_string(inputVectors.size()) + "x with single inputs") {
        for (int i = 0; i < inputVectors.size(); i++)
            table.lookup(inputVectors[0]);
    };

    BENCHMARK("Lookup " + std::to_string(result.size()) + " variables for " +
              std::to_string(inputVectors.size()) + " inputs") {
        for (auto &inputVector : inputVectors)
            table.lookup(inputVector);
    };

    if (testRedVariables) {
        table.readOutputVariableData({"rho", "visc", "omega_yc", "T", "alpha"});
        BENCHMARK("Lookup 5 variables for " + std::to_string(inputVectors.size()) + " inputs") {
            for (auto &inputVector : inputVectors)
                table.lookup(inputVector);
        };

        table.readOutputVariableData({"alpha"});
        BENCHMARK("Lookup 1 variable for " + std::to_string(inputVectors.size()) + " inputs") {
            for (auto &inputVector : inputVectors)
                table.lookup(inputVector);
        };
    }
}

TEST_CASE("Benchmark normalizeMinMax", "[benchmark]") {
    size_t nTimes = 59713;
    BENCHMARK("normalizeMinMax " + std::to_string(nTimes) + "x") {
        for (int i = 0; i < nTimes; i++)
            FLUT::Normalization::normalizeMinMax(0.4, -0.3, 3.6);
    };
}

TEST_CASE("Benchmark findIndexInGrid", "[benchmark]") {
    size_t nTimes = 59713;
    std::vector<double> grid;
    for (double i = 0; i < 101; i++)
        grid.push_back(i / 101);

    BENCHMARK("findIndexInGrid " + std::to_string(nTimes) + "x") {
        for (int i = 0; i < nTimes; i++)
            FLUT::LookupTable::findIndexInGrid(grid, 0.4);
    };
}

TEST_CASE("Benchmark 1D table", "[benchmark]") {
    BenchmarkTable(one_dim_table_cc, false);
}

TEST_CASE("Benchmark 2D table", "[benchmark]") {
    BenchmarkTable(two_dim_table_cc);
}

TEST_CASE("Benchmark 3D table", "[benchmark]") {
    BenchmarkTable(three_dim_table_cc);
}
