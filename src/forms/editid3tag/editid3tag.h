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

#ifndef EDITID3TAG_H
#define EDITID3TAG_H

#include <QWidget>
#include <QEvent>
#include <QDropEvent>
#include <QDragEnterEvent>
#include <QMimeData>

// WITH_QT, PPL_WITH_QT6 und PPL_QT_STRING_UTF8 setzt das Build-System
// (CMakeLists.txt) projektweit.
#include <pplib/audio/id3tag.h>
#include <pplib/core/file.h>
#include <pplib/types/bytearray.h>
#include <pplib/types/string.h>

namespace Ui
{
class EditId3Tag;
}

class EditId3Tag : public QWidget
{
    Q_OBJECT

public:
    explicit EditId3Tag(QWidget* parent = 0);
    ~EditId3Tag();

private:
    Ui::EditId3Tag* ui;
    QPixmap Cover;

    bool eventFilter(QObject* target, QEvent* event);
    bool handleDragEnterEvent(QDragEnterEvent* event);
    bool handleDropEvent(QDropEvent* event);
    void fillFormFromTag(const pplib::ID3Tag& Tag);
    void changeEvent(QEvent* event);

private slots:
    void on_copyButton_clicked();
    void on_insertButton_clicked();
    void on_saveButton_clicked();
    void on_delCoverButton_clicked();
    void on_coverFromClipboardButton_clicked();
    void on_reloadButton_clicked();
    void on_deleteButton_clicked();
};

#endif // EDITID3TAG_H
