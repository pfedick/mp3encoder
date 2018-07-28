/*
 * This file is part of MP3Encoder by Patrick Fedick
 *
 * $Author: patrick $
 * $Revision: 1.8 $
 * $Date: 2010/04/03 15:08:46 $
 * $Id: mp3encode.h,v 1.8 2010/04/03 15:08:46 patrick Exp $
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

#ifndef MP3ENCODE_H
#define MP3ENCODE_H

#include <QDialog>
#include <QString>
#include <QByteArray>
#include <QTimer>
#include <QStatusBar>
#include <QCloseEvent>
#include <QTreeWidgetItem>
#include <QUrl>
#include "mp3core.h"
#include "version.h"
#include "ui_mp3encode.h"



class mp3encode : public QDialog
{
    Q_OBJECT

public:
    mp3encode(CMP3Core *core, QWidget *parent = 0);
    ~mp3encode();

    CMP3Core *core;

    void InitWindow();

private:
    Ui::mp3encodeClass ui;
    QTimer *timer;
    bool dragContainsValidFiles(const QMimeData *mime);
    void handleFileDrop(const QMimeData *mime);
    void addFileToQueue(const ppl7::String &filename);
    void updateAudioProfiles();


protected:
	void closeEvent(QCloseEvent *event);
    bool eventFilter(QObject *target, QEvent *event);
    void changeEvent(QEvent* event);

private slots:
	void on_setupButton_clicked();
	void on_startButton_clicked();
	void on_stopButton_clicked();
	void on_clearButton_clicked();
    void on_id3tagEditorButton_clicked();
    void on_audioProfileComboBox_currentIndexChanged(int index);
	void timerupdate();
};

#endif // MP3ENCODE_H
