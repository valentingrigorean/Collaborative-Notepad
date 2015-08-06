#ifndef CLIENT_H
#define CLIENT_H

#include <QThread>
#include <QString>
#include <QObject>
#include <QTimer>
#include <QPlainTextEdit>
#include <QThread>
#include <stdlib.h>
#include <string.h>
#include <limits.h>


#if WIN32
#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include "C:\Users\v4l1__000\Dropbox\Facult\An2\Retele\Proiect\struct_data.h"
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <netdb.h>
#include "/home/valentin/Dropbox/Facult/An2/Retele/proiect/struct_data.h"
#endif


typedef Data Data;

class client;

class read_thread: public QThread
{
    Q_OBJECT
public:
    read_thread(client *_client);
signals:
    void done_reading(Data);
protected:
    void run();
private:
    client *m_client;
};

class client: public QObject
{
    Q_OBJECT
public:
    explicit client(QPlainTextEdit *text, QObject *parent = 0);
    ~client();

    void set_port(uint16 );
    void set_workspace(const QString&);
    void set_addr(const QString&);
    void set_file(const QString&);

    uint16 get_port()const;
    QString get_addr()const;
    char* get_workspace()const;
    char* get_file()const;

    void clear();

    bool isSet();
    bool isConnected();

    void start_thread(bool b);

    void set_connection();
    void close_connection();


    void getFile(int step);
    void updateFile(Type_remove type,int size);
    void downloadFile(int step);
    void compile();
    bool c_or_cplusplus();

    void write_data(void *data,size_t size);
    void read_data(void *data,size_t size);

signals:
    void triggerConnection();
    void lost_connection();
    void got_file();
    void too_manyusers();
private slots:
    void handle_request(Data);
private:
    void force_close();
    void update_file_from_server();

    void download_exec(bool state);
private:
    QPlainTextEdit *m_text;
    bool m_connection;
    char *m_workspace;
    char *m_file;
    QString m_addr;
#if WIN32
    SOCKET m_fd;
    WSADATA m_wsadata; 
#else
    int m_fd;    
#endif
    bool m_forceQuit;
    uint16 m_port;
    Data m_data;
    static Data _test_connection;    
    read_thread *m_thread;
    struct sockaddr_in m_socket;
};

#endif // CLIENT_H
