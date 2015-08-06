#ifndef HELPFUNCTIONS_H
#define HELPFUNCTIONS_H
#include <QString>
#include <QPlainTextEdit>
#include <QApplication>
#include "client.h"
#if WIN32
#include "C:\Users\v4l1__000\Dropbox\Facult\An2\Retele\Proiect\struct_data.h"
#else
#include "/home/valentin/Dropbox/Facult/An2/Retele/proiect/struct_data.h"
#endif


QString IntToString(int);
QString updateTitle(client *m_client);
void DrawForSelectText(QPlainTextEdit *);
void text_changed(QPlainTextEdit *edit, const QString &prevtext, client *m_client);

#endif // HELPFUNCTIONS_H
