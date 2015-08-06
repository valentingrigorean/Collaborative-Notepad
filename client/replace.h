#ifndef REPLACE_H
#define REPLACE_H

#include <QDialog>
#include <QPlainTextEdit>
#include <QTextCursor>

namespace Ui {
class Replace;
}

class Replace : public QDialog
{
    Q_OBJECT

public:
    explicit Replace(QPlainTextEdit *text,QWidget *parent = 0);
    ~Replace();

private:
    bool search();
private slots:
    void on_pushButton_clicked();

    void on_lineEdit_textChanged(const QString &arg1);

    void on_lineEdit_returnPressed();

    void on_pushButton_2_clicked();

    void on_pushButton_3_clicked();

private:
    Ui::Replace *ui;
    QPlainTextEdit *m_text;
    QTextCursor m_cursor;
};

#endif // REPLACE_H
