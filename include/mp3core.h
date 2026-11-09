/*******************************************************************************
 * This file is part of MP3Encoder by Patrick Fedick
 * Web: https://github.com/pfedick/mp3encoder
 *******************************************************************************
 * Copyright (c) 2026, Patrick Fedick <patrick@pfp.de>
 * All rights reserved.
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

#include <QComboBox>
#include <QSettings>
#include <QStatusBar>
#include <QString>
#include <QTranslator>
#include <QTreeView>
#include <QTreeWidget>
#include <QTreeWidgetItem>
// Die Defines setzt auch CMake global; die Guardierungen vermeiden
// Warnungen ueber doppelte Definition.
#ifndef WITH_QT
#define WITH_QT
#endif
#ifndef PPL_WITH_QT6
#define PPL_WITH_QT6
#endif
#ifndef PPL_QT_STRING_UTF8
#define PPL_QT_STRING_UTF8
#endif
#include <list>
#include <map>
#include <pplib/audio/audioinfo.h>
#include <pplib/audio/decoder.h>
#include <pplib/audio/encoder.h>
#include <pplib/core/dir.h>
#include <pplib/core/file.h>
#include <pplib/core/mutex.h>
#include <pplib/core/thread.h>
#include <pplib/core/threadpool.h>
#include <pplib/types/string.h>
#include <pplib/types/widestring.h>

#define READCACHESIZE 512 * 1024
#define WRITECACHESIZE 2 * 1024 * 1024

class CMP3Core;

class Config
{
public:
    class Audio
    {
    public:
        enum EncoderMode
        {
            CBR,
            ABR,
            VBR
        };
        Audio();
        int frequency;
        int quality;
        pplib::AudioInfo::ChannelMode channel_mode;
        EncoderMode mode;
        int cbr_bitrate;
        int abr_bitrate;
        int vbr_min_bitrate;
        int vbr_max_bitrate;
        void load(QSettings& settings);
        void save(QSettings& settings) const;
    };
    class PathConfig
    {
    public:
        pplib::String Source;
        pplib::String Target;
        pplib::String Backup;
        bool active;
        bool audio_override;
        bool enable_backup;
        Audio audio;
        PathConfig();
        void load(QSettings& settings);
        void save(QSettings& settings) const;
    };

private:
    void loadPaths(QSettings& settings);
    void savePaths(QSettings& settings) const;
    static pplib::String getSystemLang();

public:
    int numCPUs;
    bool autostart;
    bool recode;
    bool keepID3;
    int EncoderDelay;
    pplib::String language;
    pplib::Thread::Priority priority;
    Audio audio_default;
    std::list<PathConfig> pathconfig;
    std::map<pplib::String, Audio> audioprofiles;

public:
    Config();
    void load();
    void save() const;
    void setCurrentProfile(const QString& name);
    QString getCurrentProfile();
};

class CMP3Thread : public pplib::Thread
{
    // friend class CMP3Core;
private:
    pplib::AudioEncoder_MP3 encoder;
    bool bFileRenamed;
    bool bFinished;
    bool running;
    CMP3Core* core;
    pplib::DirEntry dir;
    pplib::String Filename;
    pplib::String TargetFile, EndFile, OriginalFile;
    pplib::String Backup;
    Config::PathConfig config;
    pplib::File ss, zz;
    double startTime;
    pplib::AudioDecoder* getDecoder(const pplib::AudioInfo& info);

    float progress, faktor;
    int length;
    int time, rest;

public:
    CMP3Thread(CMP3Core* core);
    ~CMP3Thread();
    void setConfig(const Config::Audio& default_audio, const Config::PathConfig& path);
    void setDirEntry(const pplib::DirEntry& dir);
    const Config::PathConfig& getConfig() const;
    float getFactor() const;
    int getLength() const;
    int getTime() const;
    int getRest() const;
    float getProgress() const;
    const pplib::String& getFilename() const;
    bool isRunning() const;

    void stopEncode();

    int LockFile();
    virtual void run();
    void initMp3Encoder(pplib::AudioEncoder_MP3& encoder);
    void encode();
    void updateProgress(int progress);
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
    pplib::String Filename;
    Config::Audio audio;
};

class CMP3Core
{
private:
    pplib::Mutex Mutex;
    pplib::String MyId;
    pplib::ThreadPool Threads;
    void stopEncoder();
    void addPayloadFromFileQueue(QTreeWidget* filequeue);

    QTranslator qtTranslator;
    QTranslator mp3encodeTranslator;

public:
    Config newconf;
    int numFiles;

    CMP3Core();
    void InitWindow();
    void SaveWindow(const char* name, const Rect& r);
    Rect LoadWindow(const char* name);
    void loadTranslation();

    void CheckTodo(QTreeWidget* filequeue);
    void UpdateProgress(QTreeWidget* t);
    void Pause();
    void Continue();
    void Stop();
    void FinishThreads(QTreeWidget* t = NULL);
    const char* GetMyId();

    bool isFileSupported(const pplib::String& filename);
};

void TranslateEncoding(const Config::Audio& conf, pplib::String& s);
pplib::String TranslateEncoding(const Config::Audio& conf);

void SetBitrate(QComboBox* q, int bitrate);
int GetBitrate(QComboBox* q);
QString GetQualityLabel(int i);

#endif
