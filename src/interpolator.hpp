#ifndef INTERPOLATOR_HPP_
#define INTERPOLATOR_HPP_

#include <cstdint> // For fixed-width integer types (e.g., uint32_t)
#include <vector>

namespace FLUT {

struct DimensionBounds {
    // This struct is the result of a lookup inside an input_dict.*.Values group
    // The input of the lookup was the Dimension and the NormalizedValue,
    // and the search returns the index of the next lower value in that group, and the value at the
    // lower index and the value at the higher index. The next higher index is trivial (lowerIndex +
    // 1), so it isn't saved.
    //
    // This data is nearly always needed as a group, e.g. later code that just got the index, would
    // need to look up the value again, so we save it here.
    //
    // Ideally this struct is kept small (by using the size of integers realistically needed, and
    // not 64 bit everywhere) so it can be passed around by value, instead of spilling on the stack.
    // E.g. the dimensions are realistically not going to be more than 2^^4 = 16, so we can use 4
    // bit for that.
    //
    uint32_t lowerIndex;
    uint32_t higherIndex;
    double lowerValue;
    double higherValue;
    double value;

    // Equal operator for testing purposes.
    bool operator==(const DimensionBounds &rhs) const {
        return this->lowerIndex == rhs.lowerIndex && this->higherIndex == rhs.higherIndex &&
               this->lowerValue == rhs.lowerValue && this->higherValue == rhs.higherValue &&
               this->value == rhs.value;
    }
};

struct Hypercube {
    // The hypercube defines the corners for the interpolation.
    // Example 1D (here we have 2 corners):
    // (0) ------- (1)
    //
    // Example 2D (here we have 2^2=4 corners):
    // (00) ------- (01)
    //  |            |
    //  |            |
    // (10) ------- (11)
    //
    // The index order in the hypercube in this code is
    // 1D: (0), (1)
    // 2D: (0 0), (1 0), (0 1), (1 1)
    // 3D: (0 0 0), (1 0 0), (0 1 0), (1 1 0), (0 0 1), (1 0 1), (0 1 1), (1 1 1)
    std::vector<DimensionBounds> bounds;
    std::vector<uint32_t> indexInBuffer;
};

namespace Interpolator {
/*
  All interpolation functions work under following assumptions:
    1.  For each dimension of the inputPoint a pair is stored in surroundingValues.
        The first value contains the next lower or equal value in the corresponding dimension
        for which a point in the grid exists. The second value contains the next higher value
        in the corresponding dimension for which a value in the grid exists.
        Example:  Assuming we have the points (0, 0), (1, 1), (2, 2), ... produced by
                  the function y = x. If we want to get an interpolaion for the point with
                  x = 0.5, the corresponding pair would be (0, 1) as we have valid points with
                  x = 0 and x = 1. If we interpolate in n input variables we do this for each
                  dimension, and as we have a rectilinear grid we can be assured that each of
                  the 2^n possible points created from these values is valid.
    2.  The ordering of the output values is as follows:
          Let x1, ..., xn be the indices of the input point, and let x1h, x1l, x2h, x2l, ...,
          xnh, xnl be the lower and higher values conforming to point 1. Let f(x1i, ...,xni)
          denote the data point that corresponds to the coordinates (x1i, ..., xni). Then
          indicesInBuffer is ordered as:
          f(x1l, x2l, ..., x(n-1)l, xnl),
          f(x1h, x2l, ..., x(n-1)l, xnl),
          f(x1l, x2h, ..., x(n-1)l, xnl),
          f(x1h, x2h, ..., x(n-1)l, xnl),
          ...,
          f(x1l, x2l, ..., x(n-1)h, xnl),
          f(x1h, x2l, ..., x(n-1)h, xnl),
          f(x1l, x2h, ..., x(n-1)h, xnl),
          f(x1h, x2h, ..., x(n-1)h, xnl),
          f(x1l, x2l, ..., x(n-1)l, xnh),
          f(x1h, x2l, ..., x(n-1)l, xnh),
          f(x1l, x2h, ..., x(n-1)l, xnh),
          f(x1h, x2h, ..., x(n-1)l, xnh),
          ...,
          f(x1l, x2l, ..., x(n-1)h, xnh),
          f(x1h, x2l, ..., x(n-1)h, xnh),
          f(x1l, x2h, ..., x(n-1)h, xnh),
          f(x1h, x2h, ..., x(n-1)h, xnh)

          Example:  Assume we have a function with two input variables and arbitrary many
                    output variables we want to interpolate, and have points for every integer
                    in each input dimension. If we want to get an interpolation for x = 0.3,
                    y = 3.6, we have a list of pairs according to point 1 that looks like this:
                    [(0, 1), (3, 4)].
                    The output values now have to look like this:
                    [f(0, 3), f(1, 3), f(0, 4), f(1, 4)]
*/

void interpolate(uint32_t dims, Hypercube &hypercube, const double *buffer,
                 std::vector<double> &result);

void interpolate0Dim(Hypercube &hypercube, const double *buffer, std::vector<double> &result);
void interpolate1Dim(Hypercube &hypercube, const double *buffer, std::vector<double> &result);
void interpolate2Dim(Hypercube &hypercube, const double *buffer, std::vector<double> &result);
void interpolate3Dim(Hypercube &hypercube, const double *buffer, std::vector<double> &result);
void interpolate4Dim(Hypercube &hypercube, const double *buffer, std::vector<double> &result);
void interpolate5Dim(Hypercube &hypercube, const double *buffer, std::vector<double> &result);
void interpolate6Dim(Hypercube &hypercube, const double *buffer, std::vector<double> &result);

/*
  Adjusts the pairs given as surroundingValues to prevent division by zero and resultig NaNs.
*/
void adjustSurroundingValues(Hypercube &hypercube, uint32_t N);

} // namespace Interpolator

} // namespace FLUT

#endif