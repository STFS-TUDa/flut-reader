#include "interpolator.hpp"

#include <iostream>
#include <stdexcept>
#include <vector>

namespace FLUT {

void Interpolator::interpolate(
    const uint32_t dims, // number of dimensions, explicitly given to get better performance
    Hypercube &hypercube, const double *buffer, std::vector<double> &result) {
    // Dispatch to the specialized function for the dimension. Only 0 through 6 input
    // dimensions are supported.
    switch (dims) {
        case 0:
            interpolate0Dim(hypercube, buffer, result);
            return;
        case 1:
            interpolate1Dim(hypercube, buffer, result);
            return;
        case 2:
            interpolate2Dim(hypercube, buffer, result);
            return;
        case 3:
            interpolate3Dim(hypercube, buffer, result);
            return;
        case 4:
            interpolate4Dim(hypercube, buffer, result);
            return;
        case 5:
            interpolate5Dim(hypercube, buffer, result);
            return;
        case 6:
            interpolate6Dim(hypercube, buffer, result);
            return;
        default:
            throw std::runtime_error(
                "Error: interpolation is only implemented for up to 6 input dimensions.");
    }
}

void Interpolator::interpolate0Dim(Hypercube &hypercube, const double *buffer,
                                   std::vector<double> &result) {
    result = std::vector<double>(buffer + hypercube.indexInBuffer[0],
                                 buffer + hypercube.indexInBuffer[0] + result.size());
    return;
}

// all the specialized functions use a raw buffer pointer to avoid potentially large amounts of
// vector copies
void Interpolator::interpolate1Dim(Hypercube &hypercube, const double *buffer,
                                   std::vector<double> &result) {
    // adjust inputs
    adjustSurroundingValues(hypercube, 1);

    // linear interpolation in 1 dimensions
    const double aDiff = (hypercube.bounds[0].value - hypercube.bounds[0].lowerValue) /
                         (hypercube.bounds[0].higherValue - hypercube.bounds[0].lowerValue);
    const double aDiffDual = 1 - aDiff;

    // N-dimensional interpolation (general case)
    for (int i = 0; i < result.size(); ++i) {
        // linear interpolation in a
        result[i] = buffer[hypercube.indexInBuffer[0] + i] * aDiffDual +
                    buffer[hypercube.indexInBuffer[1] + i] * aDiff;
    }
    return;
}

void Interpolator::interpolate2Dim(Hypercube &hypercube, const double *buffer,
                                   std::vector<double> &result) {
    // adjust inputs
    adjustSurroundingValues(hypercube, 2);

    // linear interpolation in 2 dimensions
    const double aDiff = (hypercube.bounds[0].value - hypercube.bounds[0].lowerValue) /
                         (hypercube.bounds[0].higherValue - hypercube.bounds[0].lowerValue);
    const double aDiffDual = 1 - aDiff;
    const double bDiff = (hypercube.bounds[1].value - hypercube.bounds[1].lowerValue) /
                         (hypercube.bounds[1].higherValue - hypercube.bounds[1].lowerValue);
    const double bDiffDual = 1 - bDiff;

    // N-dimensional interpolation (general case)
    for (int i = 0; i < result.size(); ++i) {
        // linear interpolation in a
        const double a0 = buffer[hypercube.indexInBuffer[0] + i] * aDiffDual +
                          buffer[hypercube.indexInBuffer[1] + i] * aDiff;
        const double a1 = buffer[hypercube.indexInBuffer[2] + i] * aDiffDual +
                          buffer[hypercube.indexInBuffer[3] + i] * aDiff;

        result[i] = a0 * bDiffDual + a1 * bDiff;
    }
    return;
}

void Interpolator::interpolate3Dim(Hypercube &hypercube, const double *buffer,
                                   std::vector<double> &result) {
    // adjust inputs
    adjustSurroundingValues(hypercube, 3);

    // linear interpolation in 3 dimensions
    const double aDiff = (hypercube.bounds[0].value - hypercube.bounds[0].lowerValue) /
                         (hypercube.bounds[0].higherValue - hypercube.bounds[0].lowerValue);
    const double aDiffDual = 1 - aDiff;
    const double bDiff = (hypercube.bounds[1].value - hypercube.bounds[1].lowerValue) /
                         (hypercube.bounds[1].higherValue - hypercube.bounds[1].lowerValue);
    const double bDiffDual = 1 - bDiff;
    const double cDiff = (hypercube.bounds[2].value - hypercube.bounds[2].lowerValue) /
                         (hypercube.bounds[2].higherValue - hypercube.bounds[2].lowerValue);
    const double cDiffDual = 1 - cDiff;

    // N-dimensional interpolation (general case)
    for (int i = 0; i < result.size(); ++i) {
        // linear interpolation in a
        const double a0 = buffer[hypercube.indexInBuffer[0] + i] * aDiffDual +
                          buffer[hypercube.indexInBuffer[1] + i] * aDiff;
        const double a1 = buffer[hypercube.indexInBuffer[2] + i] * aDiffDual +
                          buffer[hypercube.indexInBuffer[3] + i] * aDiff;
        const double a2 = buffer[hypercube.indexInBuffer[4] + i] * aDiffDual +
                          buffer[hypercube.indexInBuffer[5] + i] * aDiff;
        const double a3 = buffer[hypercube.indexInBuffer[6] + i] * aDiffDual +
                          buffer[hypercube.indexInBuffer[7] + i] * aDiff;

        const double b0 = a0 * bDiffDual + a1 * bDiff;
        const double b1 = a2 * bDiffDual + a3 * bDiff;

        result[i] = b0 * cDiffDual + b1 * cDiff;
    }
    return;
}

void Interpolator::interpolate4Dim(Hypercube &hypercube, const double *buffer,
                                   std::vector<double> &result) {
    // adjust inputs
    adjustSurroundingValues(hypercube, 4);

    // linear interpolation in 4 dimensions
    const double aDiff = (hypercube.bounds[0].value - hypercube.bounds[0].lowerValue) /
                         (hypercube.bounds[0].higherValue - hypercube.bounds[0].lowerValue);
    const double aDiffDual = 1 - aDiff;
    const double bDiff = (hypercube.bounds[1].value - hypercube.bounds[1].lowerValue) /
                         (hypercube.bounds[1].higherValue - hypercube.bounds[1].lowerValue);
    const double bDiffDual = 1 - bDiff;
    const double cDiff = (hypercube.bounds[2].value - hypercube.bounds[2].lowerValue) /
                         (hypercube.bounds[2].higherValue - hypercube.bounds[2].lowerValue);
    const double cDiffDual = 1 - cDiff;
    const double dDiff = (hypercube.bounds[3].value - hypercube.bounds[3].lowerValue) /
                         (hypercube.bounds[3].higherValue - hypercube.bounds[3].lowerValue);
    const double dDiffDual = 1 - dDiff;

    // N-dimensional interpolation (general case)
    for (int i = 0; i < result.size(); ++i) {
        // linear interpolation in a
        const double a0 = buffer[hypercube.indexInBuffer[0] + i] * aDiffDual +
                          buffer[hypercube.indexInBuffer[1] + i] * aDiff;
        const double a1 = buffer[hypercube.indexInBuffer[2] + i] * aDiffDual +
                          buffer[hypercube.indexInBuffer[3] + i] * aDiff;
        const double a2 = buffer[hypercube.indexInBuffer[4] + i] * aDiffDual +
                          buffer[hypercube.indexInBuffer[5] + i] * aDiff;
        const double a3 = buffer[hypercube.indexInBuffer[6] + i] * aDiffDual +
                          buffer[hypercube.indexInBuffer[7] + i] * aDiff;
        const double a4 = buffer[hypercube.indexInBuffer[8] + i] * aDiffDual +
                          buffer[hypercube.indexInBuffer[9] + i] * aDiff;
        const double a5 = buffer[hypercube.indexInBuffer[10] + i] * aDiffDual +
                          buffer[hypercube.indexInBuffer[11] + i] * aDiff;
        const double a6 = buffer[hypercube.indexInBuffer[12] + i] * aDiffDual +
                          buffer[hypercube.indexInBuffer[13] + i] * aDiff;
        const double a7 = buffer[hypercube.indexInBuffer[14] + i] * aDiffDual +
                          buffer[hypercube.indexInBuffer[15] + i] * aDiff;

        const double b0 = a0 * bDiffDual + a1 * bDiff;
        const double b1 = a2 * bDiffDual + a3 * bDiff;
        const double b2 = a4 * bDiffDual + a5 * bDiff;
        const double b3 = a6 * bDiffDual + a7 * bDiff;

        const double c0 = b0 * cDiffDual + b1 * cDiff;
        const double c1 = b2 * cDiffDual + b3 * cDiff;

        result[i] = c0 * dDiffDual + c1 * dDiff;
    }
    return;
}

void Interpolator::interpolate5Dim(Hypercube &hypercube, const double *buffer,
                                   std::vector<double> &result) {
    // adjust inputs
    adjustSurroundingValues(hypercube, 5);

    // linear interpolation in 5 dimensions
    const double aDiff = (hypercube.bounds[0].value - hypercube.bounds[0].lowerValue) /
                         (hypercube.bounds[0].higherValue - hypercube.bounds[0].lowerValue);
    const double aDiffDual = 1 - aDiff;
    const double bDiff = (hypercube.bounds[1].value - hypercube.bounds[1].lowerValue) /
                         (hypercube.bounds[1].higherValue - hypercube.bounds[1].lowerValue);
    const double bDiffDual = 1 - bDiff;
    const double cDiff = (hypercube.bounds[2].value - hypercube.bounds[2].lowerValue) /
                         (hypercube.bounds[2].higherValue - hypercube.bounds[2].lowerValue);
    const double cDiffDual = 1 - cDiff;
    const double dDiff = (hypercube.bounds[3].value - hypercube.bounds[3].lowerValue) /
                         (hypercube.bounds[3].higherValue - hypercube.bounds[3].lowerValue);
    const double dDiffDual = 1 - dDiff;
    const double eDiff = (hypercube.bounds[4].value - hypercube.bounds[4].lowerValue) /
                         (hypercube.bounds[4].higherValue - hypercube.bounds[4].lowerValue);
    const double eDiffDual = 1 - eDiff;

    // N-dimensional interpolation (general case)
    for (int i = 0; i < result.size(); ++i) {
        // linear interpolation in a
        const double a0 = buffer[hypercube.indexInBuffer[0] + i] * aDiffDual +
                          buffer[hypercube.indexInBuffer[1] + i] * aDiff;
        const double a1 = buffer[hypercube.indexInBuffer[2] + i] * aDiffDual +
                          buffer[hypercube.indexInBuffer[3] + i] * aDiff;
        const double a2 = buffer[hypercube.indexInBuffer[4] + i] * aDiffDual +
                          buffer[hypercube.indexInBuffer[5] + i] * aDiff;
        const double a3 = buffer[hypercube.indexInBuffer[6] + i] * aDiffDual +
                          buffer[hypercube.indexInBuffer[7] + i] * aDiff;
        const double a4 = buffer[hypercube.indexInBuffer[8] + i] * aDiffDual +
                          buffer[hypercube.indexInBuffer[9] + i] * aDiff;
        const double a5 = buffer[hypercube.indexInBuffer[10] + i] * aDiffDual +
                          buffer[hypercube.indexInBuffer[11] + i] * aDiff;
        const double a6 = buffer[hypercube.indexInBuffer[12] + i] * aDiffDual +
                          buffer[hypercube.indexInBuffer[13] + i] * aDiff;
        const double a7 = buffer[hypercube.indexInBuffer[14] + i] * aDiffDual +
                          buffer[hypercube.indexInBuffer[15] + i] * aDiff;
        const double a8 = buffer[hypercube.indexInBuffer[16] + i] * aDiffDual +
                          buffer[hypercube.indexInBuffer[17] + i] * aDiff;
        const double a9 = buffer[hypercube.indexInBuffer[18] + i] * aDiffDual +
                          buffer[hypercube.indexInBuffer[19] + i] * aDiff;
        const double a10 = buffer[hypercube.indexInBuffer[20] + i] * aDiffDual +
                           buffer[hypercube.indexInBuffer[21] + i] * aDiff;
        const double a11 = buffer[hypercube.indexInBuffer[22] + i] * aDiffDual +
                           buffer[hypercube.indexInBuffer[23] + i] * aDiff;
        const double a12 = buffer[hypercube.indexInBuffer[24] + i] * aDiffDual +
                           buffer[hypercube.indexInBuffer[25] + i] * aDiff;
        const double a13 = buffer[hypercube.indexInBuffer[26] + i] * aDiffDual +
                           buffer[hypercube.indexInBuffer[27] + i] * aDiff;
        const double a14 = buffer[hypercube.indexInBuffer[28] + i] * aDiffDual +
                           buffer[hypercube.indexInBuffer[29] + i] * aDiff;
        const double a15 = buffer[hypercube.indexInBuffer[30] + i] * aDiffDual +
                           buffer[hypercube.indexInBuffer[31] + i] * aDiff;

        const double b0 = a0 * bDiffDual + a1 * bDiff;
        const double b1 = a2 * bDiffDual + a3 * bDiff;
        const double b2 = a4 * bDiffDual + a5 * bDiff;
        const double b3 = a6 * bDiffDual + a7 * bDiff;
        const double b4 = a8 * bDiffDual + a9 * bDiff;
        const double b5 = a10 * bDiffDual + a11 * bDiff;
        const double b6 = a12 * bDiffDual + a13 * bDiff;
        const double b7 = a14 * bDiffDual + a15 * bDiff;

        const double c0 = b0 * cDiffDual + b1 * cDiff;
        const double c1 = b2 * cDiffDual + b3 * cDiff;
        const double c2 = b4 * cDiffDual + b5 * cDiff;
        const double c3 = b6 * cDiffDual + b7 * cDiff;

        const double d0 = c0 * dDiffDual + c1 * dDiff;
        const double d1 = c2 * dDiffDual + c3 * dDiff;

        result[i] = d0 * eDiffDual + d1 * eDiff;
    }
    return;
}

void Interpolator::interpolate6Dim(Hypercube &hypercube, const double *buffer,
                                   std::vector<double> &result) {
    // adjust inputs
    adjustSurroundingValues(hypercube, 6);

    // linear interpolation in 6 dimensions
    const double aDiff = (hypercube.bounds[0].value - hypercube.bounds[0].lowerValue) /
                         (hypercube.bounds[0].higherValue - hypercube.bounds[0].lowerValue);
    const double aDiffDual = 1 - aDiff;
    const double bDiff = (hypercube.bounds[1].value - hypercube.bounds[1].lowerValue) /
                         (hypercube.bounds[1].higherValue - hypercube.bounds[1].lowerValue);
    const double bDiffDual = 1 - bDiff;
    const double cDiff = (hypercube.bounds[2].value - hypercube.bounds[2].lowerValue) /
                         (hypercube.bounds[2].higherValue - hypercube.bounds[2].lowerValue);
    const double cDiffDual = 1 - cDiff;
    const double dDiff = (hypercube.bounds[3].value - hypercube.bounds[3].lowerValue) /
                         (hypercube.bounds[3].higherValue - hypercube.bounds[3].lowerValue);
    const double dDiffDual = 1 - dDiff;
    const double eDiff = (hypercube.bounds[4].value - hypercube.bounds[4].lowerValue) /
                         (hypercube.bounds[4].higherValue - hypercube.bounds[4].lowerValue);
    const double eDiffDual = 1 - eDiff;
    const double fDiff = (hypercube.bounds[5].value - hypercube.bounds[5].lowerValue) /
                         (hypercube.bounds[5].higherValue - hypercube.bounds[5].lowerValue);
    const double fDiffDual = 1 - fDiff;

    // N-dimensional interpolation (general case)
    for (int i = 0; i < result.size(); ++i) {
        // linear interpolation in a
        const double a0 = buffer[hypercube.indexInBuffer[0] + i] * aDiffDual +
                          buffer[hypercube.indexInBuffer[1] + i] * aDiff;
        const double a1 = buffer[hypercube.indexInBuffer[2] + i] * aDiffDual +
                          buffer[hypercube.indexInBuffer[3] + i] * aDiff;
        const double a2 = buffer[hypercube.indexInBuffer[4] + i] * aDiffDual +
                          buffer[hypercube.indexInBuffer[5] + i] * aDiff;
        const double a3 = buffer[hypercube.indexInBuffer[6] + i] * aDiffDual +
                          buffer[hypercube.indexInBuffer[7] + i] * aDiff;
        const double a4 = buffer[hypercube.indexInBuffer[8] + i] * aDiffDual +
                          buffer[hypercube.indexInBuffer[9] + i] * aDiff;
        const double a5 = buffer[hypercube.indexInBuffer[10] + i] * aDiffDual +
                          buffer[hypercube.indexInBuffer[11] + i] * aDiff;
        const double a6 = buffer[hypercube.indexInBuffer[12] + i] * aDiffDual +
                          buffer[hypercube.indexInBuffer[13] + i] * aDiff;
        const double a7 = buffer[hypercube.indexInBuffer[14] + i] * aDiffDual +
                          buffer[hypercube.indexInBuffer[15] + i] * aDiff;
        const double a8 = buffer[hypercube.indexInBuffer[16] + i] * aDiffDual +
                          buffer[hypercube.indexInBuffer[17] + i] * aDiff;
        const double a9 = buffer[hypercube.indexInBuffer[18] + i] * aDiffDual +
                          buffer[hypercube.indexInBuffer[19] + i] * aDiff;
        const double a10 = buffer[hypercube.indexInBuffer[20] + i] * aDiffDual +
                           buffer[hypercube.indexInBuffer[21] + i] * aDiff;
        const double a11 = buffer[hypercube.indexInBuffer[22] + i] * aDiffDual +
                           buffer[hypercube.indexInBuffer[23] + i] * aDiff;
        const double a12 = buffer[hypercube.indexInBuffer[24] + i] * aDiffDual +
                           buffer[hypercube.indexInBuffer[25] + i] * aDiff;
        const double a13 = buffer[hypercube.indexInBuffer[26] + i] * aDiffDual +
                           buffer[hypercube.indexInBuffer[27] + i] * aDiff;
        const double a14 = buffer[hypercube.indexInBuffer[28] + i] * aDiffDual +
                           buffer[hypercube.indexInBuffer[29] + i] * aDiff;
        const double a15 = buffer[hypercube.indexInBuffer[30] + i] * aDiffDual +
                           buffer[hypercube.indexInBuffer[31] + i] * aDiff;
        const double a16 = buffer[hypercube.indexInBuffer[32] + i] * aDiffDual +
                           buffer[hypercube.indexInBuffer[33] + i] * aDiff;
        const double a17 = buffer[hypercube.indexInBuffer[34] + i] * aDiffDual +
                           buffer[hypercube.indexInBuffer[35] + i] * aDiff;
        const double a18 = buffer[hypercube.indexInBuffer[36] + i] * aDiffDual +
                           buffer[hypercube.indexInBuffer[37] + i] * aDiff;
        const double a19 = buffer[hypercube.indexInBuffer[38] + i] * aDiffDual +
                           buffer[hypercube.indexInBuffer[39] + i] * aDiff;
        const double a20 = buffer[hypercube.indexInBuffer[40] + i] * aDiffDual +
                           buffer[hypercube.indexInBuffer[41] + i] * aDiff;
        const double a21 = buffer[hypercube.indexInBuffer[42] + i] * aDiffDual +
                           buffer[hypercube.indexInBuffer[43] + i] * aDiff;
        const double a22 = buffer[hypercube.indexInBuffer[44] + i] * aDiffDual +
                           buffer[hypercube.indexInBuffer[45] + i] * aDiff;
        const double a23 = buffer[hypercube.indexInBuffer[46] + i] * aDiffDual +
                           buffer[hypercube.indexInBuffer[47] + i] * aDiff;
        const double a24 = buffer[hypercube.indexInBuffer[48] + i] * aDiffDual +
                           buffer[hypercube.indexInBuffer[49] + i] * aDiff;
        const double a25 = buffer[hypercube.indexInBuffer[50] + i] * aDiffDual +
                           buffer[hypercube.indexInBuffer[51] + i] * aDiff;
        const double a26 = buffer[hypercube.indexInBuffer[52] + i] * aDiffDual +
                           buffer[hypercube.indexInBuffer[53] + i] * aDiff;
        const double a27 = buffer[hypercube.indexInBuffer[54] + i] * aDiffDual +
                           buffer[hypercube.indexInBuffer[55] + i] * aDiff;
        const double a28 = buffer[hypercube.indexInBuffer[56] + i] * aDiffDual +
                           buffer[hypercube.indexInBuffer[57] + i] * aDiff;
        const double a29 = buffer[hypercube.indexInBuffer[58] + i] * aDiffDual +
                           buffer[hypercube.indexInBuffer[59] + i] * aDiff;
        const double a30 = buffer[hypercube.indexInBuffer[60] + i] * aDiffDual +
                           buffer[hypercube.indexInBuffer[61] + i] * aDiff;
        const double a31 = buffer[hypercube.indexInBuffer[62] + i] * aDiffDual +
                           buffer[hypercube.indexInBuffer[63] + i] * aDiff;

        const double b0 = a0 * bDiffDual + a1 * bDiff;
        const double b1 = a2 * bDiffDual + a3 * bDiff;
        const double b2 = a4 * bDiffDual + a5 * bDiff;
        const double b3 = a6 * bDiffDual + a7 * bDiff;
        const double b4 = a8 * bDiffDual + a9 * bDiff;
        const double b5 = a10 * bDiffDual + a11 * bDiff;
        const double b6 = a12 * bDiffDual + a13 * bDiff;
        const double b7 = a14 * bDiffDual + a15 * bDiff;
        const double b8 = a16 * bDiffDual + a17 * bDiff;
        const double b9 = a18 * bDiffDual + a19 * bDiff;
        const double b10 = a20 * bDiffDual + a21 * bDiff;
        const double b11 = a22 * bDiffDual + a23 * bDiff;
        const double b12 = a24 * bDiffDual + a25 * bDiff;
        const double b13 = a26 * bDiffDual + a27 * bDiff;
        const double b14 = a28 * bDiffDual + a29 * bDiff;
        const double b15 = a30 * bDiffDual + a31 * bDiff;

        const double c0 = b0 * cDiffDual + b1 * cDiff;
        const double c1 = b2 * cDiffDual + b3 * cDiff;
        const double c2 = b4 * cDiffDual + b5 * cDiff;
        const double c3 = b6 * cDiffDual + b7 * cDiff;
        const double c4 = b8 * cDiffDual + b9 * cDiff;
        const double c5 = b10 * cDiffDual + b11 * cDiff;
        const double c6 = b12 * cDiffDual + b13 * cDiff;
        const double c7 = b14 * cDiffDual + b15 * cDiff;

        const double d0 = c0 * dDiffDual + c1 * dDiff;
        const double d1 = c2 * dDiffDual + c3 * dDiff;
        const double d2 = c4 * dDiffDual + c5 * dDiff;
        const double d3 = c6 * dDiffDual + c7 * dDiff;

        const double e0 = d0 * eDiffDual + d1 * eDiff;
        const double e1 = d2 * eDiffDual + d3 * eDiff;

        result[i] = e0 * fDiffDual + e1 * fDiff;
    }
    return;
}

/*
 Adjusts the pairs given as surroundingValues to prevent division by zero. Do
 that by detecting pairs with two identical entries. Due to the nature of the
 interpolation it is possible to simply change one of the entries to a
 different value. This results in a factor of zero in the interpolation, so the
 exact chosen value is irrelevant.
*/
void Interpolator::adjustSurroundingValues(Hypercube &hypercube, uint32_t N) {
    for (int i = 0; i < N; ++i) {
        if (hypercube.bounds[i].lowerValue == hypercube.bounds[i].higherValue) {
            hypercube.bounds[i].higherValue += 1;
        }
    }
}

} // namespace FLUT