/*
 * This file is part of MP3Encoder by Patrick Fedick
 *
 * $Author: patrick $
 * $Revision: 1.9 $
 * $Date: 2009/06/11 20:09:50 $
 * $Id: main.cpp,v 1.9 2009/06/11 20:09:50 patrick Exp $
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

#include "src/forms/mp3encode/mp3encode.h"
#include "mp3core.h"
#include <locale.h>
#include <QtGui>
#include <QApplication>
#include <QMessageBox>


int main(int argc, char *argv[])
{
	setlocale(LC_ALL, "");
	ppl7::String::setGlobalEncoding("UTF-8");
    QApplication a(argc, argv);
#if QT_VERSION < 0x050000
    QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));
    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
#endif
    CMP3Core Core;
    mp3encode w(&Core);
    w.InitWindow();
    w.show();
    a.connect(&a, SIGNAL(lastWindowClosed()), &a, SLOT(quit()));
    return a.exec();
}
