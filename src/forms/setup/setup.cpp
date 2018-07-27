/*
 * This file is part of MP3Encoder by Patrick Fedick
 *
 * $Author: patrick $
 * $Revision: 1.12 $
 * $Date: 2010/05/02 08:40:17 $
 * $Id: setup.cpp,v 1.12 2010/05/02 08:40:17 patrick Exp $
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

#include "setup.h"
#include "src/forms/editpath/editpath.h"
#include "src/forms/editaudioprofile/editaudioprofile.h"
#include <QMessageBox>
#include <QTreeWidgetItem>

void PathTreeWidgetItem::render()
{
    setText(0,conf.Source);
    setText(1,conf.Target);
    if (conf.active) setText(2,QObject::tr("aktiv"));
    else setText(2,QObject::tr("inaktiv"));
    ppl7::String s;
    if (conf.audio_override) {
        switch (conf.audio.mode) {
            case Config::Audio::EncoderMode::CBR:
                s.setf("CBR %d, ",conf.audio.cbr_bitrate);
                break;
            case Config::Audio::EncoderMode::ABR:
                s.setf("ABR %d, ",conf.audio.abr_bitrate);
                break;
            case Config::Audio::EncoderMode::VBR:
                s.setf("VBR %d-%d, ",conf.audio.vbr_min_bitrate,
                       conf.audio.vbr_max_bitrate);
            default:
                break;
        }
        s.appendf("q%d, ",conf.audio.quality);
        switch (conf.audio.channel_mode) {
            case ppl7::AudioInfo::ChannelMode::STEREO:
            case ppl7::AudioInfo::ChannelMode::DUAL_CHANNEL:
            case ppl7::AudioInfo::ChannelMode::JOINT_STEREO:
                s.append("2");
                break;
            default:
                s.append("1");
                break;
        }
    } else {
        s.set("default");
    }
    setText(3,s);
}

void AudioProfileTreeWidgetItem::render()
{
    setText(0,name);
    setText(1,TranslateEncoding(audio));
}


Setup::Setup(QWidget *parent, CMP3Core *core)
    : QDialog(parent)
{
	ui.setupUi(this);
    this->core=core;
	QObject::connect(ui.EncoderDelay,SIGNAL(valueChanged(int)),this,SLOT(changed()));
	QObject::connect(ui.frequency,SIGNAL(currentIndexChanged(int)),this,SLOT(changed()));
	QObject::connect(ui.channels,SIGNAL(currentIndexChanged(int)),this,SLOT(changed()));
	QObject::connect(ui.BitrateCBR,SIGNAL(currentIndexChanged(int)),this,SLOT(changed()));
	QObject::connect(ui.BitrateABR,SIGNAL(currentIndexChanged(int)),this,SLOT(changed()));
	QObject::connect(ui.minBitrate,SIGNAL(currentIndexChanged(int)),this,SLOT(changed()));
	QObject::connect(ui.maxBitrate,SIGNAL(currentIndexChanged(int)),this,SLOT(changed()));
	QObject::connect(ui.checkBoxAutoStart,SIGNAL(toggled(bool)),this,SLOT(changed()));
	QObject::connect(ui.checkBoxKeepTags,SIGNAL(toggled(bool)),this,SLOT(changed()));
	QObject::connect(ui.checkBoxRecode,SIGNAL(toggled(bool)),this,SLOT(changed()));
	QObject::connect(ui.Priority,SIGNAL(currentIndexChanged(int)),this,SLOT(changed()));
	ui.SetupTab->setCurrentIndex(0);
	bConfigChanged=false;
}

Setup::~Setup()
{

}

void Setup::changed()
{
	ui.ButtonUse->setEnabled(true);
	bConfigChanged=true;
}

void SetBitrate(QComboBox *q, int bitrate)
{
	int a;
	switch (bitrate) {
		case 32:
			a=0;
			break;
		case 40:
			a=1;
			break;
		case 48:
			a=2;
			break;
		case 64:
			a=3;
			break;
		case 80:
			a=4;
			break;
		case 96:
			a=5;
			break;
		case 112:
			a=6;
			break;
		case 128:
			a=7;
			break;
		case 160:
			a=8;
			break;
		case 192:
			a=9;
			break;
		case 224:
			a=10;
			break;
		case 256:
			a=11;
			break;
		case 320:
			a=12;
			break;
		default:
			a=9;
	}
	q->setCurrentIndex(a);
}

int GetBitrate(QComboBox *q)
{
	switch(q->currentIndex()) {
		case 0: return 32;
		case 1: return 40;
		case 2: return 48;
		case 3: return 64;
		case 4: return 80;
		case 5: return 96;
		case 6: return 112;
		case 7: return 128;
		case 8: return 160;
		case 9: return 192;
		case 10: return 224;
		case 11: return 256;
		case 12: return 320;
	}
	return 192;
}

void Setup::SwitchBitrateMode(const Config::Audio::EncoderMode b)
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

void Setup::setConfig(const Config &conf)
{
    int a;
    ppl7::String s;
	ui.numCPUs->setValue(conf.numCPUs);
	ui.EncoderDelay->setValue(conf.EncoderDelay);
	ui.checkBoxAutoStart->setChecked(conf.autostart);
	ui.checkBoxKeepTags->setChecked(conf.keepID3);
	ui.checkBoxRecode->setChecked(conf.recode);

	// Priority
    switch (conf.priority) {
        case ppl7::Thread::Priority::LOWEST:
        case ppl7::Thread::Priority::BELOW_NORMAL:
            a=0;
            break;
        case ppl7::Thread::Priority::HIGHEST:
        case ppl7::Thread::Priority::ABOVE_NORMAL:
            a=2;
            break;
        default:
            a=1;
            break;
    }
	ui.Priority->setCurrentIndex(a);

	// Frequency
    switch(conf.audio_default.frequency) {
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
    switch (conf.audio_default.channel_mode) {
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
    ui.quality->setValue(conf.audio_default.quality);
    on_quality_valueChanged(conf.audio_default.quality);

	// Bitrate
    SetBitrate(ui.BitrateCBR,conf.audio_default.cbr_bitrate);
    SetBitrate(ui.BitrateABR,conf.audio_default.abr_bitrate);
    SetBitrate(ui.minBitrate,conf.audio_default.vbr_min_bitrate);
    SetBitrate(ui.maxBitrate,conf.audio_default.vbr_max_bitrate);

    // Mode
    switch (conf.audio_default.mode) {
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
    InitPathList(conf.pathconfig);
	ui.ButtonUse->setEnabled(false);

    ui.profilesTreeWidget->clear();
    std::map<ppl7::String,Config::Audio>::const_iterator it;
    for (it=conf.audioprofiles.begin();it!=conf.audioprofiles.end();++it) {
        AudioProfileTreeWidgetItem *item=new AudioProfileTreeWidgetItem();
        item->name=(*it).first;
        item->audio=(*it).second;
        item->render();
        ui.profilesTreeWidget->addTopLevelItem(item);
    }
    ui.profilesTreeWidget->resizeColumnToContents(0);
    ui.profilesTreeWidget->resizeColumnToContents(1);
}

void Setup::InitPathList(const std::list<Config::PathConfig> &pathlist)
{
    ui.PathList->clear();
    std::list<Config::PathConfig>::const_iterator it;
    for (it=pathlist.begin();it!=pathlist.end();++it) {
        PathTreeWidgetItem *item=new PathTreeWidgetItem();
        item->conf=(*it);
        item->render();
        ui.PathList->addTopLevelItem(item);
    }
	ui.PathList->resizeColumnToContents(0);
	ui.PathList->resizeColumnToContents(1);
	ui.PathList->resizeColumnToContents(2);
	ui.PathList->resizeColumnToContents(3);
}

void Setup::on_numCPUs_valueChanged(int)
{
	changed();
}

void Setup::on_radioButtonCBR_toggled(bool v)
{
    if (v) SwitchBitrateMode(Config::Audio::EncoderMode::CBR);
	changed();
}

void Setup::on_radioButtonABR_toggled(bool v)
{
    if (v) SwitchBitrateMode(Config::Audio::EncoderMode::ABR);
	changed();
}

void Setup::on_radioButtonVBR_toggled(bool v)
{
    if (v) SwitchBitrateMode(Config::Audio::EncoderMode::VBR);
	changed();
}

QString GetQualityLabel(int i)
{
	QString q;
	switch (i) {
		case 0:
			q=QObject::tr("extrem hoch");
			break;
		case 1:
			q=QObject::tr("sehr hoch");
			break;
		case 2:
			q=QObject::tr("hoch (empfohlen)");
			break;
		case 3:
		case 4:
			q=QObject::tr("gut");
			break;
		case 5:
			q=QObject::tr("gut (default)");
			break;
		case 6:
		case 7:
			q=QObject::tr("durchschnitt");
			break;
		case 8:
			q=QObject::tr("schlecht");
			break;
		case 9:
			q=QObject::tr("sehr schlecht");
			break;
		default:
			q=QObject::tr("normal");
			break;
	}
	return q;
}

void Setup::on_quality_valueChanged(int i)
{
	changed();
	ui.labelQuality->setText(GetQualityLabel(i));
}

Config Setup::getConfig() const
{
	Config conf;
	conf.numCPUs=ui.numCPUs->value();
	conf.EncoderDelay=ui.EncoderDelay->value();
	conf.autostart=ui.checkBoxAutoStart->isChecked();
	conf.keepID3=ui.checkBoxKeepTags->isChecked();
	conf.recode=ui.checkBoxRecode->isChecked();

	// Priority
	switch(ui.Priority->currentIndex()) {
		case 0:
			conf.priority=ppl7::Thread::Priority::LOWEST;
			break;
		case 2:
            conf.priority=ppl7::Thread::Priority::HIGHEST;
			break;
		default:
            conf.priority=ppl7::Thread::Priority::NORMAL;
	}

	// Frequency
	switch(ui.frequency->currentIndex()) {
		case 0:
			conf.audio_default.frequency=32000;
			break;
		case 2:
			conf.audio_default.frequency=48000;
			break;
		default:
			conf.audio_default.frequency=44100;
			break;
	}
	// Channels
	switch(ui.channels->currentIndex()) {
		case 0:
			conf.audio_default.channel_mode=ppl7::AudioInfo::ChannelMode::MONO;
			break;
		case 1:
			conf.audio_default.channel_mode=ppl7::AudioInfo::ChannelMode::DUAL_CHANNEL;
			break;
		case 3:
			conf.audio_default.channel_mode=ppl7::AudioInfo::ChannelMode::JOINT_STEREO;
			break;
		case 2:
		default:
			conf.audio_default.channel_mode=ppl7::AudioInfo::ChannelMode::STEREO;
			break;
	}
	// Quality
    conf.audio_default.quality=ui.quality->value();
	// Mode
	if(ui.radioButtonCBR->isChecked()) conf.audio_default.mode=Config::Audio::EncoderMode::CBR;
	if(ui.radioButtonABR->isChecked()) conf.audio_default.mode=Config::Audio::EncoderMode::ABR;
	if(ui.radioButtonVBR->isChecked()) conf.audio_default.mode=Config::Audio::EncoderMode::VBR;
	conf.audio_default.cbr_bitrate=GetBitrate(ui.BitrateCBR);
	conf.audio_default.abr_bitrate=GetBitrate(ui.BitrateABR);
	conf.audio_default.vbr_min_bitrate=GetBitrate(ui.minBitrate);
	conf.audio_default.vbr_max_bitrate=GetBitrate(ui.maxBitrate);

    // Pathconfigs hinzufügen
    int count=ui.PathList->topLevelItemCount();
    for (int i=0;i<count;i++) {
        PathTreeWidgetItem *item=(PathTreeWidgetItem*)ui.PathList->topLevelItem(i);
        conf.pathconfig.push_back(item->conf);
    }

    // Audio Profile
    count=ui.profilesTreeWidget->topLevelItemCount();
    for (int i=0;i<count;i++) {
        AudioProfileTreeWidgetItem *item=(AudioProfileTreeWidgetItem*)ui.profilesTreeWidget->topLevelItem(i);
        conf.audioprofiles.insert(std::pair<ppl7::String, Config::Audio>(item->name, item->audio));
    }
	return conf;
}


/*
 * Buttons
 */

void Setup::on_ButtonOK_clicked()
{
    done(1);
}

void Setup::on_ButtonUse_clicked()
{
    // TODO
    //if (Save()) ui.ButtonUse->setEnabled(false);
}

void Setup::on_ButtonCancel_clicked()
{
    done(0);
}

void Setup::on_ButtonAdd_clicked()
{
    EditPath edit;
    edit.setGlobalConfig(getConfig());
    if (edit.exec()==1) {
        PathTreeWidgetItem *item=new PathTreeWidgetItem();
        item->conf=edit.getConfig();
        item->render();
        ui.PathList->addTopLevelItem(item);
    }
}

void Setup::on_ButtonEdit_clicked()
{
    PathTreeWidgetItem *item=(PathTreeWidgetItem*)ui.PathList->currentItem();
    if (!item) return;
    EditPath edit;
    edit.setGlobalConfig(getConfig());
    edit.setConfig(item->conf);
    if (edit.exec()==1) {
        item->conf=edit.getConfig();
        item->render();
    }
}

void Setup::on_ButtonDelete_clicked()
{
    QTreeWidgetItem *item=ui.PathList->currentItem();
    if (!item) return;
    int index=ui.PathList->indexOfTopLevelItem(item);
    ui.PathList->takeTopLevelItem(index);
}

void Setup::on_PathList_itemClicked(QTreeWidgetItem *, int)
{
	ui.ButtonEdit->setEnabled(true);
	ui.ButtonDelete->setEnabled(true);
}

void Setup::on_PathList_itemDoubleClicked(QTreeWidgetItem * , int )
{
	on_ButtonEdit_clicked();
}


void Setup::on_profileAddButton_clicked()
{
    EditAudioProfile edit;
    edit.setConfig("new profile", getConfig().audio_default);
    if (edit.exec()==1) {
        AudioProfileTreeWidgetItem *item=new AudioProfileTreeWidgetItem();
        item->name=edit.getName();
        item->audio=edit.getConfig();
        item->render();
        ui.profilesTreeWidget->addTopLevelItem(item);
    }
}

void Setup::on_profileEditButton_clicked()
{
    AudioProfileTreeWidgetItem *item=(AudioProfileTreeWidgetItem*)ui.profilesTreeWidget->currentItem();
    if (!item) return;

    EditAudioProfile edit;
    edit.setConfig(item->name, item->audio);
    if (edit.exec()==1) {
        item->name=edit.getName();
        item->audio=edit.getConfig();
        item->render();
    }
}

void Setup::on_profileDeleteButton_clicked()
{
    QTreeWidgetItem *item=ui.profilesTreeWidget->currentItem();
    if (!item) return;
    int index=ui.profilesTreeWidget->indexOfTopLevelItem(item);
    ui.profilesTreeWidget->takeTopLevelItem(index);
}

void Setup::on_profilesTreeWidget_itemClicked(QTreeWidgetItem *, int)
{
    ui.profileEditButton->setEnabled(true);
    ui.profileDeleteButton->setEnabled(true);
}

void Setup::on_profilesTreeWidget_itemDoubleClicked(QTreeWidgetItem *, int)
{
    on_profileEditButton_clicked();
}




