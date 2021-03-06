#ifndef _NOTEPAD_H
#define _NOTEPAD_H

#include <QtGui>
#include "../../../src/filelistwidget.h"

class Mainwin: public QMainWindow
{
    Q_OBJECT

    public:
        Mainwin();

    private slots:
        void open();
        void save();
        void exit();

    private:
        QAction *openAction;
        QAction *saveAction;
        QAction *exitAction;
        QMenu *fileMenu;

        FileListWidget *pFileList;
};

#endif

