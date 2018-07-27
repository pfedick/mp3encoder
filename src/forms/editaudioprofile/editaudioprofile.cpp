#include "editaudioprofile.h"
#include "ui_editaudioprofile.h"

EditAudioProfile::EditAudioProfile(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::EditAudioProfile)
{
    ui->setupUi(this);
    QObject::connect(ui->frequency,SIGNAL(currentIndexChanged(int)),this,SLOT(changed()));
    QObject::connect(ui->channels,SIGNAL(currentIndexChanged(int)),this,SLOT(changed()));
    QObject::connect(ui->BitrateCBR,SIGNAL(currentIndexChanged(int)),this,SLOT(changed()));
    QObject::connect(ui->BitrateABR,SIGNAL(currentIndexChanged(int)),this,SLOT(changed()));
    QObject::connect(ui->minBitrate,SIGNAL(currentIndexChanged(int)),this,SLOT(changed()));
    QObject::connect(ui->maxBitrate,SIGNAL(currentIndexChanged(int)),this,SLOT(changed()));
    bConfigChanged=false;
}

EditAudioProfile::~EditAudioProfile()
{
    delete ui;
}

void EditAudioProfile::setConfig(const ppl7::String &name, const Config::Audio &audio)
{
    ui->name->setText(name);
    // Frequency
    switch(audio.frequency) {
        case 32000:
            ui->frequency->setCurrentIndex(0);
            break;
        case 48000:
            ui->frequency->setCurrentIndex(2);
            break;
        default:
            ui->frequency->setCurrentIndex(1);
    }

    // Channels
    switch (audio.channel_mode) {
        case ppl7::AudioInfo::ChannelMode::MONO:
            ui->channels->setCurrentIndex(0); break;
        case ppl7::AudioInfo::ChannelMode::DUAL_CHANNEL:
            ui->channels->setCurrentIndex(1); break;
        case ppl7::AudioInfo::ChannelMode::STEREO:
            ui->channels->setCurrentIndex(2); break;
        case ppl7::AudioInfo::ChannelMode::JOINT_STEREO:
        default:
            ui->channels->setCurrentIndex(3); break;
    }

    // Quality
    ui->quality->setValue(audio.quality);
    on_quality_valueChanged(audio.quality);

    // Bitrate
    SetBitrate(ui->BitrateCBR,audio.cbr_bitrate);
    SetBitrate(ui->BitrateABR,audio.abr_bitrate);
    SetBitrate(ui->minBitrate,audio.vbr_min_bitrate);
    SetBitrate(ui->maxBitrate,audio.vbr_max_bitrate);

    // Mode
    switch (audio.mode) {
        case Config::Audio::EncoderMode::CBR:
            ui->radioButtonCBR->setChecked(true);
            break;
        case Config::Audio::EncoderMode::ABR:
            ui->radioButtonABR->setChecked(true);
            break;
        case Config::Audio::EncoderMode::VBR:
        default:
            ui->radioButtonVBR->setChecked(true);
            break;

    }
    ui->ButtonOK->setEnabled(false);
}

ppl7::String EditAudioProfile::getName() const
{
    return ui->name->text().trimmed();
}

Config::Audio EditAudioProfile::getConfig() const
{
    Config::Audio audio;
    // Frequency
    switch(ui->frequency->currentIndex()) {
        case 0:
            audio.frequency=32000;
            break;
        case 2:
            audio.frequency=48000;
            break;
        default:
            audio.frequency=41000;
            break;
    }

    // Channels
    switch(ui->channels->currentIndex()) {
        case 0:
            audio.channel_mode=ppl7::AudioInfo::ChannelMode::MONO;
            break;
        case 1:
            audio.channel_mode=ppl7::AudioInfo::ChannelMode::DUAL_CHANNEL;
            break;
        case 3:
            audio.channel_mode=ppl7::AudioInfo::ChannelMode::JOINT_STEREO;
            break;
        case 2:
        default:
            audio.channel_mode=ppl7::AudioInfo::ChannelMode::STEREO;
            break;
    }

    // Quality
    audio.quality=ui->quality->value();

    // Mode
    if(ui->radioButtonCBR->isChecked()) audio.mode=Config::Audio::EncoderMode::CBR;
    if(ui->radioButtonABR->isChecked()) audio.mode=Config::Audio::EncoderMode::ABR;
    if(ui->radioButtonVBR->isChecked()) audio.mode=Config::Audio::EncoderMode::VBR;
    audio.cbr_bitrate=GetBitrate(ui->BitrateCBR);
    audio.abr_bitrate=GetBitrate(ui->BitrateABR);
    audio.vbr_min_bitrate=GetBitrate(ui->minBitrate);
    audio.vbr_max_bitrate=GetBitrate(ui->maxBitrate);
    return audio;
}


void EditAudioProfile::SwitchBitrateMode(const Config::Audio::EncoderMode b)
{
    switch (b) {
        case Config::Audio::EncoderMode::CBR:
            //ui.radioButtonCBR->setChecked(true);
            ui->BitrateCBR->setEnabled(true);
            ui->labelCBRKbit->setEnabled(true);
            ui->BitrateABR->setEnabled(false);
            ui->labelABRKbit->setEnabled(false);
            ui->minBitrate->setEnabled(false);
            ui->labelVBRminimum1->setEnabled(false);
            ui->labelVBRminimum2->setEnabled(false);
            ui->labelVBRmaximum1->setEnabled(false);
            ui->labelVBRmaximum2->setEnabled(false);
            ui->maxBitrate->setEnabled(false);
            break;
        case Config::Audio::EncoderMode::ABR:
            //ui.radioButtonABR->setChecked(true);
            ui->BitrateCBR->setEnabled(false);
            ui->labelCBRKbit->setEnabled(false);
            ui->BitrateABR->setEnabled(true);
            ui->labelABRKbit->setEnabled(true);
            ui->minBitrate->setEnabled(false);
            ui->maxBitrate->setEnabled(false);
            ui->labelVBRminimum1->setEnabled(false);
            ui->labelVBRminimum2->setEnabled(false);
            ui->labelVBRmaximum1->setEnabled(false);
            ui->labelVBRmaximum2->setEnabled(false);
            break;
        case Config::Audio::EncoderMode::VBR:
        default:
            //ui.radioButtonVBR->setChecked(true);
            ui->BitrateCBR->setEnabled(false);
            ui->labelCBRKbit->setEnabled(false);
            ui->BitrateABR->setEnabled(false);
            ui->labelABRKbit->setEnabled(false);
            ui->minBitrate->setEnabled(true);
            ui->maxBitrate->setEnabled(true);
            ui->labelVBRminimum1->setEnabled(true);
            ui->labelVBRminimum2->setEnabled(true);
            ui->labelVBRmaximum1->setEnabled(true);
            ui->labelVBRmaximum2->setEnabled(true);
            break;
    }
}

void EditAudioProfile::changed()
{
    bConfigChanged=true;
    ui->ButtonOK->setEnabled(true);
}

void EditAudioProfile::on_ButtonOK_clicked()
{
    done(1);
}

void EditAudioProfile::on_ButtonCancel_clicked()
{
    done(0);
}

void EditAudioProfile::on_radioButtonCBR_toggled(bool v)
{
    if (v) SwitchBitrateMode(Config::Audio::EncoderMode::CBR);
    changed();
}

void EditAudioProfile::on_radioButtonABR_toggled(bool v)
{
    if (v) SwitchBitrateMode(Config::Audio::EncoderMode::ABR);
    changed();
}

void EditAudioProfile::on_radioButtonVBR_toggled(bool v)
{
    if (v) SwitchBitrateMode(Config::Audio::EncoderMode::VBR);
    changed();
}

void EditAudioProfile::on_quality_valueChanged(int i)
{
    changed();
    ui->labelQuality->setText(GetQualityLabel(i));
}
