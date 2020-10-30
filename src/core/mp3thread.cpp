/*
 * This file is part of MP3Encoder by Patrick Fedick
 *
 * $Author: patrick $
 * $Revision: 1.17 $
 * $Date: 2010/05/02 10:33:07 $
 * $Id: mp3thread.cpp,v 1.17 2010/05/02 10:33:07 patrick Exp $
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

#include "mp3core.h"

CMP3Thread::CMP3Thread(CMP3Core *core)
{
	this->core=core;
	bFileRenamed=false;
	bFinished=false;
	running=true;
	progress=0.0f;
	faktor=0.0f;
	length=0;
	time=0;
	rest=0;
	startTime=0;
}

CMP3Thread::~CMP3Thread()
{
	if (!bFinished) {
		if(bFileRenamed) {
			// Bisher encodetes File muss gelöscht werden
			ppl7::File::remove(TargetFile);
		}
	}
}

void CMP3Thread::setConfig(const Config::Audio &default_audio, const Config::PathConfig &path)
{
	config=path;
	if (!config.audio_override) config.audio=default_audio;
}

void CMP3Thread::setDirEntry(const ppl7::DirEntry &dir)
{
	this->dir=dir;
}

const Config::PathConfig &CMP3Thread::getConfig() const
{
	return config;
}

float CMP3Thread::getFactor() const
{
	return faktor;
}

int CMP3Thread::getLength() const
{
	return length;
}

int CMP3Thread::getTime() const
{
	return time;
}

int CMP3Thread::getRest() const
{
	return rest;
}

const ppl7::String &CMP3Thread::getFilename() const
{
	return Filename;
}

float CMP3Thread::getProgress() const
{
	return progress;
}

bool CMP3Thread::isRunning() const
{
	return running;
}


int CMP3Thread::LockFile()
{
	//config.List("config");
	//dir.Print();
	const char *myid=core->GetMyId();
	ppl7::String tmpfile, Source, Target;
	Source=config.Source;
	Target=config.Target;
	Source.trimRight("/");
	Target.trimRight("/");

	Filename=dir.Filename;

	tmpfile=dir.Filename;
	size_t l=tmpfile.len();
	for(size_t i=l-1;i>0;i--) {				// Dateiendung abschneiden
		if(tmpfile[i]=='.') {
			tmpfile.chop(l-i);
			break;
		}
	}
	//printf ("tmpfile: %s\n",(char*)tmpfile);

	OriginalFile=dir.File;
    TargetFile.setf("%s/%s.mp3.%s.tgt.enc",(const char*)Target,(const char*)tmpfile,myid);
	EndFile.setf("%s/%s.mp3",(const char*)Target,(const char*)tmpfile);


	//printf ("OriginalFile: %ls\n",(const wchar_t*)ppl7::WideString(OriginalFile));
	//printf ("EndFile: %ls\n",(const wchar_t*)ppl7::WideString(EndFile));
	//printf ("TargetFile: %ls\n",(const wchar_t*)ppl7::WideString(TargetFile));
	//fflush(stdout);


	// Kann die Zieldatei geöffnet werden?
	try {
		zz.open(TargetFile,ppl7::File::WRITE);
	} catch (...) {
		return 0;
	}

	// Quellfile öffnen
	try {
		ss.open(OriginalFile,ppl7::File::READ);
	} catch (...) {
		zz.close();
		ppl7::File::remove(TargetFile);
		return 0;
	}
	bFinished=false;
	bFileRenamed=true;
	return 1;
}

void CMP3Thread::stopEncode()
{
	encoder.stop();
}

void CMP3Thread::run()
{
	int ret=0;
	try {
		encode();
		ret=1;
	} catch (const ppl7::Exception &exp) {
        printf ("CMP3Thread::run - ");
		exp.print();
		ret=0;
	}
	zz.close();
	ss.close();
	if (ret) {		// Erfolgreich beendet
		try {
			// Löschen oder Backup?
			if (config.enable_backup) {
				ppl7::String BackupFile=config.Backup;
				BackupFile+="/"+ppl7::File::getFilename(OriginalFile);
				try {
					ppl7::File::move(OriginalFile,BackupFile);
				} catch (...) {
					ppl7::File::copy(OriginalFile,BackupFile);
					ppl7::File::remove(OriginalFile);
				}
			} else {
				ppl7::File::remove(OriginalFile);
			}
			ppl7::File::rename(TargetFile,EndFile);
			bFinished=true;
		} catch (const ppl7::Exception &exp) {
			printf("ERROR CMP3Thread::run => %s\n", (const char*)exp.toString());
			fflush(stdout);
		}

	}
	running=false;
	//LastActivity=ppl6::GetTime();
}

static void progressPPL7Encoder(int progress, void *priv)
{
	((CMP3Thread*)priv)->updateProgress(progress);
}

void CMP3Thread::initMp3Encoder(ppl7::AudioEncoder_MP3 &encoder)
{
	encoder.setProgressFunction(progressPPL7Encoder, this);
    encoder.setStereoMode(config.audio.channel_mode);
	if (config.audio.mode==Config::Audio::EncoderMode::VBR) {
		encoder.setVBR(config.audio.vbr_min_bitrate,
				config.audio.vbr_max_bitrate,
				config.audio.quality);
	} else if (config.audio.mode==Config::Audio::EncoderMode::ABR) {
		encoder.setABR(config.audio.abr_bitrate,
				config.audio.quality);
	} else {
		encoder.setCBR(config.audio.cbr_bitrate,
				config.audio.quality);
	}
}

ppl7::AudioDecoder *CMP3Thread::getDecoder(const ppl7::AudioInfo &info)
{
	if (info.Format==ppl7::AudioInfo::AIFF) {
		return new ppl7::AudioDecoder_Aiff();
	} else if (info.Format==ppl7::AudioInfo::WAVE) {
		return new ppl7::AudioDecoder_Wave();
	} else if (info.Format==ppl7::AudioInfo::MP3) {
		return new ppl7::AudioDecoder_MP3();
	}
	throw ppl7::UnsupportedAudioFormatException();
}

void CMP3Thread::encode()
{
	ppl7::ID3Tag Tag;
	initMp3Encoder(encoder);
	ppl7::AudioInfo info;
	if (ppl7::IdentAudioFile(ss,info)!=true) {
		throw ppl7::UnsupportedAudioFormatException();
	}
	if (info.HaveID3v2Tag) {
        try {
			Tag.load(ss);
		} catch (...) {
			info.HaveID3v2Tag=false;
		}
	}
	ppl7::AudioDecoder *decoder=getDecoder(info);
	try {
		length=info.Length/1000;
		startTime=ppl7::GetMicrotime();
		decoder->open(ss);
		encoder.startEncode(zz);
		if (info.HaveID3v2Tag) {
			encoder.writeID3v2Tag(Tag);
		}
		encoder.encode(*decoder);
		encoder.finish();
		if (info.HaveID3v2Tag) {
			try {
				encoder.writeID3v1Tag(Tag);
			} catch (...) {}
		}
	} catch (...) {
        delete decoder;
		throw;
	}
    delete decoder;
}

void CMP3Thread::updateProgress(int progress)
{
	this->progress=(float)progress;
	double duration=ppl7::GetMicrotime()-startTime;
	time=(int) duration;
	if (progress>0) {
		double estimated_total=duration*100.0/progress;
		double playtime=(double)length;
		faktor=playtime/estimated_total;
		rest=(int)(estimated_total-duration);
	}
}
