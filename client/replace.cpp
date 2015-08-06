#include "replace.h"
#include "ui_replace.h"

Replace::Replace(QPlainTextEdit *text,QWidget *parent) :
    QDialog(parent),    
    ui(new Ui::Replace),
    m_text(text)
{
    ui->setupUi(this);
    ui->pushButton_2->setEnabled(false);
    ui->pushButton_3->setEnabled(false);
    ui->pushButton_4->setEnabled(false);
}

Replace::~Replace()
{
    delete ui;
}

bool Replace::search()
{
    if(ui->checkBox->isChecked() && ui->checkBox_2->isChecked())
    {
        if(m_text->find(ui->lineEdit->text(),QTextDocument::FindCaseSensitively | QTextDocument::FindWholeWords) == true)
        {
            ui->pushButton_2->setEnabled(true);
            return true;
        }
        return false;
    }
    if(ui->checkBox->isChecked())
    {
        if(m_text->find(ui->lineEdit->text(),QTextDocument::FindCaseSensitively) == true)
        {
            ui->pushButton_2->setEnabled(true);
            return true;
        }
        return false;
    }
    if(ui->checkBox_2->isChecked())
    {
        if(m_text->find(ui->lineEdit->text(),QTextDocument::FindWholeWords) == true)
        {
            ui->pushButton_2->setEnabled(true);
            return true;
        }
        return false;
    }
    if(m_text->find(ui->lineEdit->text()) == true)
    {
        ui->pushButton_2->setEnabled(true);
        return true;
    }
    return false;
}

void Replace::on_lineEdit_textChanged(const QString &arg1)
{
    m_cursor = m_text->textCursor();
    m_cursor.setPosition(0);
    m_text->setTextCursor(m_cursor);

    if(ui->lineEdit->text().size() == 0)
    {
        ui->pushButton_2->setEnabled(false);
        ui->pushButton_3->setEnabled(false);
        ui->pushButton_4->setEnabled(false);
        return;
    }
    ui->pushButton_2->setEnabled(false);
    ui->pushButton_3->setEnabled(true);
    ui->pushButton_4->setEnabled(true);
}

void Replace::on_lineEdit_returnPressed()
{
    if(ui->lineEdit->text().length() == 0)
    {
        return;
    }
    search();
}

void Replace::on_pushButton_clicked()
{
    this->close();
}

void Replace::on_pushButton_2_clicked()
{
    m_text->textCursor().removeSelectedText();
    m_text->insertPlainText(ui->lineEdit_3->text());
}

void Replace::on_pushButton_3_clicked()
{
    while(search() == true)
    {
        m_text->textCursor().removeSelectedText();
        m_text->insertPlainText(ui->lineEdit_3->text());
    }
}
