// Including Catch2
#include <catch2/catch_all.hpp>

// own headers
#include "lookup_table.hpp"
// Functions used by the csv test
#include "test_utility.hpp"

#include <iostream>

auto two_dim_table_cc = std::string(DATA_DIR) + "01_2D-FLUT-cc.h5";
auto two_dim_csv_cc = std::string(DATA_DIR) + "results_2D-FLUT.csv";

auto three_dim_table_cc = std::string(DATA_DIR) + "02_3D-FLUT-cc.h5";
auto three_dim_csv_cc = std::string(DATA_DIR) + "results_3D-FLUT.csv";

auto pdf_flut_table = std::string(DATA_DIR) + "06_testFLUT_PDF.h5";
auto pdf_flut_csv = std::string(DATA_DIR) + "results_pdf_flut.csv";

TEST_CASE("Normalization 2D") {
    const std::vector<std::string> normalizedNames = {"Hnorm", "cc"};

    // Choose some values from the table to check.
    const std::vector<std::string> outputNames = {"N2",       "H2",  "O2",     "H2O",  "T",
                                                  "omega_yc", "rho", "lambda", "visc", "alpha"};
    FLUT::LookupTable table(two_dim_table_cc, outputNames, "pyflut");

    const std::vector<std::string> inputNames = table.getInputVariables();
    auto failed_rows = compareCSVNormalization(table, two_dim_csv_cc, normalizedNames);

    REQUIRE(failed_rows.size() == 0);
}

TEST_CASE("Lookup 2D") {
    // Choose some values from the table to check.
    const std::vector<std::string> outputNames = {"N2",       "H2",  "O2",     "H2O",  "T",
                                                  "omega_yc", "rho", "lambda", "visc", "alpha"};
    FLUT::LookupTable table(two_dim_table_cc, outputNames, "pyflut");

    const std::vector<std::string> inputNames = table.getInputVariables();
    auto failed_rows = compareCSVLookup(table, two_dim_csv_cc);

    REQUIRE(failed_rows.size() == 0);
}

TEST_CASE("Normalization 3D") {
    const std::vector<std::string> normalizedNames = {"Z", "Hnorm", "cc"};

    // Choose some values from the table to check.
    const std::vector<std::string> outputNames = {"N2",       "H2",  "O2",     "H2O",  "T",
                                                  "omega_yc", "rho", "lambda", "visc", "alpha"};
    FLUT::LookupTable table(three_dim_table_cc, outputNames, "pyflut");

    const std::vector<std::string> inputNames = table.getInputVariables();
    auto failed_rows = compareCSVNormalization(table, three_dim_csv_cc, normalizedNames);

    REQUIRE(failed_rows.size() == 0);
}

TEST_CASE("Lookup 3D") {
    // Choose some values from the table to check.
    const std::vector<std::string> outputNames = {"N2",       "H2",  "O2",     "H2O",  "T",
                                                  "omega_yc", "rho", "lambda", "visc", "alpha"};
    FLUT::LookupTable table(three_dim_table_cc, outputNames, "pyflut");

    const std::vector<std::string> inputNames = table.getInputVariables();
    auto failed_rows = compareCSVLookup(table, three_dim_csv_cc);

    REQUIRE(failed_rows.size() == 0);
}

TEST_CASE("Normalization PDF FLUT") {
    // Dim order: Z (not normalised, raw value), Zvar_norm, Hnorm, cc
    const std::vector<std::string> normalizedNames = {"Z", "Zvar_norm", "Hnorm", "cc"};

    const std::vector<std::string> outputNames = {"T", "rho", "N2", "H2O", "CO2", "omega_yc"};
    FLUT::LookupTable table(pdf_flut_table, outputNames, "pyflut");

    auto failed_rows = compareCSVNormalization(table, pdf_flut_csv, normalizedNames);

    REQUIRE(failed_rows.size() == 0);
}

TEST_CASE("Lookup PDF FLUT") {
    const std::vector<std::string> outputNames = {"T", "rho", "N2", "H2O", "CO2", "omega_yc"};
    FLUT::LookupTable table(pdf_flut_table, outputNames, "pyflut");

    auto failed_rows = compareCSVLookup(table, pdf_flut_csv);

    REQUIRE(failed_rows.size() == 0);
}
