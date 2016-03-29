/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

/*
    Rubber Band Library
    An audio time-stretching and pitch-shifting library.
    Copyright 2007-2014 Particular Programs Ltd.

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.

    Alternatively, if you have a valid commercial licence for the
    Rubber Band Library obtained by agreement with the copyright
    holders, you may redistribute and/or modify it under the terms
    described in that licence.

    If you wish to distribute code using the Rubber Band Library
    under terms other than those of the GNU General Public License,
    you must obtain a valid commercial licence before doing so.
*/

#ifndef _RUBBERBAND_FFT_H_
#define _RUBBERBAND_FFT_H_

#include "system/sysutils.h"

#include <string>
#include <set>

namespace RubberBand {

class FFTImpl;

/**
 * Provide the basic FFT computations we need, using one of a set of
 * candidate FFT implementations (depending on compile flags).
 *
 * Implements real->complex FFTs of power-of-two sizes only.  Note
 * that only the first half of the output signal is returned (the
 * complex conjugates half is omitted), so the "complex" arrays need
 * room for size/2+1 elements.
 *
 * The "interleaved" functions use the format sometimes called CCS --
 * size/2+1 real+imaginary pairs.  So, the array elements at indices 1
 * and size+1 will always be zero (since the signal is real).
 *
 * All pointer arguments must point to valid data. A NullArgument
 * exception is thrown if any argument is NULL.
 *
 * Neither forward nor inverse transform is scaled.
 *
 * This class is reentrant but not thread safe: use a separate
 * instance per thread, or use a mutex.
 */
class FFT
{
public:
    enum Exception {
        NullArgument, InvalidSize, InvalidImplementation, InternalError
    };

    FFT(int size, int debugLevel = 0); // may throw InvalidSize
    ~FFT();

    void forward(const double *realIn, double *realOut, double *imagOut);
    void forwardInterleaved(const double *realIn, double *complexOut);
    void forwardPolar(const double *realIn, double *magOut, double *phaseOut);
    void forwardMagnitude(const double *realIn, double *magOut);

    void forward(const float *realIn, float *realOut, float *imagOut);
    void forwardInterleaved(const float *realIn, float *complexOut);
    void forwardPolar(const float *realIn, float *magOut, float *phaseOut);
    void forwardMagnitude(const float *realIn, float *magOut);

    void inverse(const double *realIn, const double *imagIn, double *realOut);
    void inverseInterleaved(const double *complexIn, double *realOut);
    void inversePolar(const double *magIn, const double *phaseIn, double *realOut);
    void inverseCepstral(const double *magIn, double *cepOut);

    void inverse(const float *realIn, const float *imagIn, float *realOut);
    void inverseInterleaved(const float *complexIn, float *realOut);
    void inversePolar(const float *magIn, const float *phaseIn, float *realOut);
    void inverseCepstral(const float *magIn, float *cepOut);

    // Calling one or both of these is optional -- if neither is
    // called, the first call to a forward or inverse method will call
    // init().  You only need call these if you don't want to risk
    // expensive allocations etc happening in forward or inverse.
    void initFloat();
    void initDouble();

    enum Precision {
        SinglePrecision = 0x1,
        DoublePrecision = 0x2
    };
    typedef int Precisions;

    /**
     * Return the OR of all precisions supported by this
     * implementation. All of the functions (float and double) are
     * available regardless of the supported implementations, but they
     * will be calculated at the proper precision only if it is
     * available. (So float functions will be calculated using doubles
     * and then truncated if single-precision is unavailable, and
     * double functions will use single-precision arithmetic if double
     * is unavailable.)
     */
    Precisions getSupportedPrecisions() const;

    static std::set<std::string> getImplementations();
    static std::string getDefaultImplementation();
    static void setDefaultImplementation(std::string);

protected:
    FFTImpl *d;
    static std::string m_implementation;
    static void pickDefaultImplementation();
};

}

#endif

