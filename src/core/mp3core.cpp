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

#include <QByteArray>
#include <QDir>
#include <QSettings>
#include <sys/types.h>
#include <unistd.h>



#include "mp3core.h"

CMP3Core::CMP3Core()
{
	numFiles=0;
	int p=(int)getpid();
    MyId.setf("%i_%i",p,(int)ppl7::rand(66666,99999999));
    newconf.load();
}

const char *CMP3Core::GetMyId()
{
	return (const char*)MyId;
}

void CMP3Core::SaveWindow(const char *name, const Rect &r)
{
	ppl7::String tmp;
	tmp.setf("%i,%i,%i,%i",r.left,r.top,r.width,r.height);
	Mutex.lock();
	QSettings settings("Patrick F.-Productions","mp3encode");
	settings.beginGroup("windows");
	settings.setValue(name,(const char*)tmp);
	settings.endGroup();
	Mutex.unlock();
}

static Rect StringToRect(const ppl7::String &data)
{
	Rect r;
	ppl7::Array tok(data,",");
	r.left=r.top=r.width=r.height=0;
	if (tok.size()==4) {
		r.left=tok[0].toInt();
		r.top=tok[1].toInt();
		r.width=tok[2].toInt();
		r.height=tok[3].toInt();
	}
	return r;
}

Rect CMP3Core::LoadWindow(const char *name)
{
	ppl7::String tmp;
	Mutex.lock();
	QSettings settings("Patrick F.-Productions","mp3encode");
	settings.beginGroup("windows");
	tmp=settings.value(name,"0,0,0,0").toString();
	settings.endGroup();
	Mutex.unlock();
	return StringToRect(tmp);
}

void CMP3Core::CheckTodo(QTreeWidget *filequeue)
{
    if (!Mutex.tryLock()) return;
	// Sind noch CPUs frei?
	if (Threads.size()>=(size_t)newconf.numCPUs) {
        Mutex.unlock();
		return;
	}
    if (filequeue->topLevelItemCount()>0) {
        addPayloadFromFileQueue(filequeue); // entsperrt Mutex
        return;
    }
	ppl7::Dir d;
	ppl7::DirEntry dir;
	ppl7::Dir::Iterator dirIt;
	ppl7::DirEntry entry;
	ppl7::String pattern;
	ppluint64 highest,now;
    highest=now=ppl7::GetTime();
	highest+=1000;
	int count=0;
	bool found=false;
	Config::PathConfig current_config;
	std::list<Config::PathConfig>::const_iterator it;
	for (it=newconf.pathconfig.begin();it!=newconf.pathconfig.end();++it) {
		const Config::PathConfig &path=(*it);
		// Ist der Pfad aktiv?
		if (!path.active) continue;
		pattern=path.Source;
		pattern.trimRight("/");
		pattern+="/*.*";
		try {
			d.open(path.Source,d.SORT_FILENAME_IGNORCASE);
			d.reset(dirIt);
			while (d.getNext(entry,dirIt)) {
				// Es muss sich um eine gültige Datei handeln
				if (entry.isFile()==true) {
					// Matched das File auf .wav oder .mp3?
					ppl7::String Filename=entry.Filename;
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
                        if (entry.CTime.epoch() < highest && entry.CTime.epoch()+newconf.EncoderDelay < now) {
                            highest=entry.CTime.epoch();
							dir=entry;
							current_config=path;
							found=true;
						}
					}
				}
			}
		} catch (...) {

		}
	}
	numFiles=count;

	if (found) {
        CMP3Thread *thread=new CMP3Thread(this);
        thread->threadSetPriority(newconf.priority);
        thread->setConfig(newconf.audio_default, current_config);
        thread->setDirEntry(dir);
        if (thread->LockFile()) {
            try {
                thread->threadStart();
                Threads.addThread(thread);
                Mutex.unlock();
                CheckTodo(filequeue);        // recurse
                return;
            } catch (...) {

            }
        }
        delete thread;
	}
	Mutex.unlock();
}

void CMP3Core::addPayloadFromFileQueue(QTreeWidget *filequeue)
/*\note Der Mutex ist bei Aufruf der Funktion gelockt!
 */
{
    QueueTreeWidgetItem *item=(QueueTreeWidgetItem*)filequeue->topLevelItem(0);
    ppl7::DirEntry dir;
    ppl7::File::statFile(item->Filename, dir);
    if (!dir.isFile()) {
        item=(QueueTreeWidgetItem*)filequeue->takeTopLevelItem(0);
        delete (item);
        Mutex.unlock();
        return;
    }
    Config::PathConfig path_conf;
    path_conf.audio_override=false;
    path_conf.Source=dir.Path;
    path_conf.Target=dir.Path;
    path_conf.active=true;
    path_conf.enable_backup=false;

    CMP3Thread *thread=new CMP3Thread(this);
    thread->threadSetPriority(newconf.priority);
    thread->setConfig(item->audio, path_conf);
    thread->setDirEntry(dir);
    if (thread->LockFile()) {
        try {
            thread->threadStart();
            Threads.addThread(thread);
            item=(QueueTreeWidgetItem*)filequeue->takeTopLevelItem(0);
            delete item;
            Mutex.unlock();
            CheckTodo(filequeue);        // recurse
            return;
        } catch (...) {

        }
    }
    Mutex.unlock();
}

void CMP3Core::stopEncoder()
{
	ppl7::ThreadPool::iterator it;
	Threads.lock();
	for (it=Threads.begin();it!=Threads.end();++it) {
		static_cast<CMP3Thread*>(*it)->stopEncode();
	}
	Threads.unlock();
	Threads.stopThreads();
}

void CMP3Core::FinishThreads(QTreeWidget *t)
{
	Mutex.lock();
	Threads.lock();
	ppl7::ThreadPool::iterator it;
	ppl7::String tmp;
	ppl7::WideString wtmp;
    it=Threads.begin();
    while (it!=Threads.end()) {
		CMP3Thread *thread=static_cast<CMP3Thread*>(*it);
		if (thread->isRunning()==false && thread->threadIsRunning()==false) {
			// TODO: Inhalt des Threads wird in Done gespeichert
			if(t) {
				QTreeWidgetItem *item;
				item=new QTreeWidgetItem(t);
				wtmp.set(thread->getFilename());
				if (wtmp.len()>40) {
					wtmp.cut(40);
					wtmp+=L"...";
				}
				item->setText(0,wtmp);
				tmp.setf("%0i:%02i",thread->getLength()/60,thread->getLength() % 60);
				item->setText(1,tmp);
				tmp.setf("%0i:%02i",thread->getTime()/60,thread->getTime() % 60);
				item->setText(2,tmp);
				tmp.setf("%0.2f",thread->getFactor());
				item->setText(3,tmp);
				TranslateEncoding(thread->getConfig().audio,tmp);
				item->setText(4,tmp);
				t->resizeColumnToContents(0);
				t->resizeColumnToContents(1);
				t->resizeColumnToContents(2);
				t->resizeColumnToContents(3);
				t->resizeColumnToContents(4);
			}
			Threads.unlock();
			Threads.removeThread(thread);
			delete thread;
            Threads.lock();
            it=Threads.begin();
        } else {
            ++it;
        }
	}
	Threads.unlock();
	Mutex.unlock();
}

void TranslateEncoding(const Config::Audio &conf, ppl7::String &s)
{
	ppl7::String s1;
	switch (conf.mode) {
		case Config::Audio::EncoderMode::CBR:
			s.setf("CBR %d, ", conf.cbr_bitrate);
			break;
		case Config::Audio::EncoderMode::ABR:
			s.setf("ABR %d, ",conf.abr_bitrate);
			break;
		case Config::Audio::EncoderMode::VBR:
			s.setf("VBR %d-%d, ",conf.vbr_min_bitrate,
					conf.vbr_max_bitrate);
			break;

	}
	s.appendf("q%d, ",conf.quality);
	switch (conf.channel_mode) {
		case ppl7::AudioInfo::ChannelMode::STEREO:
            s.append("stereo"); break;
		case ppl7::AudioInfo::ChannelMode::DUAL_CHANNEL:
            s.append("dual"); break;
		case ppl7::AudioInfo::ChannelMode::JOINT_STEREO:
            s.append("jstereo"); break;
			break;
		default:
            s.append("mono");
			break;
	}
}

ppl7::String TranslateEncoding(const Config::Audio &conf)
{
    ppl7::String s;
    TranslateEncoding(conf,s);
    return s;
}


void CMP3Core::UpdateProgress(QTreeWidget *t)
{
	Mutex.lock();
	Threads.lock();
	t->clear();
	ppl7::String tmp;
	ppl7::WideString wtmp;
	ppl7::ThreadPool::iterator it;
	int cpu=1;
	for (it=Threads.begin();it!=Threads.end();++it) {
		CMP3Thread *thread=static_cast<CMP3Thread*>(*it);
		tmp.setf("%i",cpu);
		QTreeWidgetItem *item;
		item=new QTreeWidgetItem(t);
		item->setText(0,tmp);
		wtmp.set(thread->getFilename());
		if (wtmp.len()>40) {
			wtmp.cut(40);
			wtmp+=L"...";
		}

		item->setText(1,wtmp);
		tmp.setf("%0i:%02i",thread->getLength()/60,thread->getLength() % 60);
		item->setText(2,tmp);
		tmp.setf("%0.0f %%",thread->getProgress());
		item->setText(3,tmp);
		item->setTextAlignment(3,Qt::AlignHCenter);
		tmp.setf("%0i:%02i",thread->getTime()/60,thread->getTime() % 60);
		item->setText(4,tmp);
		tmp.setf("%0i:%02i",thread->getRest()/60,thread->getRest() % 60);
		item->setText(5,tmp);
		tmp.setf("%0.2f",thread->getFactor());
		item->setText(6,tmp);
		TranslateEncoding(thread->getConfig().audio,tmp);
		item->setText(7,tmp);
		cpu++;
	}
	if (cpu>1) {
		t->resizeColumnToContents(0);
		t->resizeColumnToContents(1);
		t->resizeColumnToContents(2);
		t->resizeColumnToContents(3);
		t->resizeColumnToContents(4);
		t->resizeColumnToContents(5);
		t->resizeColumnToContents(6);
		t->resizeColumnToContents(7);
	}
	Threads.unlock();
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


bool CMP3Core::isFileSupported(const ppl7::String &filename)
{
    if (filename.pregMatch("/\\.wav$/i")) return true;
    if (filename.pregMatch("/\\.aif{1,2}$/i")) return true;
    if (filename.pregMatch("/\\.mp3$/i")==true && newconf.recode==true) return true;
    return false;
}
