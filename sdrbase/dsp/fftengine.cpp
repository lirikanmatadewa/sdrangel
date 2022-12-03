#include "dsp/fftengine.h"
#ifdef USE_KISSFFT
#include "dsp/kissengine.h"
#endif
#ifdef USE_FFTW
#include "dsp/fftwengine.h"
#endif // USE_FFTW

FFTEngine::~FFTEngine()
{
}

FFTEngine* FFTEngine::create(const QString& fftWisdomFileName)
{
#ifdef USE_FFTW
	return new FFTWEngine(fftWisdomFileName);
#elif USE_KISSFFT
	(void) fftWisdomFileName;
	return new KissEngine;
#else // USE_KISSFFT
	return 0;
#endif
}
