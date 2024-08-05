/*  firmin.h

This file is part of a program that implements a Software-Defined Radio.

Copyright (C) 2016 Warren Pratt, NR0V
Copyright (C) 2024 Edouard Griffiths, F4EXB Adapted to SDRangel

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

The author can be reached by email at

warren@wpratt.com

*/

/********************************************************************************************************
*                                                                                                       *
*                                           Time-Domain FIR                                             *
*                                                                                                       *
********************************************************************************************************/

#ifndef wdsp_firmin_h
#define wdsp_firmin_h

#include "fftw3.h"
#include "export.h"

namespace WDSP {

class WDSP_API FIRMIN
{
public:
    int run;                // run control
    int position;           // position at which to execute
    int size;               // input/output buffer size, power of two
    float* in;             // input buffer
    float* out;            // output buffer, can be same as input
    int nc;                 // number of filter coefficients, power of two
    float f_low;           // low cutoff frequency
    float f_high;          // high cutoff frequency
    float* ring;           // internal complex ring buffer
    float* h;              // complex filter coefficients
    int rsize;              // ring size, number of complex samples, power of two
    int mask;               // mask to update indexes
    int idx;                // ring input/output index
    float samplerate;      // sample rate
    int wintype;            // filter window type
    float gain;            // filter gain

    static FIRMIN* create_firmin (int run, int position, int size, float* in, float* out,
        int nc, float f_low, float f_high, int samplerate, int wintype, float gain);
    static void destroy_firmin (FIRMIN *a);
    static void flush_firmin (FIRMIN *a);
    static void xfirmin (FIRMIN *a, int pos);
    static void setBuffers_firmin (FIRMIN *a, float* in, float* out);
    static void setSamplerate_firmin (FIRMIN *a, int rate);
    static void setSize_firmin (FIRMIN *a, int size);
    static void setFreqs_firmin (FIRMIN *a, float f_low, float f_high);

private:
    static void calc_firmin (FIRMIN *a);
};

} // namespace WDSP

#endif
