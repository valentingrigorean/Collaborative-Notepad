#include "find.h"
#include "ui_find.h"

Find::Find(QPlainTextEdit *text, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Find),
    m_text(text)
{
    ui->setupUi(this);
    this->setFixedSize(this->size());    
    ui->pushButton->setEnabled(false);
    m_cursor = text->textCursor();
    m_cursor.setPosition(0);
    text->setTextCursor(m_cursor);
}

Find::~Find()
{
    delete ui;
}

void Find::on_lineEdit_returnPressed()
{
    if(ui->lineEdit->text().length() == 0)
    {
        return;
    }
    if(ui->checkBox->isChecked() && ui->checkBox_2->isChecked())
    {
        m_text->find(ui->lineEdit->text(),QTextDocument::FindCaseSensitively | QTextDocument::FindWholeWords);
        return;
    }
    if(ui->checkBox->isChecked())
    {
        m_text->find(ui->lineEdit->text(),QTextDocument::FindCaseSensitively);
        return;
    }
    if(ui->checkBox_2->isChecked())
    {
        m_text->find(ui->lineEdit->text(),QTextDocument::FindWholeWords);
        return;
    }
    m_text->find(ui->lineEdit->text());
}

void Find::on_lineEdit_textChanged(const QString &arg1)
{
    ui->lineEdit->text().length() == 0 ? ui->pushButton->setEnabled(false)
                                       : ui->pushButton->setEnabled(true);
    m_cursor = m_text->textCursor();
    m_cursor.setPosition(0);
    m_text->setTextCursor(m_cursor);
}

void Find::on_pushButton_2_clicked()
{
    this->close();
}

void Find::on_pushButton_clicked()
{
    on_lineEdit_returnPressed();
}
