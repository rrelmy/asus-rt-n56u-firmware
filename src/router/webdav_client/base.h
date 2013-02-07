#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <time.h>
#include <dirent.h>
#include <unistd.h>
#include <netinet/in.h>
#include <ne_request.h>
#include <ne_auth.h>
#include <utime.h>
#include <sys/statvfs.h>
#include <signal.h>
#include <pthread.h>
#include <webdav_base.h>
#include <libxml/tree.h>
#include <libxml/parser.h>
#include <ne_uri.h>
#include <ne_xml.h>
#include <ne_string.h>
#include <ne_session.h>
#include <ne_request.h>
#include "list.h"
//#include <sys/time.h>


pthread_t newthid1,newthid2,newthid3;
pthread_cond_t cond,cond_socket,cond_log;
pthread_mutex_t mutex,mutex_socket,mutex_receve_socket,mutex_log;

//ne_session *sess;

#define UPLOAD 1
#define DOWNLOAD 2
#define MKCOL 3
#define MKCOL_PARENT 4

#define INOTIFY_PORT 3678

#define NO_PARENT 427

#define DEBUG 1
#define LIST_TEST 1
//#define DEVICE "/RT-N16/ASD/0417"
//char ROOTURL[MAX_CONTENT];
//char ROOTFOLDER[256];
//int rootfolder_len;
//char HOST[256];
//int create_folder_cmp;

//char mount_path[MAX_LENGTH];
char log_base_path[MAX_LENGTH];
char log_path[MAX_LENGTH];
//char temp_path[MAX_LENGTH];
char up_item_file[MAXDATASIZE];
char down_item_file[MAXDATASIZE];
//char all_local_item_file[MAX_CONTENT];
//char all_local_item_folder[MAX_CONTENT];
//char local_action_file[MAX_CONTENT];
char general_log[MAX_CONTENT];
//char base_path[512];     //base_path is the sync path
//int base_path_len;
//char sync_path[256];
//char wd_username[32];
//char wd_password[32];
char base64_auth[256];
long long int DiskAvailable;
long long int DiskAvailableShow;
long long int DiskUsedShow;
long long int PreDiskAvailableShow;
long long int PreDiskUsedShow;

#define ABORT (-42) /* magic code for abort handlers */
#define NE_XML_DECLINE (0)
#define NE_XML_ABORT (-1)
#define EVAL_DEFAULT "eval-xmlns-default"
#define EVAL_SPECIFIC "eval-xmlns-specific-"
struct context {
    ne_buffer *buf;
    ne_xml_parser *parser;
};


/* log struct */
typedef struct LOG_STRUCT{
    uint8_t  status;
    char  error[512];
    float  progress;
    char path[512];
}Log_struc;

#define S_INITIAL		70
#define S_SYNC			71
#define S_DOWNUP		72
#define S_UPLOAD		73
#define S_DOWNLOAD		74
#define S_STOP			75
#define S_ERROR			76
#define LOG_SIZE                sizeof(struct LOG_STRUCT)

#define LOCAL_SPACE_NOT_ENOUGH          900
#define SERVER_SPACE_NOT_ENOUGH         901
#define LOCAL_FILE_LOST                 902
#define SERVER_FILE_DELETED             903
#define COULD_NOT_CONNECNT_TO_SERVER    904
#define CONNECNTION_TIMED_OUT           905
#define INVALID_ARGUMENT                906
#define PARSE_XML_FAILED                907
#define COULD_NOT_READ_RESPONSE_BODY    908
#define HAVE_LOCAL_SOCKET               909
#define SERVER_ROOT_DELETED             910

int sync_up;
int sync_down;
int download_only;
int upload_only;
int finished_initial;
//int receve_socket;
int local_sync;
int server_sync;
int first_sync;
//int dounfinish;

ne_off_t prog_total;
time_t prog_time;

//#define NVRAM_
//#define MUTI_DIR

#ifndef NVRAM_
//#define CONFIG_PATH "/opt/etc/Cloud.conf"      //for N16
//#define CONFIG_PATH "/home/alan/tool/neon/neon-0.29.6/my_test/webdav.conf"   //for PC
#define CONFIG_PATH "/tmp/Cloud/webdav.conf"
#else
#define CONFIG_PATH "/tmp/Cloud/webdav.conf"
#endif

//#ifndef NVRAM_
//#define CONFIG_PATH "/opt/etc/Cloud.conf"      //for N16
//#define CONFIG_PATH "/home/alan/tool/neon/neon-0.29.6/my_test/Cloud.conf"   //for PC
//#endif

struct sync_item
{
    int action;
    char href[256];
    struct sync_item *next;
};

//struct sync_item *down_head;
//struct sync_item *up_head;
//struct sync_item *mkcol_head;
//struct sync_item *mkcol_parent_head;

typedef struct Action_Item
{
    char *action;
    char *path;
    struct Action_Item *next;
} action_item;
//head has no content
action_item *server_action_list;    //Server变化导致的Socket
action_item *dragfolder_action_list;   //dragfolder时递归动作造成的Socket重复
action_item *unfinished_list;
action_item *download_only_socket_head;

/*typedef struct local_action{
    char buf[MAXDATASIZE];
    struct local_action *next;
}Local_Action;
Local_Action *LocalActionList;
Local_Action *LocalActionTail;
Local_Action *LocalActionTmp;*/

struct queue_entry
{
    struct queue_entry * next_ptr;   /* Pointer to next entry */
    //int type;
    //char filename[256];
    char cmd_name[1024];
    //int id;
    //long long int size;
};
typedef struct queue_entry * queue_entry_t;

/*struct queue*/
struct queue_struct
{
    struct queue_entry * head;
    struct queue_entry * tail;
};
typedef struct queue_struct *queue_t;

queue_t SocketActionList;
queue_entry_t SocketActionTmp;

/*muti dir read config*/
struct asus_rule
{
    int rule;
    char base_path[512];     //base_path is the sync path
    int base_path_len;
    char rooturl[MAX_CONTENT];
    char rootfolder[256];
    char host[128];
    int rootfolder_length;
    char mount_path[MAX_LENGTH];
    char temp_path[MAX_LENGTH];
    ne_session *sess;
};

struct asus_config
{
    int type;
    //char wd_username[32];
    //char wd_password[32];
    //char url[32];
    int  enable;
    //int ismuti;
    int dir_number;
    struct asus_rule **prule;
};
struct asus_config asus_cfg;

typedef struct SYNC_LIST{

    //char base_path[512];
    char up_item_file[256];
    char down_item_file[256];
    //char *up_item_file;
    //char *down_item_file;
    //char rooturl[MAX_CONTENT];
    //char rootfolder[256];
    //char host[128];
    //int rootfolder_length;
    //int base_path_len;
    int sync_up;
    int sync_down;
    int download_only;
    int upload_only;
    int first_sync;
    int receve_socket;
    int have_local_socket;
    int init_completed;
    int no_local_root;
    //int local_sync;
    //int server_sync;
    action_item *copy_file_list;         //The copy files
    action_item *server_action_list;    //Server变化导致的Socket
    action_item *dragfolder_action_list;   //dragfolder时递归动作造成的Socket重复
    action_item *unfinished_list;
    action_item *download_only_socket_head;
    action_item *up_space_not_enough_list;
    queue_t SocketActionList;
    Server_TreeNode *ServerRootNode;
    Server_TreeNode *OldServerRootNode;
    int index;
}sync_list;
sync_list **g_pSyncList;

/*typedef struct thread_state_tag
{
    int server_sync_run_times;
    int socket_parse_run_times;
}thread_state;*/

char *change_local_same_name(char *fullname);
char *change_server_same_name(char *fullname,int index);
char *change_server_same_name_root(char *filename);
int Download(char *dofile_href,int index);
int Upload(char *filepath,int index);
int Delete(char *filepath,int index);
int Mkcol(char *folderpath,int index);
//int Mkcol_Root(char *serverpath);
time_t Getmodtime(char *serverhref,int index);
long long int GetContentLength(char *serverhref,int index);
int is_Server_Copying(char *serverhref,int index);
int Move(char *oldpath,char *newpath,int index);
//int Move_Root(char *oldpath,char *newpath);
//static void *xmalloc_fatal(size_t size);
void *xmalloc (size_t size);
char *xstrdup (const char *s);
void *xrealloc (void *ptr, size_t size);
char *oauth_url_escape(const char *string);
char *oauth_url_unescape(const char *string, size_t *olen);
int parse_config(char *path);
int getServerList();
//int getLocalList();
int send_action(int type, char *content);
void *SyncLocal();
void *SyncServer();
int parse_config(char *path);
void init_global_var();
void clear_global_var();
int cmd_parser(char *cmd,int index);
int test_if_dir(const char *dir);
int test_if_dir_empty(char *dir);
int wait_file_copying(char *filename);
unsigned long stat_file(const char *filename);
int createFolder(char *path,int index);
int compareServerList(int index);
int compareLocalList(int index);
void read_config();
int my_auth(void *userdata, const char *realm, int attempt,char *username, char *password);
//int getLocalInfo(char *dir);
int ChangeFile_modtime(char *filepath,time_t servermodtime);
int compareTime(time_t servermodtime ,char *localpath);
void wd_escape(char *unescapepath,char *escapepath);
int getCloudInfo(char *URL,void (*cmd_data)(xmlDocPtr, xmlNodePtr),int index);
//int chardata_one(void *userdata, int state, const char *cdata, size_t len);
void parseCloudInfo_one(xmlDocPtr doc, xmlNodePtr cur);
void del_all_items(char *dir,int index);
//void get_file_parent(char *filehref,char *URL);
//void get_file_parent_base_path(char *filehref,char *URL);
//void get_file_parent_local(char *filehref,char *URL);
//void Upload_ChangeFile_modtime(char *URL,char *localpath);
//time_t GetServer_modtime(char *URL,char *localpath);
//time_t GetList_modtime(char *localpath);
time_t GetFile_modtime(char *localpath);
int is_file_copying(char *filename);
long long int get_local_freespace(int index);
//static void s_progress(void *userdata, ne_off_t prog, ne_off_t total);
void sig_handler (int signum);
void free_CloudFile_item(CloudFile *head);
void free_LocalFolder_item(LocalFolder *head);
void free_LocalFile_item(LocalFile *head);
//void free_LocalAction_item(Local_Action *head);
int write_log(int status, char *message, char *filename,int index);
int add_sync_item(int action,char *item_href,struct sync_item *head);
int del_sync_item(int action,char *item_href,struct sync_item *head);
int item_empty(struct sync_item *phead);
//void print_sync_item(struct sync_item *phead,int type);
//void print_LocalAction_item();
//void print_all_local_item(char *sync_item_filename,int type);
void free_sync_item(struct sync_item *head);
//void remove_tmp(char *dofile_href);
//int parse_trans_item(char *path,int type);
int get_all_local_file_item(char *path,LocalFile *head);
//int wd_initial_compareTime(time_t servermodtime ,char *localpath,LocalFile *head);
//void chang_filename(char *newpath,char *oldpath);
int wd_create_session(int j);
int do_unfinished(int index);
//int is_have_parent(char *leafpath,char *serverpath);
int strnicmp(const char *s1, const char *s2, size_t len);
//int Mkcol_parent();
int add_FolderList_item(CloudFile *head,char *item_href);
int queue_empty (queue_t q);
queue_t queue_create ();
void queue_enqueue (queue_entry_t d, queue_t q);
queue_entry_t queue_dequeue (queue_t q);
void queue_destroy (queue_t q);
int is_copying_finished(char *filename);
Browse *browseFolder(char *URL,int index);
int initMyLocalFolder(Server_TreeNode *servertreenode,int index);
char *serverpath_to_localpath(char *from_serverpath,int index);
char *localpath_to_serverpath(char *from_localpath,int index);
int add_action_item(const char *action,const char *path,action_item *head);
int add_all_download_only_socket_list(char *cmd,const char *dir,int index);
void free_action_item(action_item *head);
action_item *get_action_item(const char *action,const char *path,action_item *head,int index);
int del_action_item(const char *action,const char *path,action_item *head);
void del_download_only_action_item(const char *action,const char *path,action_item *head);
int add_all_download_only_dragfolder_socket_list(const char *dir,int index);
action_item *create_action_item_head();
int is_local_space_enough(CloudFile *do_file,int index);
char *my_str_malloc(size_t len);
int test_if_download_temp_file(char *filename);
void my_mkdir(char *path);
void my_mkdir_r(char *path);
int newer_file(char *localpath,int index);
char *my_nstrchr(const char chr,char *str,int n);
void handle_quit_upload();
int wait_handle_socket(int index);
//int compare_file_size(CloudFile *filetmp);
int moveFolder(char *old_dir,char *new_dir,int index);
int is_server_exist(char *parenthref,char *filepath,int index);
int is_server_exist_with_type(char *filepath);
void my_base64_encode();
char *parse_name_from_path(const char *path);
int get_create_threads_state();
int ne_getrouterinfo(void (*cmd_data)(xmlDocPtr, xmlNodePtr),int index);
void parseRouterInfo(xmlDocPtr doc, xmlNodePtr cur);


