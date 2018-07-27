/*
 * This file is part of MP3Encoder by Patrick Fedick
 *
 * $Author: patrick $
 * $Revision: 1.13 $
 * $Date: 2010/05/02 08:40:17 $
 * $Id: mp3core.h,v 1.13 2010/05/02 08:40:17 patrick Exp $
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

#ifndef _MP3CORE
#define _MP3CORE

#include <QString>
#include <QSettings>
#include <QComboBox>
#include <QTreeView>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QStatusBar>
#include <QTranslator>
#define WITH_QT
#include <ppl7.h>
#include <ppl7-audio.h>
#include <list>
#include <map>

#define READCACHESIZE	512*1024
#define WRITECACHESIZE	2*1024*1024


class CMP3Core;

class Config
{
	public:
		class Audio
		{
			public:
			enum EncoderMode {
				CBR,
				ABR,
				VBR
			};
			Audio();
			int				frequency;
			int				quality;
			ppl7::AudioInfo::ChannelMode	channel_mode;
			EncoderMode		mode;
			int				cbr_bitrate;
			int				abr_bitrate;
			int				vbr_min_bitrate;
			int				vbr_max_bitrate;
			void			load(QSettings &settings);
			void			save(QSettings &settings) const;
		};
		class PathConfig
		{
			public:
				ppl7::String	Source;
				ppl7::String	Target;
				ppl7::String	Backup;
				bool	active;
				bool	audio_override;
				bool	enable_backup;
				Audio	audio;
				PathConfig();
				void load(QSettings &settings);
				void save(QSettings &settings) const;

		};


	private:
		void loadPaths(QSettings &settings);
		void savePaths(QSettings &settings) const;

	public:
		int numCPUs;
		bool	autostart;
		bool	recode;
		bool	keepID3;
		int		EncoderDelay;
		ppl7::String	language;
		ppl7::Thread::Priority	priority;
		Audio	audio_default;
		std::list<PathConfig> pathconfig;
        std::map<ppl7::String,Audio> audioprofiles;

	public:
		Config();
		void	load();
		void	save() const;
        void    setCurrentProfile(const QString &name);
        QString getCurrentProfile();

};

class CMP3Thread : public ppl7::Thread
{
	//friend class CMP3Core;
	private:
		ppl7::AudioEncoder_MP3 encoder;
		bool	bFileRenamed;
		bool	bFinished;
		bool	running;
		CMP3Core *core;
		ppl7::DirEntry dir;
		ppl7::String Filename;
		ppl7::String SourceFile, TargetFile, EndFile, OriginalFile;
		ppl7::String Backup;
        Config::PathConfig config;
		ppl7::File ss,zz;
		double startTime;
		ppl7::AudioDecoder *getDecoder(const ppl7::AudioInfo &info);

		float	progress, faktor;
		int	length;
		int	time,rest;

	public:
		CMP3Thread(CMP3Core *core);
		~CMP3Thread();
		void setConfig(const Config::Audio &default_audio, const Config::PathConfig &path);
		void setDirEntry(const ppl7::DirEntry &dir);
		const Config::PathConfig &getConfig() const;
		float getFactor() const;
		int getLength() const;
		int getTime() const;
		int getRest() const;
		float getProgress() const;
		const ppl7::String &getFilename() const;
		bool isRunning() const;

		void stopEncode();

		int LockFile();
		virtual void run();
		void	initMp3Encoder(ppl7::AudioEncoder_MP3 &encoder);
		void	encode();
		void	updateProgress(int progress);
};

class Rect
{
	public:
		int left;
		int top;
		int width;
		int height;
};

class QueueTreeWidgetItem : public QTreeWidgetItem
{
    public:
        ppl7::String Filename;
        Config::Audio audio;
};

class CMP3Core
{
	private:
		ppl7::Mutex Mutex;
		ppl7::String MyId;
		ppl7::ThreadPool Threads;
		void stopEncoder();
        void addPayloadFromFileQueue(QTreeWidget *filequeue);
        void loadTranslation();

        QTranslator qtTranslator;
        QTranslator mp3encodeTranslator;

	public:
		Config newconf;
		int numFiles;

		CMP3Core();
		void InitWindow();
		void SaveWindow(const char *name, const Rect &r);
		Rect LoadWindow(const char *name);

        void CheckTodo(QTreeWidget *filequeue);
		void UpdateProgress(QTreeWidget *t);
		void Pause();
		void Continue();
		void Stop();
		void FinishThreads(QTreeWidget *t=NULL);
		const char *GetMyId();

        bool isFileSupported(const ppl7::String &filename);
};

void TranslateEncoding(const Config::Audio &conf, ppl7::String &s);
ppl7::String TranslateEncoding(const Config::Audio &conf);

void SetBitrate(QComboBox *q, int bitrate);
int GetBitrate(QComboBox *q);
QString GetQualityLabel(int i);

#endif

