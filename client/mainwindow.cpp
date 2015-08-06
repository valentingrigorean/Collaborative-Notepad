#include <QFileDialog>
#include <QTextStream>
#include <QMessageBox>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "helpFunctions.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    init();
    init_slots();
}

MainWindow::~MainWindow()
{   
    delete ui;
}

void MainWindow::init()
{
    m_status = new QLabel(this);
    m_cursorPos = new QLabel(this);
    m_client = new client(ui->plainTextEdit,this);  
    m_replace = NULL;
    m_find = NULL;
    m_highlight = NULL;

    m_cursorPos->setText("Line:1,Col:1");
    m_status->setText("Connection:OFFLINE");

    ui->statusBar->addPermanentWidget(m_status);
    ui->statusBar->addPermanentWidget(m_cursorPos);

    this->setCentralWidget(ui->plainTextEdit);

    ui->actionCopy->setEnabled(false);
    ui->actionCut->setEnabled(false);
    ui->actionRedo->setEnabled(false);
    ui->actionUndo->setEnabled(false);
    ui->actionDisconnect->setEnabled(false);
    ui->actionDownload->setEnabled(false);
    ui->actionC_C_Compiler->setEnabled(false);
}

void MainWindow::init_slots()
{
    connect(ui->plainTextEdit,SIGNAL(copyAvailable(bool)),this,SLOT(showCopyPaste(bool)));
    connect(ui->plainTextEdit,SIGNAL(undoAvailable(bool)),this,SLOT(showUndo(bool)));
    connect(ui->plainTextEdit,SIGNAL(redoAvailable(bool)),this,SLOT(showRedo(bool)));
    connect(m_client,SIGNAL(got_file()),this,SLOT(got_file()));
    connect(m_client,SIGNAL(lost_connection()),this,SLOT(lost_connection()));
    connect(m_client,SIGNAL(triggerConnection()),this,SLOT(on_connection()));
    connect(m_client,SIGNAL(too_manyusers()),this,SLOT(too_manyusers()));    
}

//PUBLIC SLOTS
void MainWindow::on_connection()
{
    switch(m_client->isConnected())
    {
    case true:
        ui->actionDisconnect->setEnabled(true);
        ui->actionJoin_Server->setEnabled(false);
        ui->actionDownload->setEnabled(true);
        m_status->setText("Connection:ONLINE");
        this->setWindowTitle(updateTitle(m_client));
        return;
    default:
        ui->actionDisconnect->setEnabled(false);
        ui->actionJoin_Server->setEnabled(true);
        ui->actionDownload->setEnabled(false);
        ui->actionC_C_Compiler->setEnabled(false);
        m_status->setText("Connection:OFFLINE");
    }

}

void MainWindow::got_file()
{
    m_prevtext = ui->plainTextEdit->toPlainText();
}

void MainWindow::lost_connection()
{
    QMessageBox::information(this,"Error!","Can't connect to server.",QMessageBox::Ok);       
}

void MainWindow::too_manyusers()
{
    QMessageBox::information(this,"Error!","Are 2 users editing this file.",QMessageBox::Ok);
}

void MainWindow::close_find()
{
    m_find = NULL;
}

void MainWindow::close_replace()
{
    m_replace = NULL;
}

//MENU ITEMS
void MainWindow::showCopyPaste(bool b)
{
    if(b)
    {
        ui->actionCopy->setEnabled(true);
        ui->actionCut->setEnabled(true);
        return;
    }
    ui->actionCopy->setEnabled(false);
    ui->actionCut->setEnabled(false);
}

void MainWindow::showUndo(bool b)
{
    b ? ui->actionUndo->setEnabled(true) : ui->actionUndo->setEnabled(false);
}

void MainWindow::showRedo(bool b)
{
    b ? ui->actionRedo->setEnabled(true) : ui->actionRedo->setEnabled(false);
}

void MainWindow::on_actionExit_triggered()
{
    this->close();
}

void MainWindow::on_actionJoin_Server_triggered()
{
    m_client->clear();
    connect_ui con(m_client,this);
    con.exec();
    if(m_client->isSet())
    {
        if(strcmp(&m_client->get_file()[strlen(m_client->get_file()) -2],".c") == 0
                || strcmp(&m_client->get_file()[strlen(m_client->get_file()) -4],".cpp") == 0)
            ui->actionC_C_Compiler->setEnabled(true);
        if(m_client->c_or_cplusplus())
            on_actionC_C_triggered();
        m_client->set_connection();
        ui->plainTextEdit->clear();        
        m_client->getFile(0);       
    }
}

void MainWindow::on_actionDisconnect_triggered()
{
    m_client->close_connection();
}

void MainWindow::on_actionSave_as_triggered()
{
    QString filename = QFileDialog::getSaveFileName(this,"Save File",QDir::currentPath(),"Text files (*.txt);");
    QFile file(filename);
    file.open(QFile::WriteOnly);
    QTextStream out(&file);
    out << ui->plainTextEdit->toPlainText();
    file.close();
}

void MainWindow::on_actionDownload_triggered()
{
    m_client->start_thread(false);
    Download down(m_client,this);    
    down.exec();
    m_client->start_thread(true);
}

void MainWindow::on_actionUndo_triggered()
{
    ui->plainTextEdit->undo();
}

void MainWindow::on_actionRedo_triggered()
{
    ui->plainTextEdit->redo();
}

void MainWindow::on_actionCut_triggered()
{
    ui->plainTextEdit->cut();
}

void MainWindow::on_actionCopy_triggered()
{
    ui->plainTextEdit->copy();
}

void MainWindow::on_actionPaste_triggered()
{
    ui->plainTextEdit->paste();
}

void MainWindow::on_actionFind_triggered()
{
    if(m_replace || m_find)
        return;
    m_find = new Find(ui->plainTextEdit,this);
    connect(m_find,SIGNAL(finished(int)),this,SLOT(close_find()));
    m_find->show();
}

void MainWindow::on_actionReplace_triggered()
{
    if(m_find || m_replace)
        return;
    m_replace = new Replace(ui->plainTextEdit,this);
    connect(m_replace,SIGNAL(finished(int)),this,SLOT(close_replace()));   
    m_replace->show();
}

void MainWindow::on_actionC_C_Compiler_triggered()
{
    m_client->compile();
}

//Editor
void MainWindow::on_plainTextEdit_cursorPositionChanged()
{
    m_cursorPos->setText("Line:" + QString::number(ui->plainTextEdit->textCursor().blockNumber()+1)
                           + "Col:" + QString::number(ui->plainTextEdit->textCursor().columnNumber()+1));
}

void MainWindow::on_plainTextEdit_textChanged()
{
    if(m_client->isConnected())
    {
        text_changed(ui->plainTextEdit,m_prevtext,m_client);
        m_prevtext = ui->plainTextEdit->toPlainText();
    }
}



void MainWindow::on_actionC_C_triggered()
{
    if(m_highlight)
        return;
    m_highlight = new Highlighter(ui->plainTextEdit->document());
    ui->actionC_C->setIconVisibleInMenu(true);
    ui->actionPlain_Text->setIconVisibleInMenu(false);

}

void MainWindow::on_actionPlain_Text_triggered()
{
    if(m_highlight != NULL)
    {
        delete m_highlight;
        m_highlight = 0;
    }
    ui->actionC_C->setIconVisibleInMenu(false);
    ui->actionPlain_Text->setIconVisibleInMenu(true);
}
