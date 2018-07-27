/*
 * This file is part of MP3Encoder by Patrick Fedick
 *
 * $Author: patrick $
 * $Revision: 1.7 $
 * $Date: 2010/04/03 15:08:46 $
 * $Id: editpath.cpp,v 1.7 2010/04/03 15:08:46 patrick Exp $
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

#include "editpath.h"
#include <QFileDialog>
#include <QMessageBox>

EditPath::EditPath(QWidget *parent)
    : QDialog(parent)
{
    id=0;
	ui.setupUi(this);
	QObject::connect(ui.frequency,SIGNAL(currentIndexChanged(int)),this,SLOT(changed()));
	QObject::connect(ui.channels,SIGNAL(currentIndexChanged(int)),this,SLOT(changed()));
	QObject::connect(ui.BitrateCBR,SIGNAL(currentIndexChanged(int)),this,SLOT(changed()));
	QObject::connect(ui.BitrateABR,SIGNAL(currentIndexChanged(int)),this,SLOT(changed()));
	QObject::connect(ui.minBitrate,SIGNAL(currentIndexChanged(int)),this,SLOT(changed()));
	QObject::connect(ui.maxBitrate,SIGNAL(currentIndexChanged(int)),this,SLOT(changed()));
	QObject::connect(ui.individualAudio,SIGNAL(toggled(bool)),this,SLOT(changed()));
	QObject::connect(ui.SourcePath,SIGNAL(textChanged(const QString &)),this,SLOT(changed()));
	QObject::connect(ui.TargetPath,SIGNAL(textChanged(const QString &)),this,SLOT(changed()));
	QObject::connect(ui.activeCheckBox,SIGNAL(toggled(bool)),this,SLOT(changed()));
	bConfigChanged=false;
}

EditPath::~EditPath()
{

}

void EditPath::changed()
{
	bConfigChanged=true;
	ui.ButtonOK->setEnabled(true);
}

void EditPath::SwitchBitrateMode(const Config::Audio::EncoderMode b)
{
    switch (b) {
        case Config::Audio::EncoderMode::CBR:
            //ui.radioButtonCBR->setChecked(true);
            ui.BitrateCBR->setEnabled(true);
            ui.labelCBRKbit->setEnabled(true);
            ui.BitrateABR->setEnabled(false);
            ui.labelABRKbit->setEnabled(false);
            ui.minBitrate->setEnabled(false);
            ui.labelVBRminimum1->setEnabled(false);
            ui.labelVBRminimum2->setEnabled(false);
            ui.labelVBRmaximum1->setEnabled(false);
            ui.labelVBRmaximum2->setEnabled(false);
            ui.maxBitrate->setEnabled(false);
            break;
        case Config::Audio::EncoderMode::ABR:
            //ui.radioButtonABR->setChecked(true);
            ui.BitrateCBR->setEnabled(false);
            ui.labelCBRKbit->setEnabled(false);
            ui.BitrateABR->setEnabled(true);
            ui.labelABRKbit->setEnabled(true);
            ui.minBitrate->setEnabled(false);
            ui.maxBitrate->setEnabled(false);
            ui.labelVBRminimum1->setEnabled(false);
            ui.labelVBRminimum2->setEnabled(false);
            ui.labelVBRmaximum1->setEnabled(false);
            ui.labelVBRmaximum2->setEnabled(false);
            break;
        case Config::Audio::EncoderMode::VBR:
        default:
            //ui.radioButtonVBR->setChecked(true);
            ui.BitrateCBR->setEnabled(false);
            ui.labelCBRKbit->setEnabled(false);
            ui.BitrateABR->setEnabled(false);
            ui.labelABRKbit->setEnabled(false);
            ui.minBitrate->setEnabled(true);
            ui.maxBitrate->setEnabled(true);
            ui.labelVBRminimum1->setEnabled(true);
            ui.labelVBRminimum2->setEnabled(true);
            ui.labelVBRmaximum1->setEnabled(true);
            ui.labelVBRmaximum2->setEnabled(true);
            break;
    }
}

void EditPath::setGlobalConfig(const Config &conf)
{
    global_conf=conf;
}

void EditPath::setConfig(const Config::PathConfig &conf)
{
    int a;
    id=1;
    ppl7::String s;
    ui.SourcePath->setText(conf.Source);
    ui.TargetPath->setText(conf.Target);
    ui.BackupPath->setText(conf.Backup);

    ui.individualAudio->setChecked(conf.audio_override);
    ui.activeCheckBox->setChecked(conf.active);
    if (conf.enable_backup) {
		ui.BackupPath->setEnabled(true);
		ui.BackupPathButton->setEnabled(true);
		ui.backupCheckBox->setChecked(true);
	} else {
		ui.BackupPath->setEnabled(false);
		ui.BackupPathButton->setEnabled(false);
		ui.backupCheckBox->setChecked(false);
	}
	// Frequency
    switch(conf.audio.frequency) {
		case 32000:
			a=0;
			break;
		case 48000:
			a=2;
			break;
		default:
			a=1;
	}
	ui.frequency->setCurrentIndex(a);

	// Channels
    switch (conf.audio.channel_mode) {
        case ppl7::AudioInfo::ChannelMode::MONO:
            a=0; break;
        case ppl7::AudioInfo::ChannelMode::DUAL_CHANNEL:
            a=1; break;
        case ppl7::AudioInfo::ChannelMode::STEREO:
            a=2; break;
        case ppl7::AudioInfo::ChannelMode::JOINT_STEREO:
        default:
            a=3; break;
    }
	ui.channels->setCurrentIndex(a);

	// Quality
    ui.quality->setValue(conf.audio.quality);
    on_quality_valueChanged(conf.audio.quality);

	// Bitrate
    SetBitrate(ui.BitrateCBR,conf.audio.cbr_bitrate);
    SetBitrate(ui.BitrateABR,conf.audio.abr_bitrate);
    SetBitrate(ui.minBitrate,conf.audio.vbr_min_bitrate);
    SetBitrate(ui.maxBitrate,conf.audio.vbr_max_bitrate);

    // Mode
    switch (conf.audio.mode) {
        case Config::Audio::EncoderMode::CBR:
            ui.radioButtonCBR->setChecked(true);
            break;
        case Config::Audio::EncoderMode::ABR:
            ui.radioButtonABR->setChecked(true);
            break;
        case Config::Audio::EncoderMode::VBR:
        default:
            ui.radioButtonVBR->setChecked(true);
            break;

    }
	ui.ButtonOK->setEnabled(false);
}


void EditPath::on_radioButtonCBR_toggled(bool v)
{
    if (v) SwitchBitrateMode(Config::Audio::EncoderMode::CBR);
	changed();
}

void EditPath::on_radioButtonABR_toggled(bool v)
{
    if (v) SwitchBitrateMode(Config::Audio::EncoderMode::ABR);
	changed();
}

void EditPath::on_radioButtonVBR_toggled(bool v)
{
    if (v) SwitchBitrateMode(Config::Audio::EncoderMode::VBR);
	changed();
}

void EditPath::on_quality_valueChanged(int i)
{
	changed();
	ui.labelQuality->setText(GetQualityLabel(i));
}

bool EditPath::sanityCheck()
{
    Config::PathConfig conf=getConfig();
    // Source muss vorhanden sein
    if (conf.Source.size()==0) {
        QMessageBox::critical(NULL,tr("mp3encode"),
            tr("Source directory is missing"),
            QMessageBox::Ok | QMessageBox::Default);
        return false;
    }
    if (!ppl7::File::statFile(conf.Source).isDir()) {
        QMessageBox::critical(this,tr("mp3encode"),
            tr("Source directory is invalid or does not exist"),
            QMessageBox::Ok | QMessageBox::Default);
        return false;
    }

    // Target muss vorhanden sein
    if (conf.Target.size()==0) {
        QMessageBox::critical(NULL,tr("mp3encode"),
            tr("Target directory is missing"),
            QMessageBox::Ok | QMessageBox::Default);
        return false;
    }
    if (!ppl7::File::statFile(conf.Target).isDir()) {
        QMessageBox::critical(this,tr("mp3encode"),
            tr("Target directory is invalid or does not exist"),
            QMessageBox::Ok | QMessageBox::Default);
        return false;
    }

    if (conf.enable_backup) {
        // Backup-Pfad muss vorhanden sein
        if (conf.Backup.size()==0) {
            QMessageBox::critical(NULL,tr("mp3encode"),
                tr("Backup directory is missing"),
                QMessageBox::Ok | QMessageBox::Default);
            return false;
        }
        if (!ppl7::File::statFile(conf.Backup).isDir()) {
            QMessageBox::critical(this,tr("mp3encode"),
                tr("Backup directory is invalid or does not exist"),
                QMessageBox::Ok | QMessageBox::Default);
            return false;
        }

    }

    // Source-Pfade müssen einmalig sein
    std::list<Config::PathConfig>::const_iterator it;
    int dupecount=0;
    for (it=global_conf.pathconfig.begin();it!=global_conf.pathconfig.end();++it) {
        if ((*it).Source==conf.Source) dupecount++;
    }
    if ((id==0 && dupecount>0) || dupecount>1) {		// Neuer Eintrag
        QMessageBox::critical(this,tr("mp3encode"),
            tr("Source directory already exists"),
            QMessageBox::Ok | QMessageBox::Default);
        return false;
    }
    return true;
}

Config::PathConfig EditPath::getConfig() const
{
    Config::PathConfig conf;
    conf.Source=ui.SourcePath->text().trimmed();
    conf.Target=ui.TargetPath->text().trimmed();
    conf.Backup=ui.BackupPath->text().trimmed();
    conf.enable_backup=ui.backupCheckBox->isChecked();
    conf.audio_override=ui.individualAudio->isChecked();
    conf.active=ui.activeCheckBox->isChecked();

    // Frequency
    switch(ui.frequency->currentIndex()) {
        case 0:
            conf.audio.frequency=32000;
            break;
        case 2:
            conf.audio.frequency=48000;
            break;
        default:
            conf.audio.frequency=41000;
            break;
    }

    // Channels
    switch(ui.channels->currentIndex()) {
        case 0:
            conf.audio.channel_mode=ppl7::AudioInfo::ChannelMode::MONO;
            break;
        case 1:
            conf.audio.channel_mode=ppl7::AudioInfo::ChannelMode::DUAL_CHANNEL;
            break;
        case 3:
            conf.audio.channel_mode=ppl7::AudioInfo::ChannelMode::JOINT_STEREO;
            break;
        case 2:
        default:
            conf.audio.channel_mode=ppl7::AudioInfo::ChannelMode::STEREO;
            break;
    }

    // Quality
    conf.audio.quality=ui.quality->value();

    // Mode
    if(ui.radioButtonCBR->isChecked()) conf.audio.mode=Config::Audio::EncoderMode::CBR;
    if(ui.radioButtonABR->isChecked()) conf.audio.mode=Config::Audio::EncoderMode::ABR;
    if(ui.radioButtonVBR->isChecked()) conf.audio.mode=Config::Audio::EncoderMode::VBR;
    conf.audio.cbr_bitrate=GetBitrate(ui.BitrateCBR);
    conf.audio.abr_bitrate=GetBitrate(ui.BitrateABR);
    conf.audio.vbr_min_bitrate=GetBitrate(ui.minBitrate);
    conf.audio.vbr_max_bitrate=GetBitrate(ui.maxBitrate);
    return conf;
}


/*
 * Buttons
 */

void EditPath::on_ButtonOK_clicked()
{
    if (sanityCheck()) done(1);
}

void EditPath::on_ButtonCancel_clicked()
{
    done(0);
}

void EditPath::on_SourcePathButton_clicked()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("choose source directory"),
		ui.SourcePath->text(),
		QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
	if(dir.length()) {
		ui.SourcePath->setText(dir);
		changed();
	}
}

void EditPath::on_TargetPathButton_clicked()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("choose target directory"),
		ui.TargetPath->text(),
		QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
	if(dir.length()) {
		ui.TargetPath->setText(dir);
		changed();
	}
}

void EditPath::on_BackupPathButton_clicked()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("choose backup directory"),
		ui.BackupPath->text(),
		QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
	if(dir.length()) {
		ui.BackupPath->setText(dir);
		changed();
	}
}


void EditPath::on_backupCheckBox_toggled(bool )
{
	if (ui.backupCheckBox->isChecked()) {
		ui.BackupPath->setEnabled(true);
		ui.BackupPathButton->setEnabled(true);
	} else {
		ui.BackupPath->setEnabled(false);
		ui.BackupPathButton->setEnabled(false);
	}
}
