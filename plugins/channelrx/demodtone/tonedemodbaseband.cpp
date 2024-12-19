///////////////////////////////////////////////////////////////////////////////////
// Copyright (C) 2019-2021 Edouard Griffiths, F4EXB <f4exb06@gmail.com>          //
// Copyright (C) 2022 Jiří Pinkava <jiri.pinkava@rossum.ai>                      //
// Copyright (C) 2023 Jon Beniston, M7RCE <jon@beniston.com>                     //
//                                                                               //
// This program is free software; you can redistribute it and/or modify          //
// it under the terms of the GNU General Public License as published by          //
// the Free Software Foundation as version 3 of the License, or                  //
// (at your option) any later version.                                           //
//                                                                               //
// This program is distributed in the hope that it will be useful,               //
// but WITHOUT ANY WARRANTY; without even the implied warranty of                //
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the                  //
// GNU General Public License V3 for more details.                               //
//                                                                               //
// You should have received a copy of the GNU General Public License             //
// along with this program. If not, see <http://www.gnu.org/licenses/>.          //
///////////////////////////////////////////////////////////////////////////////////

#include <QDebug>

#include "dsp/dspengine.h"
#include "dsp/dspcommands.h"
#include "dsp/downchannelizer.h"

#include "tonedemodbaseband.h"

MESSAGE_CLASS_DEFINITION(ToneDemodBaseband::MsgConfigureWFMDemodBaseband, Message)

ToneDemodBaseband::ToneDemodBaseband()
{
	m_sampleFifo.setSize(SampleSinkFifo::getSizePolicy(48000));
	m_channelizer = new DownChannelizer(&m_sink);

	qDebug("ToneDemodBaseband::ToneDemodBaseband");
	QObject::connect(
		&m_sampleFifo,
		&SampleSinkFifo::dataReady,
		this,
		&ToneDemodBaseband::handleData,
		Qt::QueuedConnection
	);

	DSPEngine::instance()->getAudioDeviceManager()->addAudioSink(m_sink.getAudioFifo(), getInputMessageQueue());
	m_sink.applyAudioSampleRate(DSPEngine::instance()->getAudioDeviceManager()->getOutputSampleRate());
	m_channelSampleRate = 0;

	connect(&m_inputMessageQueue, SIGNAL(messageEnqueued()), this, SLOT(handleInputMessages()));
	connect(m_sink.getAudioFifo(), &AudioFifo::underflow, this, &ToneDemodBaseband::audioUnderflow);
	connect(m_sink.getAudioFifo(), &AudioFifo::overflow, this, &ToneDemodBaseband::audioOverflow);
}

ToneDemodBaseband::~ToneDemodBaseband()
{
	DSPEngine::instance()->getAudioDeviceManager()->removeAudioSink(m_sink.getAudioFifo());
	delete m_channelizer;
}

void ToneDemodBaseband::reset()
{
	QMutexLocker mutexLocker(&m_mutex);
	m_sampleFifo.reset();
	m_channelSampleRate = 0;
}

void ToneDemodBaseband::setChannel(ChannelAPI* channel)
{
	m_sink.setChannel(channel);
}

void ToneDemodBaseband::feed(const SampleVector::const_iterator& begin, const SampleVector::const_iterator& end)
{
	m_sampleFifo.write(begin, end);
}

void ToneDemodBaseband::handleData()
{
	QMutexLocker mutexLocker(&m_mutex);

	while ((m_sampleFifo.fill() > 0) && (m_inputMessageQueue.size() == 0))
	{
		SampleVector::iterator part1begin;
		SampleVector::iterator part1end;
		SampleVector::iterator part2begin;
		SampleVector::iterator part2end;

		std::size_t count = m_sampleFifo.readBegin(m_sampleFifo.fill(), &part1begin, &part1end, &part2begin, &part2end);

		// first part of FIFO data
		if (part1begin != part1end) {
			m_channelizer->feed(part1begin, part1end);
		}

		// second part of FIFO data (used when block wraps around)
		if (part2begin != part2end) {
			m_channelizer->feed(part2begin, part2end);
		}

		m_sampleFifo.readCommit((unsigned int)count);
	}
}

void ToneDemodBaseband::handleInputMessages()
{
	Message* message;

	while ((message = m_inputMessageQueue.pop()) != nullptr)
	{
		if (handleMessage(*message)) {
			delete message;
		}
	}
}

bool ToneDemodBaseband::handleMessage(const Message& cmd)
{
	if (MsgConfigureWFMDemodBaseband::match(cmd))
	{
		QMutexLocker mutexLocker(&m_mutex);
		MsgConfigureWFMDemodBaseband& cfg = (MsgConfigureWFMDemodBaseband&)cmd;
		qDebug() << "ToneDemodBaseband::handleMessage: MsgConfigureWFMDemodBaseband";

		applySettings(cfg.getSettings(), cfg.getForce());

		return true;
	}
	else if (DSPSignalNotification::match(cmd))
	{
		QMutexLocker mutexLocker(&m_mutex);
		DSPSignalNotification& notif = (DSPSignalNotification&)cmd;
		qDebug() << "ToneDemodBaseband::handleMessage: DSPSignalNotification: basebandSampleRate: " << notif.getSampleRate();
		m_sampleFifo.setSize(SampleSinkFifo::getSizePolicy(notif.getSampleRate()));
		m_channelizer->setBasebandSampleRate(notif.getSampleRate());
		m_sink.applyChannelSettings(m_channelizer->getChannelSampleRate(), m_channelizer->getChannelFrequencyOffset());

		if (m_channelSampleRate != m_channelizer->getChannelSampleRate())
		{
			m_sink.applyAudioSampleRate(m_sink.getAudioSampleRate()); // reapply when channel sample rate changes
			m_channelSampleRate = m_channelizer->getChannelSampleRate();
		}

		return true;
	}
	else
	{
		return false;
	}
}

void ToneDemodBaseband::applySettings(const ToneDemodSettings& settings, bool force)
{
	if ((settings.m_rfBandwidth != m_settings.m_rfBandwidth)
		|| (settings.m_inputFrequencyOffset != m_settings.m_inputFrequencyOffset) || force)
	{
		m_channelizer->setChannelization(ToneDemodSettings::requiredBW(settings.m_rfBandwidth), settings.m_inputFrequencyOffset);
		m_sink.applyChannelSettings(m_channelizer->getChannelSampleRate(), m_channelizer->getChannelFrequencyOffset());

		if (m_channelSampleRate != m_channelizer->getChannelSampleRate())
		{
			m_sink.applyAudioSampleRate(m_sink.getAudioSampleRate()); // reapply when channel sample rate changes
			m_channelSampleRate = m_channelizer->getChannelSampleRate();
		}
	}

	if ((settings.m_audioDeviceName != m_settings.m_audioDeviceName) || force)
	{
		AudioDeviceManager* audioDeviceManager = DSPEngine::instance()->getAudioDeviceManager();
		int audioDeviceIndex = audioDeviceManager->getOutputDeviceIndex(settings.m_audioDeviceName);
		//qDebug("AMDemod::applySettings: audioDeviceName: %s audioDeviceIndex: %d", qPrintable(settings.m_audioDeviceName), audioDeviceIndex);
		audioDeviceManager->removeAudioSink(m_sink.getAudioFifo());
		audioDeviceManager->addAudioSink(m_sink.getAudioFifo(), getInputMessageQueue(), audioDeviceIndex);
		int audioSampleRate = audioDeviceManager->getOutputSampleRate(audioDeviceIndex);

		if (m_sink.getAudioSampleRate() != audioSampleRate) {
			m_sink.applyAudioSampleRate(audioSampleRate);
		}
	}

	m_sink.applySettings(settings, force);

	m_settings = settings;
}

int ToneDemodBaseband::getChannelSampleRate() const
{
	return m_channelizer->getChannelSampleRate();
}


void ToneDemodBaseband::setBasebandSampleRate(int sampleRate)
{
	m_channelizer->setBasebandSampleRate(sampleRate);
	m_sink.applyChannelSettings(m_channelizer->getChannelSampleRate(), m_channelizer->getChannelFrequencyOffset());
}

void ToneDemodBaseband::audioUnderflow()
{
	m_audioFifoErrorDateTime = QDateTime::currentDateTime();
}

void ToneDemodBaseband::audioOverflow()
{
	m_audioFifoErrorDateTime = QDateTime::currentDateTime();
}
