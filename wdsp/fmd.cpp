/*  fmd.c

This file is part of a program that implements a Software-Defined Radio.

Copyright (C) 2013, 2023 Warren Pratt, NR0V
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

#include "comm.hpp"
#include "iir.hpp"
#include "fircore.hpp"
#include "fcurve.hpp"
#include "fir.hpp"
#include "wcpAGC.hpp"
#include "fmd.hpp"

namespace WDSP {

void FMD::calc()
{
    // pll
    omega_min = TWOPI * fmin / rate;
    omega_max = TWOPI * fmax / rate;
    g1 = 1.0 - exp(-2.0 * omegaN * zeta / rate);
    g2 = -g1 + 2.0 * (1 - exp(-omegaN * zeta / rate) * cos(omegaN / rate * sqrt(1.0 - zeta * zeta)));
    phs = 0.0;
    fil_out = 0.0;
    omega = 0.0;
    pllpole = omegaN * sqrt(2.0 * zeta * zeta + 1.0 + sqrt((2.0 * zeta * zeta + 1.0) * (2.0 * zeta * zeta + 1.0) + 1)) / TWOPI;
    // dc removal
    mtau = exp(-1.0 / (rate * tau));
    onem_mtau = 1.0 - mtau;
    fmdc = 0.0;
    // pll audio gain
    again = rate / (deviation * TWOPI);
    // CTCSS Removal
    sntch = SNOTCH::create_snotch(1, size, out, out, (int)rate, ctcss_freq, 0.0002);
    // detector limiter
    plim = WCPAGC::create_wcpagc (
        1,                                          // run - always ON
        5,                                          // mode
        1,                                          // 0 for max(I,Q), 1 for envelope
        out,                                     // input buff pointer
        out,                                     // output buff pointer
        size,                                    // io_buffsize
        (int)rate,                               // sample rate
        0.001,                                      // tau_attack
        0.008,                                      // tau_decay
        4,                                          // n_tau
        lim_gain,                                // max_gain (sets threshold, initial value)
        1.0,                                        // var_gain / slope
        1.0,                                        // fixed_gain
        1.0,                                        // max_input
        0.9,                                        // out_targ
        0.250,                                      // tau_fast_backaverage
        0.004,                                      // tau_fast_decay
        4.0,                                        // pop_ratio
        0,                                          // hang_enable
        0.500,                                      // tau_hang_backmult
        0.500,                                      // hangtime
        2.000,                                      // hang_thresh
        0.100);                                     // tau_hang_decay
}

void FMD::decalc()
{
    WCPAGC::destroy_wcpagc(plim);
    SNOTCH::destroy_snotch(sntch);
}

FMD::FMD(
    int _run,
    int _size,
    float* _in,
    float* _out,
    int _rate,
    double _deviation,
    double _f_low,
    double _f_high,
    double _fmin,
    double _fmax,
    double _zeta,
    double _omegaN,
    double _tau,
    double _afgain,
    int _sntch_run,
    double _ctcss_freq,
    int _nc_de,
    int _mp_de,
    int _nc_aud,
    int _mp_aud
) :
    run(_run),
    size(_size),
    in(_in),
    out(_out),
    rate((double) _rate),
    f_low(_f_low),
    f_high(_f_high),
    fmin(_fmin),
    fmax(_fmax),
    zeta(_zeta),
    omegaN(_omegaN),
    tau(_tau),
    deviation(_deviation),
    nc_de(_nc_de),
    mp_de(_mp_de),
    nc_aud(_nc_aud),
    mp_aud(_mp_aud),
    afgain(_afgain),
    sntch_run(_sntch_run),
    ctcss_freq(_ctcss_freq),
    lim_run(0),
    lim_gain(0.0001), // 2.5
    lim_pre_gain(0.01) // 0.4
{
    float* impulse;
    calc();
    // de-emphasis filter
    audio.resize(size * 2); // (float *) malloc0 (size * sizeof (complex));
    impulse = FCurve::fc_impulse (nc_de, f_low, f_high, +20.0 * log10(f_high / f_low), 0.0, 1, rate, 1.0 / (2.0 * size), 0, 0);
    pde = FIRCORE::create_fircore (size, audio.data(), out, nc_de, mp_de, impulse);
    delete[] (impulse);
    // audio filter
    impulse = FIR::fir_bandpass(nc_aud, 0.8 * f_low, 1.1 * f_high, rate, 0, 1, afgain / (2.0 * size));
    paud = FIRCORE::create_fircore (size, out, out, nc_aud, mp_aud, impulse);
    delete[] (impulse);
}

FMD::~FMD()
{
    FIRCORE::destroy_fircore (paud);
    FIRCORE::destroy_fircore (pde);
    decalc();
}

void FMD::flush()
{
    std::fill(audio.begin(), audio.end(), 0);
    FIRCORE::flush_fircore (pde);
    FIRCORE::flush_fircore (paud);
    phs = 0.0;
    fil_out = 0.0;
    omega = 0.0;
    fmdc = 0.0;
    SNOTCH::flush_snotch (sntch);
    WCPAGC::flush_wcpagc (plim);
}

void FMD::execute()
{
    if (run)
    {
        int i;
        double det, del_out;
        double vco[2], corr[2];
        for (i = 0; i < size; i++)
        {
            // pll
            vco[0]  = cos (phs);
            vco[1]  = sin (phs);
            corr[0] = + in[2 * i + 0] * vco[0] + in[2 * i + 1] * vco[1];
            corr[1] = - in[2 * i + 0] * vco[1] + in[2 * i + 1] * vco[0];
            if ((corr[0] == 0.0) && (corr[1] == 0.0)) corr[0] = 1.0;
            det = atan2 (corr[1], corr[0]);
            del_out = fil_out;
            omega += g2 * det;
            if (omega < omega_min) omega = omega_min;
            if (omega > omega_max) omega = omega_max;
            fil_out = g1 * det + omega;
            phs += del_out;
            while (phs >= TWOPI) phs -= TWOPI;
            while (phs < 0.0) phs += TWOPI;
            // dc removal, gain, & demod output
            fmdc = mtau * fmdc + onem_mtau * fil_out;
            audio[2 * i + 0] = again * (fil_out - fmdc);
            audio[2 * i + 1] = audio[2 * i + 0];
        }
        // de-emphasis
        FIRCORE::xfircore (pde);
        // audio filter
        FIRCORE::xfircore (paud);
        // CTCSS Removal
        SNOTCH::xsnotch (sntch);
        if (lim_run)
        {
            for (i = 0; i < 2 * size; i++)
                out[i] *= lim_pre_gain;
            WCPAGC::xwcpagc (plim);
        }
    }
    else if (in != out)
        std::copy( in,  in + size * 2, out);
}

void FMD::setBuffers(float* _in, float* _out)
{
    decalc();
    in = _in;
    out = _out;
    calc();
    FIRCORE::setBuffers_fircore (pde,  audio.data(), out);
    FIRCORE::setBuffers_fircore (paud, out, out);
    WCPAGC::setBuffers_wcpagc (plim, out, out);
}

void FMD::setSamplerate(int _rate)
{
    float* impulse;
    decalc();
    rate = _rate;
    calc();
    // de-emphasis filter
    impulse = FCurve::fc_impulse (nc_de, f_low, f_high, +20.0 * log10(f_high / f_low), 0.0, 1, rate, 1.0 / (2.0 * size), 0, 0);
    FIRCORE::setImpulse_fircore (pde, impulse, 1);
    delete[] (impulse);
    // audio filter
    impulse = FIR::fir_bandpass(nc_aud, 0.8 * f_low, 1.1 * f_high, rate, 0, 1, afgain / (2.0 * size));
    FIRCORE::setImpulse_fircore (paud, impulse, 1);
    delete[] (impulse);
    WCPAGC::setSamplerate_wcpagc (plim, (int)rate);
}

void FMD::setSize(int _size)
{
    float* impulse;
    decalc();
    size = _size;
    calc();
    audio.resize(size * 2); // (float *) malloc0 (size * sizeof (complex));
    // de-emphasis filter
    FIRCORE::destroy_fircore (pde);
    impulse = FCurve::fc_impulse (nc_de, f_low, f_high, +20.0 * log10(f_high / f_low), 0.0, 1, rate, 1.0 / (2.0 * size), 0, 0);
    pde = FIRCORE::create_fircore (size, audio.data(), out, nc_de, mp_de, impulse);
    delete[] (impulse);
    // audio filter
    FIRCORE::destroy_fircore (paud);
    impulse = FIR::fir_bandpass(nc_aud, 0.8 * f_low, 1.1 * f_high, rate, 0, 1, afgain / (2.0 * size));
    paud = FIRCORE::create_fircore (size, out, out, nc_aud, mp_aud, impulse);
    delete[] (impulse);
    WCPAGC::setSize_wcpagc (plim, size);
}

/********************************************************************************************************
*                                                                                                       *
*                                           RXA Properties                                              *
*                                                                                                       *
********************************************************************************************************/

void FMD::setDeviation(double _deviation)
{
    deviation = _deviation;
    again = rate / (deviation * TWOPI);
}

void FMD::setCTCSSFreq(double freq)
{
    ctcss_freq = freq;
    SNOTCH::SetSNCTCSSFreq (sntch, ctcss_freq);
}

void FMD::setCTCSSRun(int run)
{
    sntch_run = run;
    SNOTCH::SetSNCTCSSRun (sntch, sntch_run);
}

void FMD::setNCde(int nc)
{
    float* impulse;

    if (nc_de != nc)
    {
        nc_de = nc;
        impulse = FCurve::fc_impulse (nc_de, f_low, f_high, +20.0 * log10(f_high / f_low), 0.0, 1, rate, 1.0 / (2.0 * size), 0, 0);
        FIRCORE::setNc_fircore (pde, nc_de, impulse);
        delete[] (impulse);
    }
}

void FMD::setMPde(int mp)
{
    if (mp_de != mp)
    {
        mp_de = mp;
        FIRCORE::setMp_fircore (pde, mp_de);
    }
}

void FMD::setNCaud(int nc)
{
    float* impulse;

    if (nc_aud != nc)
    {
        nc_aud = nc;
        impulse = FIR::fir_bandpass(nc_aud, 0.8 * f_low, 1.1 * f_high, rate, 0, 1, afgain / (2.0 * size));
        FIRCORE::setNc_fircore (paud, nc_aud, impulse);
        delete[] (impulse);
    }
}

void FMD::setMPaud(int mp)
{
    if (mp_aud != mp)
    {
        mp_aud = mp;
        FIRCORE::setMp_fircore (paud, mp_aud);
    }
}

void FMD::setLimRun(int run)
{
    if (lim_run != run) {
        lim_run = run;
    }
}

void FMD::setLimGain(double gaindB)
{
    double gain = pow(10.0, gaindB / 20.0);

    if (lim_gain != gain)
    {
        decalc();
        lim_gain = gain;
        calc();
    }
}

void FMD::setAFFilter(double low, double high)
{
    float* impulse;

    if (f_low != low || f_high != high)
    {
        f_low = low;
        f_high = high;
        // de-emphasis filter
        impulse = FCurve::fc_impulse (nc_de, f_low, f_high, +20.0 * log10(f_high / f_low), 0.0, 1, rate, 1.0 / (2.0 * size), 0, 0);
        FIRCORE::setImpulse_fircore (pde, impulse, 1);
        delete[] (impulse);
        // audio filter
        impulse = FIR::fir_bandpass (nc_aud, 0.8 * f_low, 1.1 * f_high, rate, 0, 1, afgain / (2.0 * size));
        FIRCORE::setImpulse_fircore (paud, impulse, 1);
        delete[] (impulse);
    }
}

} // namespace WDSP
