/*  send.h

This file is part of a program that implements a Software-Defined Radio.

Copyright (C) 2013 Warren Pratt, NR0V
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

// 'send' copies samples from the sample stream and proactively pushes those samples to external
//  functions by calling them.  As such, the call and parameters are somewhat specific to the
//  need for the data.  Nevertheless, this code is written in somewhat of a generic form to
//  facilitate adding additional functions in the future.

#ifndef wdsp_sender_h
#define wdsp_sender_h

#include "export.h"

namespace WDSP {

class RXA;
class BufferProbe;

class WDSP_API SENDER
{
public:
    int run;            // 0 = send OFF, 1 = send ON
    int flag;           // secondary 'run'; AND'd with 'run'
    int mode;           // selects the specific processing and function call
    int size;           // size of the data buffer (complex samples)
    float* in;         // buffer from which to take the data
    BufferProbe *spectrumProbe; // this is the data handler actually

    static SENDER* create_sender (int run, int flag, int mode, int size, float* in);
    static void destroy_sender (SENDER *a);
    static void flush_sender (SENDER *a);
    static void xsender (SENDER *a);
    static void setBuffers_sender (SENDER *a, float* in);
    static void setSamplerate_sender (SENDER *a, int rate);
    static void setSize_sender (SENDER *a, int size);
    // RXA Properties
    static void SetSpectrum (RXA& rxa, int flag, BufferProbe *spectrumProbe);
    // TXA Properties
};

} // namespace WDSP

#endif
