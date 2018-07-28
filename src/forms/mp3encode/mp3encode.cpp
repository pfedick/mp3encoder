/*
 * This file is part of MP3Encoder by Patrick Fedick
 *
 * $Author: patrick $
 * $Revision: 1.10 $
 * $Date: 2010/04/03 15:08:46 $
 * $Id: mp3encode.cpp,v 1.10 2010/04/03 15:08:46 patrick Exp $
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

#include <QDir>
#include <QList>
#include <QUrl>
#include <QMimeData>
#include <QDrag>
#include "mp3encode.h"
#include "src/forms/setup/setup.h"
#include "src/forms/editid3tag/editid3tag.h"

mp3encode::mp3encode(CMP3Core *core, QWidget *parent)
    : QDialog(parent)
{
    this->core=core;
    ppl7::String Tmp;
	ui.setupUi(this);
	timer=new QTimer(this);
	const char *lame_version=ppl7::AudioEncoder_MP3::getLameVersion();
    Tmp.setf("MP3Encoder - Version %s.%s / Lame Version %s",
			MP3ENCODE_VERSION, MP3ENCODE_BUILD_NUMBER,
			lame_version);
	this->setWindowTitle(Tmp);

	connect(timer, SIGNAL(timeout()), this, SLOT(timerupdate()));

	QString Style="QTreeView::item {\n"
		    		"border-right: 1px solid #b9b9b9;\n"
		    		"border-bottom: 1px solid #b9b9b9;\n"
		    		"}\n"
		    		"";
	ui.Progress->setStyleSheet(Style);
	ui.Done->setStyleSheet(Style);
    ui.QueueTreeWidget->installEventFilter(this);
    QString lastProfile=core->newconf.getCurrentProfile();
    updateAudioProfiles();
    int i=ui.audioProfileComboBox->findText(lastProfile);
    if (i>=0) ui.audioProfileComboBox->setCurrentIndex(i);
}

mp3encode::~mp3encode()
{

}

void mp3encode::changeEvent(QEvent* event)
{
    if (event->type()==QEvent::LanguageChange) {
        ui.retranslateUi(this);
    }
}


void mp3encode::updateAudioProfiles()
{
    QString current=ui.audioProfileComboBox->currentText();
    ui.audioProfileComboBox->clear();
    std::map<ppl7::String,Config::Audio>::const_iterator it;
    for (it=core->newconf.audioprofiles.begin();it!=core->newconf.audioprofiles.end();++it) {
        ui.audioProfileComboBox->addItem((*it).first);
    }
    int i=ui.audioProfileComboBox->findText(current);
    if (i>=0) ui.audioProfileComboBox->setCurrentIndex(i);
}

bool mp3encode::eventFilter(QObject *target, QEvent *event)
/*!\brief Event Handler
 *
 * \param[in] target Pointer auf das QObject, für den das Event bestimmt ist
 * \param[in] event Pointer auf eine QEvent Klasse, die den Event näher beschreibt
 * \returns Liefert \c true zurück, wenn der Event von Edit::consumeEvent verarbeitet wurde. Andernfalls
 * wird die übergeordnete Basisfunktion QWidget::eventFilter aufgerufen und deren Returncode zurückgeliefert.
 */
{
    if (target==ui.QueueTreeWidget) {
        if (event->type()==QEvent::Drop) {
            event->accept();
            handleFileDrop(static_cast<QDropEvent *>(event)->mimeData());
            return true;
        } else if (event->type()==QEvent::DragEnter) {
            if (dragContainsValidFiles(static_cast<QDropEvent *>(event)->mimeData())) {
                event->accept();
            } else {
                event->ignore();
            }
            return true;
        }
    }
    return QWidget::eventFilter(target,event);
}

bool mp3encode::dragContainsValidFiles(const QMimeData *mime)
{
    QList<QUrl>	list=mime->urls();
    for (int i=0;i<list.size();i++) {
        QUrl url=list[i];
        ppl7::String file=url.toLocalFile();
        if (core->isFileSupported(file)) return true;
    }
    return false;
}

void mp3encode::handleFileDrop(const QMimeData *mime)
{
    QList<QUrl>	list=mime->urls();
    for (int i=0;i<list.size();i++) {
        QUrl url=list[i];
        ppl7::String file=url.toLocalFile();
        if (core->isFileSupported(file)) {
            addFileToQueue(file);
        }
    }
    ppl7::String s;
    s.setf("%d",ui.QueueTreeWidget->topLevelItemCount());
    ui.numQueue->setText(s);
}

void mp3encode::addFileToQueue(const ppl7::String &filename)
{
    int count=ui.QueueTreeWidget->topLevelItemCount();
    for (int i=0;i<count;i++) {
        const QueueTreeWidgetItem *item=(QueueTreeWidgetItem *)ui.QueueTreeWidget->topLevelItem(i);
        if (item->Filename==filename) return;
    }
    Config::Audio audio=core->newconf.audio_default;

    ppl7::String profilename=ui.audioProfileComboBox->currentText();
    std::map<ppl7::String,Config::Audio>::const_iterator it;
    it=core->newconf.audioprofiles.find(profilename);
    if (it!=core->newconf.audioprofiles.end()) {
        audio=(*it).second;
    }

    QueueTreeWidgetItem *item=new QueueTreeWidgetItem();
    item->Filename=filename;
    item->audio=audio;
    item->setText(0,filename);
    item->setText(1,TranslateEncoding(item->audio));
    ui.QueueTreeWidget->addTopLevelItem(item);
    ui.QueueTreeWidget->resizeColumnToContents(0);
    ui.QueueTreeWidget->resizeColumnToContents(1);
}


void mp3encode::on_setupButton_clicked()
{
    Setup setup(NULL, this->core);
    setup.setConfig(core->newconf);
    if (setup.exec()) {
        core->newconf=setup.getConfig();
        core->newconf.save();
        updateAudioProfiles();
        core->loadTranslation();
    }
}

void mp3encode::on_startButton_clicked()
{
	ui.stopButton->setEnabled(true);
	ui.startButton->setEnabled(false);
	timer->start(1000);
}

void mp3encode::on_stopButton_clicked()
{
	ui.stopButton->setEnabled(false);
	ui.startButton->setEnabled(true);
	timer->stop();
	core->Stop();
	core->UpdateProgress(ui.Progress);
}

void mp3encode::on_clearButton_clicked()
{
	ui.Done->clear();
}

void mp3encode::on_audioProfileComboBox_currentIndexChanged(int)
{
    core->newconf.setCurrentProfile(ui.audioProfileComboBox->currentText());
}

void mp3encode::timerupdate()
{
    ppl7::String s;
    s.setf("%i",core->numFiles);
	//printf("Timer\n");
    core->CheckTodo(ui.QueueTreeWidget);
	core->UpdateProgress(ui.Progress);
	core->FinishThreads(ui.Done);
	ui.numFiles->setText(s);
    s.setf("%d",ui.QueueTreeWidget->topLevelItemCount());
    ui.numQueue->setText(s);
}


void mp3encode::InitWindow()
{
    Rect r=core->LoadWindow("mp3encode");
	if (r.width>0 && r.height>0) {
		move(r.left,r.top);
		resize(r.width,r.height);
	}
}

void mp3encode::closeEvent(QCloseEvent *)
{
	if (core) {
		Rect r;
		r.left=x();
		r.top=y();
		r.width=width();
		r.height=height();
		core->SaveWindow("mp3encode",r);
	}
	on_stopButton_clicked();
}

void mp3encode::on_id3tagEditorButton_clicked()
{
    EditId3Tag *editor=new EditId3Tag();
    editor->show();
}
