#ifndef FIND_H
#define FIND_H

#include <QDialog>
#include <QPlainTextEdit>
#include <QTextCursor>

namespace Ui {
class Find;
}

class Find : public QDialog
{
    Q_OBJECT

public:
    explicit Find(QPlainTextEdit *text,QWidget *parent = 0);
    ~Find();
private slots:
    void on_lineEdit_returnPressed();

    void on_lineEdit_textChanged(const QString &arg1);

    void on_pushButton_2_clicked();

    void on_pushButton_clicked();

private:
    Ui::Find *ui;
    QPlainTextEdit *m_text;
    QTextCursor m_cursor;
};

#endif // FIND_H
