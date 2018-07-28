/*
 * This file is part of MP3Encoder by Patrick Fedick
 *
 * $Author: patrick $
 * $Revision: 1.7 $
 * $Date: 2010/05/02 08:40:17 $
 * $Id: setup.h,v 1.7 2010/05/02 08:40:17 patrick Exp $
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

#ifndef SETUP_H
#define SETUP_H

#include <QDialog>
#include <QTreeWidgetItem>
#include "ui_setup.h"
#include "mp3core.h"

class PathTreeWidgetItem : public QTreeWidgetItem
{
public:
    Config::PathConfig conf;
    void render();
};

class AudioProfileTreeWidgetItem : public QTreeWidgetItem
{
public:
    ppl7::String name;
    Config::Audio audio;
    void render();
};

class Setup : public QDialog
{
    Q_OBJECT

private:
    Ui::SetupClass ui;
    CMP3Core *core;
	bool	bConfigChanged;

    void InitPathList(const std::list<Config::PathConfig> &pathlist);
    void SwitchBitrateMode(const Config::Audio::EncoderMode b);
    void changeEvent(QEvent* event);

public:
    Setup(QWidget *parent = 0, CMP3Core *core=0);
    ~Setup();

    void setConfig(const Config &conf);
    Config getConfig() const;

    //void Init();
    //int Save();


private slots:
	void on_ButtonOK_clicked();
	void on_ButtonUse_clicked();
	void on_ButtonCancel_clicked();
	void on_ButtonAdd_clicked();
	void on_ButtonEdit_clicked();
	void on_ButtonDelete_clicked();

    void on_profileAddButton_clicked();
    void on_profileEditButton_clicked();
    void on_profileDeleteButton_clicked();

	void on_numCPUs_valueChanged(int i);

	void on_radioButtonCBR_toggled(bool v);
	void on_radioButtonABR_toggled(bool v);
	void on_radioButtonVBR_toggled(bool v);
	void on_quality_valueChanged(int i);
	void on_PathList_itemClicked(QTreeWidgetItem * item, int column);
	void on_PathList_itemDoubleClicked(QTreeWidgetItem * item, int column);

    void on_profilesTreeWidget_itemClicked(QTreeWidgetItem * item, int column);
    void on_profilesTreeWidget_itemDoubleClicked(QTreeWidgetItem * item, int column);



	void changed();
};

#endif // SETUP_H
