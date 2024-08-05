/*  resample.c

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

#include "comm.hpp"
#include "fir.hpp"
#include "resamplef.hpp"

namespace WDSP {

/************************************************************************************************
*                                                                                               *
*                             VERSION FOR NON-COMPLEX FLOATS                                    *
*                                                                                               *
************************************************************************************************/

RESAMPLEF* RESAMPLEF::create_resampleF ( int run, int size, float* in, float* out, int in_rate, int out_rate)
{
    RESAMPLEF *a = new RESAMPLEF;
    int x, y, z;
    int i, j, k;
    int min_rate;
    float full_rate;
    float fc;
    float fc_norm;
    float* impulse;
    a->run = run;
    a->size = size;
    a->in = in;
    a->out = out;
    x = in_rate;
    y = out_rate;

    while (y != 0)
    {
        z = y;
        y = x % y;
        x = z;
    }

    a->L = out_rate / x;
    a->M = in_rate / x;

    a->L = a->L <= 0 ? 1 : a->L;
    a->M = a->M <= 0 ? 1 : a->M;

    if (in_rate < out_rate)
        min_rate = in_rate;
    else
        min_rate = out_rate;

    fc = 0.45 * (float)min_rate;
    full_rate = (float)(in_rate * a->L);
    fc_norm = fc / full_rate;
    a->ncoef = (int)(60.0 / fc_norm);
    a->ncoef = (a->ncoef / a->L + 1) * a->L;
    a->cpp = a->ncoef / a->L;
    a->h = new float[a->ncoef]; // (float *) malloc0 (a->ncoef * sizeof (float));
    impulse = FIR::fir_bandpass (a->ncoef, -fc_norm, +fc_norm, 1.0, 1, 0, (float)a->L);
    i = 0;

    for (j = 0; j < a->L; j ++)
    {
        for (k = 0; k < a->ncoef; k += a->L)
            a->h[i++] = impulse[j + k];
    }

    a->ringsize = a->cpp;
    a->ring = new float[a->ringsize]; //(float *) malloc0 (a->ringsize * sizeof (float));
    a->idx_in = a->ringsize - 1;
    a->phnum = 0;

    delete[] (impulse);
    return a;
}

void RESAMPLEF::destroy_resampleF (RESAMPLEF *a)
{
    delete[] (a->ring);
    delete[] (a->h);
    delete (a);
}

void RESAMPLEF::flush_resampleF (RESAMPLEF *a)
{
    memset (a->ring, 0, a->ringsize * sizeof (float));
    a->idx_in = a->ringsize - 1;
    a->phnum = 0;
}

int RESAMPLEF::xresampleF (RESAMPLEF *a)
{
    int outsamps = 0;

    if (a->run)
    {
        int i, j, n;
        int idx_out;
        float I;

        for (i = 0; i < a->size; i++)
        {
            a->ring[a->idx_in] = (float)a->in[i];

            while (a->phnum < a->L)
            {
                I = 0.0;
                n = a->cpp * a->phnum;

                for (j = 0; j < a->cpp; j++)
                {
                    if ((idx_out = a->idx_in + j) >= a->ringsize)
                        idx_out -= a->ringsize;

                    I += a->h[n + j] * a->ring[idx_out];
                }

                a->out[outsamps] = (float)I;

                outsamps++;
                a->phnum += a->M;
            }

            a->phnum -= a->L;

            if (--a->idx_in < 0)
                a->idx_in = a->ringsize - 1;
        }
    }
    else if (a->in != a->out)
    {
        memcpy (a->out, a->in, a->size * sizeof (float));
    }

    return outsamps;
}

// Exported calls


void* RESAMPLEF::create_resampleFV (int in_rate, int out_rate)
{
    return (void *) create_resampleF (1, 0, 0, 0, in_rate, out_rate);
}


void RESAMPLEF::xresampleFV (float* input, float* output, int numsamps, int* outsamps, void* ptr)
{
    RESAMPLEF *a = (RESAMPLEF*) ptr;
    a->in = input;
    a->out = output;
    a->size = numsamps;
    *outsamps = xresampleF(a);
}


void RESAMPLEF::destroy_resampleFV (void* ptr)
{
    destroy_resampleF ( (RESAMPLEF*) ptr );
}

} // namespace WDSP
