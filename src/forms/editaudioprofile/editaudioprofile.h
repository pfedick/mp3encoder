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

#ifndef EDITAUDIOPROFILE_H
#define EDITAUDIOPROFILE_H

#include <QDialog>
#include "mp3core.h"

namespace Ui
{
class EditAudioProfile;
}

class EditAudioProfile : public QDialog
{
    Q_OBJECT

public:
    explicit EditAudioProfile(QWidget* parent = 0);
    ~EditAudioProfile();

    void setConfig(const pplib::String& name, const Config::Audio& audio);
    pplib::String getName() const;
    Config::Audio getConfig() const;

private:
    Ui::EditAudioProfile* ui;
    bool bConfigChanged;

    void SwitchBitrateMode(const Config::Audio::EncoderMode b);
    void changeEvent(QEvent* event);

private slots:
    void on_ButtonOK_clicked();
    void on_ButtonCancel_clicked();
    void on_radioButtonCBR_toggled(bool v);
    void on_radioButtonABR_toggled(bool v);
    void on_radioButtonVBR_toggled(bool v);
    void on_quality_valueChanged(int i);
    void on_name_textChanged()
    {
        changed();
    };
    void changed();
};

#endif // EDITAUDIOPROFILE_H
