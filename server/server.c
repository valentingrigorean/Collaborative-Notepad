#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <dirent.h>
#include <pthread.h>
#include "/home/valentin/Dropbox/Facult/An2/Retele/proiect/struct_data.h"

#define PORT 2021

pthread_mutex_t mlock = PTHREAD_MUTEX_INITIALIZER;
static Server* get_server();
void init_server(Server *s);
void clear_server(int sig);
void update_client();
nod_client* get_client_fromThread(pthread_t id);


//Lists
void add_user(Server *s,ClientInfo *c);
void remove_user(Server *s, nod_client *c);
nod_client *get_nod(Server *s,ClientInfo *c);
void update_client_file(Server *s, nod_client *c, int type);
int count_connection(Server *s);

//FILES
void WriteLock(int fd,off_t size);
void ReadLock(int fd,off_t size);
void UnLock(int fd,off_t size);
void update_size(FileInfo *f);
void update_file(ClientInfo *c);
void read_file(ClientInfo *c);
void update_client(Server *s,nod_client *c);

//SERVER HANDLERS
void server_loop(Server *s);
void info_server(Data data,int user);
void close_connection(Server *server,nod_client *user);
void error_info(error e);
bool read_data(int sd,void *data, size_t size, Server *server, nod_client *user);
bool write_data(int sd,void *data,size_t size,Server *server,nod_client *user);

//CLIENT
int send_file(Server *s, nod_client *c);
void alloc_fileinfo(FileInfo *f,file_data *d);
void set_fileinfo(FileInfo *f);
void check_directory(FileInfo * file);
unsigned long get_size(const char *name);
void listdir(const char *name,int level,F_Type *type,int sd);
void download_file(int sd);
void compile_file(ClientInfo *c);

static void *run(void *arg);

int main()
{
    memset(get_server(),0,sizeof(Server));
    signal(SIGINT,clear_server);    
    get_server()->pid = getpid();
    init_server(get_server());
    return 0;
}

//SERVER INIT
static Server* get_server()
{
    static Server server;
    return &server;
}

void init_server(Server *s)
{
    memset(s,0,sizeof(Server));
    if((s->sd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
        error_info(ERROR_SOCKET);

    int on = 1;
    setsockopt(s->sd,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on));

    s->addr.sin_family = AF_INET;
    s->addr.sin_addr.s_addr = htonl(INADDR_ANY);
    s->addr.sin_port = htons(PORT);

    if(bind(s->sd,(struct sockaddr *) &s->addr,sizeof(struct sockaddr)) == -1)
        error_info(ERROR_BIND);

    if(listen(s->sd,5) == -1)
        error_info(ERROR_LISTEN);

    server_loop(s);
}

void clear_server(int sig)
{  
    printf("[server] recive a signal[%d]\n",sig);
    printf("[server] clearing the memory\n");
    INF_LOOP
    {
        nod_client *c = get_server()->users.head;        
        if(c)
        {   
            printf("[server] closing connection thread[%d]...\n",c->client->currentPos);	
            pthread_kill(c->client->thread,0);		
            remove_user(get_server(),c);
        }        
        if(c == NULL)
             break;
    }
    exit(0);
}

nod_client* get_client_fromThread(pthread_t id)
{
    nod_client *aux = get_server()->users.head;
    for(;aux!=NULL;aux=aux->next)
        if(aux->client->thread = id)
            return aux;
    return NULL;
}

//LISTs
void add_user(Server *s,ClientInfo *c)
{
    nod_client * aux = malloc(sizeof(nod_client));
    aux->client = c;
    aux->next = NULL;
    if(s->users.head == NULL)
    {
        s->users.curr = s->users.head = aux;
        return;
    }
    s->users.curr->next = aux;
    s->users.curr = aux;
}

void remove_user(Server *s, nod_client *c)
{
    nod_client *aux = s->users.head;    
    update_client_file(s,c,1);
	
    if(s->users.head == s->users.curr && s->users.head == c)
    {    
		pthread_mutex_lock(&mlock);   
        s->users.head = s->users.curr = NULL;        
        free(c);
		pthread_mutex_unlock(&mlock);
        return;
    }

    if(aux == c)
    {
		pthread_mutex_lock(&mlock);
        s->users.head = s->users.head->next;
        free(c);
		pthread_mutex_unlock(&mlock);
        return;
    }	

    if(s->users.curr == c)
    {
        for(;aux->next!=c;aux=aux->next);
		pthread_mutex_lock(&mlock);
       	aux->next = NULL;        
       	free(c);
		pthread_mutex_unlock(&mlock);
        return;
    }

    for(; aux!=NULL; aux=aux->next)
        if(aux->next == c)
        {
			pthread_mutex_lock(&mlock);
            aux->next = aux->next->next;            
            free(c);
			pthread_mutex_unlock(&mlock);
            return;
        }
}

nod_client *get_nod(Server *s,ClientInfo *c)
{
    nod_client *aux = s->users.head;
    for(; aux!=NULL; aux=aux->next)
        if(aux->client == c)
            return aux;
    return NULL;
}

void free_file(FileInfo *f)
{
    free(f->file);
    free(f->path);
    free(f->workspace);
    free(f);
    f=NULL;
}

void update_client_file(Server *s, nod_client *c, int type)
{
    nod_client *aux = s->users.head;	
    
    for(;aux != NULL;aux=aux->next)
        if(strcmp(aux->client->file.path,c->client->file.path) == 0 && (aux != c))
        {
            switch(type)
            {
            case 0:
                if(aux->client->file.users == 2)
                    c->client->file.users = -1;
                else
                {
                    aux->client->file.users+=1;
                    c->client->file.users = aux->client->file.users;                   
				}
                return;
            case 1:
                aux->client->file.users--;
                return;
            case 2:

                pthread_mutex_lock(&mlock);
                copy_updatefile(&aux->client->file.update,&c->client->file.update);
                pthread_mutex_unlock(&mlock);
                
                Data data = init_data(UPDATE_CLIENT);
                write_data(aux->client->sd,&data,sizeof(Data),s,c);             
                return;
            }
        }	
    c->client->file.users = 1;
}

int count_connection(Server *s)
{
    nod_client * aux = s->users.head;
    int i = 0;
    if(s->users.head == NULL)
        return 0;
    for(;aux!=NULL;aux=aux->next,i++)
        if(aux->client->currentPos != i)
            return i;
    return i;
}

//FILES
void WriteLock(int fd, off_t size)
{
    struct flock lock;

    lock.l_type = F_WRLCK;
    lock.l_whence = SEEK_CUR;
    lock.l_start  = lseek(fd,0,SEEK_CUR);
    lock.l_len = size;
    lock.l_pid = getpid();

    if(fcntl(fd,F_SETLK,&lock) == -1)
        error_info(ERROR_FCNTL);
}

void ReadLock(int fd, off_t size)
{
    struct flock lock;

    lock.l_type = F_RDLCK;
    lock.l_whence = SEEK_CUR;
    lock.l_start  = lseek(fd,0,SEEK_CUR);
    lock.l_len = size;
    lock.l_pid = getpid();

    if(fcntl(fd,F_SETLKW,&lock) == -1)
        error_info(ERROR_FCNTL);
}

void UnLock(int fd, off_t size)
{
    struct flock lock;

    lock.l_type = F_UNLCK;
    lock.l_whence = SEEK_CUR;
    lock.l_start  = lseek(fd,0,SEEK_CUR);
    lock.l_len = size;
    lock.l_pid = getpid();

    if(fcntl(fd,F_SETLKW,&lock) == -1)
        error_info(ERROR_FCNTL);
}

void update_size(FileInfo *f)
{
    int fd;
    if( (fd=open(f->path,O_RDONLY)) == -1)
        error_info(ERROR_OPEN);
    f->size=lseek(fd,0,SEEK_END);
    close(fd);
}

void update_file(ClientInfo *c)
{
    int fd_new,fd_old;
    char *buff = malloc(sizeof(char) * c->file.size + c->file.update.size +40);
    if(buff == NULL)
    {
        error_info(ERROR_OUT_OF_MEMORY);
        return;
    }
    memset(buff,0,sizeof(char) * c->file.size + c->file.update.size+40);
    strcat(buff,c->file.path);
    strcat(buff,".tmp");
    
    if((fd_new = open(buff,O_CREAT|O_WRONLY|O_TRUNC,0777)) == -1)
    { 
        printf("#####[THREAD %d]#####\n",c->currentPos);    
        error_info(ERROR_OPEN);
        free(buff);
        return;
    }
    if((fd_old = open(c->file.path,O_RDONLY)) == -1)
    {
        printf("#####[THREAD %d]#####\n",c->currentPos);
        error_info(ERROR_OPEN);
        free(buff); 
        return;
    }
    switch(c->file.update.type)
    {
    case REMOVE_FILE:
		
        ReadLock(fd_old,c->file.size);
        
        memset(buff,0,sizeof(char) * c->file.size + c->file.update.size);
        read_data(fd_old,buff,c->file.update.cursor,NULL,NULL);        
        write_data(fd_new,buff,strlen(buff),NULL,NULL);      
       
        memset(buff,0,sizeof(char) * c->file.size + c->file.update.size);     
        lseek(fd_old,c->file.update.size,SEEK_CUR);       
        read_data(fd_old,buff,(c->file.size-c->file.update.cursor-c->file.update.size),NULL,NULL);
        write_data(fd_new,buff,strlen(buff),NULL,NULL);

        lseek(fd_old,0,SEEK_SET);
        UnLock(fd_old,c->file.size);
        break;
    case INSERT_FILE:
        ReadLock(fd_old,c->file.size);
		
        bzero(buff,sizeof(char) * c->file.size + c->file.update.size);      
        read_data(fd_old,buff,(c->file.update.cursor - c->file.update.size),NULL,NULL);
        write_data(fd_new,buff,strlen(buff),NULL,NULL);     
      
        char *buff2 = malloc(sizeof(char) * c->file.update.size +1);
        memset(buff2,0,sizeof(char) * c->file.update.size);
        read_data(c->sd,buff2,c->file.update.size,NULL,NULL);   
        write_data(fd_new,buff2,c->file.update.size,NULL,NULL);        
        free(buff2);
        
        memset(buff,0,sizeof(char) * c->file.size + c->file.update.size);          
        read_data(fd_old,buff,(c->file.size-(c->file.update.cursor - c->file.update.size)) ,NULL,NULL);
        write_data(fd_new,buff,strlen(buff),NULL,NULL);       
    
        lseek(fd_old,0,SEEK_SET);
        UnLock(fd_old,c->file.size);

        break;
	default:		
		return;
    }

    close(fd_new);    
    close(fd_old);

    if(remove(c->file.path) == -1)
        error_info(ERROR_REMOVE);
    memset(buff,0,sizeof(char) * c->file.size + c->file.update.size);
    strcat(buff,c->file.path);
    strcat(buff,".tmp");
    if(rename(buff,c->file.path) == -1)
        error_info(ERROR_RENAME);

    update_size(&c->file);
    if(c->file.users > 1)
        update_client_file(get_server(),get_nod(get_server(),c),2);
}

void read_file(ClientInfo *c)
{
    int fd,pos=0;
    char *buff = malloc(sizeof(char)*c->file.update.size +1);
    memset(buff,0,sizeof(char)*c->file.update.size +1);
    if((fd=open(c->file.path,O_RDONLY)) == -1)
        error_info(ERROR_READ);
    ReadLock(fd,c->file.size);

    pos=lseek(fd,c->file.update.cursor-c->file.update.size,SEEK_SET);
    
    read_data(fd,buff,c->file.update.size,NULL,NULL);
    write_data(c->sd,buff,strlen(buff),NULL,NULL);
}

void update_client(Server *s,nod_client *c)
{    
    write_data(c->client->sd,&c->client->file.update,sizeof(UpdateFile),s,c);
    if(c->client->file.update.type == INSERT_FILE)
        read_file(c->client);        
}

//SERVER HANDLERS
void server_loop(Server *s)
{
    INF_LOOP
    {
        ClientInfo *client = malloc(sizeof(ClientInfo));

        int length = sizeof(client->addr);

        memset(client,0,sizeof(ClientInfo));

        printf("[server] Accepting connection at %d\n",PORT);

        if((client->sd = accept(s->sd,(struct sockaddr*)&s->addr,&length)) == -1)
            error_info(ERROR_ACCEPT);

        client->currentPos = count_connection(s);

        pthread_mutex_lock(&mlock);
        add_user(s,client);
        pthread_mutex_unlock(&mlock);
   
        printf("[server] Creating thread[%d]\n",client->currentPos);		
        pthread_create(&client->thread,NULL,&run,s);
    }
}

void close_connection(Server *server, nod_client *user)
{    
	close(user->client->sd);
   	remove_user(server,user);
}

void info_server(Data data, int user)
{
    switch(data.status)
    {
    case DISCONNECTED:
        printf("[thread %d] closing conection...\n",user);
        return;   
    case GET_FILE:
        printf("[thread %d] sending file...\n",user);
        return;
    case UPDATE_FILE:
        printf("[thread %d] updating file...\n",user);
        return;
    case UPDATE_CLIENT:
        printf("[thread %d] updating client...\n",user);
        return;
    case DOWNLOAD_FILE:
        printf("[thread %d] downloading file...\n",user);
        return;
    case LIST_FILE:
        printf("[thread %d] download files structure...\n",user);
        return;
    case COMPILE_FILE:
        printf("[thread %d] compiling file...\n",user);
    default:
        return;
    }
}

bool read_data(int sd, void *data, size_t size,Server *server,nod_client *user)
{
    if((read(sd,data,size)) == -1)
    {
        printf("socket=%d\n",sd);
        error_info(ERROR_READ);
        if(server && user)
            close_connection(server,user);
        return false;
    }
    return true;
}

bool write_data(int sd, void *data, size_t size, Server *server, nod_client *user)
{    
    if((write(sd,data,size)) == -1)
    {
        error_info(ERROR_WRITE);
        if(server && user)
            close_connection(server,user);
        return false;
    }
    return true;
}

//CLIENT
int send_file(Server *s,nod_client  *c)
{
    file_data filedata;
	Data data = init_data(VALID_FILE);   
    int fd;

    read_data(c->client->sd,&filedata,sizeof(file_data),s,c);

    alloc_fileinfo(&c->client->file,&filedata);

    read_data(c->client->sd,c->client->file.workspace,filedata.l_workspace,s,c);
    read_data(c->client->sd,c->client->file.file,filedata.l_file,s,c);

    set_fileinfo(&(c->client->file));    
    
    update_client_file(s,c,0);    
	
    if(c->client->file.users == -1)
	{
		data = init_data(INVALID_FILE);
		write_data(c->client->sd,&data,sizeof(Data),s,c);
		return -1;
	}	
	
    check_directory(&c->client->file);
    if((fd = open(c->client->file.path,O_RDONLY)) == -1)    
    {
        data = init_data(CREATE_FILE);
        write_data(c->client->sd,&data,sizeof(Data),s,c);
        if((fd = open(c->client->file.path,O_CREAT,0777) == -1))
            error_info(ERROR_OPEN);  
        return 0;
    }  
    else
    {        
        int size_file = lseek(fd,0,SEEK_END);
        c->client->file.size = size_file;
        char *buff =malloc(sizeof(char)*size_file + 1);
        bzero(buff,size_file+1);
        
        write_data(c->client->sd,&data,sizeof(Data),s,c);
        write_data(c->client->sd,&size_file,sizeof(int),s,c);

        lseek(fd,0,SEEK_SET);

        read_data(fd,buff,size_file,s,c);

        write_data(c->client->sd,buff,size_file,s,c);

        close(fd);
        free(buff);
    }
	return 0;
}

void alloc_fileinfo(FileInfo *f, file_data *d)
{
    int file=d->l_file,work=d->l_workspace;

    f->file=malloc(sizeof(char)*file+1);
    f->workspace = malloc(sizeof(char)*work+1);
    f->path = malloc(sizeof(char)*(file*work)+8);

    memset(f->file,0,file+1);
    memset(f->path,0,work*file+8);
    memset(f->workspace,0,work+1);

    f->users = 0;
}

void set_fileinfo(FileInfo *f)
{
    strcat(f->path,"FILES/");
    strcat(f->path,f->workspace);
    strcat(f->path,"/");
    strcat(f->path,f->file);
}

void check_directory(FileInfo *file)
{	
    char buff[40] = "FILES/";
    strcat(buff,file->workspace);
    if(access(buff,F_OK) != 0)
        if(ENOENT == errno || ENOTDIR == errno)
            if(mkdir(buff,0777) == -1)
                error_info(ERROR_MKDIR);
}

unsigned long get_size(const char *name)
{   
    struct stat st;
    if(stat(name,&st) == 0)
        return st.st_size;
    return 0;
}

void listdir(const char *name,int level,F_Type *type,int sd)
{
    DIR *dir;   
    struct dirent *entry;        
    if (!(dir = opendir(name)))
        return;
    if (!(entry = readdir(dir)))
        return; 
    do 
    {             
        if (entry->d_type == DT_DIR) 
        {
            char path[1024];
            int len = snprintf(path, sizeof(path)-1, "%s/%s", name, entry->d_name);
            path[len] = 0;
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
                continue;
            type->type = F_DIR;
            type->level = level;                      
            strcpy(type->name,entry->d_name);  
            strcpy(type->path,name);          
            write_data(sd,type,sizeof(F_Type),NULL,NULL);
            listdir(path, level + 1,type,sd);
        }
        else
        {            
            type->type = F_FILE;
            type->level = level;         
            type->size = get_size(name);            
            strcpy(type->name,entry->d_name);
            strcpy(type->path,name);   
            write_data(sd,type,sizeof(F_Type),NULL,NULL);          
        }
    } while (entry = readdir(dir));
    closedir(dir);    
}

void download_file(int sd)
{
    int size=0,fd;
    char *path;
    read_data(sd,&size,sizeof(int),NULL,NULL);
    path = malloc(sizeof(char)*(size+1));
    read_data(sd,path,size,NULL,NULL);
    path[size] = 0;
    
    if((fd = open(path,O_RDONLY)) == -1)
    {
        free(path);
        error_info(ERROR_READ);
        size = -1;
        write_data(sd,&size,sizeof(int),NULL,NULL);
        return;
    }

    size=lseek(fd,0,SEEK_END);
    write_data(sd,&size,sizeof(int),NULL,NULL);
    
    lseek(fd,0,SEEK_SET);
    free(path);
    path = malloc(sizeof(char)*(size+10));
    
    read_data(fd,path,size,NULL,NULL);
    write_data(sd,path,size,NULL,NULL);
    
    free(path);
    close(fd);
}

void compile_file(ClientInfo *c)
{
    int size = strlen(c->file.file),fd,pid;
    char *path = malloc(sizeof(char)+strlen(c->file.file) + 50);
    char *exec;

    strcpy(path,"compiler/");
    strcat(path,c->file.file);
    exec = malloc(sizeof(char) * strlen(path)+1);
    strcpy(exec,path);
    if(strcmp(&exec[strlen(exec)-2],".c") == 0)   
        exec[strlen(exec)-2] = 0;   
    else exec[strlen(exec)-4] = 0;

    strcat(path,".error");    
    
    if(strcmp(&c->file.file[size-2],".c") == 0)
    {   
        if((fd = open(path,O_CREAT | O_RDWR,0755)) == -1)
        {          
            free(exec);
            free(path);
            error(ERROR_OPEN);    
            close(fd);        
            return;
        }
        switch(pid=fork())
        {
        case -1:
            free(exec);
            free(path);
            error(ERROR_FORK);   
            close(fd);        
            return;
        case 0:            
            dup2(fd,2);             
            execlp("gcc","gcc",c->file.path,"-o",exec,NULL);
        default:   
            wait(&pid);
            size = 0;
            lseek(fd,0,SEEK_SET);
            size = lseek(fd,0,SEEK_END);
            if(size == 0)
            {
                Data data = init_data(COMPILE_SUCCED);
                write_data(c->sd,&data,sizeof(Data),NULL,NULL);
                read_data(c->sd,&data,sizeof(Data),NULL,NULL);
                if(data.status == GET_FILE)
                {
                    int f;
                    char *buffer;
                    if((f = open(exec,O_RDONLY)) == -1)
                    {
                        free(exec);
                        free(path);
                        error(ERROR_OPEN);
                        return;
                    }
                    size = lseek(f,0,SEEK_END);
                    buffer = malloc(sizeof(char) * size);

                    write_data(c->sd,&size,sizeof(int),NULL,NULL);

                    lseek(f,0,SEEK_SET);                   

                    read_data(f,buffer,size,NULL,NULL);
                    write_data(c->sd,buffer,size,NULL,NULL);
                    
                    free(buffer);
                    close(f);
                }
            }
            else
            {
                Data data = init_data(COMPILE_FAILED);
                char *buffer = malloc(sizeof(char) *size);

                write_data(c->sd,&data,sizeof(Data),NULL,NULL);
                write_data(c->sd,&size,sizeof(int),NULL,NULL);
                read_data(fd,buffer,size,NULL,NULL);
                write_data(c->sd,buffer,size,NULL,NULL);
    
                free(buffer);
            }
        }
    }
    else
    {
        if((fd = mkfifo(path,O_CREAT | O_RDWR)) == -1)
        {
            free(exec);
            free(path);
            error(ERROR_MKFIFO);            
            return;
        }
        switch(pid=fork())
        {
        case -1:
            free(exec);
            free(path);
            error(ERROR_FORK);     
            close(fd);      
            return;
        case 0:
            dup2(fd,2);
            execlp("g++","g++",c->file.path,"-o",exec,NULL);
        default:   
            wait(&pid);
            size = 0;
            lseek(fd,0,SEEK_SET);
            size = lseek(fd,0,SEEK_END);
            if(size == 0)
            {
                Data data = init_data(COMPILE_SUCCED);
                write_data(c->sd,&data,sizeof(Data),NULL,NULL);
                read_data(c->sd,&data,sizeof(Data),NULL,NULL);
                if(data.status == GET_FILE)
                {
                    int f;
                    char *buffer;
                    if((f = open(exec,O_RDONLY)) == -1)
                    {
                        free(exec);
                        free(path);
                        error(ERROR_OPEN);
                        return;
                    }
                    size = lseek(f,0,SEEK_END);
                    buffer = malloc(sizeof(char) * size);

                    write_data(c->sd,&size,sizeof(int),NULL,NULL);

                    lseek(f,0,SEEK_SET);                   

                    read_data(f,buffer,size,NULL,NULL);
                    write_data(c->sd,buffer,size,NULL,NULL);
                    
                    close(f);
                    free(buffer);
                }
            }
            else
            {
                Data data = init_data(COMPILE_FAILED);
                char *buffer = malloc(sizeof(char) *size);

                write_data(c->sd,&data,sizeof(Data),NULL,NULL);
                write_data(c->sd,&size,sizeof(int),NULL,NULL);
                read_data(fd,buffer,size,NULL,NULL);
                write_data(c->sd,buffer,size,NULL,NULL);
    
                free(buffer);
            }
        }
    }
    remove(path);
    remove(exec);
    free(path);
    free(exec);    
    close(fd);
}

void error_info(error e)
{
    switch(e)
    {
        case ERROR_MKDIR:
            printf("[server] Error on mkdir():%d\n",errno);
            return;
        case ERROR_OPEN:
            printf("[server] Error on open():%d\n",errno);
            return;
        case ERROR_READ:
            printf("[server] Error on read():%d\n",errno);
            return;
        case ERROR_WRITE:
            printf("[server] Error on write():%d\n",errno);
            return;
        case ERROR_SOCKET:
            printf("[server] Error on socket():%d\n",errno);
            exit(ERROR_SOCKET);
        case ERROR_BIND:
            printf("[server] Error on bind():%d\n",errno);
            exit(ERROR_BIND);
        case ERROR_LISTEN:
            printf("[server] Error on listen():%d\n",errno);
            exit(ERROR_LISTEN);
        case ERROR_ACCEPT:
            printf("[server] Error on accept():%d\n",errno);
            return;
        case ERROR_RENAME:
            printf("[server] Error on rename():%d\n",errno);
            return;
        case ERROR_REMOVE:
            printf("[server] Error on remove():%d\n",errno);
            return;
        case ERROR_OUT_OF_MEMORY:
            printf("[server] Error on malloc():%d\n",errno);
            exit(ERROR_OUT_OF_MEMORY);
        case ERROR_FORK:
            printf("[server] Error on fork():%d\n",errno);
            return;
        case ERROR_MKFIFO:
            printf("[server] Error on mkfifo()%d\n",errno);
            return;
    }
    return;
}

static void *run(void * arg)
{	
    Data data;	
    F_Type type;
    nod_client *nod = ((Server*)arg)->users.curr;	

    pthread_detach(pthread_self());	     
	
    INF_LOOP
    {		
        if(!read_data(nod->client->sd,&data,sizeof(Data),(Server*)arg,nod))
            return NULL; 

        info_server(data,nod->client->currentPos);

        switch(data.status)
        {        
        case DISCONNECTED:            
            close_connection((Server*)arg,nod);
            return NULL;       
        case GET_FILE:  
            if(send_file((Server*)arg,nod) == -1)
			{
				close_connection((Server*)arg,nod);
				return NULL;
			}
            break;
        case LIST_FILE:            
            memset(&type,0,sizeof(F_Type));
            listdir("FILES",0,&type,nod->client->sd);
            type.type = -222;
            write_data(nod->client->sd,&type,sizeof(F_Type),(Server*)arg,nod);
            break;
        case DOWNLOAD_FILE:
            download_file(nod->client->sd);
            break;
        case UPDATE_FILE:					
            read_data(nod->client->sd,&nod->client->file.update,sizeof(UpdateFile),(Server*)arg,nod);		           
            update_file(nod->client);            
            break; 
        case UPDATE_CLIENT:
            update_client((Server*)arg,nod);
            break;
        case COMPILE_FILE:
            compile_file(nod->client);
            break;
        }
    }
}
