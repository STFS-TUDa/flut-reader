#include "test_utility.hpp"

#include "abstract_table.hpp"

#include <cmath>
#include <fstream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

bool isDoubleRelativeNear(double first, double second, double maxRelativeDifference) {
    const double diff = fabs(first - second);
    const double firstAbs = fabs(first);
    const double secondAbs = fabs(second);
    const double largest = (secondAbs > firstAbs) ? secondAbs : firstAbs;
    if (diff <= maxRelativeDifference) {
        return true;
    }
    return diff <= largest * maxRelativeDifference;
}

// Used to create an index map from the first row of a csv file
std::map<std::string, int> getIndexMap(std::fstream &csvFile) {
    std::map<std::string, int> indexes;
    std::string line;
    if (csvFile.good()) {
        std::string columnName;
        std::getline(csvFile, line);
        std::stringstream ss(line);
        int i = 0;
        while (std::getline(ss, columnName, ',')) {
            indexes[columnName] = i;
            i++;
        }
    } else {
        throw std::runtime_error("CSV file is not good");
    }
    return indexes;
}

// Extracts value corresponding to the given names from a row and pushes them into vec
std::vector<double> getValuesFromRow(const std::vector<std::string> &names,
                                     const std::vector<double> &row,
                                     std::map<std::string, int> &indexes) {
    std::vector<double> vec;
    for (const auto &name : names) {
        vec.push_back(row[indexes[name]]);
    }
    return vec;
}

std::vector<std::string>
compareCSVNormalization(FLUT::LookupTable &table, const std::string &csvPath,
                        const std::vector<std::string> &normalizedVariables) {
    // open the csv file
    std::fstream csvFile(csvPath);
    std::vector<std::string> failedRows;
    std::string line;

    // Create a map with indexes
    std::map<std::string, int> indexes = getIndexMap(csvFile);

    // Read rows
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
        std::vector<double> inputValues = getValuesFromRow(table.getInputVariables(), row, indexes);
        std::vector<double> normalizedValues = getValuesFromRow(normalizedVariables, row, indexes);

        // Check the lookup
        std::vector<double> actualNormalizedValues = std::vector<double>{};
        std::vector<FLUT::DimensionBounds> dimensionBounds;
        try {
            table.normalize(inputValues);
            dimensionBounds = table.getBounds();
        } catch (...) { // Catch all exceptions
            failedRows.push_back("Normalization returned error for row index: " +
                                 std::to_string(rowIndex));
            continue;
        }

        if (normalizedValues.size() != dimensionBounds.size()) {
            failedRows.push_back(
                "Size of normalization vector does not match the expected size for row index: " +
                std::to_string(rowIndex));
            continue;
        }
        for (size_t i = 0; i < normalizedValues.size(); i++) {
            if (!isDoubleRelativeNear(dimensionBounds[i].value, normalizedValues[i], 1e-12)) {
                failedRows.push_back("Results do not meet tolerances for row index: " +
                                     std::to_string(rowIndex));
                break;
            }
        }
    }
    return failedRows;
}

std::vector<std::string> compareCSVLookup(FLUT::LookupTable &table, const std::string &csvPath) {
    // open the csv file
    std::fstream csvFile(csvPath);
    std::vector<std::string> failedRows;
    std::string line;

    // Create a map with indexes
    std::map<std::string, int> indexes = getIndexMap(csvFile);

    // Read rows
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
        std::vector<double> inputValues = getValuesFromRow(table.getInputVariables(), row, indexes);
        // Add output values to expectedResult vector
        std::vector<double> expectedResult =
            getValuesFromRow(table.getLoadedOutputVariables(), row, indexes);

        // Check the lookup
        std::vector<double> actualResult = std::vector<double>{};
        try {
            actualResult = table.lookup(inputValues);
        } catch (...) { // Catch all exceptions
            failedRows.push_back("Lookup returned error for row index: " +
                                 std::to_string(rowIndex));
            continue;
        }

        if (actualResult.size() != expectedResult.size()) {
            failedRows.push_back(
                "Size of lookup vector does not match the expected size for row index: " +
                std::to_string(rowIndex));
            continue;
        }
        for (size_t i = 0; i < actualResult.size(); i++) {
            if (!isDoubleRelativeNear(actualResult[i], expectedResult[i], 1e-12)) {
                failedRows.push_back("Results do not meet tolerances for row index: " +
                                     std::to_string(rowIndex));
                break;
            }
        }
    }
    return failedRows;
}