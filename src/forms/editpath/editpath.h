/*
 * This file is part of MP3Encoder by Patrick Fedick
 *
 * $Author: patrick $
 * $Revision: 1.5 $
 * $Date: 2010/04/03 15:08:46 $
 * $Id: editpath.h,v 1.5 2010/04/03 15:08:46 patrick Exp $
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

#ifndef EDITPATH_H
#define EDITPATH_H

#include <QDialog>
#include "ui_editpath.h"
#include "mp3core.h"

class EditPath : public QDialog
{
    Q_OBJECT

public:
	int id;
    EditPath(QWidget *parent = 0);
    ~EditPath();
    void setGlobalConfig(const Config &conf);
    void setConfig(const Config::PathConfig &conf);
    Config::PathConfig getConfig() const;

private:
    Config  global_conf;
	bool	bConfigChanged;
    Ui::EditPathClass ui;
    void SwitchBitrateMode(const Config::Audio::EncoderMode b);
    bool sanityCheck();

private slots:
	void on_ButtonOK_clicked();
	void on_ButtonCancel_clicked();
	void on_SourcePathButton_clicked();
	void on_TargetPathButton_clicked();
	void on_BackupPathButton_clicked();

	void on_backupCheckBox_toggled(bool v);

	void on_radioButtonCBR_toggled(bool v);
	void on_radioButtonABR_toggled(bool v);
	void on_radioButtonVBR_toggled(bool v);
	void on_quality_valueChanged(int i);

	void changed();

};

#endif // EDITPATH_H
