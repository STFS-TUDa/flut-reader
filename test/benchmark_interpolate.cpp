// Including Catch2
#include "interpolator.hpp"

#include <catch2/catch_all.hpp>
#include <fstream>
#include <random>
#include <vector>

void benchmark_interpolate(size_t dimension, size_t nVars) {
    // Create a buffer with random variables.
    size_t buffer_size = (1 << dimension) * nVars;
    double *buffer = new double[buffer_size];
    for (size_t i = 0; i < buffer_size; i++)
        buffer[i] = rand() % 256;

    // Create a hypercube for interpolation.
    FLUT::Hypercube hypercube;
    for (size_t i = 0; i < dimension; i++)
        hypercube.bounds.push_back({0, 0, (double)i - 1, (double)i + 2, (double)i});

    // Fill indices in buffer (they are of increasing order)
    // This is not 100% realistic, but might be sufficient here.
    for (int i = 0; i < (1 << dimension); i++)
        hypercube.indexInBuffer.push_back(i);

    // Create a result vector
    std::vector<double> result(nVars);

    // Now we have a hpyercube and a buffer and a result vector
    // and can benchmark the interpolation

    BENCHMARK("interpolate; nVars=" + std::to_string(nVars)) {
        for (int i = 0; i < 10000; i++)
            FLUT::Interpolator::interpolate(dimension, hypercube, buffer, result);
    };

    delete[] buffer;
}

std::vector<size_t> nVariables = {1, 4, 8, 16, 64};

TEST_CASE("Benchmark 1D interpolation", "[Interpolator]") {
    for (auto &nVar : nVariables)
        benchmark_interpolate(1, nVar);
}

TEST_CASE("Benchmark 2D interpolation", "[Interpolator]") {
    for (auto &nVar : nVariables)
        benchmark_interpolate(2, nVar);
}

TEST_CASE("Benchmark 4D interpolation", "[Interpolator]") {
    for (auto &nVar : nVariables)
        benchmark_interpolate(4, nVar);
}

TEST_CASE("Benchmark 6D interpolation", "[Interpolator]") {
    for (auto &nVar : nVariables)
        benchmark_interpolate(6, nVar);
}