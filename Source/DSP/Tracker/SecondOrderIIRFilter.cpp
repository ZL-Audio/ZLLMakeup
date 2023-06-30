/*
===============================================================================

SecondOrderIIRFilter.cpp


By Samuel Gaehwiler from Klangfreund.
Used in the klangfreund.com/lufsmeter/

License: MIT

I'd be happy to hear about your usage of this code!
-> klangfreund.com/contact/

-------------------------------------------------------------------------------

The MIT License (MIT)

Copyright (c) 2018 Klangfreund, Samuel Gaehwiler

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.

===============================================================================
*/

#include "SecondOrderIIRFilter.h"

template<typename FloatType>
SecondOrderIIRFilter<FloatType>::SecondOrderIIRFilter(FloatType b0_at48k_,
                                                      FloatType b1_at48k_,
                                                      FloatType b2_at48k_,
                                                      FloatType a1_at48k_,
                                                      FloatType a2_at48k_)
        : b0_at48k{b0_at48k_}, b1_at48k{b1_at48k_}, b2_at48k{b2_at48k_},
          a1_at48k{a1_at48k_}, a2_at48k{a2_at48k_},

          b0{b0_at48k_}, b1{b1_at48k_}, b2{b2_at48k_},
          a1{a1_at48k_}, a2{a2_at48k_},
          numberOfChannels{0} {
    // Determine the values Q, VH, VB, VL and arctanK.
    // See 111222_my_notes_to_the_calculation_of_the_filter_coefficients.tif
    // for the derivations of these equations.
    const FloatType KoverQ = (2 - 2 * a2_at48k) / (a2_at48k - a1_at48k + 1);
    const FloatType K = sqrt((a1_at48k + a2_at48k + 1) / (a2_at48k - a1_at48k + 1));
    Q = K / KoverQ;
    arctanK = atan(K);
    VB = (b0_at48k - b2_at48k) / (1 - a2_at48k);
    VH = (b0_at48k - b1_at48k + b2_at48k) / (a2_at48k - a1_at48k + 1);
    VL = (b0_at48k + b1_at48k + b2_at48k) / (a1_at48k + a2_at48k + 1);
}

template<typename FloatType>
SecondOrderIIRFilter<FloatType>::~SecondOrderIIRFilter() = default;

//==============================================================================
template<typename FloatType>
void SecondOrderIIRFilter<FloatType>::prepareToPlay(FloatType sampleRate,
                                                    int numberOfChannels_) {
// DEB("prepareToPlay called.")

    numberOfChannels = numberOfChannels_;

// Initialize z1 and z2.
// calloc: Allocates a specified amount of memory and clears it.
    z1.calloc(numberOfChannels);
    z2.calloc(numberOfChannels);

// Determine the filter coefficients.
    const FloatType sampleRate48k = 48000;
    if (sampleRate == sampleRate48k) {
        b0 = b0_at48k;
        b1 = b1_at48k;
        b2 = b2_at48k;
        a1 = a1_at48k;
        a2 = a2_at48k;
    } else {
// See 111222_my_notes_to_the_calculation_of_the_filter_coefficients.tif
// for the derivations of these equations.
        const FloatType K = tan(arctanK * sampleRate48k / sampleRate);
        const FloatType commonFactor = 1 / (1 + K / Q + K * K);
        b0 = (VH + VB * K / Q + VL * K * K) * commonFactor;
        b1 = 2 * (VL * K * K - VH) * commonFactor;
        b2 = (VH - VB * K / Q + VL * K * K) * commonFactor;
        a1 = 2 * (K * K - 1) * commonFactor;
        a2 = (1 - K / Q + K * K) * commonFactor;
    }
}

template<typename FloatType>
void SecondOrderIIRFilter<FloatType>::releaseResources() {
}

template<typename FloatType>
void SecondOrderIIRFilter<FloatType>::processBlock(juce::AudioBuffer<FloatType> &buffer) {
    const int numOfChannels = juce::jmin(numberOfChannels, buffer.getNumChannels());

    for (int channel = 0; channel < numOfChannels; ++channel) {
        FloatType *samples = buffer.getWritePointer(channel);

        for (size_t i = 0; i < static_cast<size_t> (buffer.getNumSamples()); ++i) {
            const FloatType in = samples[i];

            FloatType factorForB0 = in - a1 * z1[channel] - a2 * z2[channel];
            FloatType out = b0 * factorForB0 + b1 * z1[channel] + b2 * z2[channel];

// Copied from juce_IIRFilter.cpp, processSamples(),
#if JUCE_INTEL
            if (!(out < static_cast<FloatType>(-1.0e-8) ||
                  out > static_cast<FloatType>(1.0e-8)))
                out = 0;
#endif

            z2[channel] = z1[channel];
            z1[channel] = factorForB0;

            samples[i] = static_cast<FloatType>(out);
        }
    }
}

template<typename FloatType>
void SecondOrderIIRFilter<FloatType>::reset() {
    z1.clear(numberOfChannels);
    z2.clear(numberOfChannels);
}

template
class SecondOrderIIRFilter<float>;

template
class SecondOrderIIRFilter<double>;