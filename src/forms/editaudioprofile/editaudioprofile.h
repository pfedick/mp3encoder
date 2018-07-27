#ifndef EDITAUDIOPROFILE_H
#define EDITAUDIOPROFILE_H

#include <QDialog>
#include "mp3core.h"

namespace Ui {
class EditAudioProfile;
}

class EditAudioProfile : public QDialog
{
    Q_OBJECT

public:
    explicit EditAudioProfile(QWidget *parent = 0);
    ~EditAudioProfile();

    void setConfig(const ppl7::String &name, const Config::Audio &audio);
    ppl7::String getName() const;
    Config::Audio getConfig() const;

private:
    Ui::EditAudioProfile *ui;
    bool	bConfigChanged;

    void SwitchBitrateMode(const Config::Audio::EncoderMode b);

private slots:
    void on_ButtonOK_clicked();
    void on_ButtonCancel_clicked();
    void on_radioButtonCBR_toggled(bool v);
    void on_radioButtonABR_toggled(bool v);
    void on_radioButtonVBR_toggled(bool v);
    void on_quality_valueChanged(int i);
    void on_name_textChanged() { changed();};
    void changed();

};

#endif // EDITAUDIOPROFILE_H
