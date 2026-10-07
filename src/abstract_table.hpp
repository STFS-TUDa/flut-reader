#ifndef ABSTRACT_TABLE_HPP_
#define ABSTRACT_TABLE_HPP_

#include <string>
#include <vector>

namespace FLUT {

// AbstractTable serves as an abstract interface for a lookup-table (LUT) implementation.
// It defines a set of pure virtual functions for managing input and output variables,
// performing lookups, and handling output variable data.
// Derived classes must provide concrete implementations for these functions.
class AbstractTable {
  public:
    virtual ~AbstractTable() = default;
    virtual std::vector<double> &lookup(const std::vector<double> &inputVector) = 0;

    virtual void printInfo() = 0;
    virtual const std::vector<std::string> &getInputVariables() const = 0;
    virtual const std::vector<std::string> &getLoadedOutputVariables() const = 0;
    virtual const std::vector<std::string> getAvailableOutputVariables() const = 0;
    virtual void containsOutputVariable(const std::string &name) = 0;
    virtual void readOutputVariableData(const std::vector<std::string> &outputVariableNames,
                                        const std::string &fileType = "pyflut") = 0;
    virtual bool hasInitializedOutputData() = 0;
};

} // namespace FLUT
#endif
