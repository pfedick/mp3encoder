#include "editid3tag.h"
#include "ui_editid3tag.h"
#define WITH_QT
#define PPL_WITH_QT6
#define PPL_QT_STRING_UTF8
#include <ppl7.h>
#include <ppl7-audio.h>
#include <QPixmap>
#include <QBuffer>
#include <QClipboard>
#include <QMessageBox>

// global clipboard
ppl7::ID3Tag Clipboard;



EditId3Tag::EditId3Tag(QWidget* parent) :
    QWidget(parent),
    ui(new Ui::EditId3Tag)
{
    ui->setupUi(this);
    Clipboard.setLocalCharset("UTF-8");
    ui->filename->installEventFilter(this);
    ui->tagsGroupBox->installEventFilter(this);
    ui->insertButton->setEnabled(true);

}

EditId3Tag::~EditId3Tag()
{
    delete ui;
}

void EditId3Tag::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);
    }
}

bool EditId3Tag::eventFilter(QObject* target, QEvent* event)
{
    int type=event->type();
    if (target == ui->filename || target == ui->tagsGroupBox) {
        if (type == QEvent::Drop) {
            handleDropEvent(static_cast<QDropEvent*>(event));
            return true;
        } else if (type == QEvent::DragEnter) {
            if (handleDragEnterEvent(static_cast<QDragEnterEvent*>(event))) {
                return true;
            }
        }
    }
    return QWidget::eventFilter(target, event);
}

static bool isDropWithAudioFile(const QMimeData* mime)
{
    if (!mime->hasUrls()) return false;
    QList<QUrl>	list=mime->urls();
    if (list.count() != 1) return false;
    QUrl url=list.first();
    /*
#if QT_VERSION >= 0x050000
    QString file=url.path(QUrl::FullyDecoded);
#else
    QString file=url.encodedPath();
#endif
    */
    return true;
}

bool EditId3Tag::handleDragEnterEvent(QDragEnterEvent* event)
{
    const QMimeData* mime=event->mimeData();
    if (!mime) {
        //event->ignore();
        return false;
    }
    if (isDropWithAudioFile(mime)) {
        event->accept();
        return true;
    }
    //event->ignore();
    return false;
}

bool EditId3Tag::handleDropEvent(QDropEvent* event)
{
    const QMimeData* mime=event->mimeData();
    if (!mime) return false;
    if (!mime->hasUrls()) return false;
    event->accept();
    QList<QUrl>	list=mime->urls();
    QUrl url=list.first();
    ppl7::String file=url.toLocalFile();
    ui->filename->setText(file);
    QApplication::setOverrideCursor(Qt::WaitCursor);
    ppl7::ID3Tag Tag;
    Tag.setLocalCharset("UTF-8");
    //if (ppl7::File::exists(file)) {
    try {
        Tag.load(file);
        //Tag.listFrames(false);
        //fillFormFromTag(Tag);
    } catch (const ppl7::Exception& exp) {
        QMessageBox::critical(NULL, "Error", exp.toString());
    }
    fillFormFromTag(Tag);
//}
    QApplication::restoreOverrideCursor();
    return true;
}

void EditId3Tag::fillFormFromTag(const ppl7::ID3Tag& Tag)
{
    Cover=QPixmap();
    ui->coverpic->setPixmap(Cover);
    ui->interpret->setText(Tag.getArtist());
    ui->titel->setText(Tag.getTitle());
    ui->comment->setText(Tag.getComment());
    ui->genre->setText(Tag.getGenre());
    ui->year->setValue(Tag.getYear().toInt());
    ui->album->setText(Tag.getAlbum());
    ui->recordlabel->setText(Tag.getLabel());
    ppl7::ByteArray cover;
    if (Tag.getPicture(3, cover)) {
        Cover.loadFromData((const uchar*)cover.adr(), cover.size());
        ui->coverpic->setPixmap(Cover.scaled(200, 200, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
}


void EditId3Tag::on_copyButton_clicked()
{
    ppl7::String file=ui->filename->text();
    //if (ppl7::File::exists(file)) {
    try {
        Clipboard.load(file);
        ui->insertButton->setEnabled(true);
    } catch (const ppl7::Exception& exp) {
        QMessageBox::critical(NULL, "Error", exp.toString());


    }
    //}
}

void EditId3Tag::on_insertButton_clicked()
{
    fillFormFromTag(Clipboard);
}

void EditId3Tag::on_reloadButton_clicked()
{
    ppl7::String file=ui->filename->text();
    //if (!ppl7::File::exists(file)) return;
    QApplication::setOverrideCursor(Qt::WaitCursor);
    ppl7::ID3Tag Tag;
    try {
        Tag.setLocalCharset("UTF-8");
        Tag.load(file);
        fillFormFromTag(Tag);
    } catch (const ppl7::Exception& exp) {
        QMessageBox::critical(NULL, "Error", exp.toString());
    }
    QApplication::restoreOverrideCursor();
}

void EditId3Tag::on_saveButton_clicked()
{
    ppl7::String file=ui->filename->text();
    //if (!ppl7::File::exists(file)) return;
    QApplication::setOverrideCursor(Qt::WaitCursor);
    try {
        ppl7::ID3Tag Tag;
        Tag.setLocalCharset("UTF-8");
        Tag.load(file);
        if (ui->deleteOtherTagsCheckBox->isChecked()) Tag.clearTags();
        Tag.setArtist(ui->interpret->text().trimmed());
        Tag.setTitle(ui->titel->text().trimmed());
        Tag.setGenre(ui->genre->text().trimmed());
        Tag.setYear(ppl7::ToString("%d", ui->year->value()));
        Tag.setAlbum(ui->album->text().trimmed());
        Tag.setLabel(ui->recordlabel->text().trimmed());
        Tag.setComment(ui->comment->text().trimmed());

        QByteArray bytes;
        QBuffer buffer(&bytes);
        buffer.open(QIODevice::WriteOnly);
        Cover.save(&buffer, "JPEG", 95);
        ppl7::ByteArray bin;
        bin.copy(bytes.data(), bytes.size());
        Tag.setPicture(3, bin, "image/jpeg");
        Tag.save();
    } catch (const ppl7::Exception& exp) {
        QMessageBox::critical(NULL, "Error", exp.toString());
    }
    QApplication::restoreOverrideCursor();
}

void EditId3Tag::on_delCoverButton_clicked()
{
    Cover=QPixmap();
    ui->coverpic->setPixmap(Cover);
}

void EditId3Tag::on_coverFromClipboardButton_clicked()
{
    QClipboard* clipboard = QApplication::clipboard();
    if (!clipboard) return;
    if (clipboard->pixmap().isNull()) return;
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    Cover=clipboard->pixmap();
    ui->coverpic->setPixmap(Cover.scaled(200, 200, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    QApplication::restoreOverrideCursor();
}

void EditId3Tag::on_deleteButton_clicked()
{
    ppl7::String file=ui->filename->text();
    //if (!ppl7::File::exists(file)) return;
    QApplication::setOverrideCursor(Qt::WaitCursor);
    try {
        ppl7::ID3Tag Tag;
        Tag.setLocalCharset("UTF-8");
        Tag.load(file);
        Tag.clearTags();
        Tag.save();
    } catch (const ppl7::Exception& exp) {
        QMessageBox::critical(NULL, "Error", exp.toString());
    }
    QApplication::restoreOverrideCursor();
}
