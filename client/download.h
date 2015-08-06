#ifndef DOWNLOAD_H
#define DOWNLOAD_H

#include <QDialog>
#include <QTreeWidget>
#include "client.h"

namespace Ui {
class Download;
}

class Download : public QDialog
{
    Q_OBJECT

public:
    explicit Download(client *client,QWidget *parent = 0);
    ~Download();

    QTreeWidgetItem* AddRoot(QString name,QString path, FileType type, unsigned long size);
    QTreeWidgetItem *AddChild(QTreeWidgetItem *parent, QString name, QString path, FileType type, unsigned long size);
private slots:

    void on_treeWidget_itemDoubleClicked(QTreeWidgetItem *item, int column);

private:
    void init_items();
    void getItems();
    void recursion(QTreeWidgetItem *father, F_Type *f);
private:
    Ui::Download *ui;
    client *m_client;
    QTreeWidgetItem *m_header;
};

#endif // DOWNLOAD_H
