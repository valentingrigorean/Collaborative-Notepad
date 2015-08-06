#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>
#include <QLabel>
#include "highlight_c.h"
#include "client.h"
#include "connect_ui.h"
#include "download.h"
#include "find.h"
#include "replace.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void on_connection();
    void got_file();
    void lost_connection();
    void too_manyusers();

    void close_find();
    void close_replace();

private slots:
    //plain text
    void on_plainTextEdit_cursorPositionChanged();
    void showCopyPaste(bool b);
    void showUndo(bool b);
    void showRedo(bool b);

    //MenuItems
    void on_actionExit_triggered();
    void on_actionJoin_Server_triggered();
    void on_actionDisconnect_triggered();
    void on_actionSave_as_triggered();
    void on_actionDownload_triggered();
    void on_actionUndo_triggered();
    void on_actionRedo_triggered();
    void on_actionCut_triggered();
    void on_actionCopy_triggered();
    void on_actionPaste_triggered();

    void on_plainTextEdit_textChanged();

    void on_actionFind_triggered();

    void on_actionReplace_triggered();

    void on_actionC_C_Compiler_triggered();

    void on_actionC_C_triggered();

    void on_actionPlain_Text_triggered();

private:
    void init();
    void init_slots();


private:
    Ui::MainWindow *ui;
    QString m_prevtext;
    QLabel *m_cursorPos;
    QLabel *m_status;
    client *m_client;
    Find *m_find;
    Replace *m_replace;
    Highlighter *m_highlight;
};

#endif // MAINWINDOW_H
