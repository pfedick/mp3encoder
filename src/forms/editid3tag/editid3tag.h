#ifndef EDITID3TAG_H
#define EDITID3TAG_H

#include <QWidget>
#include <QEvent>
#include <QDropEvent>
#include <QDragEnterEvent>
#include <QMimeData>

#define WITH_QT
#define PPL_QT_STRING_UTF8
#include <ppl7.h>
#include <ppl7-audio.h>


namespace Ui {
class EditId3Tag;
}

class EditId3Tag : public QWidget
{
    Q_OBJECT

public:
    explicit EditId3Tag(QWidget *parent = 0);
    ~EditId3Tag();

private:
    Ui::EditId3Tag *ui;
    QPixmap Cover;

    bool eventFilter(QObject *target, QEvent *event);
    bool handleDragEnterEvent(QDragEnterEvent *event);
    bool handleDropEvent(QDropEvent *event);
    void fillFormFromTag(const ppl7::ID3Tag &Tag);
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
