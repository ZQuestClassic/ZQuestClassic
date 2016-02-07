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

#include "FFT.h"
#include "system/Thread.h"
#include "system/Allocators.h"
#include "system/VectorOps.h"
#include "system/VectorOpsComplex.h"

#include "kissfft/kiss_fftr.h"

#include <cmath>
#include <iostream>
#include <map>
#include <cstdio>
#include <cstdlib>
#include <vector>

namespace RubberBand {

class FFTImpl
{
public:
    virtual ~FFTImpl() { }

    virtual FFT::Precisions getSupportedPrecisions() const = 0;

    virtual void initFloat() = 0;
    virtual void initDouble() = 0;

    virtual void forward(const double *realIn, double *realOut, double *imagOut) = 0;
    virtual void forwardInterleaved(const double *realIn, double *complexOut) = 0;
    virtual void forwardPolar(const double *realIn, double *magOut, double *phaseOut) = 0;
    virtual void forwardMagnitude(const double *realIn, double *magOut) = 0;

    virtual void forward(const float *realIn, float *realOut, float *imagOut) = 0;
    virtual void forwardInterleaved(const float *realIn, float *complexOut) = 0;
    virtual void forwardPolar(const float *realIn, float *magOut, float *phaseOut) = 0;
    virtual void forwardMagnitude(const float *realIn, float *magOut) = 0;

    virtual void inverse(const double *realIn, const double *imagIn, double *realOut) = 0;
    virtual void inverseInterleaved(const double *complexIn, double *realOut) = 0;
    virtual void inversePolar(const double *magIn, const double *phaseIn, double *realOut) = 0;
    virtual void inverseCepstral(const double *magIn, double *cepOut) = 0;

    virtual void inverse(const float *realIn, const float *imagIn, float *realOut) = 0;
    virtual void inverseInterleaved(const float *complexIn, float *realOut) = 0;
    virtual void inversePolar(const float *magIn, const float *phaseIn, float *realOut) = 0;
    virtual void inverseCepstral(const float *magIn, float *cepOut) = 0;
};

namespace FFTs {

class D_KISSFFT : public FFTImpl
{
public:
    D_KISSFFT(int size) :
        m_size(size),
        m_fplanf(0),
        m_fplani(0)
    {
#ifdef FIXED_POINT
#error KISSFFT is not configured for float values
#endif
        if (sizeof(kiss_fft_scalar) != sizeof(float)) {
            std::cerr << "ERROR: KISSFFT is not configured for float values"
                      << std::endl;
        }

        m_fbuf = new kiss_fft_scalar[m_size + 2];
        m_fpacked = new kiss_fft_cpx[m_size + 2];
        m_fplanf = kiss_fftr_alloc(m_size, 0, NULL, NULL);
        m_fplani = kiss_fftr_alloc(m_size, 1, NULL, NULL);
    }

    ~D_KISSFFT() {
        kiss_fftr_free(m_fplanf);
        kiss_fftr_free(m_fplani);
        kiss_fft_cleanup();

        delete[] m_fbuf;
        delete[] m_fpacked;
    }

    FFT::Precisions
    getSupportedPrecisions() const {
        return FFT::SinglePrecision;
    }

    void initFloat() { }
    void initDouble() { }

    void packFloat(const float *re, const float *im) {
        const int hs = m_size/2;
        for (int i = 0; i <= hs; ++i) {
            m_fpacked[i].r = re[i];
        }
        if (im) {
            for (int i = 0; i <= hs; ++i) {
                m_fpacked[i].i = im[i];
            }
        } else {
            for (int i = 0; i <= hs; ++i) {
                m_fpacked[i].i = 0.f;
            }
        }
    }

    void unpackFloat(float *re, float *im) {
        const int hs = m_size/2;
        for (int i = 0; i <= hs; ++i) {
            re[i] = m_fpacked[i].r;
        }
        if (im) {
            for (int i = 0; i <= hs; ++i) {
                im[i] = m_fpacked[i].i;
            }
        }
    }

    void packDouble(const double *re, const double *im) {
        const int hs = m_size/2;
        for (int i = 0; i <= hs; ++i) {
            m_fpacked[i].r = float(re[i]);
        }
        if (im) {
            for (int i = 0; i <= hs; ++i) {
                m_fpacked[i].i = float(im[i]);
            }
        } else {
            for (int i = 0; i <= hs; ++i) {
                m_fpacked[i].i = 0.f;
            }
        }
    }

    void unpackDouble(double *re, double *im) {
        const int hs = m_size/2;
        for (int i = 0; i <= hs; ++i) {
            re[i] = double(m_fpacked[i].r);
        }
        if (im) {
            for (int i = 0; i <= hs; ++i) {
                im[i] = double(m_fpacked[i].i);
            }
        }
    }

    void forward(const double *realIn, double *realOut, double *imagOut) {

        v_convert(m_fbuf, realIn, m_size);
        kiss_fftr(m_fplanf, m_fbuf, m_fpacked);
        unpackDouble(realOut, imagOut);
    }

    void forwardInterleaved(const double *realIn, double *complexOut) {

        v_convert(m_fbuf, realIn, m_size);
        kiss_fftr(m_fplanf, m_fbuf, m_fpacked);
        v_convert(complexOut, (float *)m_fpacked, m_size + 2);
    }

    void forwardPolar(const double *realIn, double *magOut, double *phaseOut) {

        for (int i = 0; i < m_size; ++i) {
            m_fbuf[i] = float(realIn[i]);
        }

        kiss_fftr(m_fplanf, m_fbuf, m_fpacked);

        const int hs = m_size/2;

        for (int i = 0; i <= hs; ++i) {
            magOut[i] = sqrt(double(m_fpacked[i].r) * double(m_fpacked[i].r) +
                             double(m_fpacked[i].i) * double(m_fpacked[i].i));
        }

        for (int i = 0; i <= hs; ++i) {
            phaseOut[i] = atan2(double(m_fpacked[i].i), double(m_fpacked[i].r));
        }
    }

    void forwardMagnitude(const double *realIn, double *magOut) {

        for (int i = 0; i < m_size; ++i) {
            m_fbuf[i] = float(realIn[i]);
        }

        kiss_fftr(m_fplanf, m_fbuf, m_fpacked);

        const int hs = m_size/2;

        for (int i = 0; i <= hs; ++i) {
            magOut[i] = sqrt(double(m_fpacked[i].r) * double(m_fpacked[i].r) +
                             double(m_fpacked[i].i) * double(m_fpacked[i].i));
        }
    }

    void forward(const float *realIn, float *realOut, float *imagOut) {

        kiss_fftr(m_fplanf, realIn, m_fpacked);
        unpackFloat(realOut, imagOut);
    }

    void forwardInterleaved(const float *realIn, float *complexOut) {

        kiss_fftr(m_fplanf, realIn, (kiss_fft_cpx *)complexOut);
    }

    void forwardPolar(const float *realIn, float *magOut, float *phaseOut) {

        kiss_fftr(m_fplanf, realIn, m_fpacked);

        const int hs = m_size/2;

        for (int i = 0; i <= hs; ++i) {
            magOut[i] = sqrtf(m_fpacked[i].r * m_fpacked[i].r +
                              m_fpacked[i].i * m_fpacked[i].i);
        }

        for (int i = 0; i <= hs; ++i) {
            phaseOut[i] = atan2f(m_fpacked[i].i, m_fpacked[i].r);
        }
    }

    void forwardMagnitude(const float *realIn, float *magOut) {

        kiss_fftr(m_fplanf, realIn, m_fpacked);

        const int hs = m_size/2;

        for (int i = 0; i <= hs; ++i) {
            magOut[i] = sqrtf(m_fpacked[i].r * m_fpacked[i].r +
                              m_fpacked[i].i * m_fpacked[i].i);
        }
    }

    void inverse(const double *realIn, const double *imagIn, double *realOut) {

        packDouble(realIn, imagIn);

        kiss_fftri(m_fplani, m_fpacked, m_fbuf);

        for (int i = 0; i < m_size; ++i) {
            realOut[i] = m_fbuf[i];
        }
    }

    void inverseInterleaved(const double *complexIn, double *realOut) {

        v_convert((float *)m_fpacked, complexIn, m_size + 2);

        kiss_fftri(m_fplani, m_fpacked, m_fbuf);

        for (int i = 0; i < m_size; ++i) {
            realOut[i] = m_fbuf[i];
        }
    }

    void inversePolar(const double *magIn, const double *phaseIn, double *realOut) {

        const int hs = m_size/2;

        for (int i = 0; i <= hs; ++i) {
            m_fpacked[i].r = float(magIn[i] * cos(phaseIn[i]));
            m_fpacked[i].i = float(magIn[i] * sin(phaseIn[i]));
        }

        kiss_fftri(m_fplani, m_fpacked, m_fbuf);

        for (int i = 0; i < m_size; ++i) {
            realOut[i] = m_fbuf[i];
        }
    }

    void inverseCepstral(const double *magIn, double *cepOut) {

        const int hs = m_size/2;

        for (int i = 0; i <= hs; ++i) {
            m_fpacked[i].r = float(log(magIn[i] + 0.000001));
            m_fpacked[i].i = 0.0f;
        }

        kiss_fftri(m_fplani, m_fpacked, m_fbuf);

        for (int i = 0; i < m_size; ++i) {
            cepOut[i] = m_fbuf[i];
        }
    }

    void inverse(const float *realIn, const float *imagIn, float *realOut) {

        packFloat(realIn, imagIn);
        kiss_fftri(m_fplani, m_fpacked, realOut);
    }

    void inverseInterleaved(const float *complexIn, float *realOut) {

        v_copy((float *)m_fpacked, complexIn, m_size + 2);
        kiss_fftri(m_fplani, m_fpacked, realOut);
    }

    void inversePolar(const float *magIn, const float *phaseIn, float *realOut) {

        const int hs = m_size/2;

        for (int i = 0; i <= hs; ++i) {
            m_fpacked[i].r = magIn[i] * cosf(phaseIn[i]);
            m_fpacked[i].i = magIn[i] * sinf(phaseIn[i]);
        }

        kiss_fftri(m_fplani, m_fpacked, realOut);
    }

    void inverseCepstral(const float *magIn, float *cepOut) {

        const int hs = m_size/2;

        for (int i = 0; i <= hs; ++i) {
            m_fpacked[i].r = logf(magIn[i] + 0.000001f);
            m_fpacked[i].i = 0.0f;
        }

        kiss_fftri(m_fplani, m_fpacked, cepOut);
    }

private:
    const int m_size;
    kiss_fftr_cfg m_fplanf;
    kiss_fftr_cfg m_fplani;
    kiss_fft_scalar *m_fbuf;
    kiss_fft_cpx *m_fpacked;
};

} /* end namespace FFTs */

std::string
FFT::m_implementation;

std::set<std::string>
FFT::getImplementations()
{
    std::set<std::string> impls;
    impls.insert("kissfft");
    return impls;
}

void
FFT::pickDefaultImplementation()
{
    if (m_implementation != "") return;

    std::set<std::string> impls = getImplementations();

    m_implementation = "kissfft";
}

std::string
FFT::getDefaultImplementation()
{
    return m_implementation;
}

void
FFT::setDefaultImplementation(std::string i)
{
    m_implementation = i;
}

FFT::FFT(int size, int debugLevel) :
    d(0)
{
    if ((size < 2) ||
        (size & (size-1))) {
        std::cerr << "FFT::FFT(" << size << "): power-of-two sizes only supported, minimum size 2" << std::endl;
        throw InvalidSize;
    }

    if (m_implementation == "") pickDefaultImplementation();
    std::string impl = m_implementation;

    if (debugLevel > 0) {
        std::cerr << "FFT::FFT(" << size << "): using implementation: "
                  << impl << std::endl;
    }

    d = new FFTs::D_KISSFFT(size);

    if (!d) {
        std::cerr << "FFT::FFT(" << size << "): ERROR: implementation "
                  << impl << " is not compiled in" << std::endl;
        throw InvalidImplementation;
    }
}

FFT::~FFT()
{
    delete d;
}

#define CHECK_NOT_NULL(x) \
    if (!(x)) { \
        std::cerr << "FFT: ERROR: Null argument " #x << std::endl;  \
        throw NullArgument; \
    }

void
FFT::forward(const double *realIn, double *realOut, double *imagOut)
{
    CHECK_NOT_NULL(realIn);
    CHECK_NOT_NULL(realOut);
    CHECK_NOT_NULL(imagOut);
    d->forward(realIn, realOut, imagOut);
}

void
FFT::forwardInterleaved(const double *realIn, double *complexOut)
{
    CHECK_NOT_NULL(realIn);
    CHECK_NOT_NULL(complexOut);
    d->forwardInterleaved(realIn, complexOut);
}

void
FFT::forwardPolar(const double *realIn, double *magOut, double *phaseOut)
{
    CHECK_NOT_NULL(realIn);
    CHECK_NOT_NULL(magOut);
    CHECK_NOT_NULL(phaseOut);
    d->forwardPolar(realIn, magOut, phaseOut);
}

void
FFT::forwardMagnitude(const double *realIn, double *magOut)
{
    CHECK_NOT_NULL(realIn);
    CHECK_NOT_NULL(magOut);
    d->forwardMagnitude(realIn, magOut);
}

void
FFT::forward(const float *realIn, float *realOut, float *imagOut)
{
    CHECK_NOT_NULL(realIn);
    CHECK_NOT_NULL(realOut);
    CHECK_NOT_NULL(imagOut);
    d->forward(realIn, realOut, imagOut);
}

void
FFT::forwardInterleaved(const float *realIn, float *complexOut)
{
    CHECK_NOT_NULL(realIn);
    CHECK_NOT_NULL(complexOut);
    d->forwardInterleaved(realIn, complexOut);
}

void
FFT::forwardPolar(const float *realIn, float *magOut, float *phaseOut)
{
    CHECK_NOT_NULL(realIn);
    CHECK_NOT_NULL(magOut);
    CHECK_NOT_NULL(phaseOut);
    d->forwardPolar(realIn, magOut, phaseOut);
}

void
FFT::forwardMagnitude(const float *realIn, float *magOut)
{
    CHECK_NOT_NULL(realIn);
    CHECK_NOT_NULL(magOut);
    d->forwardMagnitude(realIn, magOut);
}

void
FFT::inverse(const double *realIn, const double *imagIn, double *realOut)
{
    CHECK_NOT_NULL(realIn);
    CHECK_NOT_NULL(imagIn);
    CHECK_NOT_NULL(realOut);
    d->inverse(realIn, imagIn, realOut);
}

void
FFT::inverseInterleaved(const double *complexIn, double *realOut)
{
    CHECK_NOT_NULL(complexIn);
    CHECK_NOT_NULL(realOut);
    d->inverseInterleaved(complexIn, realOut);
}

void
FFT::inversePolar(const double *magIn, const double *phaseIn, double *realOut)
{
    CHECK_NOT_NULL(magIn);
    CHECK_NOT_NULL(phaseIn);
    CHECK_NOT_NULL(realOut);
    d->inversePolar(magIn, phaseIn, realOut);
}

void
FFT::inverseCepstral(const double *magIn, double *cepOut)
{
    CHECK_NOT_NULL(magIn);
    CHECK_NOT_NULL(cepOut);
    d->inverseCepstral(magIn, cepOut);
}

void
FFT::inverse(const float *realIn, const float *imagIn, float *realOut)
{
    CHECK_NOT_NULL(realIn);
    CHECK_NOT_NULL(imagIn);
    CHECK_NOT_NULL(realOut);
    d->inverse(realIn, imagIn, realOut);
}

void
FFT::inverseInterleaved(const float *complexIn, float *realOut)
{
    CHECK_NOT_NULL(complexIn);
    CHECK_NOT_NULL(realOut);
    d->inverseInterleaved(complexIn, realOut);
}

void
FFT::inversePolar(const float *magIn, const float *phaseIn, float *realOut)
{
    CHECK_NOT_NULL(magIn);
    CHECK_NOT_NULL(phaseIn);
    CHECK_NOT_NULL(realOut);
    d->inversePolar(magIn, phaseIn, realOut);
}

void
FFT::inverseCepstral(const float *magIn, float *cepOut)
{
    CHECK_NOT_NULL(magIn);
    CHECK_NOT_NULL(cepOut);
    d->inverseCepstral(magIn, cepOut);
}

void
FFT::initFloat()
{
    d->initFloat();
}

void
FFT::initDouble()
{
    d->initDouble();
}

FFT::Precisions
FFT::getSupportedPrecisions() const
{
    return d->getSupportedPrecisions();
}

}
