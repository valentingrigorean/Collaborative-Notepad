#include "connect_ui.h"
#include "ui_connect_ui.h"
#include <QRegExp>
#include <QValidator>
#include <QRegExpValidator>
#include <QMessageBox>

connect_ui::connect_ui(client *client, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::connect_ui),
    m_client(client)
{
    ui->setupUi(this);
    this->setFixedSize(this->size());    
    init_validators();
}

connect_ui::~connect_ui()
{
    delete ui;
}

void connect_ui::on_pushButton_2_clicked()
{
    this->close();
}

void connect_ui::on_pushButton_clicked()
{
    if(ui->lineEdit->text().size() >=7 && ui->lineEdit_2->text().size() >= 0 && ui->lineEdit_3->text().size() > 0)
    {
        m_client->set_addr(ui->lineEdit->text());
        m_client->set_port(ui->lineEdit_2->text().toShort());
        m_client->set_workspace(ui->lineEdit_3->text());
        m_client->set_file(ui->lineEdit_4->text());
        on_pushButton_2_clicked();
    }
    else
        QMessageBox::information(this,"Error!","Wrongh fomrat.Exemple:\nHost:127.0.0.1\nPort:2222\nWorkspace:qwe\nFile:can be empty.",QMessageBox::Ok);
}

void connect_ui::init_validators()
{
    QRegExp exp("[1-9]+.[0-9]+.[0-9]+.[0-9]+");
    QRegExp folder("[_a-zA-Z0-9]+");
    QRegExp file("[_a-zA-Z0-9]+.[_a-zA-Z0-9]+|[_a-zA-Z0-9]+");
    QRegExpValidator *ex = new QRegExpValidator(exp,this);
    QRegExpValidator *ex2 = new QRegExpValidator(folder,this);
    QRegExpValidator *ex3 = new QRegExpValidator(file,this);
    ui->lineEdit->setValidator(ex);
    ui->lineEdit_2->setValidator(new QIntValidator(0,65536,this));
    ui->lineEdit_3->setValidator(ex2);
    ui->lineEdit_4->setValidator(ex3);
}
