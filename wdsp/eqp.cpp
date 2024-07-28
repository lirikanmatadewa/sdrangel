/*  eq.c

This file is part of a program that implements a Software-Defined Radio.

Copyright (C) 2013, 2016, 2017 Warren Pratt, NR0V
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
#include "eqp.hpp"
#include "fircore.hpp"
#include "fir.hpp"
#include "RXA.hpp"
#include "TXA.hpp"

namespace WDSP {

int EQP::fEQcompare (const void * a, const void * b)
{
    if (*(float*)a < *(float*)b)
        return -1;
    else if (*(float*)a == *(float*)b)
        return 0;
    else
        return 1;
}

float* EQP::eq_impulse (int N, int nfreqs, float* F, float* G, double samplerate, double scale, int ctfmode, int wintype)
{
    float* fp = new float[nfreqs + 2]; // (float *) malloc0 ((nfreqs + 2)   * sizeof (float));
    float* gp = new float[nfreqs + 2]; // (float *) malloc0 ((nfreqs + 2)   * sizeof (float));
    float* A  = new float[N / 2 + 1]; // (float *) malloc0 ((N / 2 + 1) * sizeof (float));
    float* sary = new float[2 * nfreqs]; // (float *) malloc0 (2 * nfreqs * sizeof (float));
    double gpreamp, f, frac;
    float* impulse;
    int i, j, mid;
    fp[0] = 0.0;
    fp[nfreqs + 1] = 1.0;
    gpreamp = G[0];

    for (i = 1; i <= nfreqs; i++)
    {
        fp[i] = 2.0 * F[i] / samplerate;

        if (fp[i] < 0.0)
            fp[i] = 0.0;

        if (fp[i] > 1.0)
            fp[i] = 1.0;

        gp[i] = G[i];
    }

    for (i = 1, j = 0; i <= nfreqs; i++, j+=2)
    {
        sary[j + 0] = fp[i];
        sary[j + 1] = gp[i];
    }

    qsort (sary, nfreqs, 2 * sizeof (float), fEQcompare);

    for (i = 1, j = 0; i <= nfreqs; i++, j+=2)
    {
        fp[i] = sary[j + 0];
        gp[i] = sary[j + 1];
    }

    gp[0] = gp[1];
    gp[nfreqs + 1] = gp[nfreqs];
    mid = N / 2;
    j = 0;

    if (N & 1)
    {
        for (i = 0; i <= mid; i++)
        {
            f = (double)i / (double)mid;

            while ((f > fp[j + 1]) && (j < nfreqs))
                j++;

            frac = (f - fp[j]) / (fp[j + 1] - fp[j]);
            A[i] = pow (10.0, 0.05 * (frac * gp[j + 1] + (1.0 - frac) * gp[j] + gpreamp)) * scale;
        }
    }
    else
    {
        for (i = 0; i < mid; i++)
        {
            f = ((double)i + 0.5) / (double)mid;

            while ((f > fp[j + 1]) && (j < nfreqs))
                j++;

            frac = (f - fp[j]) / (fp[j + 1] - fp[j]);
            A[i] = pow (10.0, 0.05 * (frac * gp[j + 1] + (1.0 - frac) * gp[j] + gpreamp)) * scale;
        }
    }

    if (ctfmode == 0)
    {
        int k, low, high;
        double lowmag, highmag, flow4, fhigh4;

        if (N & 1)
        {
            low = (int)(fp[1] * mid);
            high = (int)(fp[nfreqs] * mid + 0.5);
            lowmag = A[low];
            highmag = A[high];
            flow4 = pow((double)low / (double)mid, 4.0);
            fhigh4 = pow((double)high / (double)mid, 4.0);
            k = low;

            while (--k >= 0)
            {
                f = (double)k / (double)mid;
                lowmag *= (f * f * f * f) / flow4;
                if (lowmag < 1.0e-20) lowmag = 1.0e-20;
                A[k] = lowmag;
            }

            k = high;

            while (++k <= mid)
            {
                f = (double)k / (double)mid;
                highmag *= fhigh4 / (f * f * f * f);
                if (highmag < 1.0e-20) highmag = 1.0e-20;
                A[k] = highmag;
            }
        }
        else
        {
            low = (int)(fp[1] * mid - 0.5);
            high = (int)(fp[nfreqs] * mid - 0.5);
            lowmag = A[low];
            highmag = A[high];
            flow4 = pow((double)low / (double)mid, 4.0);
            fhigh4 = pow((double)high / (double)mid, 4.0);
            k = low;

            while (--k >= 0)
            {
                f = (double)k / (double)mid;
                lowmag *= (f * f * f * f) / flow4;
                if (lowmag < 1.0e-20) lowmag = 1.0e-20;
                A[k] = lowmag;
            }

            k = high;

            while (++k < mid)
            {
                f = (double)k / (double)mid;
                highmag *= fhigh4 / (f * f * f * f);
                if (highmag < 1.0e-20) highmag = 1.0e-20;
                A[k] = highmag;
            }
        }
    }

    if (N & 1)
        impulse = FIR::fir_fsamp_odd(N, A, 1, 1.0, wintype);
    else
        impulse = FIR::fir_fsamp(N, A, 1, 1.0, wintype);

    // print_impulse("eq.txt", N, impulse, 1, 0);
    delete[] (sary);
    delete[] (A);
    delete[] (gp);
    delete[] (fp);
    return impulse;
}

/********************************************************************************************************
*                                                                                                       *
*                                   Partitioned Overlap-Save Equalizer                                  *
*                                                                                                       *
********************************************************************************************************/

EQP::EQP(
    int _run,
    int _size,
    int _nc,
    int _mp,
    float *_in,
    float *_out,
    int _nfreqs,
    float* _F,
    float* _G,
    int _ctfmode,
    int _wintype,
    int _samplerate
)
{
    // NOTE:  'nc' must be >= 'size'
    float* impulse;
    run = _run;
    size = _size;
    nc = _nc;
    mp = _mp;
    in = _in;
    out = _out;
    nfreqs = _nfreqs;
    F.resize(nfreqs + 1); // (float *) malloc0 ((nfreqs + 1) * sizeof (float));
    G.resize(nfreqs + 1); // (float *) malloc0 ((nfreqs + 1) * sizeof (float));
    std::copy(_F, _F + (_nfreqs + 1), F.begin());
    std::copy(_G, _G + (_nfreqs + 1), G.begin());
    ctfmode = _ctfmode;
    wintype = _wintype;
    samplerate = (double) _samplerate;
    impulse = eq_impulse (nc, nfreqs, F.data(), G.data(), samplerate, 1.0 / (2.0 * size), ctfmode, wintype);
    fircore = FIRCORE::create_fircore (size, in, out, nc, mp, impulse);
    delete[] (impulse);
}

EQP::~EQP()
{
    FIRCORE::destroy_fircore (fircore);
}

void EQP::flush()
{
    FIRCORE::flush_fircore (fircore);
}

void EQP::execute()
{
    if (run)
        FIRCORE::xfircore (fircore);
    else
        std::copy(in,  in + size * 2, out);
}

void EQP::setBuffers(float* _in, float* _out)
{
    in = _in;
    out = _out;
    FIRCORE::setBuffers_fircore (fircore, in, out);
}

void EQP::setSamplerate(int rate)
{
    float* impulse;
    samplerate = rate;
    impulse = eq_impulse (nc, nfreqs, F.data(), G.data(), samplerate, 1.0 / (2.0 * size), ctfmode, wintype);
    FIRCORE::setImpulse_fircore (fircore, impulse, 1);
    delete[] (impulse);
}

void EQP::setSize(int _size)
{
    float* impulse;
    size = _size;
    FIRCORE::setSize_fircore (fircore, size);
    impulse = eq_impulse (nc, nfreqs, F.data(), G.data(), samplerate, 1.0 / (2.0 * size), ctfmode, wintype);
    FIRCORE::setImpulse_fircore (fircore, impulse, 1);
    delete[] (impulse);
}

/********************************************************************************************************
*                                                                                                       *
*                           Partitioned Overlap-Save Equalizer:  Public Properties                      *
*                                                                                                       *
********************************************************************************************************/

void EQP::setRun(int _run)
{
    run = _run;
}

void EQP::setNC(int _nc)
{
    float* impulse;

    if (nc != _nc)
    {
        nc = _nc;
        impulse = eq_impulse (nc, nfreqs, F.data(), G.data(), samplerate, 1.0 / (2.0 * size), ctfmode, wintype);
        FIRCORE::setNc_fircore (fircore, nc, impulse);
        delete[] (impulse);
    }
}

void EQP::setMP(int _mp)
{
    if (mp != _mp)
    {
        mp = _mp;
        FIRCORE::setMp_fircore (fircore, mp);
    }
}

void EQP::setProfile(int _nfreqs, const float* _F, const float* _G)
{
    float* impulse;
    nfreqs = _nfreqs;
    F.resize(nfreqs + 1); // (float *) malloc0 ((nfreqs + 1) * sizeof (float));
    G.resize(nfreqs + 1); // (float *) malloc0 ((nfreqs + 1) * sizeof (float));
    std::copy(_F, _F + (_nfreqs + 1), F.begin());
    std::copy(_G, _G + (_nfreqs + 1), G.begin());
    impulse = eq_impulse (nc, nfreqs, F.data(), G.data(), samplerate, 1.0 / (2.0 * size), ctfmode, wintype);
    FIRCORE::setImpulse_fircore (fircore, impulse, 1);
    delete[] (impulse);
}

void EQP::setCtfmode(int _mode)
{
    float* impulse;
    ctfmode = _mode;
    impulse = eq_impulse (nc, nfreqs, F.data(), G.data(), samplerate, 1.0 / (2.0 * size), ctfmode, wintype);
    FIRCORE::setImpulse_fircore (fircore, impulse, 1);
    delete[] (impulse);
}

void EQP::setWintype(int _wintype)
{
    float* impulse;
    wintype = _wintype;
    impulse = eq_impulse (nc, nfreqs, F.data(), G.data(), samplerate, 1.0 / (2.0 * size), ctfmode, wintype);
    FIRCORE::setImpulse_fircore (fircore, impulse, 1);
    delete[] (impulse);
}

void EQP::setGrphEQ(int *rxeq)
{   // three band equalizer (legacy compatibility)
    float* impulse;
    nfreqs = 4;
    F.resize(nfreqs + 1); // (float *) malloc0 ((nfreqs + 1) * sizeof (float));
    G.resize(nfreqs + 1); // (float *) malloc0 ((nfreqs + 1) * sizeof (float));
    F[1] =  150.0;
    F[2] =  400.0;
    F[3] = 1500.0;
    F[4] = 6000.0;
    G[0] = (float)rxeq[0];
    G[1] = (float)rxeq[1];
    G[2] = (float)rxeq[1];
    G[3] = (float)rxeq[2];
    G[4] = (float)rxeq[3];
    ctfmode = 0;
    impulse = eq_impulse (nc, nfreqs, F.data(), G.data(), samplerate, 1.0 / (2.0 * size), ctfmode, wintype);
    FIRCORE::setImpulse_fircore (fircore, impulse, 1);
    delete[] (impulse);
}

void EQP::setGrphEQ10(int *rxeq)
{   // ten band equalizer (legacy compatibility)
    float* impulse;
    int i;
    nfreqs = 10;
    F.resize(nfreqs + 1); // (float *) malloc0 ((nfreqs + 1) * sizeof (float));
    G.resize(nfreqs + 1); // (float *) malloc0 ((nfreqs + 1) * sizeof (float));
    F[1]  =    32.0;
    F[2]  =    63.0;
    F[3]  =   125.0;
    F[4]  =   250.0;
    F[5]  =   500.0;
    F[6]  =  1000.0;
    F[7]  =  2000.0;
    F[8]  =  4000.0;
    F[9]  =  8000.0;
    F[10] = 16000.0;
    for (i = 0; i <= nfreqs; i++)
        G[i] = (float)rxeq[i];
    ctfmode = 0;
    impulse = eq_impulse (nc, nfreqs, F.data(), G.data(), samplerate, 1.0 / (2.0 * size), ctfmode, wintype);
    // print_impulse ("rxeq.txt", nc, impulse, 1, 0);
    FIRCORE::setImpulse_fircore (fircore, impulse, 1);
    delete[] (impulse);
}

} // namespace WDSP
