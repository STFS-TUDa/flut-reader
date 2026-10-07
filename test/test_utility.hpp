#ifndef TEST_UTILITY_HPP_
#define TEST_UTILITY_HPP_

#include "abstract_table.hpp"
#include "lookup_table.hpp"

#include <cmath>
#include <limits>
#include <map>
#include <string>
#include <vector>

bool isDoubleRelativeNear(double first, double second,
                          double maxRelativeDifference = std::numeric_limits<double>::epsilon());

// Used to create an index map from the first row of a csv file
std::map<std::string, int> getIndexMap(std::fstream &csvFile);

// Extracts value corresponding to the given names from a row and pushes them into vec
std::vector<double> getValuesFromRow(const std::vector<std::string> &names,
                                     const std::vector<double> &row,
                                     std::map<std::string, int> &indexes);

std::vector<std::string>
compareCSVNormalization(FLUT::LookupTable &table, const std::string &csvPath,
                        const std::vector<std::string> &normalizedVariables);

std::vector<std::string> compareCSVLookup(FLUT::LookupTable &table, const std::string &csvPath);

#endif