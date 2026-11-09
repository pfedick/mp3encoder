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

#include <QByteArray>
#include <QDir>
#include <QSettings>
#include <QLocale>
#include <QApplication>
#include <sys/types.h>
#include <unistd.h>

#include <pplib/core/random.h>
#include <pplib/core/regex.h>

#include "mp3core.h"

CMP3Core::CMP3Core()
{
    numFiles = 0;
    int p = (int)getpid();
    MyId.setf("%i_%i", p, (int)pplib::rand(66666, 99999999));
    newconf.load();
    loadTranslation();
}

void CMP3Core::loadTranslation()
{
    qApp->removeTranslator(&qtTranslator);
    qApp->removeTranslator(&mp3encodeTranslator);
    pplib::String filename;
    filename = ":/locale/qt_" + newconf.language;
    qtTranslator.load(filename);
    qApp->installTranslator(&qtTranslator);
    filename = ":/locale/mp3encode_" + newconf.language;
    mp3encodeTranslator.load(filename);
    qApp->installTranslator(&mp3encodeTranslator);
}

const char* CMP3Core::GetMyId()
{
    return (const char*)MyId;
}

void CMP3Core::SaveWindow(const char* name, const Rect& r)
{
    pplib::String tmp;
    tmp.setf("%i,%i,%i,%i", r.left, r.top, r.width, r.height);
    Mutex.lock();
    QSettings settings("Patrick F.-Productions", "mp3encode");
    settings.beginGroup("windows");
    settings.setValue(name, (const char*)tmp);
    settings.endGroup();
    Mutex.unlock();
}

static Rect StringToRect(const pplib::String& data)
{
    Rect r;
    pplib::Array tok(data, ",");
    r.left = r.top = r.width = r.height = 0;
    if (tok.size() == 4) {
        r.left = tok[0].toInt();
        r.top = tok[1].toInt();
        r.width = tok[2].toInt();
        r.height = tok[3].toInt();
    }
    return r;
}

Rect CMP3Core::LoadWindow(const char* name)
{
    pplib::String tmp;
    Mutex.lock();
    QSettings settings("Patrick F.-Productions", "mp3encode");
    settings.beginGroup("windows");
    tmp = settings.value(name, "0,0,0,0").toString();
    settings.endGroup();
    Mutex.unlock();
    return StringToRect(tmp);
}

void CMP3Core::CheckTodo(QTreeWidget* filequeue)
{
    if (!Mutex.tryLock()) return;
    // Sind noch CPUs frei?
    if (Threads.size() >= (size_t)newconf.numCPUs) {
        Mutex.unlock();
        return;
    }
    if (filequeue->topLevelItemCount() > 0) {
        addPayloadFromFileQueue(filequeue); // entsperrt Mutex
        return;
    }
    pplib::Dir d;
    pplib::DirEntry dir;
    pplib::DirEntry entry;
    pplib::String pattern;
    uint64_t highest, now;
    highest = now = pplib::GetTime();
    highest += 1000;
    int count = 0;
    bool found = false;
    Config::PathConfig current_config;
    std::list<Config::PathConfig>::const_iterator it;
    for (it = newconf.pathconfig.begin(); it != newconf.pathconfig.end(); ++it) {
        const Config::PathConfig& path = (*it);
        // Ist der Pfad aktiv?
        if (!path.active) continue;
        pattern = path.Source;
        pattern.trimRight("/");
        pattern += "/*.*";
        try {
            d.open(path.Source, pplib::Dir::Sort::FilenameIgnoreCase);
            for (const pplib::DirEntry& entry : d) {
                // Es muss sich um eine gültige Datei handeln
                if (entry.isFile() == true) {
                    // Matched das File auf .wav oder .mp3?
                    pplib::String Filename = entry.Filename;
                    if (isFileSupported(Filename)) {
                        count++;
                        /*
                        printf("Found: %s, ATIME=%llu, MTIME=%llu, CTIME=%llu, highest=%llu\n",
								(const char*)entry.Filename,
								entry.ATime.epoch(),
								entry.MTime.epoch(),
								entry.CTime.epoch(),
								highest
								);
                        */
                        // Das aelteste File suchen das seit mind. conf.EncoderDelay Sekunden nicht mehr angepackt wurde
                        if (entry.CTime.epoch() < highest && entry.CTime.epoch() + newconf.EncoderDelay < now) {
                            highest = entry.CTime.epoch();
                            dir = entry;
                            current_config = path;
                            found = true;
                        }
                    }
                }
            }
        }
        catch (...) {
        }
    }
    numFiles = count;

    if (found) {
        CMP3Thread* thread = new CMP3Thread(this);
        thread->threadSetPriority(newconf.priority);
        thread->setConfig(newconf.audio_default, current_config);
        thread->setDirEntry(dir);
        if (thread->LockFile()) {
            try {
                thread->threadStart();
                Threads.addThread(thread);
                Mutex.unlock();
                CheckTodo(filequeue); // recurse
                return;
            }
            catch (...) {
            }
        }
        delete thread;
    }
    Mutex.unlock();
}

void CMP3Core::addPayloadFromFileQueue(QTreeWidget* filequeue)
/*\note Der Mutex ist bei Aufruf der Funktion gelockt!
 */
{
    QueueTreeWidgetItem* item = (QueueTreeWidgetItem*)filequeue->topLevelItem(0);
    pplib::DirEntry dir;
    try {
        pplib::File::statFile(item->Filename, dir);
        if (!dir.isFile()) {
            item = (QueueTreeWidgetItem*)filequeue->takeTopLevelItem(0);
            delete (item);
            Mutex.unlock();
            return;
        }
    }
    catch (...) {
        printf("DEBUG File::statFile: %ls\n", (const wchar_t*)pplib::WideString(item->Filename));
        item = (QueueTreeWidgetItem*)filequeue->takeTopLevelItem(0);
        delete (item);
        Mutex.unlock();
        return;
    }
    Config::PathConfig path_conf;
    path_conf.audio_override = false;
    path_conf.Source = dir.Path;
    path_conf.Target = dir.Path;
    path_conf.active = true;
    path_conf.enable_backup = false;

    CMP3Thread* thread = new CMP3Thread(this);
    thread->threadSetPriority(newconf.priority);
    thread->setConfig(item->audio, path_conf);
    thread->setDirEntry(dir);
    if (thread->LockFile()) {
        try {
            thread->threadStart();
            Threads.addThread(thread);
            item = (QueueTreeWidgetItem*)filequeue->takeTopLevelItem(0);
            delete item;
            Mutex.unlock();
            CheckTodo(filequeue); // recurse
            return;
        }
        catch (...) {
        }
    }
    Mutex.unlock();
}

void CMP3Core::stopEncoder()
{
    // getThreads() liefert eine Momentaufnahme des Pools und sperrt intern.
    for (pplib::Thread* poolThread : Threads.getThreads()) {
        static_cast<CMP3Thread*>(poolThread)->stopEncode();
    }
    Threads.stopThreads();
}

void CMP3Core::FinishThreads(QTreeWidget* t)
{
    Mutex.lock();
    pplib::String tmp;
    pplib::WideString wtmp;
    for (pplib::Thread* poolThread : Threads.getThreads()) {
        CMP3Thread* thread = static_cast<CMP3Thread*>(poolThread);
        if (thread->isRunning() == false && thread->threadIsRunning() == false) {
            // TODO: Inhalt des Threads wird in Done gespeichert
            if (t) {
                QTreeWidgetItem* item;
                item = new QTreeWidgetItem(t);
                wtmp.set(thread->getFilename());
                if (wtmp.len() > 40) {
                    wtmp.cut(40);
                    wtmp += L"...";
                }
                item->setText(0, wtmp);
                tmp.setf("%0i:%02i", thread->getLength() / 60, thread->getLength() % 60);
                item->setText(1, tmp);
                tmp.setf("%0i:%02i", thread->getTime() / 60, thread->getTime() % 60);
                item->setText(2, tmp);
                tmp.setf("%0.2f", thread->getFactor());
                item->setText(3, tmp);
                TranslateEncoding(thread->getConfig().audio, tmp);
                item->setText(4, tmp);
                t->resizeColumnToContents(0);
                t->resizeColumnToContents(1);
                t->resizeColumnToContents(2);
                t->resizeColumnToContents(3);
                t->resizeColumnToContents(4);
            }
            Threads.removeThread(thread);
            delete thread;
        }
    }
    Mutex.unlock();
}

void TranslateEncoding(const Config::Audio& conf, pplib::String& s)
{
    pplib::String s1;
    switch (conf.mode) {
    case Config::Audio::EncoderMode::CBR:
        s.setf("CBR %d, ", conf.cbr_bitrate);
        break;
    case Config::Audio::EncoderMode::ABR:
        s.setf("ABR %d, ", conf.abr_bitrate);
        break;
    case Config::Audio::EncoderMode::VBR:
        s.setf("VBR %d-%d, ", conf.vbr_min_bitrate, conf.vbr_max_bitrate);
        break;
    }
    s.appendf("q%d, ", conf.quality);
    switch (conf.channel_mode) {
    case pplib::AudioInfo::ChannelMode::STEREO:
        s.append("stereo");
        break;
    case pplib::AudioInfo::ChannelMode::DUAL_CHANNEL:
        s.append("dual");
        break;
    case pplib::AudioInfo::ChannelMode::JOINT_STEREO:
        s.append("jstereo");
        break;
        break;
    default:
        s.append("mono");
        break;
    }
}

pplib::String TranslateEncoding(const Config::Audio& conf)
{
    pplib::String s;
    TranslateEncoding(conf, s);
    return s;
}

void CMP3Core::UpdateProgress(QTreeWidget* t)
{
    Mutex.lock();
    t->clear();
    pplib::String tmp;
    pplib::WideString wtmp;
    int cpu = 1;
    for (pplib::Thread* poolThread : Threads.getThreads()) {
        CMP3Thread* thread = static_cast<CMP3Thread*>(poolThread);
        tmp.setf("%i", cpu);
        QTreeWidgetItem* item;
        item = new QTreeWidgetItem(t);
        item->setText(0, tmp);
        wtmp.set(thread->getFilename());
        if (wtmp.len() > 40) {
            wtmp.cut(40);
            wtmp += L"...";
        }

        item->setText(1, wtmp);
        tmp.setf("%0i:%02i", thread->getLength() / 60, thread->getLength() % 60);
        item->setText(2, tmp);
        tmp.setf("%0.0f %%", thread->getProgress());
        item->setText(3, tmp);
        item->setTextAlignment(3, Qt::AlignHCenter);
        tmp.setf("%0i:%02i", thread->getTime() / 60, thread->getTime() % 60);
        item->setText(4, tmp);
        tmp.setf("%0i:%02i", thread->getRest() / 60, thread->getRest() % 60);
        item->setText(5, tmp);
        tmp.setf("%0.2f", thread->getFactor());
        item->setText(6, tmp);
        TranslateEncoding(thread->getConfig().audio, tmp);
        item->setText(7, tmp);
        cpu++;
    }
    if (cpu > 1) {
        t->resizeColumnToContents(0);
        t->resizeColumnToContents(1);
        t->resizeColumnToContents(2);
        t->resizeColumnToContents(3);
        t->resizeColumnToContents(4);
        t->resizeColumnToContents(5);
        t->resizeColumnToContents(6);
        t->resizeColumnToContents(7);
    }
    Mutex.unlock();
}

void CMP3Core::Pause()
{
    //Threads.threadSuspend();
}

void CMP3Core::Continue()
{
    //Threads.Resume();
}

void CMP3Core::Stop()
{
    stopEncoder();
    Threads.signalStopThreads();
    Threads.stopThreads();
    FinishThreads();
}

bool CMP3Core::isFileSupported(const pplib::String& filename)
{
    const int flags = pplib::RegEx::Flags::CASELESS;
    if (pplib::RegEx::match("\\.wav$", filename, flags)) return true;
    if (pplib::RegEx::match("\\.aif{1,2}$", filename, flags)) return true;
    if (pplib::RegEx::match("\\.mp3$", filename, flags) == true && newconf.recode == true) return true;
    return false;
}
