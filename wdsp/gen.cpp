/*  gen.c

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
#include <chrono>

#include "comm.hpp"
#include "gen.hpp"
#include "RXA.hpp"
#include "TXA.hpp"

namespace WDSP {

void GEN::calc_tone (GEN *a)
{
    a->tone.phs = 0.0;
    a->tone.delta = TWOPI * a->tone.freq / a->rate;
    a->tone.cosdelta = cos (a->tone.delta);
    a->tone.sindelta = sin (a->tone.delta);
}

void GEN::calc_tt (GEN *a)
{
    a->tt.phs1 = 0.0;
    a->tt.phs2 = 0.0;
    a->tt.delta1 = TWOPI * a->tt.f1 / a->rate;
    a->tt.delta2 = TWOPI * a->tt.f2 / a->rate;
    a->tt.cosdelta1 = cos (a->tt.delta1);
    a->tt.cosdelta2 = cos (a->tt.delta2);
    a->tt.sindelta1 = sin (a->tt.delta1);
    a->tt.sindelta2 = sin (a->tt.delta2);
}

void GEN::calc_sweep (GEN *a)
{
    a->sweep.phs = 0.0;
    a->sweep.dphs = TWOPI * a->sweep.f1 / a->rate;
    a->sweep.d2phs = TWOPI * a->sweep.sweeprate / (a->rate * a->rate);
    a->sweep.dphsmax = TWOPI * a->sweep.f2 / a->rate;
}

void GEN::calc_sawtooth (GEN *a)
{
    a->saw.period = 1.0 / a->saw.f;
    a->saw.delta = 1.0 / a->rate;
    a->saw.t = 0.0;
}

void GEN::calc_triangle (GEN *a)
{
    a->tri.period = 1.0 / a->tri.f;
    a->tri.half = 0.5 * a->tri.period;
    a->tri.delta = 1.0 / a->rate;
    a->tri.t = 0.0;
    a->tri.t1 = 0.0;
}

void GEN::calc_pulse (GEN *a)
{
    int i;
    float delta, theta;
    a->pulse.pperiod = 1.0 / a->pulse.pf;
    a->pulse.tphs = 0.0;
    a->pulse.tdelta = TWOPI * a->pulse.tf / a->rate;
    a->pulse.tcosdelta = cos (a->pulse.tdelta);
    a->pulse.tsindelta = sin (a->pulse.tdelta);
    a->pulse.pntrans = (int)(a->pulse.ptranstime * a->rate);
    a->pulse.pnon = (int)(a->pulse.pdutycycle * a->pulse.pperiod * a->rate);
    a->pulse.pnoff = (int)(a->pulse.pperiod * a->rate) - a->pulse.pnon - 2 * a->pulse.pntrans;
    if (a->pulse.pnoff < 0) a->pulse.pnoff = 0;
    a->pulse.pcount = a->pulse.pnoff;
    a->pulse.state = 0;
    a->pulse.ctrans = new float[a->pulse.pntrans + 1]; // (float *) malloc0 ((a->pulse.pntrans + 1) * sizeof (float));
    delta = PI / (float)a->pulse.pntrans;
    theta = 0.0;
    for (i = 0; i <= a->pulse.pntrans; i++)
    {
        a->pulse.ctrans[i] = 0.5 * (1.0 - cos (theta));
        theta += delta;
    }
}

void GEN::calc_gen (GEN *a)
{
    calc_tone (a);
    calc_tt (a);
    calc_sweep (a);
    calc_sawtooth (a);
    calc_triangle (a);
    calc_pulse (a);
}

void GEN::decalc_gen (GEN *a)
{
    delete[] (a->pulse.ctrans);
}

GEN* GEN::create_gen (int run, int size, float* in, float* out, int rate, int mode)
{
    GEN *a = new GEN;
    a->run = run;
    a->size = size;
    a->in = in;
    a->out = out;
    a->rate = (float)rate;
    a->mode = mode;
    // tone
    a->tone.mag = 1.0;
    a->tone.freq = 1000.0;
    // two-tone
    a->tt.mag1 = 0.5;
    a->tt.mag2 = 0.5;
    a->tt.f1 = +  900.0;
    a->tt.f2 = + 1700.0;
    // noise
    srand ((unsigned int) time (0));
    a->noise.mag = 1.0;
    // sweep
    a->sweep.mag = 1.0;
    a->sweep.f1 = -20000.0;
    a->sweep.f2 = +20000.0;
    a->sweep.sweeprate = +4000.0;
    // sawtooth
    a->saw.mag = 1.0;
    a->saw.f = 500.0;
    // triangle
    a->tri.mag = 1.0;
    a->tri.f = 500.0;
    // pulse
    a->pulse.mag = 1.0;
    a->pulse.pf = 0.25;
    a->pulse.pdutycycle = 0.25;
    a->pulse.ptranstime = 0.002;
    a->pulse.tf = 1000.0;
    calc_gen (a);
    return a;
}

void GEN::destroy_gen (GEN *a)
{
    decalc_gen (a);
    delete (a);
}

void GEN::flush_gen (GEN *a)
{
    a->pulse.state = 0;
}

enum pstate
{
    OFF,
    UP,
    ON,
    DOWN
};

void GEN::xgen (GEN *a)
{
    if (a->run)
    {
        switch (a->mode)
        {
        case 0: // tone
            {
                int i;
                float t1, t2;
                float cosphase = cos (a->tone.phs);
                float sinphase = sin (a->tone.phs);
                for (i = 0; i < a->size; i++)
                {
                    a->out[2 * i + 0] = + a->tone.mag * cosphase;
                    a->out[2 * i + 1] = - a->tone.mag * sinphase;
                    t1 = cosphase;
                    t2 = sinphase;
                    cosphase = t1 * a->tone.cosdelta - t2 * a->tone.sindelta;
                    sinphase = t1 * a->tone.sindelta + t2 * a->tone.cosdelta;
                    a->tone.phs += a->tone.delta;
                    if (a->tone.phs >= TWOPI) a->tone.phs -= TWOPI;
                    if (a->tone.phs <   0.0 ) a->tone.phs += TWOPI;
                }
                break;
            }
        case 1: // two-tone
            {
                int i;
                float tcos, tsin;
                float cosphs1 = cos (a->tt.phs1);
                float sinphs1 = sin (a->tt.phs1);
                float cosphs2 = cos (a->tt.phs2);
                float sinphs2 = sin (a->tt.phs2);
                for (i = 0; i < a->size; i++)
                {
                    a->out[2 * i + 0] = + a->tt.mag1 * cosphs1 + a->tt.mag2 * cosphs2;
                    a->out[2 * i + 1] = - a->tt.mag1 * sinphs1 - a->tt.mag2 * sinphs2;
                    tcos = cosphs1;
                    tsin = sinphs1;
                    cosphs1 = tcos * a->tt.cosdelta1 - tsin * a->tt.sindelta1;
                    sinphs1 = tcos * a->tt.sindelta1 + tsin * a->tt.cosdelta1;
                    a->tt.phs1 += a->tt.delta1;
                    if (a->tt.phs1 >= TWOPI) a->tt.phs1 -= TWOPI;
                    if (a->tt.phs1 <   0.0 ) a->tt.phs1 += TWOPI;
                    tcos = cosphs2;
                    tsin = sinphs2;
                    cosphs2 = tcos * a->tt.cosdelta2 - tsin * a->tt.sindelta2;
                    sinphs2 = tcos * a->tt.sindelta2 + tsin * a->tt.cosdelta2;
                    a->tt.phs2 += a->tt.delta2;
                    if (a->tt.phs2 >= TWOPI) a->tt.phs2 -= TWOPI;
                    if (a->tt.phs2 <   0.0 ) a->tt.phs2 += TWOPI;
                }
                break;
            }
        case 2: // noise
            {
                int i;
                float r1, r2, c, rad;
                for (i = 0; i < a->size; i++)
                {
                    do
                    {
                        r1 = 2.0 * (float)rand() / (float)RAND_MAX - 1.0;
                        r2 = 2.0 * (float)rand() / (float)RAND_MAX - 1.0;
                        c = r1 * r1 + r2 * r2;
                    } while (c >= 1.0);
                    rad = sqrt (-2.0 * log (c) / c);
                    a->out[2 * i + 0] = a->noise.mag * rad * r1;
                    a->out[2 * i + 1] = a->noise.mag * rad * r2;
                }
                break;
            }
        case 3:  // sweep
            {
                int i;
                for (i = 0; i < a->size; i++)
                {
                    a->out[2 * i + 0] = + a->sweep.mag * cos(a->sweep.phs);
                    a->out[2 * i + 1] = - a->sweep.mag * sin(a->sweep.phs);
                    a->sweep.phs += a->sweep.dphs;
                    a->sweep.dphs += a->sweep.d2phs;
                    if (a->sweep.phs >= TWOPI) a->sweep.phs -= TWOPI;
                    if (a->sweep.phs <   0.0 ) a->sweep.phs += TWOPI;
                    if (a->sweep.dphs > a->sweep.dphsmax)
                        a->sweep.dphs = TWOPI * a->sweep.f1 / a->rate;
                }
                break;
            }
        case 4:  // sawtooth (audio only)
            {
                int i;
                for (i = 0; i < a->size; i++)
                {
                    if (a->saw.t > a->saw.period) a->saw.t -= a->saw.period;
                    a->out[2 * i + 0] = a->saw.mag * (a->saw.t * a->saw.f - 1.0);
                    a->out[2 * i + 1] = 0.0;
                    a->saw.t += a->saw.delta;
                }
            }
            break;
        case 5:  // triangle (audio only)
            {
                int i;
                for (i = 0; i < a->size; i++)
                {
                    if (a->tri.t > a->tri.period) a->tri.t1 = a->tri.t -= a->tri.period;
                    if (a->tri.t > a->tri.half) a->tri.t1 -= a->tri.delta;
                    else                        a->tri.t1 += a->tri.delta;
                    a->out[2 * i + 0] = a->tri.mag * (4.0 * a->tri.t1 * a->tri.f - 1.0);
                    a->out[2 * i + 1] = 0.0;
                    a->tri.t += a->tri.delta;
                }
            }
            break;
        case 6:  // pulse (audio only)
            {
                int i;
                float t1, t2;
                float cosphase = cos (a->pulse.tphs);
                float sinphase = sin (a->pulse.tphs);
                for (i = 0; i < a->size; i++)
                {
                    if (a->pulse.pnoff != 0)
                        switch (a->pulse.state)
                        {
                        case OFF:
                            a->out[2 * i + 0] = 0.0;
                            if (--a->pulse.pcount == 0)
                            {
                                a->pulse.state = UP;
                                a->pulse.pcount = a->pulse.pntrans;
                            }
                            break;
                        case UP:
                            a->out[2 * i + 0] = a->pulse.mag * cosphase * a->pulse.ctrans[a->pulse.pntrans - a->pulse.pcount];
                            if (--a->pulse.pcount == 0)
                            {
                                a->pulse.state = ON;
                                a->pulse.pcount = a->pulse.pnon;
                            }
                            break;
                        case ON:
                            a->out[2 * i + 0] = a->pulse.mag * cosphase;
                            if (--a->pulse.pcount == 0)
                            {
                                a->pulse.state = DOWN;
                                a->pulse.pcount = a->pulse.pntrans;
                            }
                            break;
                        case DOWN:
                            a->out[2 * i + 0] = a->pulse.mag * cosphase * a->pulse.ctrans[a->pulse.pcount];
                            if (--a->pulse.pcount == 0)
                            {
                                a->pulse.state = OFF;
                                a->pulse.pcount = a->pulse.pnoff;
                            }
                            break;
                        }
                    else
                        a->out[2 * i + 0] = 0.0;
                    a->out[2 * i + 1] = 0.0;
                    t1 = cosphase;
                    t2 = sinphase;
                    cosphase = t1 * a->pulse.tcosdelta - t2 * a->pulse.tsindelta;
                    sinphase = t1 * a->pulse.tsindelta + t2 * a->pulse.tcosdelta;
                    a->pulse.tphs += a->pulse.tdelta;
                    if (a->pulse.tphs >= TWOPI) a->pulse.tphs -= TWOPI;
                    if (a->pulse.tphs <   0.0 ) a->pulse.tphs += TWOPI;
                }
            }
            break;
        default:    // silence
            {
                std::fill(a->out, a->out + a->size * 2, 0);
                break;
            }
        }
    }
    else if (a->in != a->out)
        std::copy( a->in,  a->in + a->size * 2, a->out);
}

void GEN::setBuffers_gen (GEN *a, float* in, float* out)
{
    a->in = in;
    a->out = out;
}

void GEN::setSamplerate_gen (GEN *a, int rate)
{
    decalc_gen (a);
    a->rate = rate;
    calc_gen (a);
}

void GEN::setSize_gen (GEN *a, int size)
{
    a->size = size;
    flush_gen (a);
}


/********************************************************************************************************
*                                                                                                       *
*                                           RXA Properties                                              *
*                                                                                                       *
********************************************************************************************************/

// 'PreGen', gen0

void GEN::SetPreGenRun (RXA& rxa, int run)
{
    rxa.gen0.p->run = run;
}

void GEN::SetPreGenMode (RXA& rxa, int mode)
{
    rxa.gen0.p->mode = mode;
}

void GEN::SetPreGenToneMag (RXA& rxa, float mag)
{
    rxa.gen0.p->tone.mag = mag;
}

void GEN::SetPreGenToneFreq (RXA& rxa, float freq)
{
    rxa.gen0.p->tone.freq = freq;
    calc_tone (rxa.gen0.p);
}

void GEN::SetPreGenNoiseMag (RXA& rxa, float mag)
{
    rxa.gen0.p->noise.mag = mag;
}

void GEN::SetPreGenSweepMag (RXA& rxa, float mag)
{
    rxa.gen0.p->sweep.mag = mag;
}

void GEN::SetPreGenSweepFreq (RXA& rxa, float freq1, float freq2)
{
    rxa.gen0.p->sweep.f1 = freq1;
    rxa.gen0.p->sweep.f2 = freq2;
    calc_sweep (rxa.gen0.p);
}

void GEN::SetPreGenSweepRate (RXA& rxa, float rate)
{
    rxa.gen0.p->sweep.sweeprate = rate;
    calc_sweep (rxa.gen0.p);
}


/********************************************************************************************************
*                                                                                                       *
*                                           TXA Properties                                              *
*                                                                                                       *
********************************************************************************************************/

// 'PreGen', gen0

void GEN::SetPreGenRun (TXA& txa, int run)
{
    txa.gen0.p->run = run;
}

void GEN::SetPreGenMode (TXA& txa, int mode)
{
    txa.gen0.p->mode = mode;
}

void GEN::SetPreGenToneMag (TXA& txa, float mag)
{
    txa.gen0.p->tone.mag = mag;
}

void GEN::SetPreGenToneFreq (TXA& txa, float freq)
{
    txa.gen0.p->tone.freq = freq;
    calc_tone (txa.gen0.p);
}

void GEN::SetPreGenNoiseMag (TXA& txa, float mag)
{
    txa.gen0.p->noise.mag = mag;
}

void GEN::SetPreGenSweepMag (TXA& txa, float mag)
{
    txa.gen0.p->sweep.mag = mag;
}

void GEN::SetPreGenSweepFreq (TXA& txa, float freq1, float freq2)
{
    txa.gen0.p->sweep.f1 = freq1;
    txa.gen0.p->sweep.f2 = freq2;
    calc_sweep (txa.gen0.p);
}

void GEN::SetPreGenSweepRate (TXA& txa, float rate)
{
    txa.gen0.p->sweep.sweeprate = rate;
    calc_sweep (txa.gen0.p);
}

void GEN::SetPreGenSawtoothMag (TXA& txa, float mag)
{
    txa.gen0.p->saw.mag = mag;
}

void GEN::SetPreGenSawtoothFreq (TXA& txa, float freq)
{
    txa.gen0.p->saw.f = freq;
    calc_sawtooth (txa.gen0.p);
}

void GEN::SetPreGenTriangleMag (TXA& txa, float mag)
{
    txa.gen0.p->tri.mag = mag;
}

void GEN::SetPreGenTriangleFreq (TXA& txa, float freq)
{
    txa.gen0.p->tri.f = freq;
    calc_triangle (txa.gen0.p);
}

void GEN::SetPreGenPulseMag (TXA& txa, float mag)
{
    txa.gen0.p->pulse.mag = mag;
}

void GEN::SetPreGenPulseFreq (TXA& txa, float freq)
{
    txa.gen0.p->pulse.pf = freq;
    calc_pulse (txa.gen0.p);
}

void GEN::SetPreGenPulseDutyCycle (TXA& txa, float dc)
{
    txa.gen0.p->pulse.pdutycycle = dc;
    calc_pulse (txa.gen0.p);
}

void GEN::SetPreGenPulseToneFreq (TXA& txa, float freq)
{
    txa.gen0.p->pulse.tf = freq;
    calc_pulse (txa.gen0.p);
}

void GEN::SetPreGenPulseTransition (TXA& txa, float transtime)
{
    txa.gen0.p->pulse.ptranstime = transtime;
    calc_pulse (txa.gen0.p);
}

// 'PostGen', gen1

void GEN::SetPostGenRun (TXA& txa, int run)
{
    txa.gen1.p->run = run;
}

void GEN::SetPostGenMode (TXA& txa, int mode)
{
    txa.gen1.p->mode = mode;
}

void GEN::SetPostGenToneMag (TXA& txa, float mag)
{
    txa.gen1.p->tone.mag = mag;
}

void GEN::SetPostGenToneFreq (TXA& txa, float freq)
{
    txa.gen1.p->tone.freq = freq;
    calc_tone (txa.gen1.p);
}

void GEN::SetPostGenTTMag (TXA& txa, float mag1, float mag2)
{
    txa.gen1.p->tt.mag1 = mag1;
    txa.gen1.p->tt.mag2 = mag2;
}

void GEN::SetPostGenTTFreq (TXA& txa, float freq1, float freq2)
{
    txa.gen1.p->tt.f1 = freq1;
    txa.gen1.p->tt.f2 = freq2;
    calc_tt (txa.gen1.p);
}

void GEN::SetPostGenSweepMag (TXA& txa, float mag)
{
    txa.gen1.p->sweep.mag = mag;
}

void GEN::SetPostGenSweepFreq (TXA& txa, float freq1, float freq2)
{
    txa.gen1.p->sweep.f1 = freq1;
    txa.gen1.p->sweep.f2 = freq2;
    calc_sweep (txa.gen1.p);
}

void GEN::SetPostGenSweepRate (TXA& txa, float rate)
{
    txa.gen1.p->sweep.sweeprate = rate;
    calc_sweep (txa.gen1.p);
}

} // namespace WDSP
