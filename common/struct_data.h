#ifndef STURCT_DATA_H__
#define STURCT_DATA_H__

#define uint32 unsigned int
#define uint16 unsigned short
#define INF_LOOP for(;;)
#ifndef __cplusplus
typedef enum bool
{
	false,
	true
}bool;
#endif

typedef enum Type_remove
{
	REMOVE_FILE,
	INSERT_FILE
}Type_remove;

typedef struct UpdateFile
{
	int cursor;
	int size;
	Type_remove type;
}UpdateFile;

typedef struct FileInfo
{
    int users;
	size_t size;
	UpdateFile update;
    char *workspace;
    char *file;
    char *path;
}FileInfo;

typedef struct ClientInfo
{
	struct sockaddr_in addr;
	int sd;
	int currentPos;
	pthread_t thread;
    struct FileInfo file;
}ClientInfo;

typedef struct nod_client
{
    ClientInfo *client;
    struct nod_client *next;
}nod_client;

typedef struct list_users
{
    nod_client *head;
    nod_client *curr;
}list_users;


typedef struct file_data
{
	int l_workspace;
	int l_file;
}file_data;

typedef struct Server
{	
	struct sockaddr_in addr;	
	int sd;		
	pid_t pid;
    list_users users;
}Server;

typedef enum error
{
	ERROR_SOCKET,
	ERROR_BIND,
	ERROR_LISTEN,
	ERROR_ACCEPT,
	ERROR_READ,
    ERROR_WRITE,
    ERROR_OPEN,
	ERROR_MKDIR,
	ERROR_GET_FILE,
	ERROR_FCNTL,
    ERROR_RENAME,
    ERROR_REMOVE,
    ERROR_OUT_OF_MEMORY,
    ERROR_FORK,
    ERROR_MKFIFO
}error;

typedef enum FileType
{
    F_DIR,
    F_FILE
}FileType;

typedef struct F_Type
{    
    int level;
    FileType type;
    unsigned long size;
    char name[256];    
    char path[256];      
}F_Type;

typedef enum Command_Type
{	
    DISCONNECTED = 0,	
    GET_FILE,
    LIST_FILE,
    DOWNLOAD_FILE,
    UPDATE_FILE,
    UPDATE_CLIENT,
    VALID_FILE,
    CREATE_FILE,
    INVALID_FILE,
    COMPILE_FILE,
    COMPILE_FAILED,
    COMPILE_SUCCED,
    UNKNOWN_COMMAND
}Command_Type;

typedef struct Data
{
	Command_Type status;
}Data;

inline Data init_data(Command_Type status)
{
	Data data;
	data.status = status;
	return data;
}

inline void copy_updatefile(UpdateFile *dest,UpdateFile *src)
{
    dest->cursor = src->cursor;
    dest->size = src->size;
    dest->type = src->type;	
}

#endif

