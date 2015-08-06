#include "helpFunctions.h"
#include <QMessageBox>

QString IntToString(int val)
{
    return QString::number(val,10);
}

void DrawForSelectText(QPlainTextEdit *edit)
{
     QTextEdit::ExtraSelection selection;
     QColor color = QColor(Qt::red);
     selection.format.setBackground(color);
     selection.cursor = edit->textCursor();
}

QString updateTitle(client *m_client)
{    
    QString work(m_client->get_workspace());
    QString file(m_client->get_file());
    return ("~/" +m_client->get_addr() + ":" +::IntToString(m_client->get_port())+ "/"+work +"/" +file + "   -editor");
}

void text_changed(QPlainTextEdit *edit, const QString &prevtext, client *m_client)
{
    if(m_client)
    {
        if(edit->toPlainText().length() == prevtext.length())
            return;
        if(edit->toPlainText().length()>prevtext.length())
            m_client->updateFile(INSERT_FILE,edit->toPlainText().length() - prevtext.length());
        else
            m_client->updateFile(REMOVE_FILE,prevtext.length() -  edit->toPlainText().length());
    }
}

