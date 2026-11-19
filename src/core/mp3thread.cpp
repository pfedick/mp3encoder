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

#include "mp3core.h"
#include <pplib/core/functions.h>

CMP3Thread::CMP3Thread(CMP3Core* core)
{
    this->core = core;
    bFileRenamed = false;
    bFinished = false;
    running = true;
    progress = 0.0f;
    faktor = 0.0f;
    length = 0;
    time = 0;
    rest = 0;
    startTime = 0;
}

CMP3Thread::~CMP3Thread()
{
    if (!bFinished) {
        if (bFileRenamed) {
            // Bisher encodetes File muss gelöscht werden
            pplib::File::remove(TargetFile);
        }
    }
}

void CMP3Thread::setConfig(const Config::Audio& default_audio, const Config::PathConfig& path)
{
    config = path;
    if (!config.audio_override) config.audio = default_audio;
}

void CMP3Thread::setDirEntry(const pplib::DirEntry& dir)
{
    this->dir = dir;
}

const Config::PathConfig& CMP3Thread::getConfig() const
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

const pplib::String& CMP3Thread::getFilename() const
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
    const char* myid = core->GetMyId();
    pplib::String tmpfile, Source, Target, Suffix;
    Source = config.Source;
    Target = config.Target;
    Source.trimRight("/");
    Target.trimRight("/");

    Suffix = "mp3";
    if (config.audio.format == pplib::AudioInfo::AudioFormat::FLAC) Suffix = "flac";
    if (config.audio.format == pplib::AudioInfo::AudioFormat::AIFF) Suffix = "aiff";

    Filename = dir.Filename;

    tmpfile = dir.Filename;
    size_t l = tmpfile.len();
    for (size_t i = l - 1; i > 0; i--) { // Dateiendung abschneiden
        if (tmpfile[i] == '.') {
            tmpfile.chop(l - i);
            break;
        }
    }
    //printf ("tmpfile: %s\n",(char*)tmpfile);

    OriginalFile = dir.File;
    TargetFile.setf("%s/%s.%s.%s.tgt.enc", (const char*)Target, (const char*)tmpfile, (const char*)Suffix, myid);
    EndFile.setf("%s/%s.%s", (const char*)Target, (const char*)tmpfile, (const char*)Suffix);

    //printf ("OriginalFile: %ls\n",(const wchar_t*)pplib::WideString(OriginalFile));
    //printf ("EndFile: %ls\n",(const wchar_t*)pplib::WideString(EndFile));
    //printf ("TargetFile: %ls\n",(const wchar_t*)pplib::WideString(TargetFile));
    //fflush(stdout);

    // Kann die Zieldatei geöffnet werden?
    try {
        zz.open(TargetFile, pplib::File::FileMode::WRITE);
    }
    catch (...) {
        return 0;
    }

    // Quellfile öffnen
    try {
        ss.open(OriginalFile, pplib::File::FileMode::READ);
    }
    catch (...) {
        zz.close();
        pplib::File::remove(TargetFile);
        return 0;
    }
    bFinished = false;
    bFileRenamed = true;
    return 1;
}

void CMP3Thread::stopEncode()
{
    if (config.audio.format == pplib::AudioInfo::AudioFormat::FLAC) {
        flac_encoder.stopEncoder();
    } else if (config.audio.format == pplib::AudioInfo::AudioFormat::AIFF) {
        aiff_encoder.stopEncoder();
    } else {
        mp3_encoder.stopEncoder();
    }
}

void CMP3Thread::run()
{
    int ret = 0;
    try {
        encode();
        ret = 1;
    }
    catch (const pplib::Exception& exp) {
        printf("CMP3Thread::run - ");
        exp.print();
        ret = 0;
    }
    zz.close();
    ss.close();
    if (ret) { // Erfolgreich beendet
        try {
            // Löschen oder Backup?
            if (config.enable_backup) {
                pplib::String BackupFile = config.Backup;
                BackupFile += "/" + pplib::File::getFilename(OriginalFile);
                try {
                    pplib::File::move(OriginalFile, BackupFile);
                }
                catch (...) {
                    pplib::File::copy(OriginalFile, BackupFile);
                    pplib::File::remove(OriginalFile);
                }
            } else {
                pplib::File::remove(OriginalFile);
            }
            pplib::File::rename(TargetFile, EndFile);
            bFinished = true;
        }
        catch (const pplib::Exception& exp) {
            printf("ERROR CMP3Thread::run => %s\n", (const char*)exp.toString());
            fflush(stdout);
        }
    }
    running = false;
    //LastActivity=ppl6::GetTime();
}

static void progressPPL7Encoder(int progress, void* priv)
{
    ((CMP3Thread*)priv)->updateProgress(progress);
}

pplib::AudioDecoder* CMP3Thread::getDecoder(const pplib::AudioInfo& info)
{
    if (info.Format == pplib::AudioInfo::AudioFormat::AIFF) {
        return new pplib::AudioDecoder_Aiff();
    } else if (info.Format == pplib::AudioInfo::AudioFormat::WAVE) {
        return new pplib::AudioDecoder_Wave();
    } else if (info.Format == pplib::AudioInfo::AudioFormat::MP3) {
        return new pplib::AudioDecoder_MP3();
    } else if (info.Format == pplib::AudioInfo::AudioFormat::FLAC) {
        return new pplib::AudioDecoder_Flac();
    } else if (info.Format == pplib::AudioInfo::AudioFormat::OGG) {
        return new pplib::AudioDecoder_Ogg();
    }
    throw pplib::UnsupportedAudioFormatException();
}

void CMP3Thread::encodeMp3(pplib::AudioDecoder* decoder, const pplib::AudioInfo& info, const pplib::ID3Tag& Tag)
{
    mp3_encoder.setProgressFunction(progressPPL7Encoder, this);
    mp3_encoder.setStereoMode(config.audio.channel_mode);
    if (config.audio.mode == Config::Audio::EncoderMode::VBR) {
        mp3_encoder.setVBR(config.audio.vbr_min_bitrate, config.audio.vbr_max_bitrate, config.audio.quality);
    } else if (config.audio.mode == Config::Audio::EncoderMode::ABR) {
        mp3_encoder.setABR(config.audio.abr_bitrate, config.audio.quality);
    } else {
        mp3_encoder.setCBR(config.audio.cbr_bitrate, config.audio.quality);
    }
    mp3_encoder.startEncode(zz);
    if (info.HaveID3v2Tag) {
        mp3_encoder.writeID3v2Tag(Tag);
    }
    mp3_encoder.encode(*decoder);
    mp3_encoder.finish();
    if (info.HaveID3v2Tag) {
        try {
            mp3_encoder.writeID3v1Tag(Tag);
        }
        catch (...) {
        }
    }
}

void CMP3Thread::encodeFlac(pplib::AudioDecoder* decoder, const pplib::AudioInfo& info, const pplib::ID3Tag& Tag)
{
    flac_encoder.setProgressFunction(progressPPL7Encoder, this);
    flac_encoder.setCompressionLevel(config.audio.flac_compression_level);
    flac_encoder.startEncode(zz);
    if (info.HaveID3v2Tag) {
        flac_encoder.setVorbisComments(Tag);
    }
    flac_encoder.encode(*decoder);
    flac_encoder.finish();
}

void CMP3Thread::encodeAiff(pplib::AudioDecoder* decoder, const pplib::AudioInfo& info, const pplib::ID3Tag& Tag)
{
    aiff_encoder.setProgressFunction(progressPPL7Encoder, this);
    aiff_encoder.startEncode(zz);
    if (info.HaveID3v2Tag) {
        aiff_encoder.writeID3v2Tag(Tag);
    }
    aiff_encoder.encode(*decoder);
    aiff_encoder.finish();
}

void CMP3Thread::encode()
{
    pplib::ID3Tag Tag;
    if (config.audio.format != pplib::AudioInfo::AudioFormat::FLAC && config.audio.format != pplib::AudioInfo::AudioFormat::AIFF &&
        config.audio.format != pplib::AudioInfo::AudioFormat::MP3) {
        throw pplib::UnsupportedAudioFormatException();
    }
    pplib::AudioInfo info;
    if (pplib::IdentAudioFile(ss, info) != true) {
        throw pplib::UnsupportedAudioFormatException();
    }
    if (Tag.tryLoad(ss)) {
        info.HaveID3v2Tag = true;
    } else {
        info.HaveID3v2Tag = false;
    }
    pplib::AudioDecoder* decoder = getDecoder(info);
    try {
        length = info.Length / 1000;
        startTime = pplib::GetMicrotime();
        decoder->open(ss);
        if (config.audio.format == pplib::AudioInfo::AudioFormat::FLAC) {
            encodeFlac(decoder, info, Tag);
        } else if (config.audio.format == pplib::AudioInfo::AudioFormat::AIFF) {
            encodeAiff(decoder, info, Tag);
        } else {
            encodeMp3(decoder, info, Tag);
        }
    }
    catch (...) {
        delete decoder;
        throw;
    }
    delete decoder;
}

void CMP3Thread::updateProgress(int progress)
{
    this->progress = (float)progress;
    double duration = pplib::GetMicrotime() - startTime;
    time = (int)duration;
    if (progress > 0) {
        double estimated_total = duration * 100.0 / progress;
        double playtime = (double)length;
        faktor = playtime / estimated_total;
        rest = (int)(estimated_total - duration);
    }
}
