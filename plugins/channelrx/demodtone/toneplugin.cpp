///////////////////////////////////////////////////////////////////////////////////////
// Copyright (C) 2014 John Greb <hexameron>                                          //
// Copyright (C) 2015-2022 Edouard Griffiths, F4EXB <f4exb06@gmail.com>              //
// Copyright (C) 2019 Davide Gerhard <rainbow@irh.it>                                //
// Copyright (C) 2020 Kacper Michajłow <kasper93@gmail.com>                          //
//                                                                                   //
// This program is free software; you can redistribute it and/or modify              //
// it under the terms of the GNU General Public License as published by              //
// the Free Software Foundation as version 3 of the License, or                      //
// (at your option) any later version.                                               //
//                                                                                   //
// This program is distributed in the hope that it will be useful,                   //
// but WITHOUT ANY WARRANTY; without even the implied warranty of                    //
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the                      //
// GNU General Public License V3 for more details.                                   //
//                                                                                   //
// You should have received a copy of the GNU General Public License                 //
// along with this program. If not, see <http://www.gnu.org/licenses/>.              //
///////////////////////////////////////////////////////////////////////////////////////
#include "TonePlugin.h"

#include <QtPlugin>
#include "plugin/pluginapi.h"

#ifndef SERVER_MODE
#include "tonedemodgui.h"
#endif
#include "tonedemod.h"
#include "tonedemodwebapiadapter.h"
#include "TonePlugin.h"

const PluginDescriptor TonePlugin::m_pluginDescriptor = {
	ToneDemod::m_channelId,
	QStringLiteral("Tone Demodulator"),
	QStringLiteral("7.21.4"),
	QStringLiteral("(c) Edouard Griffiths, F4EXB"),
	QStringLiteral("https://github.com/f4exb/sdrangel"),
	true,
	QStringLiteral("https://github.com/f4exb/sdrangel")
};

TonePlugin::TonePlugin(QObject* parent) :
	QObject(parent),
	m_pluginAPI(0)
{
}

const PluginDescriptor& TonePlugin::getPluginDescriptor() const
{
	return m_pluginDescriptor;
}

void TonePlugin::initPlugin(PluginAPI* pluginAPI)
{
	m_pluginAPI = pluginAPI;

	// register WFM demodulator
	m_pluginAPI->registerRxChannel(ToneDemod::m_channelIdURI, ToneDemod::m_channelId, this);
}

void TonePlugin::createRxChannel(DeviceAPI* deviceAPI, BasebandSampleSink** bs, ChannelAPI** cs) const
{
	if (bs || cs)
	{
		ToneDemod* instance = new ToneDemod(deviceAPI);

		if (bs) {
			*bs = instance;
		}

		if (cs) {
			*cs = instance;
		}
	}
}

#ifdef SERVER_MODE
ChannelGUI* TonePlugin::createRxChannelGUI(
	DeviceUISet* deviceUISet,
	BasebandSampleSink* rxChannel) const
{
	(void)deviceUISet;
	(void)rxChannel;
	return nullptr;
}
#else
ChannelGUI* TonePlugin::createRxChannelGUI(DeviceUISet* deviceUISet, BasebandSampleSink* rxChannel) const
{
	return ToneDemodGUI::create(m_pluginAPI, deviceUISet, rxChannel);
}
#endif

ChannelWebAPIAdapter* TonePlugin::createChannelWebAPIAdapter() const
{
	return new ToneDemodWebAPIAdapter();
}
