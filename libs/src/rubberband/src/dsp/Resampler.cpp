/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*- vi:set ts=8 sts=4 sw=4: */

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

#include "Resampler.h"

#include <cstdlib>
#include <cmath>

#include <iostream>

#include "system/Allocators.h"
#include "speex/speex_resampler.h"

namespace RubberBand {

class ResamplerImpl
{
public:
    virtual ~ResamplerImpl() { }

    virtual int resample(const float *const *const in,
                         float *const *const out,
                         int incount,
                         float ratio,
                         bool final) = 0;

    virtual int resampleInterleaved(const float *const in,
                                    float *const out,
                                    int incount,
                                    float ratio,
                                    bool final) = 0;

    virtual int getChannelCount() const = 0;

    virtual void reset() = 0;
};

namespace Resamplers {

class D_Speex : public ResamplerImpl
{
public:
    D_Speex(Resampler::Quality quality, int channels, int maxBufferSize,
            int debugLevel);
    ~D_Speex();

    int resample(const float *const *const in,
                 float *const *const out,
                 int incount,
                 float ratio,
                 bool final);

    int resampleInterleaved(const float *const in,
                            float *const out,
                            int incount,
                            float ratio,
                            bool final = false);

    int getChannelCount() const { return m_channels; }

    void reset();

protected:
    SpeexResamplerState *m_resampler;
    float *m_iin;
    float *m_iout;
    int m_channels;
    int m_iinsize;
    int m_ioutsize;
    float m_lastratio;
    bool m_initial;
    int m_debugLevel;

    void setRatio(float);
};

D_Speex::D_Speex(Resampler::Quality quality, int channels, int maxBufferSize,
                 int debugLevel) :
    m_resampler(0),
    m_iin(0),
    m_iout(0),
    m_channels(channels),
    m_iinsize(0),
    m_ioutsize(0),
    m_lastratio(1),
    m_initial(true),
    m_debugLevel(debugLevel)
{
    int q = (quality == Resampler::Best ? 10 :
             quality == Resampler::Fastest ? 0 : 4);

    if (m_debugLevel > 0) {
        std::cerr << "Resampler::Resampler: using Speex implementation with q = "
                  << q
                  << std::endl;
    }

    int err = 0;
    m_resampler = speex_resampler_init_frac(m_channels,
                                            1, 1,
                                            48000, 48000, // irrelevant
                                            q,
                                            &err);


    if (err) {
        std::cerr << "Resampler::Resampler: failed to create Speex resampler"
                  << std::endl;
        throw Resampler::ImplementationError;
    }

    if (maxBufferSize > 0 && m_channels > 1) {
        m_iinsize = maxBufferSize * m_channels;
        m_ioutsize = maxBufferSize * m_channels * 2;
        m_iin = allocate<float>(m_iinsize);
        m_iout = allocate<float>(m_ioutsize);
    }
}

D_Speex::~D_Speex()
{
    speex_resampler_destroy(m_resampler);
    deallocate<float>(m_iin);
    deallocate<float>(m_iout);
}

void
D_Speex::setRatio(float ratio)
{
    // Speex wants a ratio of two unsigned integers, not a single
    // float.  Let's do that.

    unsigned int big = 272408136U;
    unsigned int denom = 1, num = 1;

    if (ratio < 1.f) {
        denom = big;
        double dnum = double(big) * double(ratio);
        num = (unsigned int)dnum;
    } else if (ratio > 1.f) {
        num = big;
        double ddenom = double(big) / double(ratio);
        denom = (unsigned int)ddenom;
    }

    if (m_debugLevel > 1) {
        std::cerr << "D_Speex: Desired ratio " << ratio << ", requesting ratio "
                  << num << "/" << denom << " = " << float(double(num)/double(denom))
                  << std::endl;
    }

    int err = speex_resampler_set_rate_frac
        (m_resampler, denom, num, 48000, 48000);
    //!!! check err

    speex_resampler_get_ratio(m_resampler, &denom, &num);

    if (m_debugLevel > 1) {
        std::cerr << "D_Speex: Desired ratio " << ratio << ", got ratio "
                  << num << "/" << denom << " = " << float(double(num)/double(denom))
                  << std::endl;
    }

    m_lastratio = ratio;

    if (m_initial) {
        speex_resampler_skip_zeros(m_resampler);
        m_initial = false;
    }
}

int
D_Speex::resample(const float *const *const in,
                  float *const *const out,
                  int incount,
                  float ratio,
                  bool final)
{
    if (ratio != m_lastratio) {
        setRatio(ratio);
    }

    unsigned int uincount = incount;
    unsigned int outcount = lrintf(ceilf(incount * ratio)); //!!! inexact now

    float *data_in, *data_out;

    if (m_channels == 1) {
        data_in = const_cast<float *>(*in);
        data_out = *out;
    } else {
        if (incount * m_channels > m_iinsize) {
            m_iin = reallocate<float>(m_iin, m_iinsize, incount * m_channels);
            m_iinsize = incount * m_channels;
        }
        if (outcount * m_channels > m_ioutsize) {
            m_iout = reallocate<float>(m_iout, m_ioutsize, outcount * m_channels);
            m_ioutsize = outcount * m_channels;
        }
        v_interleave(m_iin, in, m_channels, incount);
        data_in = m_iin;
        data_out = m_iout;
    }

    int err = speex_resampler_process_interleaved_float(m_resampler,
                                                        data_in,
                                                        &uincount,
                                                        data_out,
                                                        &outcount);

//    if (incount != int(uincount)) {
//        std::cerr << "Resampler: NOTE: Consumed " << uincount
//                  << " of " << incount << " frames" << std::endl;
//    }

//    if (outcount != lrintf(ceilf(incount * ratio))) {
//        std::cerr << "Resampler: NOTE: Obtained " << outcount
//                  << " of " << lrintf(ceilf(incount * ratio)) << " frames"
//                  << std::endl;
//    }

    //!!! check err, respond appropriately


    if (m_channels > 1) {
        v_deinterleave(out, m_iout, m_channels, outcount);
    }

    return outcount;
}

int
D_Speex::resampleInterleaved(const float *const in,
                             float *const out,
                             int incount,
                             float ratio,
                             bool final)
{
    if (ratio != m_lastratio) {
        setRatio(ratio);
    }

    unsigned int uincount = incount;
    unsigned int outcount = lrintf(ceilf(incount * ratio)); //!!! inexact now

    float *data_in = const_cast<float *>(in);
    float *data_out = out;

    int err = speex_resampler_process_interleaved_float(m_resampler,
                                                        data_in,
                                                        &uincount,
                                                        data_out,
                                                        &outcount);

//    std::cerr << "D_SPEEX: incount " << incount << " ratio " << ratio << " req " << lrintf(ceilf(incount * ratio)) << " final " << final << " output_frames_gen " << outcount << std::endl;

    return outcount;
}

void
D_Speex::reset()
{
    speex_resampler_reset_mem(m_resampler);
}

} /* end namespace Resamplers */

Resampler::Resampler(Resampler::Quality quality, int channels,
                     int maxBufferSize, int debugLevel)
{
    m_method = -1;

    switch (quality) {

    case Resampler::Best:
        m_method = 2;
        break;

    case Resampler::FastestTolerable:
        m_method = 2;
        break;

    case Resampler::Fastest:
        m_method = 2;
        break;
    }

    if (m_method == -1) {
        std::cerr << "Resampler::Resampler(" << quality << ", " << channels
                  << ", " << maxBufferSize << "): No implementation available!"
                  << std::endl;
        abort();
    }

    switch (m_method) {
    case 0:
        std::cerr << "Resampler::Resampler(" << quality << ", " << channels
                  << ", " << maxBufferSize << "): No implementation available!"
                  << std::endl;
        abort();
        break;

    case 1:
        std::cerr << "Resampler::Resampler(" << quality << ", " << channels
                  << ", " << maxBufferSize << "): No implementation available!"
                  << std::endl;
        abort();
        break;

    case 2:
        d = new Resamplers::D_Speex(quality, channels, maxBufferSize, debugLevel);
        break;

    case 3:
        std::cerr << "Resampler::Resampler(" << quality << ", " << channels
                  << ", " << maxBufferSize << "): No implementation available!"
                  << std::endl;
        abort();
        break;
    }

    if (!d) {
        std::cerr << "Resampler::Resampler(" << quality << ", " << channels
                  << ", " << maxBufferSize
                  << "): Internal error: No implementation selected"
                  << std::endl;
        abort();
    }
}

Resampler::~Resampler()
{
    delete d;
}

int
Resampler::resample(const float *const *const in,
                    float *const *const out,
                    int incount, float ratio, bool final)
{
    return d->resample(in, out, incount, ratio, final);
}

int
Resampler::resampleInterleaved(const float *const in,
                               float *const out,
                               int incount, float ratio, bool final)
{
    return d->resampleInterleaved(in, out, incount, ratio, final);
}

int
Resampler::getChannelCount() const
{
    return d->getChannelCount();
}

void
Resampler::reset()
{
    d->reset();
}

}
