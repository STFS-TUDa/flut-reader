// Including Catch2
#include <catch2/catch_all.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

// own headers
#include "interpolator.hpp"

#include <vector>

TEST_CASE("Test 1D interpolation", "[Interpolator]") {
    double *buffer = new double[2]{12, 47};
    FLUT::Hypercube hypercube;
    // (dimension, lowerIndex, higherIndex, lowerValue, higherValue, value)
    hypercube.bounds = {
        {0, 0, 1, 8, 5.5}
    };
    hypercube.indexInBuffer = {0, 1};
    std::vector<double> expected{34.5};
    std::vector<double> result(1);

    FLUT::Interpolator::interpolate1Dim(hypercube, buffer, result);
    REQUIRE_THAT(result[0], Catch::Matchers::WithinAbs(expected[0], 1.0e-10));

    size_t dimension = hypercube.bounds.size();
    FLUT::Interpolator::interpolate(dimension, hypercube, buffer, result);
    REQUIRE_THAT(result[0], Catch::Matchers::WithinAbs(expected[0], 1.0e-10));

    delete[] buffer;
}

TEST_CASE("Test 2D interpolation", "[Interpolator]") {
    double *buffer = new double[4]{21, 47, 39, 85};
    FLUT::Hypercube hypercube;
    // (dimension, lowerIndex, higherIndex, lowerValue, higherValue, value)
    hypercube.bounds = {
        {0, 0, 1, 3, 2},
        {0, 0, 2, 4, 3},
    };
    hypercube.indexInBuffer = {0, 1, 2, 3};
    std::vector<double> expected{48};
    std::vector<double> result(1);

    FLUT::Interpolator::interpolate2Dim(hypercube, buffer, result);
    REQUIRE_THAT(result[0], Catch::Matchers::WithinAbs(expected[0], 1.0e-10));

    size_t dimension = hypercube.bounds.size();
    FLUT::Interpolator::interpolate(dimension, hypercube, buffer, result);
    REQUIRE_THAT(result[0], Catch::Matchers::WithinAbs(expected[0], 1.0e-10));

    delete[] buffer;
}

TEST_CASE("Test 3D interpolation", "[Interpolator]") {
    double *buffer = new double[8]{115, 313, 250, 673, 217, 586, 469, 1252};
    FLUT::Hypercube hypercube;
    hypercube.bounds = {
        {0, 0, 1, 4, 2.5},
        {0, 0, 2, 5, 3.6},
        {0, 0, 3, 6, 4.8},
    };
    hypercube.indexInBuffer = {0, 1, 2, 3, 4, 5, 6, 7};
    std::vector<double> expected{526.18};
    std::vector<double> result(expected.size());

    FLUT::Interpolator::interpolate3Dim(hypercube, buffer, result);
    REQUIRE_THAT(result[0], Catch::Matchers::WithinAbs(expected[0], 1.0e-10));

    size_t dimension = hypercube.bounds.size();
    FLUT::Interpolator::interpolate(dimension, hypercube, buffer, result);
    REQUIRE_THAT(result[0], Catch::Matchers::WithinAbs(expected[0], 1.0e-10));

    delete[] buffer;
}

TEST_CASE("Test 4D interpolation", "[Interpolator]") {
    double *buffer = new double[16]{1339, 4291, 3323, 10563, 2823, 8975,  6967,  21951,
                                    2523, 8035, 6235, 19683, 5303, 16767, 13031, 40815};
    FLUT::Hypercube hypercube;
    hypercube.bounds = {
        {0, 0, 1, 5, 3.7},
        {0, 0, 2, 6, 4.2},
        {0, 0, 3, 7, 5.8},
        {0, 0, 4, 8, 7.4}
    };
    for (int i = 0; i < 16; i++)
        hypercube.indexInBuffer.emplace_back(i);
    std::vector<double> expected{18344.204};
    std::vector<double> result(expected.size());

    FLUT::Interpolator::interpolate4Dim(hypercube, buffer, result);
    REQUIRE_THAT(result[0], Catch::Matchers::WithinAbs(expected[0], 1.0e-10));

    size_t dimension = hypercube.bounds.size();
    FLUT::Interpolator::interpolate(dimension, hypercube, buffer, result);
    REQUIRE_THAT(result[0], Catch::Matchers::WithinAbs(expected[0], 1.0e-10));

    delete[] buffer;
}

TEST_CASE("Test 5D interpolation", "[Interpolator]") {
    double *buffer = new double[32]{5058,  15208, 11808, 41808,  8778,  21403, 16928, 55153,
                                    10533, 31058, 24433, 84183,  17953, 42203, 33878, 106228,
                                    9653,  29303, 22503, 80503,  16523, 40148, 31523, 102998,
                                    20278, 60678, 47153, 164653, 34223, 81098, 64498, 204223};
    FLUT::Hypercube hypercube;
    hypercube.bounds = {
        {0, 0, 1,  6, 1.3},
        {0, 0, 2,  7, 4.8},
        {0, 0, 3,  8, 7.2},
        {0, 0, 4,  9, 8.5},
        {0, 0, 5, 10, 6.3}
    };
    for (int i = 0; i < 32; i++)
        hypercube.indexInBuffer.push_back(i);

    std::vector<double> expected{33450.5822};
    std::vector<double> result(expected.size());

    FLUT::Interpolator::interpolate5Dim(hypercube, buffer, result);
    REQUIRE_THAT(result[0], Catch::Matchers::WithinAbs(expected[0], 1.0e-10));

    size_t dimension = hypercube.bounds.size();
    FLUT::Interpolator::interpolate(dimension, hypercube, buffer, result);
    REQUIRE_THAT(result[0], Catch::Matchers::WithinAbs(expected[0], 1.0e-10));

    delete[] buffer;
}

TEST_CASE("Test 6D interpolation", "[Interpolator]") {
    double *buffer = new double[64]{0, 1, 1, 3,  1, 3,  3,  6,  1, 3,  3,  6,  3,  6,  6,  10,
                                    1, 3, 3, 6,  3, 6,  6,  10, 3, 6,  6,  10, 6,  10, 10, 15,
                                    1, 3, 3, 6,  3, 6,  6,  10, 3, 6,  6,  10, 6,  10, 10, 15,
                                    3, 6, 6, 10, 6, 10, 10, 15, 6, 10, 10, 15, 10, 15, 15, 21};
    FLUT::Hypercube hypercube;
    hypercube.bounds = {
        {0, 0, 0, 1,  0.3},
        {0, 0, 0, 1,  0.5},
        {0, 0, 0, 1, 0.75},
        {0, 0, 0, 1,  0.6},
        {0, 0, 0, 1,  0.1},
        {0, 0, 0, 1,  0.2}
    };
    for (int i = 0; i < 64; i++)
        hypercube.indexInBuffer.emplace_back(i);

    std::vector<double> expected{4.795};

    std::vector<double> result(expected.size());

    size_t dimension = hypercube.bounds.size();
    FLUT::Interpolator::interpolate(dimension, hypercube, buffer, result);
    REQUIRE_THAT(result[0], Catch::Matchers::WithinAbs(expected[0], 1.0e-10));

    delete[] buffer;
}

TEST_CASE("Test sameValue interpolation", "[Interpolator]") {
    double *buffer = new double[32]{10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
                                    10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10};
    FLUT::Hypercube hypercube;
    hypercube.bounds = {
        {0, 0, 0, 0, 0},
        {0, 0, 1, 1, 1},
        {0, 0, 2, 2, 2},
        {0, 0, 3, 3, 3},
        {0, 0, 4, 4, 4},
    };
    for (int i = 0; i < 32; i++)
        hypercube.indexInBuffer.emplace_back(i);

    std::vector<double> expected{10};
    std::vector<double> result(expected.size());

    FLUT::Interpolator::interpolate1Dim(hypercube, buffer, result);
    REQUIRE_THAT(result[0], Catch::Matchers::WithinAbs(expected[0], 1.0e-10));

    FLUT::Interpolator::interpolate2Dim(hypercube, buffer, result);
    REQUIRE_THAT(result[0], Catch::Matchers::WithinAbs(expected[0], 1.0e-10));

    FLUT::Interpolator::interpolate3Dim(hypercube, buffer, result);
    REQUIRE_THAT(result[0], Catch::Matchers::WithinAbs(expected[0], 1.0e-10));

    FLUT::Interpolator::interpolate4Dim(hypercube, buffer, result);
    REQUIRE_THAT(result[0], Catch::Matchers::WithinAbs(expected[0], 1.0e-10));

    FLUT::Interpolator::interpolate5Dim(hypercube, buffer, result);
    REQUIRE_THAT(result[0], Catch::Matchers::WithinAbs(expected[0], 1.0e-10));

    delete[] buffer;
}
