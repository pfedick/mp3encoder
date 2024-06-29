/*
 * This file is part of MP3Encoder by Patrick Fedick
 *
 * $Author: patrick $
 * $Revision: 1.18 $
 * $Date: 2010/05/02 08:40:17 $
 * $Id: mp3core.cpp,v 1.18 2010/05/02 08:40:17 patrick Exp $
 *
 *
 * Copyright (c) 2009 Patrick Fedick
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#include <QSettings>
#include <QLocale>
#include "mp3core.h"

Config::Audio::Audio()
{
	frequency=44100;
	quality=2;
	channel_mode=ppl7::AudioInfo::ChannelMode::JOINT_STEREO;
	mode=EncoderMode::VBR;
	cbr_bitrate=320;
	abr_bitrate=192;
	vbr_min_bitrate=32;
	vbr_max_bitrate=320;
}

void Config::Audio::load(QSettings& settings)
{
	ppl7::String tmp;
	frequency=settings.value("frequency", "44100").toInt();
	quality=settings.value("quality", "2").toInt();

	tmp=settings.value("channels", "jstereo").toString();
	if (tmp == "stereo") channel_mode=ppl7::AudioInfo::ChannelMode::STEREO;
	else if (tmp == "mono") channel_mode=ppl7::AudioInfo::ChannelMode::MONO;
	else if (tmp == "dual") channel_mode=ppl7::AudioInfo::ChannelMode::DUAL_CHANNEL;
	else channel_mode=ppl7::AudioInfo::ChannelMode::JOINT_STEREO;

	tmp=settings.value("mode", "vbr").toString();
	if (tmp == "abr") mode=EncoderMode::ABR;
	else if (tmp == "cbr") mode=EncoderMode::CBR;
	else mode=EncoderMode::VBR;

	settings.beginGroup("cbr");
	cbr_bitrate=settings.value("bitrate", "320").toInt();
	settings.endGroup();

	settings.beginGroup("abr");
	abr_bitrate=settings.value("bitrate", "192").toInt();
	settings.endGroup();

	settings.beginGroup("vbr");
	vbr_min_bitrate=settings.value("bitrate_min", "32").toInt();
	vbr_max_bitrate=settings.value("bitrate_max", "320").toInt();
	settings.endGroup();
}

void Config::Audio::save(QSettings& settings) const
{
	settings.setValue("frequency", frequency);
	settings.setValue("quality", quality);
	switch (channel_mode) {
		case ppl7::AudioInfo::ChannelMode::STEREO:
			settings.setValue("channels", "stereo");
			break;
		case ppl7::AudioInfo::ChannelMode::MONO:
			settings.setValue("channels", "mono");
			break;
		case ppl7::AudioInfo::ChannelMode::DUAL_CHANNEL:
			settings.setValue("channels", "dual");
			break;
		case ppl7::AudioInfo::ChannelMode::JOINT_STEREO:
			settings.setValue("channels", "jstereo");
			break;
	}
	switch (mode) {
		case EncoderMode::VBR:
			settings.setValue("mode", "vbr");
			break;
		case EncoderMode::ABR:
			settings.setValue("mode", "abr");
			break;
		case EncoderMode::CBR:
			settings.setValue("mode", "cbr");
			break;
	}

	settings.beginGroup("cbr");
	settings.setValue("bitrate", cbr_bitrate);
	settings.endGroup();

	settings.beginGroup("abr");
	settings.setValue("bitrate", abr_bitrate);
	settings.endGroup();

	settings.beginGroup("vbr");
	settings.setValue("bitrate_min", vbr_min_bitrate);
	settings.setValue("bitrate_max", vbr_max_bitrate);
	settings.endGroup();
}


Config::PathConfig::PathConfig()
{
	active=true;
	audio_override=false;
	enable_backup=false;
}

void Config::PathConfig::load(QSettings& settings)
{
	active=settings.value("active", true).toBool();
	audio_override=settings.value("audio_override", false).toBool();
	enable_backup=settings.value("enableBackup", false).toBool();
	Source=settings.value("source", true).toString();
	Target=settings.value("target", true).toString();
	Backup=settings.value("backup", true).toString();
	settings.beginGroup("audio");
	audio.load(settings);
	settings.endGroup();
}

void Config::PathConfig::save(QSettings& settings) const
{
	settings.setValue("active", active);
	settings.setValue("audio_override", audio_override);
	settings.setValue("enableBackup", enable_backup);
	settings.setValue("source", Source);
	settings.setValue("target", Target);
	settings.setValue("backup", Backup);
	settings.beginGroup("audio");
	audio.save(settings);
	settings.endGroup();
}

Config::Config()
{
	numCPUs=1;
	EncoderDelay=60;
	autostart=false;
	recode=true;
	keepID3=true;
	priority=ppl7::Thread::Priority::NORMAL;
}

ppl7::String Config::getSystemLang()
{
	QLocale locale=QLocale::system();
	ppl7::String lang;
	switch (locale.language()) {
		case QLocale::German: lang="de"; break;
		default: lang="en"; break;
	}
	return lang;
}

void Config::load()
{
	ppl7::String tmp;
	QSettings settings("Patrick F.-Productions", "mp3encode");
	settings.beginGroup("global");
	language=settings.value("language", Config::getSystemLang()).toString();
	settings.endGroup();
	settings.beginGroup("encoder");
	numCPUs=settings.value("numCPUs", 1).toInt();
	EncoderDelay=settings.value("EncoderDelay", 60).toInt();
	tmp=settings.value("priority", "normal").toString();
	if (tmp == "low") priority=ppl7::Thread::Priority::LOWEST;
	else if (tmp == "high") priority=ppl7::Thread::Priority::HIGHEST;
	else priority=ppl7::Thread::Priority::NORMAL;
	autostart=settings.value("autostart", false).toBool();
	recode=settings.value("recode", true).toBool();
	keepID3=settings.value("keepID3", true).toBool();
	settings.endGroup();

	settings.beginGroup("default");
	audio_default.load(settings);
	settings.endGroup();

	// Audio-Profiles laden
	settings.beginGroup("audioprofiles");
	QStringList groups = settings.childGroups();
	for (int i = 0; i < groups.size(); ++i) {
		settings.beginGroup(groups.at(i));
		Audio profile;
		profile.load(settings);
		ppl7::String name=groups.at(i);
		audioprofiles.insert(std::pair<ppl7::String, Audio>(name, profile));
		settings.endGroup();
	}
	settings.endGroup();
	loadPaths(settings);
}

void Config::save() const
{
	QSettings settings("Patrick F.-Productions", "mp3encode");
	settings.beginGroup("global");
	settings.setValue("language", language);
	settings.endGroup();
	settings.beginGroup("encoder");
	settings.setValue("numCPUs", numCPUs);
	settings.setValue("EncoderDelay", EncoderDelay);
	switch (priority) {
		case ppl7::Thread::Priority::LOWEST:
		case ppl7::Thread::Priority::BELOW_NORMAL:
			settings.setValue("priority", "low");
			break;
		case ppl7::Thread::Priority::HIGHEST:
		case ppl7::Thread::Priority::ABOVE_NORMAL:
			settings.setValue("priority", "high");
			break;
		default:
			settings.setValue("priority", "normal");
			break;
	}
	settings.setValue("autostart", autostart);
	settings.setValue("recode", recode);
	settings.setValue("keepID3", keepID3);
	settings.endGroup();

	settings.beginGroup("default");
	audio_default.save(settings);
	settings.endGroup();

	// Audio-Profiles speichern
	settings.beginGroup("audioprofiles");
	std::map<ppl7::String, Audio>::const_iterator it;
	for (it=audioprofiles.begin();it != audioprofiles.end();++it) {
		settings.beginGroup((*it).first);
		it->second.save(settings);
		settings.endGroup();
	}
	settings.endGroup();

	savePaths(settings);
}


void Config::loadPaths(QSettings& settings)
{
	ppl7::String tmp;
	int i=0;
	while (1) {
		tmp.setf("Path_%i", i);
		settings.beginGroup(QString(tmp));
		if (settings.contains("active")) {
			PathConfig path;
			path.load(settings);
			pathconfig.push_back(path);
		} else {
			break;
		}
		settings.endGroup();
		i++;
	}
}


void Config::savePaths(QSettings& settings) const
{
	ppl7::String tmp;
	int i=0;
	std::list<PathConfig>::const_iterator it;
	for (it=pathconfig.begin();it != pathconfig.end();++it) {
		const PathConfig& path=(*it);
		tmp.setf("Path_%i", i);
		settings.beginGroup(QString(tmp));
		path.save(settings);
		settings.endGroup();
		i++;
	}
}

void Config::setCurrentProfile(const QString& name)
{
	QSettings settings("Patrick F.-Productions", "mp3encode");
	settings.beginGroup("misc");
	settings.setValue("currentProfile", name);
	settings.endGroup();
}

QString Config::getCurrentProfile()
{
	QSettings settings("Patrick F.-Productions", "mp3encode");
	settings.beginGroup("misc");
	return settings.value("currentProfile").toString();
}
