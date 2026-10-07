// interface
#ifndef CONTAINERS_HPP_
#define CONTAINERS_HPP_

#include <algorithm>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

namespace FLUT {

struct DataArray {
    std::vector<std::string> variableNames;
    std::vector<uint32_t> dimensionOffsets;
    std::vector<size_t> dimensionSizes;

    void calculateDimensionOffsets();

    const std::vector<uint32_t> &getDimensionOffset();
    const uint32_t dimensionOffset(const size_t i);
    const size_t dimensionSize(const size_t i);

    const size_t nVariables();
};

struct NormalizationData : DataArray {
    std::vector<double> values;
};

// Per-dimension normalization specification.
// std::variant holds exactly one of the concrete spec types — the active type identifies which
// normalization formula is used and carries only the parameters that formula needs.
// Adding a new normalization type means adding a new struct here and a new branch in normalize();
// no other bookkeeping arrays are required.
struct DefaultNormSpec {
    NormalizationData normValues; // interpolated min/max bounds from the table
};

struct VarianceNormSpec {
    uint32_t baseVariableIndex{0}; // index of the mean variable used to scale the variance
};

using NormSpec = std::variant<DefaultNormSpec, VarianceNormSpec>;

struct InputDict {
    // Attributes
    std::vector<std::string> names;
    std::vector<std::vector<double>> values;
    std::vector<bool> normalizedDims;
    std::vector<NormSpec> normSpec;
    std::unordered_map<std::string, std::unordered_map<std::string, std::string>>
        normalizationSettings;

    // Constructor
    InputDict(const std::string &fileName, const std::string &fileType = "pyflut");
    InputDict(size_t nVars);

    // Struct methods
    size_t size();
    bool isNormalized(const uint32_t i);
    void printInfo() const;
};

struct OutputDict {
    // Attributes
    std::unordered_map<std::string, size_t> outputDict;

    // Constructor
    OutputDict(const std::string &fileName, const std::string &fileType = "pyflut");
    OutputDict(std::unordered_map<std::string, size_t> map);

    // Struct methods
    const std::vector<std::string> getNames() const;
    size_t getIndex(const std::string &name);
    void containsVariables(const std::vector<std::string> &names);
    void containsVariable(const std::string &name);
    void printInfo() const;
};

struct DataSharedMemory : DataArray {
  public:
    DataSharedMemory(const std::string &fileName,
                     const std::vector<std::string> &outputVariablesToExtract,
                     const std::string &fileType_ = "pyflut");
    ~DataSharedMemory();

    void printInfo() const;

    // Shared memory buffer
    double *buffer;

  protected:
    // Helper functions for construction
    std::vector<std::string> defineOutputVariables(const std::string fileName,
                                                   const std::vector<std::string> &outputVariables);
    std::vector<size_t> getDataDimension(const std::string &fileName);
    void reorderVariables(const std::vector<std::string> &names);
    void readDataIntoBuffer(const std::string &fileName);
    void setBufferName(const std::string &fileName,
                       const std::vector<std::string> indicesToExtract);
    size_t nElementsPerVariable();

    // Entries for shared memory handling.
    size_t bufferSize;
    std::basic_string<char, std::char_traits<char>, std::allocator<char>> bufferName;
    bool hasExclusiveLock = false;

    // Metadata
    const std::string fileType;
};

} // namespace FLUT

#endif
