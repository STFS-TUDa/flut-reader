#ifndef FLUT_COMBINED_HPP_
#define FLUT_COMBINED_HPP_

#include "abstract_table.hpp"
#include "containers.hpp"
#include "interpolator.hpp"

#include <cstdint>  // For fixed-width integer types (e.g., uint32_t)
#include <iostream> // For input/output (e.g., std::cerr, std::cout)
#include <memory>   // For std::shared_ptr, std::make_unique
#include <string>   // For std::string
#include <vector>   // For std::vector

namespace FLUT {

namespace Normalization {
// Default normalization
// xnorm = (x - xmin) / (xmax - xmin)
const double normalizeMinMax(const double value, const double minVal, const double maxVal);
// Variance normalization: scales by the maximum possible variance of a [0,1]-bounded variable
// xnorm = variance / (baseValue * (1 - baseValue))
const double normalizeVariance(const double variance, const double baseValue);
} // namespace Normalization

class LookupTable : public AbstractTable {
  public:
    LookupTable(const std::string &path, const std::string &fileType = "pyflut");
    LookupTable(const std::string &path, const std::vector<std::string> &outputVariableNames,
                const std::string &fileType = "pyflut");
    ~LookupTable() = default;

    // Interface functions inherited from AbstractTable
    std::vector<double> &lookup(const std::vector<double> &inputVector) override;

    void printInfo() override;
    const std::vector<std::string> &getInputVariables() const override;
    const std::vector<std::string> &getLoadedOutputVariables() const override;
    const std::vector<std::string> getAvailableOutputVariables() const override;
    void readOutputVariableData(const std::vector<std::string> &outputVariableNames,
                                const std::string &fileType = "pyflut") override;
    void containsOutputVariable(const std::string &name) override;
    bool hasInitializedOutputData() override;

    // Private functions
    // These are currently not private because they need to be
    // accessible for the benchmark suite.
    static DimensionBounds findIndexInGrid(const std::vector<double> &grid, const double value);

    void normalize(const std::vector<double> &inputVector);
    static void extractHypercubeIndices(Hypercube &hypercube,
                                        const std::vector<uint32_t> &dimensionOffset);
    static void extractIndex(uint32_t corner, Hypercube &hypercube,
                             const std::vector<uint32_t> &dimensionOffset);

    // Getter functions used for testing and benchmarking.
    std::vector<DimensionBounds> &getBounds();
    Hypercube &getHypercube();
    double *getBuffer();
    const std::vector<uint32_t> &getDimensionOffset();

  protected:
    // Pre-initializes the memory of the datastructures that are used for the lookup.
    // This leads to a significant improvement in performance
    void reserveMemory(uint32_t numberInputVariables);

    // Data-structure used for the lookup
    // The memory for these elements is allocated prior to the lookup. (see reserveMemory)
    // This gives a significant improvement in performance
    Hypercube hypercube; // Hypercube used as input for the interpolation
    std::vector<double>
        interpolationResult; // Result of the final interpolation (n output variables)
    std::vector<double> normalizationResult; // Result of the normalization (2 output variables)

    // Data structures that hold the information contained in the LUT
    const std::string hdf5FilePath;
    FLUT::InputDict input_dict;
    FLUT::OutputDict output_dict;
    // The output_data is allocated as a shared_ptr.
    // This way we can use a constructor that does not output_data.
    // It is read when readOutputVariableData is called.
    // This is necessary for OpenFOAM, since during the initial FLUT generation
    // the output-variables from the table are not known.
    std::shared_ptr<FLUT::DataSharedMemory> output_data{nullptr};
};

} // namespace FLUT

#endif
