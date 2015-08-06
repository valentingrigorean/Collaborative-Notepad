#ifndef CONNECT_UI_H
#define CONNECT_UI_H

#include <QDialog>
#include <QCloseEvent>
#include "client.h"

namespace Ui {
class connect_ui;
}

class connect_ui : public QDialog
{
    Q_OBJECT

public:
    explicit connect_ui(client *client, QWidget *parent = 0);
    ~connect_ui();
protected:

private slots:
    void on_pushButton_2_clicked();

    void on_pushButton_clicked();

private:
    Ui::connect_ui *ui;
    client *m_client;
    void init_validators();
};

#endif // CONNECT_UI_H
