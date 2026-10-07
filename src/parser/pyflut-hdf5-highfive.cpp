#include "containers.hpp"

#include <highfive/highfive.hpp>
#include <iostream>
#include <unordered_map>
#include <vector>

namespace H5FlutParser {

// Define names of the groups to read from.
// This way they can be easily changed when the
// Naming in the FLUT would change.
std::string INPUT_DICT_NAME("/input_dict");
std::string OUTPUT_DICT_NAME("/output_dict");
std::string DATA_NAME("/data");

// Helper function to the read-in
// Read a python dictionary with the depth of 1.
template <typename T>
std::unordered_map<std::string, T> readDictFromHdf5(HighFive::Group group) {
    std::unordered_map<std::string, T> result;
    for (size_t i = 0; i < group.getNumberObjects(); i++) {
        // Get name of object
        std::string name = group.getObjectName(i);

        // Read in data value
        auto dataset = group.getDataSet(name);
        T data;
        dataset.read(data);

        // Save value in map
        result[name] = data;
    }
    return result;
}

FLUT::NormalizationData extractNormalizationValues(const std::string &fileName, size_t dimension,
                                                   std::vector<std::string> normalizationNames) {
    // Load data from output data
    FLUT::DataSharedMemory data(fileName, normalizationNames, "pyflut");

    // Check the number of dimension required
    size_t nVariables = normalizationNames.size();

    FLUT::NormalizationData normalizationValues;
    normalizationValues.variableNames = normalizationNames;
    if (dimension == 0) {
        normalizationValues.dimensionSizes = {nVariables};
        normalizationValues.values = std::vector<double>(data.buffer, data.buffer + nVariables);
    } else {
        // Define the number of values
        size_t nValues = 1;
        for (int i = 0; i < dimension; i++) {
            nValues *= data.dimensionSize(i);
            normalizationValues.dimensionSizes.emplace_back(data.dimensionSize(i));
        }
        normalizationValues.dimensionSizes.emplace_back(nVariables);

        // Read in normalization sizes from sharedBuffer.
        normalizationValues.values.resize(nVariables * nValues);
        for (size_t i = 0; i < nValues; i++) {
            size_t index = i * nVariables;
            for (size_t j = 0; j < nVariables; j++) {
                normalizationValues.values[index + j] =
                    data.buffer[data.dimensionOffset(dimension - 1) * i + j];
            }
        }
    }

    normalizationValues.calculateDimensionOffsets();

    return normalizationValues;
}

FLUT::InputDict readInputDict(const std::string &fileName) {
    HighFive::File file(fileName, HighFive::File::ReadOnly);
    HighFive::Group group = file.getGroup(INPUT_DICT_NAME);

    size_t nVars(group.getNumberObjects());

    FLUT::InputDict input_dict(nVars);

    for (size_t i = 0; i < nVars; i++) {
        std::string name = group.getObjectName(i);
        // Read in index of input_variable
        auto dataset = group.getDataSet(name + "/Index");
        size_t index;
        dataset.read(index);

        // Read values of input_variable
        dataset = group.getDataSet(name + "/Values");
        std::vector<double> value;
        dataset.read(value);

        // Check if variable needs to be normalized
        // If this is the case. Exchange the name of the normalized
        // variable with the physical name transported in OpenFOAM.
        bool normalize = false;

        // New Table format
        // Switch out if testing is adjusted.
        if (group.exist(name + "/Normalization_settings/Normalize"))
            group.getDataSet(name + "/Normalization_settings/Normalize").read(normalize);
        if (normalize) {
            std::unordered_map<std::string, std::string> normalizationSettingsVariable =
                readDictFromHdf5<std::string>(
                    group.getGroup(name + "/Normalization_settings/Settings/"));
            normalizationSettingsVariable["Normalized_name"] = name;
            name = normalizationSettingsVariable["Physical_name"];
            input_dict.normalizationSettings[name] = normalizationSettingsVariable;

            const std::string normType =
                normalizationSettingsVariable.count("Normalization_type")
                    ? normalizationSettingsVariable.at("Normalization_type")
                    : "default";

            if (normType == "default") {
                std::vector<std::string> normValueNames;
                normValueNames.emplace_back(
                    normalizationSettingsVariable["Min_name_normalization"]);
                normValueNames.emplace_back(
                    normalizationSettingsVariable["Max_name_normalization"]);
                input_dict.normSpec[index] = FLUT::DefaultNormSpec{
                    extractNormalizationValues(fileName, index, normValueNames)};
            } else if (normType == "variance") {
                input_dict.normSpec[index] = FLUT::VarianceNormSpec{};
            } else {
                throw std::invalid_argument("Unknown Normalization_type '" + normType +
                                            "' for variable '" + name + "'");
            }
        }

        // Save value in vector at given position.
        input_dict.names[index] = name;
        input_dict.values[index] = value;
        input_dict.normalizedDims[index] = normalize;
    }

    // Resolve base variable indices for variance dimensions.
    // Done after the loop so all names are guaranteed to be populated.
    for (size_t i = 0; i < nVars; i++) {
        if (auto *spec = std::get_if<FLUT::VarianceNormSpec>(&input_dict.normSpec[i])) {
            const std::string &baseName =
                input_dict.normalizationSettings[input_dict.names[i]]["Base_variable_name"];
            auto it = std::find(input_dict.names.begin(), input_dict.names.end(), baseName);
            if (it == input_dict.names.end())
                throw std::invalid_argument("Base_variable_name '" + baseName +
                                            "' not found in input variables.");
            spec->baseVariableIndex = std::distance(input_dict.names.begin(), it);
        }
    }

    return input_dict;
}

FLUT::OutputDict readOutputDict(const std::string &fileName) {
    HighFive::File file(fileName, HighFive::File::ReadOnly);
    return FLUT::OutputDict(readDictFromHdf5<size_t>(file.getGroup(OUTPUT_DICT_NAME)));
}

std::vector<size_t> getDataDimension(const std::string &fileName) {
    HighFive::File file(fileName, HighFive::File::ReadOnly);
    auto dataset = file.getDataSet(DATA_NAME);

    // Define the variables describing the data read from the FLUT
    // Dimension sizes
    return dataset.getDimensions();
}

void readDataIntoBuffer(const std::string &fileName, FLUT::DataSharedMemory *sharedMemoryData) {
    HighFive::File file(fileName, HighFive::File::ReadOnly);
    auto dataset = file.getDataSet(DATA_NAME);

    // Load output_dict to get information on the variable indices.
    FLUT::OutputDict output_dict(fileName, "pyflut");

    // Extract the outputVariables (in correct order)
    // that should be extracted.
    std::vector<std::string> outputVariables = sharedMemoryData->variableNames;

    // Extract the indices corresponding to the outputVariables.
    std::vector<size_t> indices;
    for (auto &v : outputVariables) {
        indices.emplace_back(output_dict.getIndex(v));
    }

    // Order the variableNames in the sharedMemoryDate object
    // according to the indice in the H5File.
    // This is the way the data will be read into the buffer by data.select(indices).read( ... )
    std::sort(sharedMemoryData->variableNames.begin(), sharedMemoryData->variableNames.end(),
              [&](const std::string &a, const std::string &b) {
                  return output_dict.getIndex(a) < output_dict.getIndex(b);
              });

    // Read data into buffer.
    // // Since we read a raw pointer here, we have to use read_raw which does an dimensionality
    // check.
    dataset.select(indices).read_raw(sharedMemoryData->buffer);
}

} // namespace H5FlutParser