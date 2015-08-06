#include <QMessageBox>
#include <QTextCursor>
#include <sys/stat.h>
#include <fcntl.h>
#include "client.h"
#include "helpFunctions.h"

//THREAD
read_thread::read_thread(client *_client):
    m_client(_client)
{
}

void read_thread::run()
{
    Data m_data;
    m_client->read_data(&m_data,sizeof(Data));
    emit done_reading(m_data);
}

//CLIENT

client::client(QPlainTextEdit *text, QObject *parent):
    QObject(parent),
    m_text(text)

{  
    m_connection = false;
    m_thread = new read_thread(this);
    qRegisterMetaType<Data>("Data");
    connect(m_thread,SIGNAL(done_reading(Data)),this,SLOT(handle_request(Data)));
    m_forceQuit = false;
    m_workspace = NULL;
    m_file = NULL;
}

void client::handle_request(Data data)
{    
    switch(data.status)
    {
    case DISCONNECTED:
        force_close();
        break;    
    case UPDATE_CLIENT:
        write_data(&data,sizeof(Data));
        update_file_from_server();
        break;
    case UPDATE_FILE:
    case DOWNLOAD_FILE:
    case CREATE_FILE:
    case GET_FILE:
    case LIST_FILE:
    case COMPILE_FILE:
    case UNKNOWN_COMMAND:
        break;    
    case INVALID_FILE:
        close_connection();
        emit too_manyusers();
        break;
    case VALID_FILE:
        getFile(1);
        break;
    case COMPILE_SUCCED:
    {
        QMessageBox mbox;
        mbox.setText("The file compiled succeful.");
        mbox.setInformativeText("Do you want to download it?");
        mbox.setStandardButtons(QMessageBox::Ok|QMessageBox::Cancel);
        mbox.setDefaultButton(QMessageBox::Cancel);
        mbox.exec() == QMessageBox::Ok ? download_exec(true)
                                       : download_exec(false);

    }
        break;
    case COMPILE_FAILED:
        QMessageBox::information(NULL,"","ERROR!");
        break;
    }
    if(m_connection)
        m_thread->start();
}

client::~client()
{
    if(m_connection)    
       close_connection();
    if(m_file)
        delete m_file;
    if(m_workspace)
        delete m_workspace;
}

bool client::isSet()
{
    if(m_addr.isEmpty() || m_port == 0)
        return false;
    return true;
}

bool client::isConnected()
{
    return m_connection;
}

QString client::get_addr() const
{
    return m_addr;
}

uint16 client::get_port() const
{
    return m_port;
}

char* client::get_workspace() const
{
    return m_workspace;
}

char* client::get_file() const
{
    return m_file;
}

void client::set_addr(const QString &addr)
{
    m_addr = addr;
}

void client::set_port(unsigned short port)
{
    m_port = port;
}

void client::set_workspace(const QString &workspace)
{
    QByteArray text = workspace.toLocal8Bit();
    if(m_workspace != NULL)
        delete m_workspace;
    m_workspace = new char[text.size()+1];
    strcpy(m_workspace,text.data());
}

void client::set_file(const QString &file)
{
    QByteArray text = file.toLocal8Bit();    
    if(m_file != NULL)
        delete m_file;
    m_file = new char[text.size()+1];
    strcpy(m_file,text.data());
}

void client::clear()
{
    start_thread(false);
    m_addr.clear();
    m_port = 0;
    m_connection = false;
}

void client::set_connection()
{
    const char * addr = m_addr.toLatin1().data();
#if WIN32
    WSAStartup(MAKEWORD(2,2),&m_wsadata);
    m_fd = socket(AF_INET,SOCK_STREAM,0);
    memset(&m_socket,0,sizeof(m_socket));
    m_socket.sin_family = AF_INET;
    m_socket.sin_addr.s_addr = inet_addr(addr);
    m_socket.sin_port = htons(m_port);
    if(::connect(m_fd,(struct sockaddr*)&m_socket,sizeof(struct sockaddr)) != -1)
    {
        m_connection = true;
        m_thread->start();
    }
#else
    m_fd = socket(AF_INET,SOCK_STREAM,0);
    memset(&m_socket,0,sizeof(m_socket));
    m_socket.sin_family = AF_INET;
    m_socket.sin_addr.s_addr = inet_addr(addr);
    m_socket.sin_port = htons(m_port);
    if(::connect(m_fd,(struct sockaddr*)&m_socket,sizeof(struct sockaddr)) != -1)
    {
        m_connection = true;
        m_thread->start();
    }
#endif
    emit triggerConnection();
}

void client::close_connection()
{    
    if(m_connection)
    {
        m_data.status = DISCONNECTED;
        write_data(&m_data,sizeof(Data));
        m_connection = false;
        m_thread->quit();
    }
#if WIN32
    closesocket(m_fd);
    WSACleanup();
#else
    ::close(m_fd);
#endif
    emit triggerConnection();
}

void client::force_close()
{
    m_connection = false;
    m_thread->quit();
#if WIN32
    closesocket(m_fd);
    WSACleanup();
#else
    ::close(m_fd);
#endif
    emit triggerConnection();
}

void client::download_exec(bool state)
{
    if(state == false)
    {
        m_data = init_data(UNKNOWN_COMMAND);
        return;
    }

    int size,fd;
    char *buffer,*exec;

    m_data = init_data(GET_FILE);
    write_data(&m_data,sizeof(Data));

    exec = (char*)malloc(sizeof(char) * strlen(get_file())+20);
    strcpy(exec,get_file());

    strcmp(&exec[strlen(exec)-2],".c") == 0 ? exec[strlen(exec)-2] = 0
            : exec[strlen(exec)-4] = 0;

    read_data(&size,sizeof(int));

    buffer = (char*)malloc(sizeof(char)*size);
    read_data(buffer,size);

    fd = ::open(exec,O_CREAT|O_WRONLY);
    ::write(fd,buffer,size);
    close(fd);
}

void client::start_thread(bool b)
{
    b==true ? m_thread->start() : m_thread->terminate();
}

void client::getFile(int step)
{
    if(step == 0)
    {
        file_data file;
        m_data.status = GET_FILE;
        file.l_file = strlen(m_file);
        file.l_workspace = strlen(m_workspace);

        write_data(&m_data,sizeof(Data));
        write_data(&file,sizeof(file_data));
        write_data(m_workspace,file.l_workspace);
        write_data(m_file,file.l_file);
        return;
    }    
    int size_file = 0;
    char *buff;

    read_data(&size_file,sizeof(int));
    buff =(char*)malloc(sizeof(char)*size_file);
    memset(buff,0,size_file);
    read_data(buff,size_file);
    m_text->blockSignals(true);
    m_text->setPlainText(buff);
    m_text->blockSignals(false);
    free(buff);
    emit got_file();
}

void client::updateFile(Type_remove type,int size)
{
    if(m_connection)
    {
        UpdateFile up;
        up.type = type;
        up.cursor = m_text->textCursor().position();
        m_data.status = UPDATE_FILE;
        up.size = size;       
        write_data(&m_data,sizeof(Data));
        write_data(&up,sizeof(UpdateFile));
        if(type == INSERT_FILE)
        {
            QByteArray text = m_text->toPlainText().toLocal8Bit();
            char *buff =(char*)malloc(sizeof(char)*up.size +1);
            int j=0;
            for(int i = up.cursor - up.size;i<up.cursor;i++)
                buff[j++] = text[i];
            buff[j] = '\0';
            write_data(buff,sizeof(char)*strlen(buff));
        }
    }
}

void client::update_file_from_server()
{
    UpdateFile update;
    QString str = m_text->toPlainText();
    QTextCursor cursor = m_text->textCursor();
    read_data(&update,sizeof(UpdateFile));
    switch(update.type)
    {
    case REMOVE_FILE:       
        str.remove(update.cursor,update.size);
        m_text->blockSignals(true);
        m_text->setPlainText(str);
        if(cursor.position() == (update.cursor-update.size))
        {
            cursor.movePosition(QTextCursor::Left,QTextCursor::MoveAnchor,update.size);
            m_text->setTextCursor(cursor);
        }
        m_text->blockSignals(false);
        break;
    case INSERT_FILE:
        char *buff =(char*)malloc(sizeof(char)*update.size+1);
        memset(buff,0,update.size+1);
        read_data(buff,update.size);
        str.insert(update.cursor-1,buff);
        m_text->blockSignals(true);
        m_text->setPlainText(str);
        if(cursor.position() == (update.cursor+update.size))
        {
            cursor.movePosition(QTextCursor::Right,QTextCursor::MoveAnchor,update.size);
            m_text->setTextCursor(cursor);
        }
        m_text->blockSignals(false);
        free(buff);
        break;
    }
    emit got_file();
}

void client::compile()
{
    m_data = init_data(COMPILE_FILE);
    write_data(&m_data,sizeof(Data));
}

bool client::c_or_cplusplus()
{
    if((strcmp(&m_file[strlen(m_file)-2],".c") == 0) ||
            strcmp(&m_file[strlen(m_file)-4],".cpp"))
        return true;
    return false;
}

void client::write_data(void *data, size_t size)
{
#if WIN32
    if(send(m_fd,(char*)data,size,0) == -1)
        close_connection();
#else
    if(::write(m_fd,data,size) == -1)
        close_connection();
#endif
}

void client::read_data(void *data, size_t size)
{
#if WIN32
    int status=recv(m_fd,(char*)data,size,0);
    switch(status)
    {
    case -1:
    case 0:
        force_close();
        emit lost_connection();
        break;
    default:
        return;
    }
#else
    int status=::read(m_fd,data,size);
    switch(status)
    {
    case -1:
    case 0:
        force_close();
        emit lost_connection();
        break;
    default:
        return;
    }
#endif
}
