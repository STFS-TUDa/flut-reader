#include "lookup_table.hpp"

#include "interpolator.hpp"

#include <algorithm>
#include <cstdint> // For fixed-width integer types (e.g., uint32_t)
#include <iostream>
#include <memory>
#include <string>
#include <vector>

//
// Normalization functions
//
const double FLUT::Normalization::normalizeMinMax(const double value, const double minVal,
                                                  const double maxVal) {
    // If the value is outside the interpolated min and max values, return 0 or 1
    // This is also valid for the case where minVal and maxVal are identical
    if (value <= minVal) {
        return 0.0;
    }
    if (value >= maxVal) {
        return 1.0;
    }

    // Calculation inside the table boundaries
    if (minVal == maxVal) {
        // Needs to be checked if upper and lower bound are the same.
        // The normalization would result in a NaN value, which crashes the lookup.
        return 0.0;
    }
    return (value - minVal) / (maxVal - minVal);
};

const double FLUT::Normalization::normalizeVariance(const double variance, const double baseValue) {
    if (variance <= 0.0) {
        return 0.0;
    }
    const double maxVariance = baseValue * (1.0 - baseValue);
    // baseValue at 0 or 1: variance is identically zero, clamp to avoid division by zero.
    if (maxVariance <= 0.0) {
        return 0.0;
    }
    if (variance >= maxVariance) {
        return 1.0;
    }
    return variance / maxVariance;
};

//
// Lookup table
//
FLUT::LookupTable::LookupTable(const std::string &path, const std::string &fileType)
    : hdf5FilePath(path),
      input_dict(path, fileType),
      output_dict(path, fileType) {
    reserveMemory(input_dict.size());
}

FLUT::LookupTable::LookupTable(const std::string &path,
                               const std::vector<std::string> &outputVariableNames,
                               const std::string &fileType)
    : hdf5FilePath(path),
      input_dict(path, fileType),
      output_dict(path, fileType) {
    reserveMemory(input_dict.size());
    readOutputVariableData(outputVariableNames, fileType);
}

void FLUT::LookupTable::reserveMemory(uint32_t numberInputVariables) {
    // Preallocate memory for the vectors used in lookup()

    // Preallocate memory in the entries vector.
    // 1 << inputVector.size() evaluates to 2^inputVector.size().
    // Example: inputVector.size()=3 -> 2^3 = 8 elements are reserved.
    // The 8 elements correspond to the corners of a cube in 3 dimensions.
    hypercube.indexInBuffer.reserve(1 << numberInputVariables);
    hypercube.bounds.reserve(numberInputVariables);

    // Used by Default (min/max) normalization to store the interpolated bounds.
    // Only needs to be enlarged if a new type also requires a table-interpolated bounds lookup.
    normalizationResult.resize(2);
}

void FLUT::LookupTable::readOutputVariableData(const std::vector<std::string> &outputVariableNames,
                                               const std::string &fileType) {
    // Delete old output_data object first (to free up the shared_memory).
    // Since the filename of the shared_memory is unique this should not be necessary.
    if (hasInitializedOutputData())
        output_data.reset();

    // Create a new DataSharedMemory object that reads in the output-data.
    output_data =
        std::make_unique<FLUT::DataSharedMemory>(hdf5FilePath, outputVariableNames, fileType);

    // The result vector of the lookup is resized according to the
    // number of output variables.
    interpolationResult.resize(output_data->nVariables());
}

std::vector<double> &FLUT::LookupTable::lookup(const std::vector<double> &inputVector) {
    // The if condition takes around 50 ns.
    // For better performance this could be removed
    // For now it is kept for the sake of good error messages.
    if (output_data == nullptr)
        throw std::runtime_error(
            "Error: 'output_data' has not been read from the file. Please call "
            "'readOutputVariableData( )' before attempting a lookup operation.");

    normalize(inputVector);
    extractHypercubeIndices(hypercube, output_data->getDimensionOffset());
    FLUT::Interpolator::interpolate(hypercube.bounds.size(), hypercube, output_data->buffer,
                                    interpolationResult);
    return interpolationResult;
}

void FLUT::LookupTable::normalize(const std::vector<double> &inputVector) {
    // Reset bounds in hypercube
    hypercube.bounds.clear();

    // Do the normalization to define the bounds of the hypercube
    // For each dimension, a new interpolation cube is defined:
    // dim = 0: point (1 point)
    // dim = 1: line (2 points)
    // dim = 2: square (4 points)
    // dim = 3: cube (8 points)
    // ...
    // At each point, the index in the data-buffer is extracted.
    // The value at the edge can then be deterimined by buffer[indexInBuffer].
    for (uint32_t dimension = 0; dimension < inputVector.size(); ++dimension) {
        if (input_dict.isNormalized(dimension)) {
            double normalizedValue;
            if (auto *spec = std::get_if<DefaultNormSpec>(&input_dict.normSpec[dimension])) {
                extractHypercubeIndices(hypercube, spec->normValues.dimensionOffsets);
                FLUT::Interpolator::interpolate(
                    dimension, hypercube, spec->normValues.values.data(), normalizationResult);
                normalizedValue = FLUT::Normalization::normalizeMinMax(
                    inputVector[dimension], normalizationResult[0], normalizationResult[1]);
            } else if (auto *spec =
                           std::get_if<VarianceNormSpec>(&input_dict.normSpec[dimension])) {
                normalizedValue = FLUT::Normalization::normalizeVariance(
                    inputVector[dimension], inputVector[spec->baseVariableIndex]);
            } else {
                throw std::logic_error("Unhandled NormSpec variant in normalize()");
            }
            // Find the index of the normalized value.
            hypercube.bounds.emplace_back(
                findIndexInGrid(input_dict.values[dimension], normalizedValue));
        } else {
            // Find the index of the non-normalized value.
            hypercube.bounds.emplace_back(
                findIndexInGrid(input_dict.values[dimension], inputVector[dimension]));
        }
    }

    // Delete the indice values, that are unique for every interpolation.
    // The bounds are kept, and will be reused.
    hypercube.indexInBuffer.clear();
}

void FLUT::LookupTable::extractHypercubeIndices(Hypercube &hypercube,
                                                const std::vector<uint32_t> &dimensionOffset) {
    uint32_t n_corners = (1 << hypercube.bounds.size());
    hypercube.indexInBuffer.resize(n_corners);

    for (uint32_t corner = 0; corner < n_corners; corner++) {
        extractIndex(corner, hypercube, dimensionOffset);
    }
}

/**
 *
 * This method has to retrieve a table entry based on a vector of indices.
 * Usually this kind of array lookup is generated by the compiler (e.g. buffer[i][j][k]), but this
 * doesn't work here because the number of dimensionSizes is not known at compile time.
 *
 * This means we have to basically reimplement multidimensional array indexing in C++.
 * In each step we calculate the beginning of the sub-area in the large buffer,
 * which then be treated as an array of one less dimension.
 * This intuitively makes sense: a 3D array can be seen as a 1D array of 2D
 * arrays, a 2D array, is a 1D array of 1D arrays
 *
 * To get the element x of a 3D array, you would usually write:
 * x = buffer[i][j][k]
 *
 * This compiles down to the following:
 * x = *(&buffer + sizeof(float) * (i * dim1 * dim2 + j * dim2 + k))
 * where dimN is the size of that dimension (i.e. the number of elements in that dimension)
 * every index (i,j,k) needs to be multiplied by the sizes of its sub-dimensions,
 * to get the i-th array of N-1 dimensions
 *
 **/
void FLUT::LookupTable::extractIndex(uint32_t corner, Hypercube &hypercube,
                                     const std::vector<uint32_t> &dimensionOffset) {
    auto &bounds = hypercube.bounds;
    hypercube.indexInBuffer[corner] = 0;

    for (uint8_t i = 0; i < bounds.size(); ++i) {
        // Use a conditional expression to remove branching
        // This computes the index based on the i-th bit of 'corner'
        uint32_t mask = (corner >> i) & 1; // Extract the i-th bit of 'corner'
        uint32_t boundsIndex = mask * bounds[i].higherIndex + (1 - mask) * bounds[i].lowerIndex;

        // Assign indices[i] based on the extracted bit in a branchless manner
        hypercube.indexInBuffer[corner] += boundsIndex * dimensionOffset[i];
    }
}

FLUT::DimensionBounds FLUT::LookupTable::findIndexInGrid(const std::vector<double> &grid,
                                                         const double value) {
    // If value is lower than the lowest value of the grid
    // Fall back to the lowest value
    if (value < grid[0]) {
        return {0, 0, grid[0], grid[0], grid[0]};
    }

    // If value is higher than the highest value of the grid
    // Fall back to the highest value
    const uint32_t size = grid.size();
    if (value >= grid[size - 1]) {
        return {(size - 1), (size - 1), grid[size - 1], grid[size - 1], grid[size - 1]};
    }

    // Value is inside the grid.
    uint32_t upperIndex = std::upper_bound(grid.begin(), grid.end(), value) - grid.begin();

    return {upperIndex - 1, upperIndex, grid[upperIndex - 1], grid[upperIndex], value};
}

//
// Getter functions
//
void FLUT::LookupTable::printInfo() {
    std::cout << "  -- LUT Information: -- " << std::endl;
    std::cout << "  FileName: " << hdf5FilePath << std::endl;
    std::cout << std::endl;
    input_dict.printInfo();
    std::cout << std::endl;
    output_dict.printInfo();
    std::cout << std::endl;

    if (output_data) {
        output_data->printInfo();
    }
    std::cout << "  ---------------------- " << std::endl;
}

const std::vector<std::string> &FLUT::LookupTable::getInputVariables() const {
    return input_dict.names;
}

const std::vector<std::string> FLUT::LookupTable::getAvailableOutputVariables() const {
    return output_dict.getNames();
}

const std::vector<std::string> &FLUT::LookupTable::getLoadedOutputVariables() const {
    return output_data->variableNames;
}

void FLUT::LookupTable::containsOutputVariable(const std::string &name) {
    output_dict.containsVariable(name);
}

bool FLUT::LookupTable::hasInitializedOutputData() {
    return output_data != nullptr;
}

std::vector<FLUT::DimensionBounds> &FLUT::LookupTable::getBounds() {
    return hypercube.bounds;
}

FLUT::Hypercube &FLUT::LookupTable::getHypercube() {
    return hypercube;
}

double *FLUT::LookupTable::getBuffer() {
    return output_data->buffer;
}

const std::vector<uint32_t> &FLUT::LookupTable::getDimensionOffset() {
    return output_data->getDimensionOffset();
}
