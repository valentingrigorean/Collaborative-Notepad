#include "download.h"
#include "ui_download.h"
#include <QMessageBox>
#include <QFileDialog>
#include <stdio.h>

Download::Download(client *client, QWidget *parent) :   
    QDialog(parent),
    ui(new Ui::Download),
    m_client(client)
{
    ui->setupUi(this);  
    init_items();
    getItems();    
}

Download::~Download()
{
    delete ui;
    delete m_header;    
}

void Download::init_items()
{    
    m_header = new QTreeWidgetItem();
    m_header->setText(0,QString("File Name")); 
    m_header->setText(1,QString("Size(Bytes)"));
    m_header->setText(2,QString("Type"));
    m_header->setText(3,QString("Path"));
    ui->treeWidget->setHeaderItem(m_header);
    ui->treeWidget->setColumnWidth(0,250);
}

QTreeWidgetItem* Download::AddChild(QTreeWidgetItem *parent, QString name,QString path, FileType type, unsigned long size)
{
    QTreeWidgetItem *item = new QTreeWidgetItem(parent);
    type == F_DIR ? item->setIcon(0,*(new QIcon(":/ICONS/resources/dir"))):
                    item->setIcon(0,*(new QIcon(":/ICONS/resources/file")));
    item->setText(0,name);
    if(type == F_FILE)
        item->setText(1,QString::number(size));
    type == F_DIR ? item->setText(2,"Folder") :
                    item->setText(2,"File");
    if(type == F_FILE)
        item->setText(3,path);
    return item;
}

QTreeWidgetItem* Download::AddRoot(QString name,QString path,FileType type,unsigned long size)
{
    QTreeWidgetItem *item = new QTreeWidgetItem(ui->treeWidget);
    type == F_DIR ? item->setIcon(0,*(new QIcon(":/ICONS/resources/dir"))):
                    item->setIcon(0,*(new QIcon(":/ICONS/resources/file")));
    item->setText(0,name);
    if(type == F_FILE)
        item->setText(1,QString::number(size));
    type == F_DIR ? item->setText(2,"Folder") :
                    item->setText(2,"File");
    if(type == F_FILE)
        item->setText(3,path);
    return item;
}

void Download::getItems()
{   
    F_Type f;
    Data data = init_data(LIST_FILE);
    QTreeWidgetItem *father=NULL,*child=NULL;
    m_client->write_data(&data,sizeof(Data));
    start:
    m_client->read_data(&f,sizeof(F_Type));   
    if(f.type == -222)
        goto end;
    if(f.level == 0)
        goto j_father;
    if(f.level > 0)
        goto j_child;
    goto end;
    j_father:
    father=AddRoot(f.name,f.path,f.type,f.size);
    goto start;
    j_child:
    if(f.level == 1)
        child=AddChild(father,f.name,f.path,f.type,f.size);
    else if(f.level>=1)
        child=AddChild(child,f.name,f.path,f.type,f.size);
    goto start;
    end:    
    return;
}

void Download::on_treeWidget_itemDoubleClicked(QTreeWidgetItem *item, int column)
{
    if(item->text(2) == "File")
    {
        char *path;
        char *file;
        char *s_path;
        Data data = init_data(DOWNLOAD_FILE);
        int size=0;
        FILE *fd;

        QByteArray text = item->text(3).toLocal8Bit();
        path = new char[text.size()+5];
        strcpy(path,text.data());

        text = item->text(0).toLocal8Bit();
        file = new char[text.size()+5];
        strcpy(file,text.data());

        s_path = new char[strlen(path)+strlen(file)+8];
        strcpy(s_path,path);
        strcat(s_path,"/");
        strcat(s_path,file);

        m_client->write_data(&data,sizeof(Data));
        size=strlen(s_path);
        m_client->write_data(&size,sizeof(int));
        m_client->write_data(s_path,size);

        m_client->read_data(&size,sizeof(int));
        if(size == -1)
        {
            QMessageBox::information(this,"Error Download!","Failed to download the file.",QMessageBox::Ok);
            delete path;
            delete file;
            delete s_path;
            return;
        }

        delete s_path;
        s_path = new char[size+10];
        memset(s_path,0,size+10);

        m_client->read_data(s_path,size);

        QString filename = QFileDialog::getSaveFileName(this,"Save File",QDir::currentPath(),"All files (*.*);");
        delete path;
        text = filename.toLocal8Bit();
        path = new char[text.size() +5];
        strcpy(path,text.data());

        if((fd = fopen(path,"wb")) == NULL)
        {
            QMessageBox::information(this,"Erorr!","Failed to write the file on the disk.",QMessageBox::Ok);
            delete path;
            delete file;
            delete s_path;
            return;
        }
        fwrite(s_path,size,1,fd);
        fclose(fd);


        delete path;
        delete file;
        delete s_path;
    }
}
