///////////////////////////////////////////////////////////////////////////////////
// Copyright (C) 2012 maintech GmbH, Otto-Hahn-Str. 15, 97204 Hoechberg, Germany //
// written by Christian Daniel                                                   //
// Copyright (C) 2014 John Greb <hexameron@spam.no>                              //
// Copyright (C) 2015, 2017-2020 Edouard Griffiths, F4EXB <f4exb06@gmail.com>    //
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

#include <QtGlobal>

#include "util/syncmessenger.h"
#include "util/message.h"
#include <QDebug>
#include <QTime>

SyncMessenger::SyncMessenger() :
	m_complete(0),
	m_waiting(0),
	m_message(0),
	m_result(0)
{
	qRegisterMetaType<Message>("Message");
}

SyncMessenger::~SyncMessenger()
{}

int SyncMessenger::sendWait(Message& message, unsigned long msPollTime)
{
	QTime startTime = QTime::currentTime();
	qDebug("SyncMessenger::sendWait START - UI thread waiting at %s (poll timeout: %lu ms)",
		qPrintable(startTime.toString("hh:mm:ss.zzz")),
		msPollTime);

	m_message = &message;
	m_mutex.lock();

#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
	m_complete.storeRelaxed(0);
	m_waiting.storeRelaxed(1);
#else
	m_complete.store(0);
	m_waiting.store(1);
#endif

	emit messageSent();

#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
	while (!m_complete.loadRelaxed()) {
		m_waitCondition.wait(&m_mutex, msPollTime);
	}
#else
	while (!m_complete.load()) {
		m_waitCondition.wait(&m_mutex, msPollTime);
	}
#endif

	int result = m_result;
	m_mutex.unlock();

	QTime endTime = QTime::currentTime();
	int elapsedMs = startTime.msecsTo(endTime);
	qDebug("SyncMessenger::sendWait END - UI thread unblocked after %d ms (finished at %s), result=%d",
		elapsedMs,
		qPrintable(endTime.toString("hh:mm:ss.zzz")),
		result);

	return result;
}

bool SyncMessenger::isWaiting() const
{
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
	return m_waiting.loadRelaxed() != 0;
#else
	return m_waiting.load() != 0;
#endif
}

void SyncMessenger::done(int result)
{
	m_result = result;

#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
	m_complete.storeRelaxed(1);
	m_waiting.storeRelaxed(0);
#else
	m_complete.store(1);
	m_waiting.store(0);
#endif

	m_waitCondition.wakeAll();
}


