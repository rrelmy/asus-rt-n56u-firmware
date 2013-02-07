#ifndef LIST_H
#define LIST_H

#define ADD_TREE_NODE    1
#define DEL_TREE_NODE    2

#define MAX_LENGTH 128
#define MIN_LENGTH 32
#define MAX_CONTENT 256
#define MAXDATASIZE 1024
#define MINSIZE 64

#define getb(type) (type*)malloc(sizeof(type))

typedef struct ATTR
{
    char *lastaccesstime;
    char *creationtime;
    char *lastwritetime;
}Attr;

struct TreeNode
{
    char FilePath[256];
    char FileName[256];
    int level;
    int isfolder;
    long long size;
    Attr *pattr;
    struct TreeNode *Child;
    struct TreeNode *NextBrother;
};

typedef struct TreeNode Hb_TreeNode;

Hb_TreeNode *LocalRootNode;

struct tree_stack
{
    Hb_TreeNode *point;
    struct tree_stack *next;
};

typedef struct tree_stack s_tree;

Hb_TreeNode *create_tree_rootnode(const char *path);
void FindDir(Hb_TreeNode* TreeNode,const char *path);
int modify_tree_node(char *fullname, Hb_TreeNode *rootnode,int type);
Hb_TreeNode *get_tree_node(const char *filename,Hb_TreeNode *rootnode);
void free_tree_node(Hb_TreeNode *rootnode);
void SearchTree(Hb_TreeNode* treeRoot);
void write_tree_to_file(const char *logname,Hb_TreeNode *treeRoot);
Hb_TreeNode *read_file_to_tree(const char *logname);
void SearchTree1(Hb_TreeNode* treeRoot);
void rename_update_tree(const char *oldname,const char *newname);

void print_stree();
void push_stree(Hb_TreeNode *node);
Hb_TreeNode *pop_stree();

/*server item tree struct and function*/
typedef struct node{
    char *href;
    char creationdate[MAX_LENGTH];
    char getcontentlanguage[MIN_LENGTH];
    char getcontentlength[MIN_LENGTH];
    char getcontenttype[MIN_LENGTH];
    char getlastmodified[MAX_LENGTH];
    char status[MAX_LENGTH];
    int isFolder;
    time_t modtime;
    struct node *next;
}CloudFile;
//CloudFile *FolderList;
//CloudFile *FileList;
//CloudFile *FolderTail;
//CloudFile *FileTail;
CloudFile *FolderCurrent;
CloudFile *FolderTmp;
//CloudFile *OldFolderList;
//CloudFile *OldFileList;
CloudFile *FileList_one;
CloudFile *FileTail_one;
CloudFile *FileTmp_one;
CloudFile *TreeFolderList;
CloudFile *TreeFileList;
CloudFile *TreeFolderTail;
CloudFile *TreeFileTail;

typedef struct BROWSE
{
    int foldernumber;
    int filenumber;
    CloudFile *folderlist;
    CloudFile *filelist;
}Browse;


struct ServerTreeNode
{
    int level;
    char *parenthref;
    Browse *browse;
    struct ServerTreeNode *Child;
    struct ServerTreeNode *NextBrother;
};

typedef struct ServerTreeNode Server_TreeNode;

Server_TreeNode *ServerRootNode;
Server_TreeNode *OldServerRootNode;

Server_TreeNode *create_server_treeroot();
int browse_to_tree(char *parenthref,Server_TreeNode *node,int index);
CloudFile *get_CloudFile_node(Server_TreeNode* treeRoot,const char *dofile_href,int a);
void SearchServerTree(Server_TreeNode* treeRoot);
void free_server_tree(Server_TreeNode *node);


/*Local item struct and function of every floor*/
typedef struct LOCALFOLDER{
    char *path;
    char name[MAX_CONTENT];
    struct LOCALFOLDER *next;
}LocalFolder;

typedef struct LOCALFILE{
    char *path;
    char name[MAX_CONTENT];
    char creationtime[MINSIZE];
    char lastaccesstime[MINSIZE];
    char lastwritetime[MINSIZE];
    long long size;
    struct LOCALFILE *next;
}LocalFile;

//LocalFile *LocalFileList;
//LocalFolder *LocalFolderList;
//LocalFolder *LocalFolderTmp;
//LocalFile *LocalFileTmp;
//LocalFolder *LocalFolderTail;
//LocalFile *LocalFileTail;
//LocalFolder *LocalFolderCurrent;
//LocalFile *SavedLocalFileList;


typedef struct LOCAL
{
    int foldernumber;
    int filenumber;
    LocalFolder *folderlist;
    LocalFile *filelist;
}Local;

Local *Find_Floor_Dir(const char *path);
void free_localfloor_node(Local *local);


#endif



