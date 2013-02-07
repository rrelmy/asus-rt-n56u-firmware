#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include "base.h"

#if 0
#include <mcheck.h>
#endif

#ifdef NVRAM_
#include <bcmnvram.h>
#include <shutils.h>
#endif

//#define CONFIG_PATH "/opt/etc/Cloud.conf"      //for N16 test
#define MYPORT 3569
#define BACKLOG 100 /* max listen num*/

#define OTRHRS 0
#define HREF 1
#define CREATIONDATE 2
#define GETCONTENTLANGUAGE 3
#define GETCONTENTLENGTH 4
#define GETCONTENTTYPE 5
#define GETLASTMODIFIED 6
#define STATUS 7


int infoType;
int stop_up = 0;
int stop_down = 0 ;
//int exit_proc = 0;
int no_config = 0;

int add_port_for_host(int index,struct asus_config *config){

    char *temp;
    const char chr = ':';

    temp = my_nstrchr(chr,config->prule[index]->host,2);
    if(temp != NULL)      //HOST already has port
    {
        return 0;
    }
    else                //HOST has no port
    {
        temp = strstr(config->prule[index]->host,"https");
        if(temp == NULL)     //HOST start with http
        {
            sprintf(config->prule[index]->host,"%s:8082",config->prule[index]->host);
        }
        else                 //HOST start with https
        {
            sprintf(config->prule[index]->host,"%s:443",config->prule[index]->host);
        }
    }

    return 0;
}

void parse_server_url(char *pstr,int i,struct asus_config *config){

    char *temp;
    char *p;
    int n;
    char *to_serverpath = NULL;
    const char chr = '/';
    const char *split = "/";

    //memset(ROOTURL,0,sizeof(ROOTURL));
    //memset(ROOTFOLDER,0,sizeof(ROOTFOLDER));
    //memset(HOST,0,sizeof(HOST));

    to_serverpath = (char *)malloc(sizeof(char));
    memset(to_serverpath,'\0',sizeof(to_serverpath));

    printf("pstr = %s\n",pstr);

    temp = my_nstrchr(chr,pstr,3);

    printf("temp = %s\n",temp);

    strncpy(config->prule[i]->host,pstr,strlen(pstr)-strlen(temp));

    add_port_for_host(i,config);

    printf("config->prule[%d]->host = %s\n",i,config->prule[i]->host);

    temp = temp + 1;

    printf("temp1 = %s\n",temp);

    p=strtok(temp,split);
    while(p!=NULL)
    {
        p = oauth_url_escape(p);
        //block_letter_to_lower_case(p);
        n = strlen(p);
        //printf("p = %s, n = %d\n",p,n);
        to_serverpath = realloc(to_serverpath, strlen(to_serverpath)+n+2);
        //printf("to_serverpath = %s\n",to_serverpath);
        sprintf(to_serverpath,"%s/%s",to_serverpath,p);

        free(p);
        p=strtok(NULL,split);
    }


    sprintf(config->prule[i]->rooturl,"%s%s",config->prule[i]->host,to_serverpath);
    sprintf(config->prule[i]->rootfolder,"%s",to_serverpath);
    config->prule[i]->rootfolder_length = strlen(config->prule[i]->rootfolder);

    free(to_serverpath);
    printf("config->prule[%d]->rooturl = %s\nconfig->prule[%d]->rootfolder = %s\nconfig->prule[%d]->host = %s\n",
           i,config->prule[i]->rooturl,i,config->prule[i]->rootfolder,i,config->prule[i]->host);
    //strcpy(config->prule[i]->host,HOST);
    //strcpy(config->prule[i]->rootfolder,ROOTFOLDER);
    //strcpy(config->prule[i]->rooturl,ROOTURL); 
}

void parse_mount_path(char *p,int i,struct asus_config *config){

    char *temp;
    const char chr = '/';

    temp = my_nstrchr(chr,p,3);

    //strncpy(mount_path,p,strlen(p)-strlen(temp));
    strncpy(config->prule[i]->mount_path,p,strlen(p)-strlen(temp));
    printf("config->prule[%d]->mount_path = %s\n",i,config->prule[i]->mount_path);
}

/*int parse_config(char *path)
{
    printf("#####parse_config####\n");
    int type;
    int status;
    int rule;
    FILE *fp;
    //DIR *dir;

    char buffer[256];
    const char *split = ",";
    char *p;

    memset(buffer, '\0', sizeof(buffer));

    if (access(path,0) == 0)
    {
        if(( fp = fopen(path,"rb"))==NULL)
        {
            fprintf(stderr,"read Cloud error");
        }
        while(fgets(buffer,256,fp)!=NULL)
        {
            p=strtok(buffer,split);
            int i=0;
            while(p!=NULL)
            {
                switch (i)
                {
                case 0 :
                    type = atoi(p);
                    if( type != 2)
                        return 1;
                    break;
                case 1:
                    status = atoi(p);
                    if(status != 1)
                        return 1;
                    break;
                case 2:
                    strcpy(wd_username,p);
                    break;
                case 3:
                    strcpy(wd_password,p);
                    break;
                case 4:
                    //printf("p = %s\n",p);
                    //strcpy(HOST,p);
                    parse_server_url(p);
                    break;
                case 5:
                    rule = atoi(p);
                    //printf("rule = %d\n",rule);
                    if( rule == 0 )
                    {
                        sync_up = 1;
                        sync_down = 1;
                    }
                    else if( rule == 1)
                    {
                        sync_up = 1;
                        sync_down = 0;
                        download_only = 1;
                    }
                    else if( rule == 2)
                    {
                        sync_up = 0;
                        sync_down = 1;
                        upload_only = 1;
                    }

                    break;
                 case 6:
                    strcpy(base_path,p);
                    if( base_path[ strlen(base_path)-1 ] == '\n' )
                        base_path[ strlen(base_path)-1 ] = '\0';
                    parse_mount_path(base_path);
                    break;
                     default:
                    break;
                }

                i++;
                p=strtok(NULL,split);
            }


        }
        //printf("sync_up = %d\nsync_down = %d\n",sync_up,sync_down);
        fclose(fp);

        my_mkdir_r(base_path);
    }
    //printf("type is %d,status is %d,username is %s,password is %s,rule is %d,base_path is %s\n",
    //type,status,username,password,rule,base_path);
    return 0;
}*/


//Used for Single Directory
#if 0
int parse_config(char *path)
{
    printf("#####parse_config####\n");
    int type;
    int status;
    int rule;
    FILE *fp;
    //DIR *dir;

    char buffer[256];
    //const char *split = ",";
    char *p;

    memset(buffer, '\0', sizeof(buffer));

    if (access(path,0) == 0)
    {
        if(( fp = fopen(path,"rb"))==NULL)
        {
            fprintf(stderr,"read Cloud error");
        }
        int i=0;
        while(fgets(buffer,256,fp)!=NULL)
        {
            if( buffer[ strlen(buffer)-1 ] == '\n' )
                buffer[ strlen(buffer)-1 ] = '\0';
            p=buffer;
            switch (i)
            {
            case 0 :
                type = atoi(p);
                if( type != 1)
                    return 1;
                break;
            case 1:
                status = atoi(p);
                if(status != 1)
                    return 1;
                break;
            case 2:
                strcpy(wd_username,p);
                break;
            case 3:
                strcpy(wd_password,p);
                break;
            case 4:
                //printf("p = %s\n",p);
                //strcpy(HOST,p);
                parse_server_url(p);
                break;
            case 5:
                rule = atoi(p);
                //printf("rule = %d\n",rule);
                if( rule == 0 )
                {
                    sync_up = 1;
                    sync_down = 1;
                }
                else if( rule == 1)
                {
                    sync_up = 1;
                    sync_down = 0;
                    download_only = 1;
                }
                else if( rule == 2)
                {
                    sync_up = 0;
                    sync_down = 1;
                    upload_only = 1;
                }

                break;
            case 6:
                strcpy(base_path,p);
                /*if( base_path[ strlen(base_path)-1 ] == '\n' )
                    base_path[ strlen(base_path)-1 ] = '\0';*/
                parse_mount_path(base_path);
                break;
                     default:
                break;

            }
            i++;
        }
        //printf("sync_up = %d\nsync_down = %d\n",sync_up,sync_down);
        fclose(fp);

        my_mkdir_r(base_path);
    }
    //printf("type is %d,status is %d,username is %s,password is %s,rule is %d,base_path is %s\n",
    //type,status,username,password,rule,base_path);
    return 0;
}
#endif

//old parse_config_mutidir.Used for one Server folder and several Local folders
#if 0
int parse_config_mutidir(char *path,struct asus_config *config)
{
    printf("#####parse_config####\n");
    FILE *fp;
    //DIR *dir;

    char buffer[256];
    char *p;

    int i = 0;
    int k = 0;
    int len = 0;

    memset(buffer, '\0', sizeof(buffer));

    if (access(path,0) == 0)
    {
        if(( fp = fopen(path,"rb"))==NULL)
        {
            fprintf(stderr,"read Cloud error");
        }
        while(fgets(buffer,256,fp)!=NULL)
        {
            if( buffer[ strlen(buffer)-1 ] == '\n' )
                buffer[ strlen(buffer)-1 ] = '\0';
            p=buffer;
            if(i == 0)
            {
                config->type = atoi(p);
                printf("config->type = %d\n",config->type);
                if( config->type != 2)
                    return 1;
            }
            else if(i == 1)
            {
                strcpy(wd_username,p);
                printf("wd_username = %s\n",wd_username);
            }
            else if(i == 2)
            {
                strcpy(wd_password,p);
                printf("wd_password = %s\n",wd_password);
            }
            else if(i == 3)
            {
                config->enable = atoi(p);
                printf("config->enable = %d\n",config->enable);
                if(config->enable != 1)
                    return 1;
            }
            else if(i == 4)
            {
                parse_server_url(p);
            }
            else if(i == 5)
            {
                config->dir_number = atoi(p);
                config->prule = (struct asus_rule **)malloc(sizeof(struct asus_rule*)*config->dir_number);
                printf("config->dir_number = %d\n",config->dir_number);
            }
            else
            {
                k = (i-6)/2 ;
                if((i-6)%2 == 0)
                {
                    config->prule[k] = (struct asus_rule*)malloc(sizeof(struct asus_rule));
                    config->prule[k]->rule = atoi(p);
                    printf("config->prule[%d]->rule = %d\n",k,config->prule[k]->rule);
                }
                else
                {
                    len = strlen(p);
                    if(p[len-1] == '\n')
                        p[len-1] = '\0';

                    strcpy(config->prule[k]->base_path,p);
                    /* trim path last char '/' */
                    len = strlen(config->prule[k]->base_path);
                    if(config->prule[k]->base_path[len-1] == '/')
                        config->prule[k]->base_path[len-1] = '\0';
                    printf("config->prule[%d]->base_path = %s\n",k,config->prule[k]->base_path);
                }
            }
            i++;
            /*switch (i)
            {
            case 5:
                rule = atoi(p);
                //printf("rule = %d\n",rule);
                if( rule == 0 )
                {
                    sync_up = 1;
                    sync_down = 1;
                }
                else if( rule == 1)
                {
                    sync_up = 1;
                    sync_down = 0;
                    download_only = 1;
                }
                else if( rule == 2)
                {
                    sync_up = 0;
                    sync_down = 1;
                    upload_only = 1;
                }

                break;
            case 6:
                strcpy(base_path,p);
                parse_mount_path(base_path);
                break;
                     default:
                break;

            }*/
        }
        fclose(fp);

        parse_mount_path(config->prule[0]->base_path);

        for( i = 0 ; i < config->dir_number;i++)
            my_mkdir_r(config->prule[i]->base_path);
    }
    //printf("type is %d,status is %d,username is %s,password is %s,rule is %d,base_path is %s\n",
    //type,status,username,password,rule,base_path);
    return 0;
}
#endif

//new parse_config_mutidir.Used for several Server folders and several Local folders
int parse_config_mutidir(char *path,struct asus_config *config)
{
    printf("#####parse_config####\n");
    FILE *fp;
    //DIR *dir;

    char buffer[256];
    char *p;

    int i = 0;
    //int k = 0;
    int len = 0;
    int j = 0;
    //strcpy(wd_username,"admin");    //for test
    //strcpy(wd_password,"admin");    //for test

    memset(buffer, '\0', sizeof(buffer));

    if (access(path,0) == 0)
    {
        if(( fp = fopen(path,"rb"))==NULL)
        {
            fprintf(stderr,"read Cloud error");
        }
        config->prule = (struct asus_rule **)malloc(sizeof(struct asus_rule*)*config->dir_number);
        while(fgets(buffer,256,fp)!=NULL)
        {
            if( buffer[ strlen(buffer)-1 ] == '\n' )
                buffer[ strlen(buffer)-1 ] = '\0';
            p=buffer;
            if(i == (j*6))
            {
                config->type = atoi(p);
                printf("config->type = %d\n",config->type);
                if( config->type != 1)
                    return 1;
            }
            else if(i == (j*6+2))
            {
                config->prule[j] = (struct asus_rule*)malloc(sizeof(struct asus_rule));
                parse_server_url(p,j,config);
                //config->dir_number = 1;
                //config->prule = (struct asus_rule **)malloc(sizeof(struct asus_rule*)*config->dir_number);
            }
            else if(i == (j*6+3))
            {
                //config->prule[j] = (struct asus_rule*)malloc(sizeof(struct asus_rule));
                config->prule[j]->rule = atoi(p);
                printf("config->dir_number = %d\n",config->dir_number);
            }
            else if(i == (j*6+5))
            {
                len = strlen(p);
                if(p[len-1] == '\n')
                    p[len-1] = '\0';

                strcpy(config->prule[j]->base_path,p);
                /* trim path last char '/' */
                len = strlen(config->prule[j]->base_path);
                if(config->prule[j]->base_path[len-1] == '/')
                    config->prule[j]->base_path[len-1] = '\0';
                printf("config->prule[%d]->base_path = %s\n",0,config->prule[j]->base_path);
                parse_mount_path(config->prule[j]->base_path,j,config);
                j++;
            }
            i++;
            /*switch (i)
            {
            case 5:
                rule = atoi(p);
                //printf("rule = %d\n",rule);
                if( rule == 0 )
                {
                    sync_up = 1;
                    sync_down = 1;
                }
                else if( rule == 1)
                {
                    sync_up = 1;
                    sync_down = 0;
                    download_only = 1;
                }
                else if( rule == 2)
                {
                    sync_up = 0;
                    sync_down = 1;
                    upload_only = 1;
                }

                break;
            case 6:
                strcpy(base_path,p);
                parse_mount_path(base_path);
                break;
                     default:
                break;

            }*/
        }
        fclose(fp);

        //parse_mount_path(config->prule[0]->base_path);

        for( i = 0 ; i < config->dir_number;i++)
            my_mkdir_r(config->prule[i]->base_path);
    }
    //printf("type is %d,status is %d,username is %s,password is %s,rule is %d,base_path is %s\n",
    //type,status,username,password,rule,base_path);
    return 0;
}


void clear_global_var(){

    prog_total = -1;
    prog_time = 0;

    download_only = 0;
    upload_only = 0;

    local_sync = 0;
    server_sync = 1;

    first_sync = 1;

    finished_initial = 0;

    stop_progress = 0;
    exit_loop = 0;
    //receve_socket = 0;

    DiskAvailable = 0;
    DiskAvailableShow = 0;
    DiskUsedShow = 0;
    PreDiskAvailableShow = 0;
    PreDiskUsedShow = 0;

    //dounfinish = 0;

    pthread_mutex_init(&mutex, NULL);
    pthread_mutex_init(&mutex_socket, NULL);
    pthread_mutex_init(&mutex_receve_socket, NULL);
    pthread_mutex_init(&mutex_log, NULL);
    //pthread_mutex_init(&mutex_sync, NULL);
    pthread_cond_init(&cond, NULL);
    pthread_cond_init(&cond_socket, NULL);
    pthread_cond_init(&cond_log, NULL);

    //memset(base_path,0,sizeof(base_path));
    //memset(mount_path,0,sizeof(mount_path));
    memset(log_path,0,sizeof(log_path));
    //memset(temp_path,0,sizeof(temp_path));
    memset(general_log,0,sizeof(general_log));
    //memset(sync_path,0,sizeof(sync_path));
    //memset(wd_username,0,sizeof(wd_username));
    //memset(wd_password,0,sizeof(wd_password));
    memset(base64_auth,'\0',sizeof(base64_auth));
    //memset(HOST,0,sizeof(HOST));
    memset(up_item_file, 0, sizeof(up_item_file));
    memset(down_item_file, 0, sizeof(down_item_file));
    //memset(all_local_item_file, 0, sizeof(all_local_item_file));
    //memset(all_local_item_folder, 0, sizeof(all_local_item_folder));
    //memset(local_action_file, 0, sizeof(local_action_file));
    //memset(ROOTURL,0,sizeof(ROOTURL));
    memset(&asus_cfg,0,sizeof(struct asus_config));
    //sprintf(HOST,"http://10.0.10.71:8082");    //for N16
    //sprintf(HOST,"http://192.168.1.1:8082");    //for PC


}

void replace_char_in_str(char *str,char newchar,char oldchar){

    int i;
    int len;
    len = strlen(str);
    for(i=0;i<len;i++)
    {
        if(str[i] == oldchar)
        {
            //printf("str[i] = %c\n",str[i]);
            str[i] = newchar;
            //printf("str[i] 222 = %c\n",str[i]);
        }
    }

}

//old init_global_var.Used for one Server folder and several Local folders
#if 0
void init_global_var()
{
    printf("go into init_global_var \n");
#ifdef MUTI_DIR
    int ret;
    int i;
    int num = asus_cfg.dir_number;
    char *filename = NULL;
    char *filename_tmp = NULL;
    char *folderhref = NULL;

    char *temp = NULL;
    const char chr = '/';

    g_pSyncList = (sync_list **)malloc(sizeof(sync_list *)*num);
    memset(g_pSyncList,0,sizeof(g_pSyncList));

    printf("memset over\n");

    rootfolder_len = strlen(ROOTFOLDER);
    sprintf(log_path,"%s/Cloud/log",log_base_path);
    sprintf(temp_path,"%s/Cloud/temp",mount_path);
    sprintf(general_log,"%s/WebDAV",log_path);
    my_mkdir_r(temp_path);
    my_mkdir_r(log_path);
    printf("log_path = %s\ntemp_path = %s\ngeneral_log = %s\n",log_path,temp_path,general_log);  

    printf("before for \n");

    for(i=0;i<num;i++)
    {
        g_pSyncList[i] = (sync_list *)malloc(sizeof(sync_list));
        memset(g_pSyncList[i],0,sizeof(sync_list));

        g_pSyncList[i]->index = i;
        g_pSyncList[i]->receve_socket = 0;
        g_pSyncList[i]->first_sync = 1;
        g_pSyncList[i]->local_sync = 0;
        g_pSyncList[i]->server_sync = 0;
        g_pSyncList[i]->ServerRootNode = NULL;
        g_pSyncList[i]->OldServerRootNode = NULL;

        asus_cfg.prule[i]->base_path_len = strlen(asus_cfg.prule[i]->base_path);

        printf("g_pSyncList[%d]->index = %d\n",i,g_pSyncList[i]->index);

        char *base_path_tmp;
        base_path_tmp = my_str_malloc(sizeof(asus_cfg.prule[i]->base_path));
        sprintf(base_path_tmp,"%s",asus_cfg.prule[i]->base_path);
        replace_char_in_str(base_path_tmp,'_','/');
        snprintf(g_pSyncList[i]->up_item_file,255,"%s/%s%s_up_item",temp_path,wd_username,base_path_tmp);
        snprintf(g_pSyncList[i]->down_item_file,255,"%s/%s%s_down_item",temp_path,wd_username,base_path_tmp);
        free(base_path_tmp);

        g_pSyncList[i]->server_action_list = create_action_item_head();
        g_pSyncList[i]->dragfolder_action_list = create_action_item_head();
        g_pSyncList[i]->unfinished_list = create_action_item_head();
        g_pSyncList[i]->up_space_not_enough_list = create_action_item_head();
        if(asus_cfg.prule[i]->rule == 1)
        {
            g_pSyncList[i]->download_only_socket_head = create_action_item_head();
        }
        g_pSyncList[i]->SocketActionList = queue_create();

        printf("g_pSyncList[%d]->down_item_file = %s\n",i,g_pSyncList[i]->down_item_file);

        filename_tmp = parse_name_from_path(asus_cfg.prule[i]->base_path);
        filename = oauth_url_escape(filename_tmp);
        folderhref = my_str_malloc(strlen(filename)+strlen(ROOTURL)+2);
        sprintf(folderhref,"%s/%s",ROOTURL,filename);
        sprintf(g_pSyncList[i]->rooturl,"%s",folderhref);

        temp = my_nstrchr(chr,g_pSyncList[i]->rooturl,3);
        strcpy(g_pSyncList[i]->rootfolder,temp);
        g_pSyncList[i]->rootfolder_length = strlen(g_pSyncList[i]->rootfolder);


        printf("folderhref = %s\n",folderhref);

        printf("g_pSyncList[i]->rooturl = %s\n,g_pSyncList[i]->rootfolder = %s\n,g_pSyncList[i]->rootfolder_length = %d\n",
               g_pSyncList[i]->rooturl,g_pSyncList[i]->rootfolder,g_pSyncList[i]->rootfolder_length);

        int exist = 0;
        exist = is_server_exist_with_type(folderhref);
        printf("exist = %d\n",exist);
        if(exist == 0)
        {
            do{
                ret = Mkcol_Root(folderhref);
                if(ret != 0)
                {
                    sleep(2);
                }
            }while(ret != 0 && exit_loop ==0);
        }
        else if(exist == 1)
        {
            char *newname;
            newname = change_server_same_name_root(filename_tmp);
            char *oldfolderhref;
            oldfolderhref = my_str_malloc(strlen(filename)+strlen(ROOTFOLDER)+2);
            sprintf(oldfolderhref,"%s/%s",ROOTFOLDER,filename);
            ret = Move_Root(oldfolderhref,newname);
            if(ret == 0)
            {
                do{
                    ret = Mkcol_Root(oldfolderhref);
                    if(ret != 0)
                    {
                        sleep(2);
                    }
                }while(ret != 0 && exit_loop == 0);
            }
            free(newname);
            free(oldfolderhref);
        }

        free(filename_tmp);
        free(filename);
        free(folderhref);
    }
#else
    rootfolder_len = strlen(ROOTFOLDER);
    base_path_len = strlen(base_path);
    sprintf(log_path,"%s/Cloud/log",log_base_path);
    sprintf(temp_path,"%s/Cloud/temp",mount_path);

    char *base_path_tmp;
    base_path_tmp = my_str_malloc(sizeof(base_path));
    sprintf(base_path_tmp,"%s",base_path);

    replace_char_in_str(base_path_tmp,'_','/');
    //printf("base_path_tmp = %s\n",base_path_tmp);

    snprintf(up_item_file,255,"%s/%s%s_up_item",temp_path,wd_username,base_path_tmp);
    snprintf(down_item_file,255,"%s/%s%s_down_item",temp_path,wd_username,base_path_tmp);
    //sprintf(all_local_item_file,"%s/%s_all_local_item_file",log_path,wd_username);
    //sprintf(all_local_item_folder,"%s/%s_all_local_item_folder",log_path,wd_username);
    //sprintf(local_action_file,"%s/%s_local_action",log_path,wd_username);
    sprintf(general_log,"%s/WebDAV",log_path);
    //sprintf(ROOTURL,"%s%s",HOST,ROOTFOLDER);

    server_action_list = create_action_item_head();
    dragfolder_action_list = create_action_item_head();
    unfinished_list = create_action_item_head();

    if(download_only == 1)
    {
        download_only_socket_head = create_action_item_head();
    }

    SocketActionList = queue_create();

    //printf("init_global_var start!!\n");

    //sprintf(password,"%s",MD5_string(password));

    //printf("username is %s,pwd is %s,mount path is %s\n",username,password,base_path);
    /*DIR *dir;
    char cloud_path[128];
    sprintf(cloud_path,"%s/cloud",mount_path);
    if(NULL == (dir = opendir(cloud_path)))
    {
        printf("open %s error!\n",cloud_path);
        mkdir(cloud_path,0777);
    }
    else
        closedir(dir);
    if(NULL == (dir = opendir(log_path)))
    {
        printf("open %s error!\n",log_path);
        mkdir(log_path,0777);
    }
    else
        closedir(dir);
    if(NULL == (dir = opendir(temp_path)))
    {
        printf("open %s error!\n",temp_path);
        mkdir(temp_path,0777);
    }
    else
        closedir(dir);*/
    free(base_path_tmp);

    my_mkdir_r(temp_path);
    my_mkdir_r(log_path);
#endif
}
#endif


//new parse_config_mutidir.Used for several Server folders and several Local folders
void init_global_var()
{
    printf("go into init_global_var \n");
#if 1
    int ret;
    int i;
    int num = asus_cfg.dir_number;
    //char *filename = NULL;
    //char *filename_tmp = NULL;
    //char *folderhref = NULL;

    char *temp = NULL;
    const char chr = '/';

    g_pSyncList = (sync_list **)malloc(sizeof(sync_list *)*num);
    memset(g_pSyncList,0,sizeof(g_pSyncList));

    printf("memset over\n");

    //rootfolder_len = strlen(ROOTFOLDER);
    //sprintf(log_path,"%s/Cloud/log",log_base_path);
    //sprintf(temp_path,"%s/Cloud/temp",mount_path);
    //sprintf(general_log,"%s/WebDAV",log_path);
    //my_mkdir_r(temp_path);
    //my_mkdir_r(log_path);
    //printf("log_path = %s\ntemp_path = %s\ngeneral_log = %s\n",log_path,temp_path,general_log);
    //printf("log_path = %s\ngeneral_log = %s\n",log_path,general_log);

    //printf("before for \n");

    for(i=0;i<num;i++)
    {
        g_pSyncList[i] = (sync_list *)malloc(sizeof(sync_list));
        memset(g_pSyncList[i],0,sizeof(sync_list));

        g_pSyncList[i]->index = i;
        g_pSyncList[i]->receve_socket = 0;
        g_pSyncList[i]->have_local_socket = 0;
        g_pSyncList[i]->first_sync = 1;
        g_pSyncList[i]->no_local_root = 0;
        //g_pSyncList[i]->local_sync = 0;
        //g_pSyncList[i]->server_sync = 0;
        g_pSyncList[i]->init_completed = 0;
        g_pSyncList[i]->ServerRootNode = NULL;
        g_pSyncList[i]->OldServerRootNode = NULL;

        sprintf(asus_cfg.prule[i]->temp_path,"%s/Cloud/temp",asus_cfg.prule[i]->mount_path);
        my_mkdir_r(asus_cfg.prule[i]->temp_path);
        printf("temp_path = %s\n",asus_cfg.prule[i]->temp_path);

        asus_cfg.prule[i]->base_path_len = strlen(asus_cfg.prule[i]->base_path);

        printf("g_pSyncList[%d]->index = %d\n",i,g_pSyncList[i]->index);

        //g_pSyncList[i]->up_item_file = my_str_malloc(256);
        //g_pSyncList[i]->down_item_file = my_str_malloc(256);

        char *base_path_tmp;
        base_path_tmp = my_str_malloc(sizeof(asus_cfg.prule[i]->base_path));
        sprintf(base_path_tmp,"%s",asus_cfg.prule[i]->base_path);
        replace_char_in_str(base_path_tmp,'_','/');
        //snprintf(g_pSyncList[i]->up_item_file,255,"%s/%s%s_up_item",temp_path,wd_username,base_path_tmp);
        //snprintf(g_pSyncList[i]->down_item_file,255,"%s/%s%s_down_item",temp_path,wd_username,base_path_tmp);
        snprintf(g_pSyncList[i]->up_item_file,255,"%s/%s_up_item",
                 asus_cfg.prule[i]->temp_path,base_path_tmp);
        snprintf(g_pSyncList[i]->down_item_file,255,"%s/%s_down_item",
                 asus_cfg.prule[i]->temp_path,base_path_tmp);

        free(base_path_tmp);

        g_pSyncList[i]->server_action_list = create_action_item_head();
        g_pSyncList[i]->dragfolder_action_list = create_action_item_head();
        g_pSyncList[i]->unfinished_list = create_action_item_head();
        g_pSyncList[i]->up_space_not_enough_list = create_action_item_head();
        g_pSyncList[i]->copy_file_list = create_action_item_head();
        if(asus_cfg.prule[i]->rule == 1)
        {
            g_pSyncList[i]->download_only_socket_head = create_action_item_head();
        }
        else
        {
            g_pSyncList[i]->download_only_socket_head = NULL;
        }
        g_pSyncList[i]->SocketActionList = queue_create();

        printf("g_pSyncList[%d]->down_item_file = %s\n",i,g_pSyncList[i]->down_item_file);
        printf("g_pSyncList[%d]->up_item_file = %s\n",i,g_pSyncList[i]->up_item_file);

        /*filename_tmp = parse_name_from_path(asus_cfg.prule[i]->base_path);
        filename = oauth_url_escape(filename_tmp);
        folderhref = my_str_malloc(strlen(filename)+strlen(ROOTURL)+2);
        sprintf(folderhref,"%s/%s",ROOTURL,filename);*/
        //sprintf(g_pSyncList[i]->rooturl,"%s",ROOTURL);

        //temp = my_nstrchr(chr,g_pSyncList[i]->rooturl,3);
        //strcpy(g_pSyncList[i]->rootfolder,temp);
        //g_pSyncList[i]->rootfolder_length = strlen(g_pSyncList[i]->rootfolder);


        //printf("folderhref = %s\n",folderhref);

        //printf("g_pSyncList[i]->rooturl = %s\n,g_pSyncList[i]->rootfolder = %s\n,g_pSyncList[i]->rootfolder_length = %d\n",
        //       g_pSyncList[i]->rooturl,g_pSyncList[i]->rootfolder,g_pSyncList[i]->rootfolder_length);

        /*int exist = 0;
        exist = is_server_exist_with_type(folderhref);
        printf("exist = %d\n",exist);
        if(exist == 0)
        {
            do{
                ret = Mkcol_Root(folderhref);
                if(ret != 0)
                {
                    sleep(2);
                }
            }while(ret != 0 && exit_loop ==0);
        }
        else if(exist == 1)
        {
            char *newname;
            newname = change_server_same_name_root(filename_tmp);
            char *oldfolderhref;
            oldfolderhref = my_str_malloc(strlen(filename)+strlen(ROOTFOLDER)+2);
            sprintf(oldfolderhref,"%s/%s",ROOTFOLDER,filename);
            ret = Move_Root(oldfolderhref,newname);
            if(ret == 0)
            {
                do{
                    ret = Mkcol_Root(oldfolderhref);
                    if(ret != 0)
                    {
                        sleep(2);
                    }
                }while(ret != 0 && exit_loop == 0);
            }
            free(newname);
            free(oldfolderhref);
        }*/

        //free(filename_tmp);
        //free(filename);
        //free(folderhref);
    }
#else
    rootfolder_len = strlen(ROOTFOLDER);
    base_path_len = strlen(base_path);
    sprintf(log_path,"%s/Cloud/log",log_base_path);
    sprintf(temp_path,"%s/Cloud/temp",mount_path);

    char *base_path_tmp;
    base_path_tmp = my_str_malloc(sizeof(base_path));
    sprintf(base_path_tmp,"%s",base_path);

    replace_char_in_str(base_path_tmp,'_','/');
    //printf("base_path_tmp = %s\n",base_path_tmp);

    snprintf(up_item_file,255,"%s/%s%s_up_item",temp_path,wd_username,base_path_tmp);
    snprintf(down_item_file,255,"%s/%s%s_down_item",temp_path,wd_username,base_path_tmp);
    //sprintf(all_local_item_file,"%s/%s_all_local_item_file",log_path,wd_username);
    //sprintf(all_local_item_folder,"%s/%s_all_local_item_folder",log_path,wd_username);
    //sprintf(local_action_file,"%s/%s_local_action",log_path,wd_username);
    sprintf(general_log,"%s/WebDAV",log_path);
    //sprintf(ROOTURL,"%s%s",HOST,ROOTFOLDER);

    server_action_list = create_action_item_head();
    dragfolder_action_list = create_action_item_head();
    unfinished_list = create_action_item_head();

    if(download_only == 1)
    {
        download_only_socket_head = create_action_item_head();
    }

    SocketActionList = queue_create();

    //printf("init_global_var start!!\n");

    //sprintf(password,"%s",MD5_string(password));

    //printf("username is %s,pwd is %s,mount path is %s\n",username,password,base_path);
    /*DIR *dir;
    char cloud_path[128];
    sprintf(cloud_path,"%s/cloud",mount_path);
    if(NULL == (dir = opendir(cloud_path)))
    {
        printf("open %s error!\n",cloud_path);
        mkdir(cloud_path,0777);
    }
    else
        closedir(dir);
    if(NULL == (dir = opendir(log_path)))
    {
        printf("open %s error!\n",log_path);
        mkdir(log_path,0777);
    }
    else
        closedir(dir);
    if(NULL == (dir = opendir(temp_path)))
    {
        printf("open %s error!\n",temp_path);
        mkdir(temp_path,0777);
    }
    else
        closedir(dir);*/
    free(base_path_tmp);

    my_mkdir_r(temp_path);
    my_mkdir_r(log_path);
#endif
}

int get_path_to_index(char *path)
{
    int i;
    char *root_path = NULL;
    char *temp = NULL;
    root_path = my_str_malloc(512);

    temp = my_nstrchr('/',path,4);
    if(temp == NULL)
    {
        sprintf(root_path,"%s",path);
    }
    else
    {
        snprintf(root_path,strlen(path)-strlen(temp)+1,"%s",path);
    }

    for(i=0;i<asus_cfg.dir_number;i++)
    {
        if(!strcmp(root_path,asus_cfg.prule[i]->base_path))
            break;
    }

    printf("get_path_to_index root_path = %s\n",root_path);

    free(root_path);

    return i;
}


int cmd_parser(char *cmd,int index)
{


    //if(server_modify == 1)   //if operate by self not parser socket command
    //    return 0;

    if( strstr(cmd,"(conflict)") != NULL )
        return 0;

    printf("socket command is %s \n",cmd);


    if( !strncmp(cmd,"exit",4))
    {
    	printf("exit socket\n");
    	return 0;
    }

    if(!strncmp(cmd,"rmroot",6))
    {
        g_pSyncList[index]->no_local_root = 1;
        return 0;
    }


    char cmd_name[64];
    //char cmd_param[512];
    char *path;
    char *temp;
    char filename[256];
    char *fullname;
    char oldname[256],newname[256];
    char *oldpath;
    char action[64];
    char *cmp_name;
    char *mv_newpath;
    char *mv_oldpath;
    char *ch;
    int status;
    //Asusconfig asusconfig;

    memset(cmd_name, 0, sizeof(cmd_name));
    //memset(cmd_param, 0, sizeof(cmd_param));
    //memset(path,0,sizeof(path));
    //memset(temp,0,sizeof(temp));
    memset(oldname,'\0',sizeof(oldname));
    memset(newname,'\0',sizeof(newname));
    //memset(oldpath,0,sizeof(oldpath));
    //memset(fullname,0,sizeof(fullname));
    memset(action,0,sizeof(action));
    //memset(cmp_name,0,sizeof(cmd_name));
    //memset(mv_newpath,0,sizeof(mv_newpath));
    //memset(mv_oldpath,0,sizeof(mv_oldpath));
    //memset(mv_newpath_tmp,0,sizeof(mv_newpath_tmp));
    //memset(mv_oldpath_tmp,0,sizeof(mv_oldpath_tmp));

    ch = cmd;
    int i = 0;
    while(*ch != '@')
    {
        i++;
        ch++;
    }

    memcpy(cmd_name, cmd, i);

    char *p = NULL;
    ch++;
    i++;

    temp = my_str_malloc((size_t)(strlen(ch)+1));

    strcpy(temp,ch);
    p = strchr(temp,'@');

    //printf("temp = %s\n",temp);
    //printf("p = %s\n",p);
    //printf("strlen(temp)- strlen(p) = %d\n",strlen(temp)- strlen(p));

    path = my_str_malloc((size_t)(strlen(temp)- strlen(p)+1));

    //printf("path = %s\n",path);

    if(p!=NULL)
        snprintf(path,strlen(temp)- strlen(p)+1,"%s",temp);

    //free(temp);

    p++;
    if(strcmp(cmd_name, "rename") == 0)
    {
        char *p1 = NULL;

        p1 = strchr(p,'@');

        if(p1 != NULL)
            strncpy(oldname,p,strlen(p)- strlen(p1));

        p1++;

        strcpy(newname,p1);
        printf("cmd_name: [%s],path: [%s],oldname: [%s],newname: [%s]\n",cmd_name,path,oldname,newname);
        if(newname[0] == '.' || (strstr(path,"/.")) != NULL)
        {
            free(temp);
            free(path);
            return 0;
        }
    }
    else if(strcmp(cmd_name, "move") == 0)
    {
        char *p1 = NULL;

        p1 = strchr(p,'@');

        oldpath = my_str_malloc((size_t)(strlen(p)- strlen(p1)+1));

        if(p1 != NULL)
            snprintf(oldpath,strlen(p)- strlen(p1)+1,"%s",p);

        p1++;

        strcpy(oldname,p1);

        printf("cmd_name: [%s],path: [%s],oldpath: [%s],oldname: [%s]\n",cmd_name,path,oldpath,oldname);

        /*if(strncmp(path,asus_cfg.prule[index]->base_path,asus_cfg.prule[index]->base_path_len) != 0)
        {
            int k;
            for(k=0;k<asus_cfg.dir_number;k++)
            {
                if(strstr(oldpath,asus_cfg.prule[k]->base_path))
                    break;
            }
            pthread_mutex_lock(&mutex_receve_socket);
            //receve_socket = 1;
            g_pSyncList[k]->receve_socket = 1;
            pthread_mutex_unlock(&mutex_receve_socket);

            pthread_mutex_lock(&mutex_socket);
            SocketActionTmp = malloc (sizeof(struct queue_entry));
            memset(SocketActionTmp,0,sizeof(struct queue_entry));
            sprintf(SocketActionTmp->cmd_name,"%s",cmd);
            queue_enqueue(SocketActionTmp,g_pSyncList[k]->SocketActionList);
            printf("SocketActionTmp->cmd_name = %s\n",SocketActionTmp->cmd_name);
            pthread_mutex_unlock(&mutex_socket);

            free(temp);
            free(path);
            free(oldpath);
            return 0;
        }*/
        if(oldname[0] == '.' || (strstr(path,"/.")) != NULL)
        {
            free(temp);
            free(path);
            free(oldpath);
            return 0;
        }
    }
    else
    {
        strcpy(filename,p);
        fullname = my_str_malloc((size_t)(strlen(path)+strlen(filename)+2));
        printf("cmd_name: [%s],path: [%s],filename: [%s]\n",cmd_name,path,filename);
        if(filename[0] == '.' || (strstr(path,"/.")) != NULL)
        {
            free(temp);
            free(path);
            return 0;
        }
    }

    free(temp);


    if( !strcmp(cmd_name,"rename") )
    {
        cmp_name = my_str_malloc((size_t)(strlen(path)+strlen(newname)+2));
        sprintf(cmp_name,"%s/%s",path,newname);
    }
    else
    {
        cmp_name = my_str_malloc((size_t)(strlen(path)+strlen(filename)+2));
        sprintf(cmp_name,"%s/%s",path,filename);
    }

    if( strcmp(cmd_name, "createfile") == 0 )
    {
        strcpy(action,"createfile");
        action_item *item;

        item = get_action_item("copyfile",cmp_name,g_pSyncList[index]->copy_file_list,index);

        if(item != NULL)
        {
            printf("##### delete copyfile %s ######\n",cmp_name);
            //pthread_mutex_lock(&mutex);
            del_action_item("copyfile",cmp_name,g_pSyncList[index]->copy_file_list);
        }
    }
    else if( strcmp(cmd_name, "remove") == 0  || strcmp(cmd_name, "delete") == 0)
    {
        strcpy(action,"remove");
    }
    else if( strcmp(cmd_name, "createfolder") == 0 )
    {
        strcpy(action,"createfolder");
    }
    else if( strcmp(cmd_name, "rename") == 0 )
    {
        strcpy(action,"rename");
    }

    if(g_pSyncList[index]->server_action_list->next != NULL)
    {
        action_item *item;

        item = get_action_item(action,cmp_name,g_pSyncList[index]->server_action_list,index);

        if(item != NULL)
        {
            printf("##### %s %s by WebDAV Server self ######\n",action,cmp_name);
            //pthread_mutex_lock(&mutex);
            del_action_item(action,cmp_name,g_pSyncList[index]->server_action_list);
            printf("#### del action item success!\n");
            //pthread_mutex_unlock(&mutex);
            //local_sync = 0;
            free(path);
            free(fullname);
            free(cmp_name);
            return 0;
        }
    }

    if(g_pSyncList[index]->dragfolder_action_list->next != NULL)
    {
        action_item *item;

        item = get_action_item(action,cmp_name,g_pSyncList[index]->dragfolder_action_list,index);

        if(item != NULL)
        {
            printf("##### %s %s by dragfolder recursion self ######\n",action,cmp_name);
            //pthread_mutex_lock(&mutex);
            del_action_item(action,cmp_name,g_pSyncList[index]->dragfolder_action_list);

            //pthread_mutex_unlock(&mutex);
            //local_sync = 0;
            free(path);
            free(fullname);
            free(cmp_name);
            return 0;
        }
    }
    free(cmp_name);




#if DEBUG
    printf("###### %s is start ######\n",cmd_name);
    //write_system_log(cmd_name,"start");
#endif
    
    if( strcmp(cmd_name, "copyfile") != 0 )
    {
        g_pSyncList[index]->have_local_socket = 1;
    }

    if( strcmp(cmd_name, "createfile") == 0 || strcmp(cmd_name, "dragfile") == 0 )
    {
        sprintf(fullname,"%s/%s",path,filename);
        int exist;
        //sprintf(fullnametmp,"%s/%s",path,filename);
        //wd_escape(fullnametmp,escapepath);
        //printf("escapepath : %s\n",escapepath);
        //printf("fullname : %s\n",fullname);
        //char *server_path;
        //server_path = localpath_to_serverpath(fullname,index);
        exist = is_server_exist(path,fullname,index);
        //free(server_path);
        if(!exist)
        {
            status = Upload(fullname,index);

            if(status != 0)
            {
#if DEBUG
                printf("upload %s failed\n",fullname);
                //write_system_log("error","uploadfile fail");
#endif
                free(path);
                free(fullname);
                return status;
            }
            else
            {
                char *serverpath;
                serverpath = localpath_to_serverpath(fullname,index);
                //printf("serverpath = %s\n",serverpath);
                time_t modtime;
                modtime = Getmodtime(serverpath,index);
                if(modtime != -1)
                    ChangeFile_modtime(fullname,modtime);
                else
                {
                    printf("ChangeFile_modtime failed!\n");
                }
                free(fullname);
                free(serverpath);
            }
        }
        /*else if(exist == -2)
        {
            free(path);
            free(fullname);
            return COULD_NOT_CONNECNT_TO_SERVER;
        }*/
        else
        {
            if((newer_file(fullname,index)) != 2 && (newer_file(fullname,index)) != -1)
            {
                if(1)
                {
                    char *newname;
                    newname = change_server_same_name(fullname,index);
                    Move(fullname,newname,index);
                    printf("newname = %s\n",newname);
                    free(newname);

                    status = Upload(fullname,index);

                    if(status != 0)
                    {
#if DEBUG
                        printf("upload %s failed\n",fullname);
                        //write_system_log("error","uploadfile fail");
#endif
                        free(path);
                        free(fullname);
                        return status;
                    }
                    else
                    {
                        char *serverpath;
                        serverpath = localpath_to_serverpath(fullname,index);
                        //printf("serverpath = %s\n",serverpath);
                        time_t modtime;
                        modtime = Getmodtime(serverpath,index);
                        if(modtime != -1)
                            ChangeFile_modtime(fullname,modtime);
                        else
                        {
                            printf("ChangeFile_modtime failed!\n");
                        }
                        free(fullname);
                        free(serverpath);
                    }

                }
                /*else
                {
                    char *newname;
                    newname = change_local_same_name(fullname);
                    rename(fullname,newname);
                    add_action_item("rename",newname,server_action_list);
                    printf("newname = %s\n",newname);
                    free(newname);
                    status = Upload(newname);

                    if(status != 0)
                    {
#if DEBUG
                        printf("upload %s failed\n",newname);
                        //write_system_log("error","uploadfile fail");
#endif
                        free(path);
                        free(newname);
                        free(fullname);
                        return status;
                    }
                    else
                    {
                        char *serverpath;
                        serverpath = localpath_to_serverpath(newname);
                        //printf("serverpath = %s\n",serverpath);
                        time_t modtime;
                        modtime = Getmodtime(serverpath);
                        if(modtime != -1)
                            ChangeFile_modtime(newname,modtime);
                        else
                        {
                            printf("ChangeFile_modtime failed!\n");
                        }
                        free(fullname);
                        free(newname);
                        free(serverpath);
                    }
                }*/
            }
        }

    }

    else if( strcmp(cmd_name, "copyfile") == 0 )
    {
        sprintf(fullname,"%s/%s",path,filename);
        /*while(is_copying_finished(fullname) !=0 ){
            sleep(1);
        }*/
        add_action_item("copyfile",fullname,g_pSyncList[index]->copy_file_list);

        free(fullname);
    }

    else if( strcmp(cmd_name, "modify") == 0 )
    {
        sprintf(fullname,"%s/%s",path,filename);

        char *serverpath;
        serverpath = localpath_to_serverpath(fullname,index);
        char *urlpath;
        char *m_temp;
        const char m_chr = ':';

        m_temp = my_nstrchr(m_chr,asus_cfg.prule[index]->host,2);

        urlpath = my_str_malloc(strlen(serverpath)+strlen(asus_cfg.prule[index]->host)+1);
        snprintf(urlpath,strlen(asus_cfg.prule[index]->host)-strlen(m_temp)+1,"%s",
                 asus_cfg.prule[index]->host);

        sprintf(urlpath,"%s%s",urlpath,serverpath);

        printf("urlpath = %s\n",urlpath);
        //free(urlpath);
        //free(serverpath);

        CloudFile *filetmp = NULL;
        filetmp = get_CloudFile_node(g_pSyncList[index]->OldServerRootNode,urlpath,0x2);
        free(urlpath);
        time_t modtime1,modtime2;
        modtime1 = Getmodtime(serverpath,index);
        if(filetmp == NULL)
        {
            modtime2 = (time_t)-1;
        }
        else
        {
            modtime2 = filetmp->modtime;
        }
        printf("modtime1 = %ld\nmodtime2 = %ld\n",modtime1,modtime2);

        if(modtime1 == modtime2 || modtime1 == -1 || asus_cfg.prule[index]->rule == 2)  //Upload
        {
            status = Upload(fullname,index);

            if(status != 0)
            {
#if DEBUG
                printf("upload %s failed\n",fullname);
                //write_system_log("error","uploadfile fail");
#endif
                free(path);
                free(fullname);
                free(serverpath);
                return status;
            }
            else
            {
                //char *serverpath;
                //serverpath = localpath_to_serverpath(fullname,index);
                //printf("serverpath = %s\n",serverpath);
                time_t modtime;
                modtime = Getmodtime(serverpath,index);
                if(modtime != -1)
                    ChangeFile_modtime(fullname,modtime);
                else
                {
                    printf("ChangeFile_modtime failed!\n");
                }
                free(fullname);
                free(serverpath);
            }


        }
        else if(modtime1 != modtime2 && modtime1 != -1 && asus_cfg.prule[index]->rule == 0)    //Rename Server and Upload
        {
            char *newname;
            newname = change_server_same_name(fullname,index);
            printf("newname = %s\n",newname);
            status = Move(fullname,newname,index);
            free(newname);
            if(status != 0)
            {
                printf("upload %s failed\n",fullname);
                free(path);
                free(fullname);
                free(serverpath);
                return status;
            }
            status = Upload(fullname,index);

            if(status != 0)
            {
#if DEBUG
                printf("upload %s failed\n",fullname);
                //write_system_log("error","uploadfile fail");
#endif
                free(path);
                free(fullname);
                free(serverpath);
                return status;
            }
            else
            {
                //printf("serverpath = %s\n",serverpath);
                time_t modtime;
                modtime = Getmodtime(serverpath,index);
                if(modtime != -1)
                    ChangeFile_modtime(fullname,modtime);
                else
                {
                    printf("ChangeFile_modtime failed!\n");
                }
                free(fullname);
                free(serverpath);
            }

        }

        //if(filetmp != NULL)
        //printf("filetmp->href = %s\n",filetmp->href);
#if 0
        if((newer_file(fullname) == 1) && (filetmp != NULL) && sync_up == 1)
        {
            action_item *item;
            item = get_action_item("download",filetmp->href,unfinished_list);
            if(is_local_space_enough(filetmp))
            {
                add_action_item("createfile",fullname,server_action_list);
                status = Download(filetmp->href);
                if (status == NE_OK)
                {
                    time_t modtime;
                    modtime = Getmodtime(serverpath);
                    if(modtime != -1)
                        ChangeFile_modtime(fullname,modtime);
                    else
                    {
                        printf("ChangeFile_modtime failed!\n");
                    }
                    if(item != NULL)
                    {
                        del_action_item("download",filetmp->href,unfinished_list);
                    }
                }
                else
                {
                    free(path);
                    free(fullname);
                    free(serverpath);
                    return status;
                }
            }
            else
            {
                write_log(S_ERROR,"local space is not enough!","");
            }
            /*else if((newer_file(fullname)) == 0)
            {

                if(item == NULL)
                {
                    add_action_item("download",filetmp->href,unfinished_list);
                }
                free(fullname);
                free(serverpath);
            }*/

        }
#endif
        //else
        //{
#if 0
        status = Upload(fullname,index);

        if(status != 0)
        {
#if DEBUG
            printf("upload %s failed\n",fullname);
            //write_system_log("error","uploadfile fail");
#endif
            free(path);
            free(fullname);
            //free(serverpath);
            return status;
        }
        else
        {
            char *serverpath;
            serverpath = localpath_to_serverpath(fullname,index);
            //printf("serverpath = %s\n",serverpath);
            time_t modtime;
            modtime = Getmodtime(serverpath);
            if(modtime != -1)
                ChangeFile_modtime(fullname,modtime);
            else
            {
                printf("ChangeFile_modtime failed!\n");
            }
            free(fullname);
            free(serverpath);
        }
        //}
#endif

        /*time_t another_modtime;
    	time_t list_modtime;
    	time_t file_modtime;
    	sprintf(fullname,"%s/%s",path,filename);
    	char escapepath_tmp[512];
    	char *path_tmp;
    	path_tmp = strstr(path,base_path) + base_path_len;
    	memset(escapepath_tmp,0,sizeof(escapepath_tmp));
    	wd_escape(path_tmp,escapepath_tmp);
    	char URL[512];
    	memset(URL,0,sizeof(URL));
        sprintf(URL,"%s%s%s",HOST,ROOTFOLDER,escapepath_tmp);

    	another_modtime = GetServer_modtime(URL,fullname);
    	list_modtime = GetList_modtime(fullname);
    	file_modtime = GetFile_modtime(fullname);
    	printf("another_modtime = %lu,list_modtime = %lu,file_modtime = %lu\n",another_modtime,list_modtime,file_modtime);

    	//sprintf(fullnametmp,"%s/%s",path,filename);
        //wd_escape(fullnametmp,escapepath);
        //printf("escapepath : %s\n",escapepath);
        //printf("fullname : %s\n",fullname);

    	if(another_modtime > list_modtime && another_modtime < file_modtime){
            char new_fullname[512];
            memset(new_fullname,0,sizeof(new_fullname));
            sprintf(new_fullname,"%s(%d)",fullname,rand());
            rename(fullname,new_fullname);
            status = Upload(new_fullname);

            free(fullname);

            if(status != 0)
            {
#if DEBUG
                printf("upload %s failed\n",new_fullname);
                //write_system_log("error","uploadfile fail");
#endif
                free(path);
                //free(fullname);
                return status;
            }
            else
            {
                char *serverpath;
                serverpath = localpath_to_serverpath(new_fullname);
                //printf("serverpath = %s\n",serverpath);
                time_t modtime;
                modtime = Getmodtime(serverpath);
                if(modtime != -1)
                    ChangeFile_modtime(new_fullname,modtime);
                else
                {
                    printf("ChangeFile_modtime failed!\n");
                }
                free(serverpath);
            }
    	}
    	else
    	{
            status = Upload(fullname);

            free(fullname);

            if(status != 0)
            {
#if DEBUG
                printf("upload failed\n");
                //write_system_log("error","uploadfile fail");
#endif
                free(path);
                //free(fullname);
                return status;
            }
            else
            {
                char *serverpath;
                serverpath = localpath_to_serverpath(fullname);
                //printf("serverpath = %s\n",serverpath);
                time_t modtime;
                modtime = Getmodtime(serverpath);
                if(modtime != -1)
                    ChangeFile_modtime(fullname,modtime);
                else
                {
                    printf("ChangeFile_modtime failed!\n");
                }
                free(serverpath);
            }
        }*/

    }

    else if(strcmp(cmd_name, "delete") == 0  || strcmp(cmd_name, "remove") == 0){
    	sprintf(fullname,"%s/%s",path,filename);
    	//sprintf(fullnametmp,"%s/%s",path,filename);
        //wd_escape(fullnametmp,escapepath);
    	//printf("escapepath : %s\n",escapepath);
    	//printf("fullname : %s\n",fullname);
        status = Delete(fullname,index);

        free(fullname);

    	if(status != 0)
    	{
#if DEBUG
            printf("delete failed\n");
    	    //write_system_log("error","uploadfile fail");
#endif
            free(path);
            //free(fullname);
            return status;
    	}
    }
    else if(strcmp(cmd_name, "move") == 0 || strcmp(cmd_name, "rename") == 0)
    {
        if(strcmp(cmd_name, "move") == 0)
        {
            mv_newpath = my_str_malloc((size_t)(strlen(path)+strlen(oldname)+2));
            mv_oldpath = my_str_malloc((size_t)(strlen(oldpath)+strlen(oldname)+2));
            sprintf(mv_newpath,"%s/%s",path,oldname);
            sprintf(mv_oldpath,"%s/%s",oldpath,oldname);
            free(oldpath);
            //sprintf(mv_newpath_tmp,"%s/%s",path,oldname);
            //sprintf(mv_oldpath_tmp,"%s/%s",oldpath,oldname);
    	}
        else
        {
            mv_newpath = my_str_malloc((size_t)(strlen(path)+strlen(newname)+2));
            mv_oldpath = my_str_malloc((size_t)(strlen(path)+strlen(oldname)+2));
            sprintf(mv_newpath,"%s/%s",path,newname);
            sprintf(mv_oldpath,"%s/%s",path,oldname);
            //sprintf(mv_newpath_tmp,"%s/%s",path,oldname);
            //sprintf(mv_oldpath_tmp,"%s/%s",path,newname);
    	}
    	//wd_escape(mv_oldpath,escapepath);
    	//wd_escape(mv_newpath,escapepath1);
        if(strcmp(cmd_name,"rename") == 0)
        {
            int exist = 0;
            if(test_if_dir(mv_newpath))
            {
                exist = is_server_exist(path,mv_newpath,index);
                if(exist)
                {
                    char *newname;
                    newname = change_server_same_name(mv_newpath,index);
                    printf("newname = %s\n",newname);
                    status = Move(mv_newpath,newname,index);

                    free(newname);
                    if(status == 0)
                    {
                        status = Move(mv_oldpath,mv_newpath,index);
                    }
                }
                else
                {
                    status = Move(mv_oldpath,mv_newpath,index);
                }
            }
            else
            {
                exist = is_server_exist(path,mv_newpath,index);
                if(exist)
                {
                    printf("have the same name\n");
                    char *newname;
                    newname = change_server_same_name(mv_newpath,index);
                    printf("newname = %s\n",newname);
                    status = Move(mv_newpath,newname,index);

                    free(newname);
                    if(status == 0)
                    {
                        //sleep(2);
                        status = Move(mv_oldpath,mv_newpath,index);
                    }

                }
                else
                {
                    printf("have no the same name\n");
                    status = Move(mv_oldpath,mv_newpath,index);
                }

            }
        }
        else    //move
        {
            int exist = 0;
            int old_index;
            old_index = get_path_to_index(mv_oldpath);
            if(index == old_index)
            {
                if(test_if_dir(mv_newpath))
                {
                    exist = is_server_exist(path,mv_newpath,index);

                    if(exist)
                    {
                        char *newname;
                        newname = change_server_same_name(mv_newpath,index);
                        printf("newname = %s\n",newname);
                        status = Move(mv_newpath,newname,index);

                        free(newname);
                        if(status == 0)
                        {
                            status = moveFolder(mv_oldpath,mv_newpath,index);
                            if(status == 0)
                            {
                                Delete(mv_oldpath,index);
                            }
                        }
                    }
                    else
                    {
                        status = moveFolder(mv_oldpath,mv_newpath,index);
                        if(status == 0)
                        {
                            Delete(mv_oldpath,index);
                        }
                    }
                }
                else
                {
                    exist = is_server_exist(path,mv_newpath,index);

                    if(exist)
                    {
                        char *newname;
                        newname = change_server_same_name(mv_newpath,index);
                        printf("newname = %s\n",newname);
                        status = Move(mv_newpath,newname,index);

                        free(newname);
                        if(status == 0)
                        {
                            //sleep(2);
                            status = Move(mv_oldpath,mv_newpath,index);
                        }

                    }
                    else
                    {
                        status = Move(mv_oldpath,mv_newpath,index);
                    }

                }
            }
            else
            {
                if(asus_cfg.prule[old_index]->rule == 1)
                {
                    del_download_only_action_item("move",mv_oldpath,g_pSyncList[old_index]->download_only_socket_head);
                }
                else
                {
                    Delete(mv_oldpath,old_index);
                }

                if(test_if_dir(mv_newpath))
                {
                    status = createFolder(mv_newpath,index);
                    //printf("create_folder_cmp = %d\n",create_folder_cmp);
                    //free(fullname);
                    if(status != 0)
                    {
#if DEBUG
                        printf("createFolder failed status = %d\n",status);
                        //write_system_log("error","uploadfile fail");
#endif
                        free(path);
                        free(mv_oldpath);
                        free(mv_newpath);
                        return status;
                    }

                }
                else
                {
                    status = Upload(mv_newpath,index);

                    if(status != 0)
                    {
#if DEBUG
                        printf("move %S to %s failed\n",mv_oldpath,mv_newpath);
                        //write_system_log("error","uploadfile fail");
#endif
                        free(path);
                        free(mv_oldpath);
                        free(mv_newpath);
                        return status;
                    }
                    else
                    {
                        char *serverpath;
                        serverpath = localpath_to_serverpath(mv_newpath,index);
                        time_t modtime;
                        modtime = Getmodtime(serverpath,index);
                        if(modtime != -1)
                            ChangeFile_modtime(mv_newpath,modtime);
                        else
                        {
                            printf("ChangeFile_modtime failed!\n");
                        }
                        free(serverpath);
                    }
                }
            }

        }


        free(mv_oldpath);
        free(mv_newpath);

        if(status != 0)
        {
#if DEBUG
            printf("move/rename failed\n");
            //write_system_log("error","uploadfile fail");
#endif
            free(path);
            return status;
        }
    }
    else if(strcmp(cmd_name, "dragfolder") == 0)
    {
    	sprintf(fullname,"%s/%s",path,filename);
    	//sprintf(fullnametmp,"%s/%s",path,filename);
    	//wd_escape(fullnametmp,escapepath);
        //create_folder_cmp = 0;
        status = createFolder(fullname,index);
        //printf("create_folder_cmp = %d\n",create_folder_cmp);
        free(fullname);
    	if(status != 0)
    	{
#if DEBUG
            printf("createFolder failed status = %d\n",status);
    	    //write_system_log("error","uploadfile fail");
#endif
            free(path);         
    	    return status;
    	}
    }
    else if(strcmp(cmd_name, "createfolder") == 0)
    {
        sprintf(fullname,"%s/%s",path,filename);
        //sprintf(fullnametmp,"%s/%s",path,filename);
        //wd_escape(fullnametmp,escapepath);
        //create_folder_cmp = 0;
        status = Mkcol(fullname,index);
        //printf("create_folder_cmp = %d\n",create_folder_cmp);
        free(fullname);
        if(status != 0)
        {
#if DEBUG
            printf("createFolder failed status = %d\n",status);
            //write_system_log("error","uploadfile fail");
#endif
            free(path);
            return status;
        }
    }

    free(path);
    //free(fullname);
    return 0;
}

int download_only_add_socket_item(char *cmd,int index){

    printf("download_only_add_socket_item receive socket : %s\n",cmd);

    if( strstr(cmd,"(conflict)") != NULL )
        return 0;

    printf("socket command is %s \n",cmd);


    if( !strncmp(cmd,"exit",4))
    {
        printf("exit socket\n");
        return 0;
    }

    if(!strncmp(cmd,"rmroot",6))
    {
        g_pSyncList[index]->no_local_root = 1;
        return 0;
    }


    char cmd_name[64];
    char *path = NULL;
    char *temp = NULL;
    char filename[256];
    char *fullname = NULL;
    char oldname[256],newname[256];
    char *oldpath = NULL;
    char action[64];
    //char *cmp_name = NULL;
    //char *mv_newpath;
    //char *mv_oldpath;
    char *ch = NULL;
    char *old_fullname = NULL;
    //int status;

    memset(cmd_name,'\0',sizeof(cmd_name));
    memset(oldname,'\0',sizeof(oldname));
    memset(newname,'\0',sizeof(newname));
    memset(action,'\0',sizeof(action));

    ch = cmd;
    int i = 0;
    while(*ch != '@')
    {
        i++;
        ch++;
    }

    memcpy(cmd_name, cmd, i);

    char *p = NULL;
    ch++;
    i++;

    temp = my_str_malloc((size_t)(strlen(ch)+1));

    strcpy(temp,ch);
    p = strchr(temp,'@');

    //printf("temp = %s\n",temp);
    //printf("p = %s\n",p);
    //printf("strlen(temp)- strlen(p) = %d\n",strlen(temp)- strlen(p));

    path = my_str_malloc((size_t)(strlen(temp)- strlen(p)+1));

    //printf("path = %s\n",path);

    if(p!=NULL)
        snprintf(path,strlen(temp)- strlen(p)+1,"%s",temp);

    //free(temp);

    p++;
    if(strcmp(cmd_name, "rename") == 0)
    {
        char *p1 = NULL;

        p1 = strchr(p,'@');

        if(p1 != NULL)
            strncpy(oldname,p,strlen(p)- strlen(p1));

        p1++;

        strcpy(newname,p1);
        printf("cmd_name: [%s],path: [%s],oldname: [%s],newname: [%s]\n",cmd_name,path,oldname,newname);
    }
    else if(strcmp(cmd_name, "move") == 0)
    {
        char *p1 = NULL;

        p1 = strchr(p,'@');

        oldpath = my_str_malloc((size_t)(strlen(p)- strlen(p1)+1));

        if(p1 != NULL)
            snprintf(oldpath,strlen(p)- strlen(p1)+1,"%s",p);

        p1++;

        strcpy(oldname,p1);

        printf("cmd_name: [%s],path: [%s],oldpath: [%s],oldname: [%s]\n",cmd_name,path,oldpath,oldname);
        /*if(strncmp(path,asus_cfg.prule[index]->base_path,asus_cfg.prule[index]->base_path_len) != 0)
        {
            int k;
            for(k=0;k<asus_cfg.dir_number;k++)
            {
                if(strstr(oldpath,asus_cfg.prule[k]->base_path))
                    break;
            }
            pthread_mutex_lock(&mutex_receve_socket);
            //receve_socket = 1;
            g_pSyncList[k]->receve_socket = 1;
            pthread_mutex_unlock(&mutex_receve_socket);

            pthread_mutex_lock(&mutex_socket);
            SocketActionTmp = malloc (sizeof(struct queue_entry));
            memset(SocketActionTmp,0,sizeof(struct queue_entry));
            sprintf(SocketActionTmp->cmd_name,"%s",cmd);
            queue_enqueue(SocketActionTmp,g_pSyncList[k]->SocketActionList);
            printf("SocketActionTmp->cmd_name = %s\n",SocketActionTmp->cmd_name);
            pthread_mutex_unlock(&mutex_socket);

            free(temp);
            free(path);
            free(oldpath);
            return 0;
        }*/
    }
    else
    {
        strcpy(filename,p);
        //fullname = my_str_malloc((size_t)(strlen(path)+strlen(filename)+2));
        printf("cmd_name: [%s],path: [%s],filename: [%s]\n",cmd_name,path,filename);
    }

    free(temp);

    /*if( !strncmp(cmd_name,"copyfile",strlen("copyfile")) )
    {
        add_action_item("copyfile",fullname,g_pSyncList[index]->copy_file_list);
        return 0;
    }*/
    if( !strncmp(cmd_name,"rename",strlen("rename")) )
    {
        fullname = my_str_malloc((size_t)(strlen(path)+strlen(newname)+2));
        old_fullname = my_str_malloc((size_t)(strlen(path)+strlen(oldname)+2));
        sprintf(fullname,"%s/%s",path,newname);
        sprintf(old_fullname,"%s/%s",path,oldname);
        free(path);
    }
    else if( !strncmp(cmd_name,"move",strlen("move")) )
    {
        fullname = my_str_malloc((size_t)(strlen(path)+strlen(oldname)+2));
        old_fullname = my_str_malloc((size_t)(strlen(oldpath)+strlen(oldname)+2));
        sprintf(fullname,"%s/%s",path,oldname);
        sprintf(old_fullname,"%s/%s",oldpath,oldname);
        free(oldpath);
        free(path);
    }
    else
    {
        fullname = my_str_malloc((size_t)(strlen(path)+strlen(filename)+2));
        sprintf(fullname,"%s/%s",path,filename);
        free(path);
    }

    if( !strncmp(cmd_name,"copyfile",strlen("copyfile")) )
    {
        add_action_item("copyfile",fullname,g_pSyncList[index]->copy_file_list);
        return 0;
    }

    /*if( !strcmp(cmd_name,"rename") )
        sprintf(cmp_name,"%s/%s",socket_cmd.path,socket_cmd.newname);
    else
        sprintf(cmp_name,"%s/%s",socket_cmd.path,socket_cmd.filename);*/

    if( strcmp(cmd_name, "createfile") == 0 )
    {
        strcpy(action,"createfile");
        action_item *item;

        item = get_action_item("copyfile",fullname,g_pSyncList[index]->copy_file_list,index);

        if(item != NULL)
        {
            printf("##### delete copyfile %s ######\n",fullname);
            //pthread_mutex_lock(&mutex);
            del_action_item("copyfile",fullname,g_pSyncList[index]->copy_file_list);
        }
    }
    else if( strcmp(cmd_name, "remove") == 0  || strcmp(cmd_name, "delete") == 0)
    {
        strcpy(action,"remove");
        del_download_only_action_item(action,fullname,g_pSyncList[index]->download_only_socket_head);
    }
    else if( strcmp(cmd_name, "createfolder") == 0 )
    {
        strcpy(action,"createfolder");
    }
    else if( strcmp(cmd_name, "rename") == 0 )
    {
        strcpy(action,"rename");
        del_download_only_action_item(action,old_fullname,g_pSyncList[index]->download_only_socket_head);
        free(old_fullname);
    }
    else if( strcmp(cmd_name, "move") == 0 )
    {
        strcpy(action,"move");
        del_download_only_action_item(action,old_fullname,g_pSyncList[index]->download_only_socket_head);
        //free(old_fullname);
    }

    if(g_pSyncList[index]->server_action_list->next != NULL)
    {
        action_item *item;

        item = get_action_item(action,fullname,g_pSyncList[index]->server_action_list,index);

        if(item != NULL)
        {
            printf("##### %s %s by WebDAV Server self ######\n",action,fullname);
            //pthread_mutex_lock(&mutex);
            del_action_item(action,fullname,g_pSyncList[index]->server_action_list);

            //pthread_mutex_unlock(&mutex);
            //local_sync = 0;
            free(fullname);
            return 0;
        }
    }

    if(g_pSyncList[index]->dragfolder_action_list->next != NULL)
    {
        action_item *item;

        item = get_action_item(action,fullname,g_pSyncList[index]->dragfolder_action_list,index);

        if(item != NULL)
        {
            printf("##### %s %s by dragfolder recursion self ######\n",action,fullname);
            //pthread_mutex_lock(&mutex);
            del_action_item(action,fullname,g_pSyncList[index]->dragfolder_action_list);

            //pthread_mutex_unlock(&mutex);
            //local_sync = 0;
            free(fullname);
            return 0;
        }
    }

    if( strcmp(cmd_name, "copyfile") != 0 )
    {
        g_pSyncList[index]->have_local_socket = 1;
    }


    //}
    //printf("add download_only_socket_head fullname = %s\n",fullname);
    if(strcmp(cmd_name, "rename") == 0)
    {
        if(test_if_dir(fullname))
        {
            add_all_download_only_socket_list(cmd_name,fullname,index);
        }
        else
        {
            add_action_item(cmd_name,fullname,g_pSyncList[index]->download_only_socket_head);
        }
    }
    else if(strcmp(cmd_name, "move") == 0)
    {
        int old_index;
        old_index = get_path_to_index(old_fullname);
        if(old_index == index)
        {
            if(test_if_dir(fullname))
            {
                add_all_download_only_socket_list(cmd_name,fullname,index);
            }
            else
            {
                add_action_item(cmd_name,fullname,g_pSyncList[index]->download_only_socket_head);
            }
        }
        else
        {
            if(asus_cfg.prule[old_index]->rule == 1)
            {
                del_download_only_action_item("",old_fullname,g_pSyncList[old_index]->download_only_socket_head);
            }
            else
            {
                Delete(old_fullname,old_index);
            }
            if(test_if_dir(fullname))
            {
                add_all_download_only_socket_list(cmd_name,fullname,index);
            }
            else
            {
                add_action_item(cmd_name,fullname,g_pSyncList[index]->download_only_socket_head);
            }
        }
        free(old_fullname);

    }
    else if(strcmp(cmd_name, "createfolder") == 0 || strcmp(cmd_name, "dragfolder") == 0)
    {
        add_action_item(cmd_name,fullname,g_pSyncList[index]->download_only_socket_head);
        add_all_download_only_dragfolder_socket_list(fullname,index);
    }
    else if( strcmp(cmd_name, "createfile") == 0  || strcmp(cmd_name, "dragfile") == 0 || strcmp(cmd_name, "modify") == 0)
    {
        add_action_item(cmd_name,fullname,g_pSyncList[index]->download_only_socket_head);
    }

    free(fullname);
    return 0;
}

static int startelm(void *userdata, int state,
                    const char *nspace, const char *name,
                    const char **atts)
{
    //printf("startelm\n");
    struct context *ctx = userdata;
    ne_buffer *buf = ctx->buf;
    //printf("buf->data = %s\n",buf->data);
    //printf("nspace = %s\n",nspace);
    //printf("name = %s\n",name);
    int n;

    if (strcmp(name, "decline") == 0)
        return NE_XML_DECLINE;

    if (strcmp(name, EVAL_DEFAULT) == 0) {
        //printf("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\n");
        const char *val = ne_xml_resolve_nspace(ctx->parser, NULL, 0);
        //printf("val = %s\n",val);

        ne_buffer_concat(ctx->buf, EVAL_DEFAULT "=[", val, "]", NULL);
        return NE_XML_DECLINE;
    }
    else if (strncmp(name, EVAL_SPECIFIC, strlen(EVAL_SPECIFIC)) == 0) {
        //printf("bbbbbbbbbbbbbbbbbbbbbbbbbbbb\n");
        const char *which = name + strlen(EVAL_SPECIFIC);
        const char *r = ne_xml_resolve_nspace(ctx->parser, which, strlen(which));

        ne_buffer_concat(ctx->buf, name, "=[", r, "]", NULL);
        return NE_XML_DECLINE;
    }

    ne_buffer_concat(buf, "<", name, NULL);
    for (n = 0; atts && atts[n] != NULL; n+=2) {
        ne_buffer_concat(buf, " ", atts[n], "='", atts[n+1], "'", NULL);
    }
    ne_buffer_zappend(buf, ">");

    //printf("buf->data = %s\n",buf->data);
    //printf("startelm finished!\n");

    return state + 1;
}

static int chardata(void *userdata, int state, const char *cdata, size_t len)
{
    struct context *ctx = userdata;
    ne_buffer_append(ctx->buf, cdata, len);
    return strncmp(cdata, "!ABORT!", len) == 0 ? ABORT : NE_XML_DECLINE;
}

static int endelm(void *userdata, int state,
                  const char *nspace, const char *name)
{
    //printf("endelm\n");
    struct context *ctx = userdata;
    ne_buffer_concat(ctx->buf, "</", name, ">", NULL);
    return 0;
}

#if 0
static int  startelm(void *userdata, int state,
                     const char *nspace, const char *name,
                     const char **atts)
{
    //printf("startelm\n");
    struct context *ctx = userdata;
    ne_buffer *buf = ctx->buf;
    //printf("buf->data = %s\n",buf->data);
    //printf("nspace = %s\n",nspace);
    //printf("name = %s\n",name);
    int n;

    if (strcmp(name, "decline") == 0)
        return NE_XML_DECLINE;

    if (strcmp(name, EVAL_DEFAULT) == 0) {
    	//printf("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\n");
        const char *val = ne_xml_resolve_nspace(ctx->parser, NULL, 0);
        //printf("val = %s\n",val);

        ne_buffer_concat(ctx->buf, EVAL_DEFAULT "=[", val, "]", NULL);
        return NE_XML_DECLINE;
    }
    else if (strncmp(name, EVAL_SPECIFIC, strlen(EVAL_SPECIFIC)) == 0) {
    	//printf("bbbbbbbbbbbbbbbbbbbbbbbbbbbb\n");
        const char *which = name + strlen(EVAL_SPECIFIC);
        const char *r = ne_xml_resolve_nspace(ctx->parser, which, strlen(which));
        
        ne_buffer_concat(ctx->buf, name, "=[", r, "]", NULL);
        return NE_XML_DECLINE;
    }

    if(!(strcmp(name,"href")))
    	infoType = 1;
    else if(!(strcmp(name,"creationdate")))
    	infoType = 2;
    else if(!(strcmp(name,"getcontentlanguage")))
        infoType = 3;
    else if(!(strcmp(name,"getcontentlength")))
        infoType = 4;
    else if(!(strcmp(name,"getcontenttype")))
        infoType = 5;
    else if(!(strcmp(name,"getlastmodified")))
        infoType = 6;
    else if(!(strcmp(name,"status")))
        infoType = 7;
    //else
    //infoType = 0;
    //printf("infoType = %d\n",infoType);

    ne_buffer_concat(buf, "<", nspace, name, NULL);
    for (n = 0; atts && atts[n] != NULL; n+=2) {
	ne_buffer_concat(buf, " ", atts[n], "='", atts[n+1], "'", NULL);
    }
    ne_buffer_zappend(buf, ">");

    //printf("buf->data = %s\n",buf->data);
    //printf("startelm finished!\n");

    return state + 1;
}
#endif


#if 0
int chardata(void *userdata, int state, const char *cdata, size_t len)
{
    //printf("chardata start!\n");
    struct context *ctx = userdata;
    //memset(cloudfiles[cloudfilesContent],0,sizeof(CloudFile));
    //printf("len = %d\n",len);
    //printf("cdata = %s\n",cdata);
    //printf("ctx->buf->data = %s\n",ctx->buf->data);
    ne_buffer_append(ctx->buf, cdata, len);
    //printf("cdata1 = %s\n",cdata);
    //printf("ctx->buf->data1 = %s\n",ctx->buf->data);
    //if((*cdata) != '\r' && (*cdata) !='\n'){
    //printf("infoType = %d\n",infoType);
    switch(infoType){
    case 1:	//printf("len = %d\n",len);
        //printf("cdata = %s\n",cdata);
        FolderTmp = (CloudFile *)malloc(sizeof(CloudFile));
        memset(FolderTmp,0,sizeof(CloudFile));
        strncpy(FolderTmp->href,cdata,len);
        printf("href = %s\n",FolderTmp->href);
        break;
    case 2:	strncpy(FolderTmp->creationdate,cdata,len);
        //printf("creationdate = %s\n",FolderTmp->creationdate);
        break;
    case 3:	strncpy(FolderTmp->getcontentlanguage,cdata,len);
        //printf("getcontentlanguage = %s\n",FolderTmp->getcontentlanguage);
        break;
    case 4:	strncpy(FolderTmp->getcontentlength,cdata,len);
        //printf("getcontentlength = %s\n",FolderTmp->getcontentlength);
        break;
    case 5:	strncpy(FolderTmp->getcontenttype,cdata,len);
        if(!(strcmp(FolderTmp->getcontenttype,"httpd/unix-directory")))
            FolderTmp->isFolder = 1;
        else
            FolderTmp->isFolder = 0;
        //printf("getcontenttype = %s\n",FolderTmp->getcontenttype);
        break;
    case 6:	strncpy(FolderTmp->getlastmodified,cdata,len);
        printf("getlastmodified = %s\n",FolderTmp->getlastmodified);
        FolderTmp->modtime =ne_httpdate_parse(FolderTmp->getlastmodified);
        printf("modtime = %lu\n",FolderTmp->modtime);
        break;
    case 7:	strncpy(FolderTmp->status,cdata,len);
        //FolderTmp->next = NULL;
        //printf("status = %s\n",FolderTmp->status);
        if(FolderTmp->isFolder == 1)
        {
            FolderTail->next = FolderTmp;
            FolderTail = FolderTmp;
            FolderTail->next = NULL;
            //printf("status = %s\n",FolderTail->status);
        }
        else if(FolderTmp->isFolder == 0)
        {
            FileTail->next = FolderTmp;
            FileTail = FolderTmp;
            FileTail->next = NULL;
            //printf("status = %s\n",FileTail->status);
        }
        //free(FolderTmp);
        break;
    	default:break;
    	}
    infoType = 0;
    //}


    //printf("chardata finished!\n");
    return strncmp(cdata, "!ABORT!", len) == 0 ? ABORT : NE_XML_DECLINE;

    //return NE_XML_DECLINE;
}

int chardata_tree(void *userdata, int state, const char *cdata, size_t len)
{
    //printf("chardata start!\n");
    struct context *ctx = userdata;
    //memset(cloudfiles[cloudfilesContent],0,sizeof(CloudFile));
    //printf("len = %d\n",len);
    //printf("cdata = %s\n",cdata);
    //printf("ctx->buf->data = %s\n",ctx->buf->data);
    ne_buffer_append(ctx->buf, cdata, len);
    //printf("cdata1 = %s\n",cdata);
    //printf("ctx->buf->data1 = %s\n",ctx->buf->data);
    //if((*cdata) != '\r' && (*cdata) !='\n'){
    //printf("infoType = %d\n",infoType);
    switch(infoType){
    case 1:	//printf("len = %d\n",len);
        //printf("cdata = %s\n",cdata);
        FolderTmp = (CloudFile *)malloc(sizeof(CloudFile));
        memset(FolderTmp,0,sizeof(CloudFile));
        strncpy(FolderTmp->href,cdata,len);
        //printf("href = %s\n",FolderTmp->href);
        break;
    case 2:	strncpy(FolderTmp->creationdate,cdata,len);
        //printf("creationdate = %s\n",FolderTmp->creationdate);
        break;
    case 3:	strncpy(FolderTmp->getcontentlanguage,cdata,len);
        //printf("getcontentlanguage = %s\n",FolderTmp->getcontentlanguage);
        break;
    case 4:	strncpy(FolderTmp->getcontentlength,cdata,len);
        //printf("getcontentlength = %s\n",FolderTmp->getcontentlength);
        break;
    case 5:	strncpy(FolderTmp->getcontenttype,cdata,len);
        if(!(strcmp(FolderTmp->getcontenttype,"httpd/unix-directory")))
            FolderTmp->isFolder = 1;
        else
            FolderTmp->isFolder = 0;
        //printf("getcontenttype = %s\n",FolderTmp->getcontenttype);
        break;
    case 6:	strncpy(FolderTmp->getlastmodified,cdata,len);
        //printf("getlastmodified = %s\n",FolderTmp->getlastmodified);
        FolderTmp->modtime =ne_httpdate_parse(FolderTmp->getlastmodified);
        //printf("modtime = %lu\n",FolderTmp->modtime);
        break;
    case 7:	strncpy(FolderTmp->status,cdata,len);
        //FolderTmp->next = NULL;
        //printf("status = %s\n",FolderTmp->status);
        if(FolderTmp->isFolder == 1)
        {
            TreeFolderTail->next = FolderTmp;
            TreeFolderTail = FolderTmp;
            TreeFolderTail->next = NULL;
            //printf("status = %s\n",FolderTail->status);
        }
        else if(FolderTmp->isFolder == 0)
        {
            TreeFileTail->next = FolderTmp;
            TreeFileTail = FolderTmp;
            TreeFileTail->next = NULL;
            //printf("status = %s\n",FileTail->status);
        }
        //free(FolderTmp);
        break;
        default:break;
        }
    infoType = 0;
    //}


    //printf("chardata finished!\n");
    return strncmp(cdata, "!ABORT!", len) == 0 ? ABORT : NE_XML_DECLINE;

    //return NE_XML_DECLINE;
}

int chardata_one(void *userdata, int state, const char *cdata, size_t len)
{
    //printf("chardata_one start!\n");
    struct context *ctx = userdata;
    //memset(cloudfiles[cloudfilesContent],0,sizeof(CloudFile));
    //printf("len = %d\n",len);
    //printf("cdata = %s\n",cdata);
    //printf("ctx->buf->data = %s\n",ctx->buf->data);
    ne_buffer_append(ctx->buf, cdata, len);
    //printf("cdata1 = %s\n",cdata);
    //printf("ctx->buf->data1 = %s\n",ctx->buf->data);
    //if((*cdata) != '\r' && (*cdata) !='\n'){
    //printf("infoType = %d\n",infoType);
    switch(infoType){
    case 1:	/*printf("len = %d\n",len);
    			printf("cdata = %s\n",cdata);*/
        FileTmp_one = (CloudFile *)malloc(sizeof(CloudFile));
        memset(FileTmp_one,0,sizeof(CloudFile));
        strncpy(FileTmp_one->href,cdata,len);
        //printf("href = %s\n",FileTmp_one->href);
        break;
    case 2:	strncpy(FileTmp_one->creationdate,cdata,len);
        //printf("creationdate = %s\n",FolderTmp->creationdate);
        break;
    case 3:	strncpy(FileTmp_one->getcontentlanguage,cdata,len);
        //printf("getcontentlanguage = %s\n",FolderTmp->getcontentlanguage);
        break;
    case 4:	strncpy(FileTmp_one->getcontentlength,cdata,len);
        //printf("getcontentlength = %s\n",FolderTmp->getcontentlength);
        break;
    case 5:	strncpy(FileTmp_one->getcontenttype,cdata,len);
        if(!(strcmp(FileTmp_one->getcontenttype,"httpd/unix-directory")))
            FileTmp_one->isFolder = 1;
        else
            FileTmp_one->isFolder = 0;
        //printf("getcontenttype = %s\n",FolderTmp->getcontenttype);
        break;
    case 6:	strncpy(FileTmp_one->getlastmodified,cdata,len);
        //printf("getlastmodified = %s\n",FolderTmp->getlastmodified);
        FileTmp_one->modtime =ne_httpdate_parse(FileTmp_one->getlastmodified);
        //printf("modtime = %lu\n",FileTmp_one->modtime);
        break;
    case 7:	strncpy(FileTmp_one->status,cdata,len);
        //FolderTmp->next = NULL;
        //printf("status = %s\n",FolderTmp->status);
        FileTail_one->next = FileTmp_one;
        FileTail_one = FileTmp_one;
        FileTail_one->next = NULL;
        break;
    default:break;
    }
    infoType = 0;
    //}


    //printf("chardata finished!\n");
    return strncmp(cdata, "!ABORT!", len) == 0 ? ABORT : NE_XML_DECLINE;

    //return NE_XML_DECLINE;
}
#endif


#if 0
/*parse CloudeInfo to ServerList*/
void parseCloudInfo(xmlDocPtr doc, xmlNodePtr cur)
{
    //printf("*********parseCloudInfo start **********\n");
    xmlChar *key;
    cur = cur->xmlChildrenNode;
    //printf("cur->name = %s\n",cur->name);
    while (cur != NULL)
    {
        key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);

        if(!(xmlStrcmp(cur->name, (const xmlChar *)"response")))
        {
            parseCloudInfo(doc,cur);
            if(FolderTmp->isFolder == 3)
            {
                free_CloudFile_item(FolderTmp);
            }
            /*else
            {
                printf("get the info right!\n");
            }*/
        }
        else if(!(xmlStrcmp(cur->name, (const xmlChar *)"propstat")))
            parseCloudInfo(doc,cur);
        else if(!(xmlStrcmp(cur->name, (const xmlChar *)"prop")))
            parseCloudInfo(doc,cur);
        else if(!(xmlStrcmp(cur->name, (const xmlChar *)"href")))
        {
            FolderTmp = (CloudFile *)malloc(sizeof(CloudFile));
            memset(FolderTmp,0,sizeof(CloudFile));
            FolderTmp->next = NULL;
            FolderTmp->href = (char *)malloc(sizeof(char)*(strlen(key)+1));
            memset(FolderTmp->href,'\0',sizeof(FolderTmp->href));
            strcpy(FolderTmp->href,key);
            FolderTmp->isFolder = 3;
            //printf("href = %s\n",FolderTmp->href);
        }
        else if(!(xmlStrcmp(cur->name, (const xmlChar *)"creationdate")))
        {
            strcpy(FolderTmp->creationdate,key);
            //printf("creationdate = %s\n",FolderTmp->creationdate);
        }
        else if(!(xmlStrcmp(cur->name, (const xmlChar *)"getcontentlanguage")))
        {
            strcpy(FolderTmp->getcontentlanguage,key);
            //printf("getcontentlanguage = %s\n",FolderTmp->getcontentlanguage);
        }
        else if(!(xmlStrcmp(cur->name, (const xmlChar *)"getcontentlength")))
        {
            strcpy(FolderTmp->getcontentlength,key);
            //printf("getcontentlength = %s\n",FolderTmp->getcontentlength);
        }
        else if(!(xmlStrcmp(cur->name, (const xmlChar *)"getcontenttype")))
        {
            strcpy(FolderTmp->getcontenttype,key);
            if(!(strcmp(FolderTmp->getcontenttype,"httpd/unix-directory")))
                FolderTmp->isFolder = 1;
            else
                FolderTmp->isFolder = 0;
            //printf("getcontenttype = %s\n",FolderTmp->getcontenttype);
        }
        else if(!(xmlStrcmp(cur->name, (const xmlChar *)"getlastmodified")))
        {
            strcpy(FolderTmp->getlastmodified,key);
            //printf("getlastmodified = %s\n",FolderTmp->getlastmodified);
            FolderTmp->modtime = ne_httpdate_parse(FolderTmp->getlastmodified);
            //printf("modtime = %lu\n",FolderTmp->modtime);
        }
        else if(!(xmlStrcmp(cur->name, (const xmlChar *)"status")))
        {
            strcpy(FolderTmp->status,key);
            //printf("href = %s\n",FolderTmp->href);
            //printf("FolderTmp->status = %s\n",FolderTmp->status);
            //printf("FolderTmp->isFolder = %d\n",FolderTmp->isFolder);
            if(FolderTmp->isFolder == 1)
            {
                //printf("href = %s\n",FolderTmp->href);
                FolderTail->next = FolderTmp;
                FolderTail = FolderTmp;
                FolderTail->next = NULL;
                //printf("status = %s\n",FolderTail->status);
            }
            else if(FolderTmp->isFolder == 0)
            {
                //printf("href = %s\n",FolderTmp->href);
                FileTail->next = FolderTmp;
                FileTail = FolderTmp;
                FileTail->next = NULL;
                //printf("status = %s\n",FileTail->status);
            }
        }
        //else if(!(xmlStrcmp(cur->name, (const xmlChar *)"/response")))
        //printf("RRRRRRRRRRRRRRRRRRRRRRRRresponse over!\n");

        //printf("cur->name = %s\n",cur->name);
        //printf("key = %s\n",key);

        xmlFree(key);
        cur = cur->next;
    }
}
#endif

/*parse CloudeInfo to tree_ServerList*/
void parseCloudInfo_tree(xmlDocPtr doc, xmlNodePtr cur)
{
    xmlChar *key;
    cur = cur->xmlChildrenNode;
    while (cur != NULL)
    {
        key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);

        if(!(xmlStrcmp(cur->name, (const xmlChar *)"response")))
        {
            parseCloudInfo_tree(doc,cur);
            if(FolderTmp->isFolder == 3)
            {
                free_CloudFile_item(FolderTmp);
            }
            /*else
            {
                printf("get the info right!\n");
            }*/
        }
        else if(!(xmlStrcmp(cur->name, (const xmlChar *)"propstat")))
            parseCloudInfo_tree(doc,cur);
        else if(!(xmlStrcmp(cur->name, (const xmlChar *)"prop")))
            parseCloudInfo_tree(doc,cur);
        else if(!(xmlStrcmp(cur->name, (const xmlChar *)"href")))
        {
            FolderTmp = (CloudFile *)malloc(sizeof(CloudFile));
            memset(FolderTmp,0,sizeof(CloudFile));
            FolderTmp->next = NULL;
            FolderTmp->href = (char *)malloc(sizeof(char)*(strlen(key)+1));
            memset(FolderTmp->href,'\0',sizeof(FolderTmp->href));
            strcpy(FolderTmp->href,key);
            FolderTmp->isFolder = 3;
            //printf("href = %s\n",FolderTmp->href);
        }
        else if(!(xmlStrcmp(cur->name, (const xmlChar *)"creationdate")))
        {
            strcpy(FolderTmp->creationdate,key);
        }
        else if(!(xmlStrcmp(cur->name, (const xmlChar *)"getcontentlanguage")))
        {
            strcpy(FolderTmp->getcontentlanguage,key);
        }
        else if(!(xmlStrcmp(cur->name, (const xmlChar *)"getcontentlength")))
        {
            strcpy(FolderTmp->getcontentlength,key);
        }
        else if(!(xmlStrcmp(cur->name, (const xmlChar *)"getcontenttype")))
        {
            strcpy(FolderTmp->getcontenttype,key);
            if(!(strcmp(FolderTmp->getcontenttype,"httpd/unix-directory")))
                FolderTmp->isFolder = 1;
            else
                FolderTmp->isFolder = 0;
        }
        else if(!(xmlStrcmp(cur->name, (const xmlChar *)"getlastmodified")))
        {
            strcpy(FolderTmp->getlastmodified,key);
            FolderTmp->modtime = ne_httpdate_parse(FolderTmp->getlastmodified);
        }
        else if(!(xmlStrcmp(cur->name, (const xmlChar *)"status")))
        {
            strcpy(FolderTmp->status,key);
            if(FolderTmp->isFolder == 1)
            {
                TreeFolderTail->next = FolderTmp;
                TreeFolderTail = FolderTmp;
                TreeFolderTail->next = NULL;
                //printf("status = %s\n",FolderTail->status);
            }
            else if(FolderTmp->isFolder == 0)
            {
                TreeFileTail->next = FolderTmp;
                TreeFileTail = FolderTmp;
                TreeFileTail->next = NULL;
                //printf("status = %s\n",FileTail->status);
            }
        }
        //else if(!(xmlStrcmp(cur->name, (const xmlChar *)"/response")))
        //printf("RRRRRRRRRRRRRRRRRRRRRRRRresponse over!\n");

        //printf("cur->name = %s\n",cur->name);
        //printf("key = %s\n",key);

        xmlFree(key);
        cur = cur->next;
    }
}

/*parse CloudeInfo to one_ServerList*/
void parseCloudInfo_one(xmlDocPtr doc, xmlNodePtr cur)
{
    xmlChar *key;
    cur = cur->xmlChildrenNode;
    while (cur != NULL)
    {
        //printf("cur->name = %s\n",cur->name);
        key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);

        if(!(xmlStrcmp(cur->name, (const xmlChar *)"response")))
        {
            parseCloudInfo_one(doc,cur);
            if(FileTmp_one->isFolder == 3)
            {
                free_CloudFile_item(FileTmp_one);
            }
            /*else
            {
                printf("get the info right!\n");
            }*/
        }
        else if(!(xmlStrcmp(cur->name, (const xmlChar *)"propstat")))
            parseCloudInfo_one(doc,cur);
        else if(!(xmlStrcmp(cur->name, (const xmlChar *)"prop")))
            parseCloudInfo_one(doc,cur);
        else if(!(xmlStrcmp(cur->name, (const xmlChar *)"href")))
        {
            FileTmp_one = (CloudFile *)malloc(sizeof(CloudFile));
            memset(FileTmp_one,0,sizeof(CloudFile));
            FileTmp_one->next = NULL;
            FileTmp_one->href = (char *)malloc(sizeof(char)*(strlen(key)+1));
            memset(FileTmp_one->href,'\0',sizeof(FileTmp_one->href));
            strcpy(FileTmp_one->href,key);
            FileTmp_one->isFolder = 3;
            //printf("href = %s\n",FileTmp_one->href);
        }
        else if(!(xmlStrcmp(cur->name, (const xmlChar *)"creationdate")))
        {
            strcpy(FileTmp_one->creationdate,key);
        }
        else if(!(xmlStrcmp(cur->name, (const xmlChar *)"getcontentlanguage")))
        {
            strcpy(FileTmp_one->getcontentlanguage,key);
        }
        else if(!(xmlStrcmp(cur->name, (const xmlChar *)"getcontentlength")))
        {
            strcpy(FileTmp_one->getcontentlength,key);
        }
        else if(!(xmlStrcmp(cur->name, (const xmlChar *)"getcontenttype")))
        {
            strcpy(FileTmp_one->getcontenttype,key);
            if(!(strcmp(FileTmp_one->getcontenttype,"httpd/unix-directory")))
                FileTmp_one->isFolder = 1;
            else
                FileTmp_one->isFolder = 0;
        }
        else if(!(xmlStrcmp(cur->name, (const xmlChar *)"getlastmodified")))
        {
            strcpy(FileTmp_one->getlastmodified,key);
            FileTmp_one->modtime = ne_httpdate_parse(FileTmp_one->getlastmodified);
        }
        else if(!(xmlStrcmp(cur->name, (const xmlChar *)"status")))
        {
            strcpy(FileTmp_one->status,key);
            FileTail_one->next = FileTmp_one;
            FileTail_one = FileTmp_one;
            FileTail_one->next = NULL;
        }

        //printf("cur->name = %s\n",cur->name);
        //printf("key = %s\n",key);

        xmlFree(key);
        cur = cur->next;
    }
}

void parseRouterInfo(xmlDocPtr doc, xmlNodePtr cur)
{
    xmlChar *key;
    cur = cur->xmlChildrenNode;
    while (cur != NULL)
    {
        key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
        //printf("key = %s\n",key);

        if(!(xmlStrcmp(cur->name, (const xmlChar *)"result")))
        {
            parseRouterInfo(doc,cur);
        }
        else if(!(xmlStrcmp(cur->name, (const xmlChar *)"disk_space")))
        {
            parseRouterInfo(doc,cur);
        }
        else if(!(xmlStrcmp(cur->name, (const xmlChar *)"item")))
        {
            parseRouterInfo(doc,cur);
        }
        else if(!(xmlStrcmp(cur->name, (const xmlChar *)"DiskAvailable")))
        {
            //printf("key = %s\n",key);
            DiskAvailable = 1024*(atoll(key));
            //printf("DiskAvailable = %lld\n",DiskAvailable);
            DiskAvailableShow = (atoll(key))/1024;
            //printf("DiskAvailableShow = %lld\n",DiskAvailableShow);
        }
        else if(!(xmlStrcmp(cur->name, (const xmlChar *)"DiskUsed")))
        {
            //printf("key = %s\n",key);
            DiskUsedShow = (atoll(key))/1024;
            //printf("DiskUsedShow = %lld\n",DiskUsedShow);
        }

        xmlFree(key);
        cur = cur->next;
    }
}

/*used for get XML root node*/
int my_parseMemory(char *parseBuf,int parseBufLength,void (*cmd_data)(xmlDocPtr, xmlNodePtr)){
    xmlDocPtr doc;
    xmlNodePtr cur;

    doc = xmlParseMemory(parseBuf,parseBufLength);

    if (doc == NULL ) {
        //fprintf(stderr,"%s not parsed successfully. \n",docname);
        printf("parsed failed. \n");
        return -1;
    }

    cur = xmlDocGetRootElement(doc);

    if (cur == NULL) {
        //fprintf(stderr,"%s empty document\n",docname);
        printf("empty document\n");

        xmlFreeDoc(doc);
        return -1;
    }

    while(cur != NULL)
    {
        //printf("cur->name = %s\n",cur->name);
        cmd_data(doc, cur);
        cur = cur->next;
    }

    xmlFreeDoc(doc);


    return 0;
}

int ne_getrouterinfo(void (*cmd_data)(xmlDocPtr, xmlNodePtr),int index)
{
    struct context ctx;
    ne_uri uri = {0};
    //printf("HOST = %s\n",HOST);
    if (ne_uri_parse(asus_cfg.prule[index]->rooturl, &uri) || uri.host==NULL || uri.path==NULL)
    {
        printf("Could not parse url %s\n", asus_cfg.prule[index]->rooturl);
        return -1;
    }

    ne_xml_parser *p;
    ne_buffer *buf1 = ne_buffer_create();
    p = ne_xml_create();

    ctx.buf = buf1;
    ctx.parser = p;
    //printf("uri.path = %s\n",uri.path);
    ne_request *req = ne_request_create(asus_cfg.prule[index]->sess,"GETROUTERINFO",uri.path);

    int ret;

    ne_xml_push_handler(p, startelm, chardata, endelm, &ctx);
    //printf("uri.path = %s\n",uri.path);
    ret = ne_xml_dispatch_request(req, p);
    //printf("ne_getrouterinfo ret = %d\n",ret);

    if(ret == NE_OK && ne_get_status(req)->klass != 2)
    {
        ret = NE_ERROR;
    }

    if(ret != NE_OK)
    {
        ne_xml_destroy(p);
        ne_buffer_destroy(buf1);
        ne_request_destroy(req);
        ne_uri_free(&uri);
        char error_info[200];
        memset(error_info,0,sizeof(error_info));
        sprintf(error_info,"%s",ne_get_error(asus_cfg.prule[index]->sess));
        printf("ERROR info :%s\n ret = %d\n",error_info,ret);
        if(ret == 3)
        {
            //write_log(S_ERROR,"Validation Failed!","",index);
            return INVALID_ARGUMENT;
        }
        if(strstr(error_info,"Could not connect to server") != NULL)
        {
            //write_log(S_ERROR,"Could not connect to server!","",index);
            return COULD_NOT_CONNECNT_TO_SERVER;
        }
        return ret;
    }
    //printf("ret = %d\n",ret);

    //printf("ctx->buf->data1 = %s\n",ctx.buf->data);
    ret = my_parseMemory(ctx.buf->data,(int)ctx.buf->length,cmd_data);
    if(ret != 0)
    {
        DiskAvailable = -1;
        DiskAvailableShow = -1;
        DiskUsedShow = -1;
        ne_xml_destroy(p);
        ne_buffer_destroy(buf1);
        ne_uri_free(&uri);
        ne_request_destroy(req);
        return PARSE_XML_FAILED;
    }
    else
    {
        PreDiskAvailableShow = DiskAvailableShow;
        PreDiskUsedShow = DiskUsedShow;
    }

    ne_xml_destroy(p);
    ne_buffer_destroy(buf1);
    ne_uri_free(&uri);
    ne_request_destroy(req);
    //ne_buffer_destroy(buf);
    //ne_sock_exit();
    //fclose(debug);
    //fclose(rep_body);
    //printf("getCloudInfo finished!\n");
    return ret;
}

int getCloudInfo(char *URL,void (*cmd_data)(xmlDocPtr, xmlNodePtr),int index){
    //printf("****************getCloudInfo****************\n");
    //printf("URL = %s\n",URL);
    struct context ctx;
    //printf("%s\n",URL);
    ne_uri uri = {0};
    //printf("URL = %s\n",URL);
    if (ne_uri_parse(URL, &uri) || uri.host==NULL || uri.path==NULL)
    {
        printf("Could not parse url %s\n", URL);
        return -1;
    }

    //printf("uri.scheme = %s\n",uri.scheme);
    ne_xml_parser *p;
    ne_buffer *buf1 = ne_buffer_create();
    p = ne_xml_create();
    //struct context ctx;

    ctx.buf = buf1;
    ctx.parser = p;
    //printf("uri.scheme = %s\n",uri.scheme);
    //printf("uri.host = %s\n",uri.host);
    //printf("uri.port = %d\n",uri.port);
    //printf("URL = %s\n",URL);

    //printf("uri.path = %s\n",uri.path);
    ne_request *req = ne_request_create(asus_cfg.prule[index]->sess,"PROPFIND",uri.path);

    int ret;

    char buf[128];
    memset(buf,'\0',sizeof(buf));
    sprintf(buf,"<?xml version=\"1.0\" encoding=\"utf-8\"?>\n<propfind xmlns=\"DAV:\"><allprop/></propfind>\n");

    ne_add_request_header(req,"Depth","1");
    ne_add_request_header(req,"Content-Type","application/xml");
    ne_set_request_body_buffer(req,buf,84);

    ne_xml_push_handler(p, startelm, chardata, endelm, &ctx);
    ret = ne_xml_dispatch_request(req, p);

    if(ret == NE_OK && ne_get_status(req)->klass != 2)
    {
        ret = NE_ERROR;
    }

    //ret = ne_xml_parse_response(req, p);
    //printf("ERROR info :%s\n",ne_get_error(sess));
    //printf("ret = %d\n",ret);
    if(ret != NE_OK){
        ne_xml_destroy(p);
        ne_buffer_destroy(buf1);
        ne_request_destroy(req);
        ne_uri_free(&uri);
        //ne_buffer_destroy(buf);
        //ne_sock_exit();
        //fclose(debug);
        //fclose(rep_body);
        //printf("getCloudInfo finished!\n");
        char error_info[200];
        memset(error_info,0,sizeof(error_info));
        sprintf(error_info,"%s",ne_get_error(asus_cfg.prule[index]->sess));
        printf("ERROR info :%s\n ret = %d\n",error_info,ret);
        //write_log(S_ERROR,error_info,"",index);
        if(ret == 3)
        {
            //write_log(S_ERROR,"Validation Failed!","",index);
            return INVALID_ARGUMENT;
        }
        if(strstr(error_info,"Could not connect to server") != NULL)
        {
            //write_log(S_ERROR,"Could not connect to server!","",index);
            return COULD_NOT_CONNECNT_TO_SERVER;
        }
        if(strstr(error_info,"404") != NULL)
        {
            write_log(S_ERROR,"Server Deleted Sync Folder!","",index);
            return SERVER_ROOT_DELETED;
        }
        return ret;
    }
    //printf("ret = %d\n",ret);

    //printf("ctx->buf->data1 = %s\n",ctx.buf->data);
    ret = my_parseMemory(ctx.buf->data,(int)ctx.buf->length,cmd_data);

    ne_xml_destroy(p);
    ne_buffer_destroy(buf1);
    ne_uri_free(&uri);
    ne_request_destroy(req);
    //ne_buffer_destroy(buf);
    //ne_sock_exit();
    //fclose(debug);
    //fclose(rep_body);
    //printf("getCloudInfo finished!\n");
    return ret;
}

Browse *browseFolder(char *URL,int index){
    //printf("browseFolder URL = %s\n",URL);
    int status;
    int i=0;

    Browse *browse = getb(Browse);
    if( NULL == browse )
    {
        printf("create memery error\n");
        exit(-1);
    }
    memset(browse,0,sizeof(Browse));

    TreeFolderList = (CloudFile *)malloc(sizeof(CloudFile));
    memset(TreeFolderList,0,sizeof(CloudFile));
    TreeFileList = (CloudFile *)malloc(sizeof(CloudFile));
    memset(TreeFileList,0,sizeof(CloudFile));

    TreeFolderList->href = NULL;
    TreeFileList->href = NULL;

    TreeFolderTail = TreeFolderList;
    TreeFileTail = TreeFileList;
    TreeFolderTail->next = NULL;
    TreeFileTail->next = NULL;

    status = getCloudInfo(URL,parseCloudInfo_tree,index);
    if(status != 0)
    {
        free_CloudFile_item(TreeFolderList);
        free_CloudFile_item(TreeFileList);
        TreeFolderList = NULL;
        TreeFileList = NULL;
        free(browse);
        //printf("get Cloud Info ERROR! \n");
        return NULL;
    }

    browse->filelist = TreeFileList;
    browse->folderlist = TreeFolderList;

    CloudFile *de_foldercurrent,*de_filecurrent;
    de_foldercurrent = TreeFolderList->next;
    de_filecurrent = TreeFileList->next;
    while(de_foldercurrent != NULL){
        ++i;
        de_foldercurrent = de_foldercurrent->next;
    }
    browse->foldernumber = i;
    i = 0;
    while(de_filecurrent != NULL){
        ++i;
        de_filecurrent = de_filecurrent->next;
    }
    browse->filenumber = i;
    return browse;
}

/*int initMyLocalFolder(){
	CloudFile *initFolder;
	initFolder = FolderList->next;
	char *path;
	char LocalFolderPath[256];
	memset(LocalFolderPath,0,sizeof(LocalFolderPath));
	while(initFolder->href != NULL){
		//if(strlen(initFolder->href) > 32){
		path = strstr(initFolder->href,"/RT-N16/");
		path = oauth_url_unescape(path,NULL);
		sprintf(LocalFolderPath,"%s%s",base_path,path);
		printf("path = %s\n",path);
		printf("getlastmodified = %s\n",initFolder->getlastmodified);
		if(NULL == opendir(LocalFolderPath))
		mkdir(LocalFolderPath,0777);
		//}
		initFolder = initFolder->next;
	}
	return 1;
}*/

/*int initMyLocalFile(){
	CloudFile *initFile;
	initFile = FileList->next;
	char *path;
	while(initFile->href != NULL){
		path = strstr(initFile->href,"/RT-N16/");
		path = oauth_url_unescape(path,NULL);
		printf("path = %s\n",path);
		printf("getlastmodified = %s\n",initFile->getlastmodified);
		Download(initFile->href);
		initFile = initFile->next;
	}
	return 1;
}*/

/*Used for get Server's Folder and File List*/
/*int getServerList(){
    printf("**********getServerList****************\n");

    //char *root_url = "http://10.0.10.91:8082/RT-N16/";   //for N16
    //char *root_url;     //for Local PC
    //char ROOTURL[MAX_CONTENT];
    //memset(ROOTURL,0,sizeof(ROOTURL));
    int status;

    FolderList = (CloudFile *)malloc(sizeof(CloudFile));
    memset(FolderList,0,sizeof(CloudFile));
    FileList = (CloudFile *)malloc(sizeof(CloudFile));
    memset(FileList,0,sizeof(CloudFile));

    FolderList->href = NULL;
    FileList->href = NULL;

    FolderTail = FolderList;
    FileTail = FileList;
    FolderTail->next = NULL;
    FileTail->next = NULL;

    //printf("finished malloc !\n");

    //printf("HOST = %s\n",HOST);
    //printf("ROOTURL = %s\n",ROOTURL);

    status = getCloudInfo(ROOTURL,parseCloudInfo);
    if(status != 0)
    {
        free_CloudFile_item(FolderList);
        free_CloudFile_item(FileList);
        FolderList = NULL;
        FileList = NULL;
        printf("get Cloud Info ERROR! \n");
        return status;
    }

    FolderCurrent = FolderList->next;
    while(FolderCurrent != NULL){
        //if(strlen(FolderCurrent->href) > 32)
        //printf("FolderCurrent->href = %s\n",FolderCurrent->href);
        char *tmp;
        tmp = strstr(FolderCurrent->href,"/RT");
        //printf("tmp = %s\n",tmp);
        //printf("tmp length = %d\n",strlen(tmp));
        if(strlen(tmp) > 8)
            status = getCloudInfo(FolderCurrent->href,parseCloudInfo);
        if(status != 0)
        {
            printf("get Cloud Info ERROR! \n");
            return status;
        }
        FolderCurrent = FolderCurrent->next;
        //printf("FolderCurrent->href = %s\n",FolderCurrent->href);
    }

#if 0
    CloudFile *de_foldercurrent,*de_filecurrent;
    de_foldercurrent = FolderList->next;
    de_filecurrent = FileList->next;
    while(de_foldercurrent != NULL){
        printf("serverfolder->href = %s\n",de_foldercurrent->href);
        de_foldercurrent = de_foldercurrent->next;
    }
    while(de_filecurrent != NULL){
        printf("serverfile->href = %s\n",de_filecurrent->href);
        de_filecurrent = de_filecurrent->next;
    }

#endif
    //char *LocalPath = "/tmp/mnt/eric/tool";   //for N16
    //char *LocalPath = "/home/alan/tool";     //for Local PC

    //initMyLocalFolder();
    //initMyLocalFile();
    return 0;

}*/

#if 1
int send_action(int type, char *content)
{
    if(exit_loop)
    {
        return 0;
    }
    int sockfd, numbytes;
    char buf[MAXDATASIZE];
    char str[1024];
    int port;

    //if(type == 1)
    port = INOTIFY_PORT;

    struct sockaddr_in their_addr; /* connector's address information */


    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        //exit(1);
        return -1;
    }

    bzero(&(their_addr), sizeof(their_addr)); /* zero the rest of the struct */
    their_addr.sin_family = AF_INET; /* host byte order */
    their_addr.sin_port = htons(port); /* short, network byte order */
    their_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    //their_addr.sin_addr.s_addr = ((struct in_addr *)(he->h_addr))->s_addr;
    bzero(&(their_addr.sin_zero), sizeof(their_addr.sin_zero)); /* zero the rest of the struct */
    if (connect(sockfd, (struct sockaddr *)&their_addr,sizeof(struct
                                                              sockaddr)) == -1) {
        perror("connect");
        //exit(1);
        return -1;
    }

    sprintf(str,"%d@%s",type,content);

    //printf("send content is %s \n",str);

    if (send(sockfd, str, strlen(str), 0) == -1) {
        perror("send");
        //exit(1);
        return -1;
    }

    if ((numbytes=recv(sockfd, buf, MAXDATASIZE, 0)) == -1) {
        perror("recv");
        //exit(1);
        return -1;
    }

    buf[numbytes] = '\0';
    close(sockfd);
    printf("send_action finished!\n");
    return 0;
}
#endif

/*char *get_root_path(char *path , int n)
{
    //printf("parse path is %s \n",path);
    int i;
    char *m_path = NULL;
    m_path = my_str_malloc(256);

    //memset(info_dest_path,'\0',sizeof(info_dest_path));

    char *new_path = NULL;
    new_path = path;

    for(i= 0;i< n ;i++)
    {
        new_path = strchr(new_path,'/');
        if(new_path == NULL)
            break;
        new_path++;
    }

    if( i > 3)
        strncpy(m_path,path,strlen(path)-strlen(new_path)-1);
    else
        strcpy(m_path,path);

    printf("root path is [%s]\n",m_path);

    return m_path;
}*/

char *get_socket_base_path(char *cmd){

    //printf("get_socket_base_path cmd : %s\n",cmd);

    char *temp = NULL;
    char *temp1 = NULL;
    char path[1024];
    char *root_path = NULL;

    if(!strncmp(cmd,"rmroot",6))
    {
        temp = strchr(cmd,'/');
        root_path = my_str_malloc(512);
        sprintf(root_path,"%s",temp);
    }
    else
    {
        temp = strchr(cmd,'/');
        temp1 = strchr(temp,'@');
        memset(path,0,sizeof(path));
        strncpy(path,temp,strlen(temp)-strlen(temp1));

        //printf("get_socket_base_path path = %s\n",path);

        root_path = my_str_malloc(512);

        temp = my_nstrchr('/',path,4);
        if(temp == NULL)
        {
            sprintf(root_path,"%s",path);
        }
        else
        {
            snprintf(root_path,strlen(path)-strlen(temp)+1,"%s",path);
        }
    }
    //printf("get_socket_base_path root_path = %s\n",root_path);
    return root_path;
}

int add_socket_item(char *buf){

    int i;
    //local_sync = 1;
    char *r_path;
    r_path = get_socket_base_path(buf);
    for(i=0;i<asus_cfg.dir_number;i++)
    {
        if(!strcmp(r_path,asus_cfg.prule[i]->base_path))
            break;
    }
    free(r_path);
    pthread_mutex_lock(&mutex_receve_socket);
    //receve_socket = 1;
    g_pSyncList[i]->receve_socket = 1;
    pthread_mutex_unlock(&mutex_receve_socket);

    SocketActionTmp = malloc (sizeof (struct queue_entry));
    memset(SocketActionTmp,0,sizeof(struct queue_entry));
    sprintf(SocketActionTmp->cmd_name,"%s",buf);
    queue_enqueue(SocketActionTmp,g_pSyncList[i]->SocketActionList);
    printf("SocketActionTmp->cmd_name = %s\n",SocketActionTmp->cmd_name);

    //free(r_path);
    return 0;
}

#if 1
void *SyncLocal()
{
    //printf("it is go to SyncLocal\n");
    //signal(SIGINT,exit_thread);
    //LocalActionTail = LocalActionList;

    int sockfd, new_fd; /* listen on sock_fd, new connection on new_fd*/
    int numbytes;
    char buf[MAXDATASIZE];
    int yes = 1;
    int ret;

    fd_set read_fds;
    fd_set master;
    int fdmax;
    struct timeval timeout;

    FD_ZERO(&read_fds);
    FD_ZERO(&master);

    struct sockaddr_in my_addr; /* my address information */
    struct sockaddr_in their_addr; /* connector's address information */
    int sin_size;

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }

    if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
    {
    	perror("Server-setsockopt() error lol!");
    	exit(1);
    }

    my_addr.sin_family = AF_INET; /* host byte order */
    my_addr.sin_port = htons(MYPORT); /* short, network byte order */
    my_addr.sin_addr.s_addr = INADDR_ANY; /* auto-fill with my IP */
    bzero(&(my_addr.sin_zero), sizeof(my_addr.sin_zero)); /* zero the rest of the struct */

    if (bind(sockfd, (struct sockaddr *)&my_addr, sizeof(struct
                                                         sockaddr))== -1) {
        perror("bind");
        exit(1);
    }
    if (listen(sockfd, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }
    sin_size = sizeof(struct sockaddr_in);

    FD_SET(sockfd,&master);
    fdmax = sockfd;

    while(!exit_loop)
    { /* main accept() loop */

    	//printf("it is go to SyncLocal's while\n");

        //if(exit_proc == 1)
        //exit(1);
        timeout.tv_sec = 0;
        timeout.tv_usec = 100;

        read_fds = master;

        ret = select(fdmax+1,&read_fds,NULL,NULL,&timeout);

        switch (ret)
        {
        case 0:
            //printf("No data in ten seconds\n");
            continue;
            break;
        case -1:
            perror("select");
            continue;
            break;
        default:
            if ((new_fd = accept(sockfd, (struct sockaddr *)&their_addr, \
                                 &sin_size)) == -1) {
                perror("accept");
                continue;
            }
            memset(buf, 0, sizeof(buf));

            if ((numbytes=recv(new_fd, buf, MAXDATASIZE, 0)) == -1) {
                perror("recv");
                exit(1);
            }

            if(buf[strlen(buf)] == '\n')
            {
                buf[strlen(buf)] = '\0';
            }

            close(new_fd);
            
            //if(sync_down == 1)
            //{


            pthread_mutex_lock(&mutex_socket);
            add_socket_item(buf);
            pthread_mutex_unlock(&mutex_socket);
            //printf("add %s OK \n",buf);
            //}
        }


        //printf("server: got connection from %s\n",inet_ntoa(their_addr.sin_addr));




        //else
        //printf("recv command is %s \n",buf);
        //printf("sync_down = %d",sync_down);
        /*if(sync_down == 1)
        {
            //printf("sync_down = %d",sync_down);
            //printf("command is : %s\n",buf);
            //sleep(4);
            while (server_sync == 1)
            {
                sleep(2);
            }
            //do{
            local_sync = 1;
            ret = cmd_parser(buf);
            //}while(ret != 0 && exit_loop == 0 && ret != -1);
            printf("ret = %d\n",ret);

            if(ret != NE_OK && ret != -1)
            {
            	//save multiple buf
            	LocalActionTmp = (Local_Action *)malloc(sizeof(Local_Action));
            	memset(LocalActionTmp,0,sizeof(Local_Action));
            	sprintf(LocalActionTmp->buf,"%s",buf);
            	LocalActionTail->next = LocalActionTmp;
            	LocalActionTail = LocalActionTmp;
            	LocalActionTail->next = NULL;
            	printf("LocalActionTmp->buf = %s\n",LocalActionTmp->buf);
            }

            free_LocalFolder_item(LocalFolderList);
            free_LocalFile_item(LocalFileList);
            getLocalList();

        }*/
        /*
        memset(buf, 0, sizeof(buf));
        strcpy(buf, "rev cmd");
        if (send(new_fd, buf, strlen(buf), 0) == -1) {
            perror("send");
            exit(1);
        }

        local_sync = 0;*/
        //usleep(1000*10);
        //sleep(1);
        //while(waitpid(-1,NULL,WNOHANG) > 0);
    }
    close(sockfd);

    printf("stop WebDAV local sync\n");

    stop_down = 1;

    //pthread_cancel(newthid2);

    //pthread_exit(0);

    //return 0;
}
#endif

void *Socket_Parser(){
    printf("*******Socket_Parser start********\n");
    queue_entry_t socket_execute;
    int status = 0;
    //int mysync = 1;
    int has_socket = 0;
    int i;
    struct timeval now;
    struct timespec outtime;
    int fail_flag;

    while(!exit_loop)
    {
        //local_sync = 1;

        //printf("*******Socket_Parser start loop********\n");
        for(i=0;i<asus_cfg.dir_number;i++)
        {
            fail_flag = 0;
            //pthread_mutex_lock(&mutex_sync);
            while (server_sync == 1 && exit_loop ==0)
            {
                //printf("server sync = %d\n",server_sync);
                usleep(1000*10);
            }
            local_sync = 1;
            //pthread_mutex_unlock(&mutex_sync);

            if(exit_loop)
                break;

            if(asus_cfg.prule[i]->rule == 1)    //Dwonload Only
            {

                while(exit_loop ==0)
                {
                    while(g_pSyncList[i]->SocketActionList->head != NULL && exit_loop == 0 && server_sync == 0)
                    {
                        has_socket = 1;
                        socket_execute = g_pSyncList[i]->SocketActionList->head;
                        status = download_only_add_socket_item(socket_execute->cmd_name,i);
                        if(status == 0 || status == SERVER_SPACE_NOT_ENOUGH
                           || status == LOCAL_FILE_LOST || status == SERVER_FILE_DELETED
                           || status == SERVER_ROOT_DELETED)
                        {
                            //printf("########will del socket item##########\n");
                            pthread_mutex_lock(&mutex_socket);
                            socket_execute = queue_dequeue(g_pSyncList[i]->SocketActionList);
                            free(socket_execute);
                            //printf("del socket item ok\n");
                            pthread_mutex_unlock(&mutex_socket);
                        }
                        else
                        {
                            fail_flag = 1;
                            printf("######## socket item fail########\n");
                            break;
                        }
                        //sleep(2);
                        usleep(1000*20);
                    }

                    if(fail_flag)
                        break;

                    if(g_pSyncList[i]->copy_file_list->next == NULL)
                    {
                        break;
                    }
                    else
                    {
                        usleep(1000*100);
                        //sleep(1);
                    }
                }


                if(g_pSyncList[i]->dragfolder_action_list->next != NULL && g_pSyncList[i]->SocketActionList->head == NULL)
                {
                    free_action_item(g_pSyncList[i]->dragfolder_action_list);
                    g_pSyncList[i]->dragfolder_action_list = create_action_item_head();
                }
                if(g_pSyncList[i]->server_action_list->next != NULL && g_pSyncList[i]->SocketActionList->head == NULL)
                {
                    free_action_item(g_pSyncList[i]->server_action_list);
                    g_pSyncList[i]->server_action_list = create_action_item_head();
                }
                pthread_mutex_lock(&mutex_receve_socket);
                //receve_socket = 0;
                if(g_pSyncList[i]->SocketActionList->head == NULL)
                    g_pSyncList[i]->receve_socket = 0;
                pthread_mutex_unlock(&mutex_receve_socket);


            }
            else   //Upload Only and Sync
            {
                if(asus_cfg.prule[i]->rule == 2)     //upload only规则时，处理未完成的上传动作
                {
                    /*pthread_mutex_lock(&mutex_socket);
                    local_sync = 1;
                    pthread_mutex_unlock(&mutex_socket);*/
                    action_item *p,*p1;
                    p = g_pSyncList[i]->unfinished_list->next;
                    p1 = g_pSyncList[i]->up_space_not_enough_list->next;
                    if((p != NULL || p1 != NULL) && exit_loop == 0)
                    {
                        do_unfinished(i);
                    }
                }

                /*while ((server_sync == 1 || SocketActionList->head == NULL) && sync_down ==1)
                {
                    printf("server_sync = %d\n",server_sync);
                    sleep(2);
                }*/
                while(exit_loop ==0)
                {
                    while(g_pSyncList[i]->SocketActionList->head != NULL && exit_loop == 0 && server_sync == 0)
                    {
                        has_socket = 1;
                        socket_execute = g_pSyncList[i]->SocketActionList->head;
                        status = cmd_parser(socket_execute->cmd_name,i);
                        if(status == 0 || status == SERVER_SPACE_NOT_ENOUGH
                           || status == LOCAL_FILE_LOST || status == SERVER_FILE_DELETED
                           || status == SERVER_ROOT_DELETED)
                        {
                            //printf("########will del socket item##########\n");
                            pthread_mutex_lock(&mutex_socket);
                            socket_execute = queue_dequeue(g_pSyncList[i]->SocketActionList);
                            free(socket_execute);
                            //printf("del socket item ok\n");
                            pthread_mutex_unlock(&mutex_socket);
                        }
                        else
                        {
                            fail_flag = 1;
                            printf("######## socket item fail########\n");
                            break;
                        }
                        //sleep(2);
                        usleep(1000*20);
                    }

                    if(fail_flag)
                        break;

                    if(g_pSyncList[i]->copy_file_list->next == NULL)
                    {
                        break;
                    }
                    else
                    {
                        //sleep(1);
                        usleep(1000*100);
                    }
                }


                if(g_pSyncList[i]->dragfolder_action_list->next != NULL && g_pSyncList[i]->SocketActionList->head == NULL)
                {
                    //printf("#### clear dragfolder_action_list!\n");
                    free_action_item(g_pSyncList[i]->dragfolder_action_list);
                    g_pSyncList[i]->dragfolder_action_list = create_action_item_head();
                }
                //printf("#### clear dragfolder_action_list success!\n");
                if(g_pSyncList[i]->server_action_list->next != NULL && g_pSyncList[i]->SocketActionList->head == NULL)
                {
                    //printf("#### clear server_action_list!\n");
                    free_action_item(g_pSyncList[i]->server_action_list);
                    g_pSyncList[i]->server_action_list = create_action_item_head();
                }
                //printf("#### clear server_action_list success!\n");
                pthread_mutex_lock(&mutex_receve_socket);
                //receve_socket = 0;
                if(g_pSyncList[i]->SocketActionList->head == NULL)
                    g_pSyncList[i]->receve_socket = 0;
                pthread_mutex_unlock(&mutex_receve_socket);



            }
            printf("#### SocketActionList is NULL\n");
            if(has_socket == 1)
            {
                printf("write log!\n");
                has_socket = 0;
                //if(!fail_flag)
                //write_log(S_SYNC,"","",i);
                /*has_socket = 0;
                pthread_mutex_lock(&mutex_socket);
                gettimeofday(&now, NULL);
                outtime.tv_sec = now.tv_sec + 3;
                outtime.tv_nsec = now.tv_usec * 1000;
                pthread_cond_timedwait(&cond_socket, &mutex_socket, &outtime);
                pthread_mutex_unlock(&mutex_socket);*/
            }

        }

        /*if(has_socket == 1)
        {
            printf("write log!\n");
            write_log(S_SYNC,"","");
            has_socket = 0;
        }*/

        //printf("write log success!\n");

        pthread_mutex_lock(&mutex_socket);
        local_sync = 0;
        //server_sync = 1;
        //pthread_mutex_unlock(&mutex_socket);
        //printf("*******Socket_Parser start loop end********\n");
        //sleep(5);
        if(!exit_loop)
        {
            gettimeofday(&now, NULL);
            outtime.tv_sec = now.tv_sec + 2;
            outtime.tv_nsec = now.tv_usec * 1000;
            pthread_cond_timedwait(&cond_socket, &mutex_socket, &outtime);
        }
        pthread_mutex_unlock(&mutex_socket);
    }


    printf("stop WebDAV Socket_Parser\n");
    stop_down = 1;
}

#if 0
void *Download_Only_Socket_Parser(){

    printf("*******Download_Only_Socket_Parser start********\n");

    queue_entry_t socket_execute;
    int status = 0;
    //int mysync = 1;
    int has_socket = 0;

    while(sync_up == 1)
    {
        //local_sync = 1;
        //printf("*******Socket_Parser start loop********\n");

        while ((server_sync == 1 || SocketActionList->head == NULL) && sync_up ==1){
            printf("server_sync = %d\n",server_sync);
            sleep(2);
        }

        while(SocketActionList->head != NULL && sync_up == 1 && server_sync == 0)
        {
            has_socket = 1;
            socket_execute = SocketActionList->head;
            status = download_only_add_socket_item(socket_execute->cmd_name);
            if(status == 0 || status == SERVER_SPACE_NOT_ENOUGH
               || status == LOCAL_FILE_LOST || status == SERVER_FILE_DELETED)
            {
                //printf("########will del socket item##########\n");
                pthread_mutex_lock(&mutex_socket);
                socket_execute = queue_dequeue(SocketActionList);
                free(socket_execute);
                //printf("del socket item ok\n");
                pthread_mutex_unlock(&mutex_socket);
            }
            else
                printf("######## socket item fail########\n");
            sleep(2);
        }

        if(has_socket == 1)
        {
            write_log(S_SYNC,"","");
            has_socket = 0;
        }

        pthread_mutex_lock(&mutex_receve_socket);
        receve_socket = 0;
        pthread_mutex_unlock(&mutex_receve_socket);

        pthread_mutex_lock(&mutex_socket);
        local_sync = 0;
        pthread_mutex_unlock(&mutex_socket);
        if(dragfolder_action_list->next != NULL)
        {
            free_action_item(dragfolder_action_list);
            dragfolder_action_list = create_action_item_head();
        }
        if(server_action_list->next != NULL)
        {
            free_action_item(server_action_list);
            server_action_list = create_action_item_head();
        }
        //printf("*******Socket_Parser start loop end********\n");
        sleep(5);
    }


    printf("stop WebDAV Socket_Parser\n");
    stop_down = 1;

}
#endif

#if 0
int compareServerList(){
    printf("**************compareServerList**************\n");

    CloudFile *foldertmp,*oldfoldertmp,*filetmp,*oldfiletmp;
    int cmp;
    foldertmp = FolderList->next;
    oldfoldertmp = OldFolderList->next;
    filetmp = FileList->next;
    oldfiletmp = OldFileList->next;

    while (foldertmp->href != NULL || oldfoldertmp->href != NULL){
        //printf("foldertmp->href = %s\n",foldertmp->href);
        //printf("oldfoldertmp->href = %s\n",oldfoldertmp->href);
        if((foldertmp->href != NULL && oldfoldertmp->href == NULL) || (foldertmp->href == NULL && oldfoldertmp->href != NULL)){
            //printf("111111111111\n");
            return 0;
        }
        if ((cmp = strcmp(foldertmp->href,oldfoldertmp->href)) != 0){
            //printf("2222222222222\n");
            return 0;
        }
        foldertmp = foldertmp->next;
        oldfoldertmp = oldfoldertmp->next;
    }

    while (filetmp->href != NULL || oldfiletmp->href != NULL){
        if((filetmp->href != NULL && oldfiletmp->href == NULL) || (filetmp->href == NULL && oldfiletmp->href != NULL)){
            return 0;
        }
        if ((cmp = strcmp(filetmp->href,oldfiletmp->href)) != 0){
            return 0;
        }
        if ((cmp = strcmp(filetmp->getlastmodified,oldfiletmp->getlastmodified)) != 0){
            return 0;
        }
        filetmp = filetmp->next;
        oldfiletmp = oldfiletmp->next;
    }
    printf("ServerList is no changed!\n");
    return -1;
}
#endif

/*
 *judge is server changed
 *0:server changed
 *1:server is not changed
*/
int isServerChanged(Server_TreeNode *newNode,Server_TreeNode *oldNode)
{
    //printf("isServerChanged start!\n");
    int res = 1;
    int serverchanged = 0;
    if(newNode->browse == NULL && oldNode->browse == NULL)
    {
        printf("########Server is not change\n");
        return 1;
    }
    else if(newNode->browse == NULL && oldNode->browse != NULL)
    {
        printf("########Server changed1\n");
        return 0;
    }
    else if(newNode->browse != NULL && oldNode->browse == NULL)
    {
        printf("########Server changed2\n");
        return 0;
    }
    else
    {
        if(newNode->browse->filenumber != oldNode->browse->filenumber || newNode->browse->foldernumber != oldNode->browse->foldernumber)
        {
            printf("########Server changed3\n");
            return 0;
        }
        else
        {
            int cmp;
            CloudFile *newfoldertmp = NULL;
            CloudFile *oldfoldertmp = NULL;
            CloudFile *newfiletmp = NULL;
            CloudFile *oldfiletmp = NULL;
            if(newNode->browse != NULL)
            {
                if(newNode->browse->foldernumber > 0)
                    newfoldertmp = newNode->browse->folderlist->next;
                if(newNode->browse->filenumber > 0)
                    newfiletmp = newNode->browse->filelist->next;
            }
            if(oldNode->browse != NULL)
            {
                if(oldNode->browse->foldernumber > 0)
                    oldfoldertmp = oldNode->browse->folderlist->next;
                if(oldNode->browse->filenumber > 0)
                    oldfiletmp = oldNode->browse->filelist->next;
            }

            while (newfoldertmp != NULL || oldfoldertmp != NULL)
            {
                if ((cmp = strcmp(newfoldertmp->href,oldfoldertmp->href)) != 0){
                    printf("########Server changed4\n");
                    return 0;
                }
                newfoldertmp = newfoldertmp->next;
                oldfoldertmp = oldfoldertmp->next;
            }
            while (newfiletmp != NULL || oldfiletmp != NULL)
            {
                if ((cmp = strcmp(newfiletmp->href,oldfiletmp->href)) != 0)
                {
                    printf("########Server changed5\n");
                    return 0;
                }
                if (newfiletmp->modtime != oldfiletmp->modtime)
                {
                    //printf("newpath=%s,newtime=%lu\n",newfiletmp->href,newfiletmp->modtime);
                    //printf("oldpath=%s,oldtime=%lu\n",oldfiletmp->href,oldfiletmp->modtime);
                    printf("########Server changed6\n");
                    return 0;
                }
                newfiletmp = newfiletmp->next;
                oldfiletmp = oldfiletmp->next;
            }
        }

        if((newNode->Child == NULL && oldNode->Child != NULL) || (newNode->Child != NULL && oldNode->Child == NULL))
        {
            printf("########Server changed7\n");
            return 0;
        }
        if((newNode->NextBrother == NULL && oldNode->NextBrother != NULL) || (newNode->NextBrother!= NULL && oldNode->NextBrother == NULL))
        {
            printf("########Server changed8\n");
            return 0;
        }

        if(newNode->Child != NULL && oldNode->Child != NULL)
        {
            res = isServerChanged(newNode->Child,oldNode->Child);
            if(res == 0)
            {
                serverchanged = 1;
            }
        }
        if(newNode->NextBrother != NULL && oldNode->NextBrother != NULL)
        {
            res = isServerChanged(newNode->NextBrother,oldNode->NextBrother);
            if(res == 0)
            {
                serverchanged = 1;
            }
        }
    }
    if(serverchanged == 1)
    {
        printf("########Server changed9\n");
        return 0;
    }
    else
    {
        //printf("########Server is not change\n");
        return 1;
    }
}
/*ret = 0,server changed
 *ret = 1,server is no changed
*/
int compareServerList(int index)
{
    int ret;
    printf("#########compareServerList\n");

    if(g_pSyncList[index]->ServerRootNode->Child != NULL && g_pSyncList[index]->OldServerRootNode->Child != NULL)
    {
        ret = isServerChanged(g_pSyncList[index]->ServerRootNode->Child,g_pSyncList[index]->OldServerRootNode->Child);
        return ret;
    }
    else if(g_pSyncList[index]->ServerRootNode->Child == NULL && g_pSyncList[index]->OldServerRootNode->Child == NULL)
    {
        ret = 1;
        return ret;
    }
    else
    {
        ret = 0;
        return ret;
    }
}

#if 0
int compareLocalList(){
    printf("**************compareLocalList**************\n");

    CloudFile *foldertmp,*filetmp;
    LocalFolder *localfoldertmp;
    LocalFile *localfiletmp;
    int cmp;
    int ret;
    /******time = 0 ,download; time = 1 ,upload; time = 2 ,no action*********/
    int time;
    int changelist = 0;
    //char *hreftmp;
    //char *pathtmp;
    char createpath[512];
    char href_unescape[512];
    //char createhref[256];
    foldertmp = OldFolderList->next;
    localfoldertmp = LocalFolderList->next;
    filetmp = OldFileList->next;
    localfiletmp = LocalFileList->next;

    memset(createpath,0,sizeof(createpath));
    memset(href_unescape,0,sizeof(href_unescape));

    /*ServerList's folders compared with LocalList's folders*/
    while (foldertmp->href != NULL){
        cmp = 1;
        char *hreftmp;
        //memset(href_unescape,0,sizeof(href_unescape));
        hreftmp = strstr(foldertmp->href,ROOTFOLDER)+rootfolder_len;
        hreftmp = oauth_url_unescape(hreftmp,NULL);
        //printf("hreftmp = %s\n",hreftmp);
        //printf("foldertmp->href = %s\n",foldertmp->href);
        while (localfoldertmp->path != NULL){
            char *pathtmp;
            pathtmp = strstr(localfoldertmp->path,base_path) + base_path_len;
            if ((cmp = strcmp(hreftmp,pathtmp)) == 0){
                //localfoldertmp = LocalFolderList->next;
                break;
            }
            else {
                localfoldertmp = localfoldertmp->next;
            }
        }
        if (cmp != 0){
            //hreftmp = oauth_url_unescape(hreftmp,NULL);
            sprintf(createpath,"%s%s",base_path,hreftmp);
            if(NULL == opendir(createpath))
                mkdir(createpath,0777);
            memset(createpath,0,sizeof(createpath));
            changelist = 1;
        }
        foldertmp = foldertmp->next;
        localfoldertmp = LocalFolderList->next;
    }

    foldertmp = OldFolderList->next;
    localfoldertmp = LocalFolderList->next;

    /*LocalList's folders compared with ServerList's folders*/
    while (localfoldertmp->path != NULL){
        cmp = 1;
        char *pathtmp;
        pathtmp = strstr(localfoldertmp->path,base_path) + base_path_len;
        while (foldertmp->href != NULL){
            char *hreftmp;
            hreftmp = strstr(foldertmp->href,ROOTFOLDER) + rootfolder_len;
            hreftmp = oauth_url_unescape(hreftmp,NULL);
            if ((cmp = strcmp(hreftmp,pathtmp)) == 0){
                //foldertmp = OldFolderList->next;
                break;
            }
            else {
                foldertmp = foldertmp->next;
            }
        }
        if (cmp != 0){
            printf("localfoldertmp->path = %s\n",localfoldertmp->path);
            del_all_items(localfoldertmp->path);
            changelist = 1;
        }
        localfoldertmp = localfoldertmp->next;
        foldertmp = OldFolderList->next;
    }
    if(changelist){
        free_LocalFolder_item(LocalFolderList);
        free_LocalFile_item(LocalFileList);
        getLocalList();
        localfoldertmp = LocalFolderList->next;
        localfiletmp = LocalFileList->next;
    }


    /*ServerList's files compared with LocalList's files*/
    while (filetmp->href != NULL){
        cmp = 1;
        char *hreftmp;
        hreftmp = strstr(filetmp->href,ROOTFOLDER) + rootfolder_len;
        hreftmp = oauth_url_unescape(hreftmp,NULL);
        //printf("hreftmp = %s\n",hreftmp);
        while (localfiletmp->path != NULL){
            char *pathtmp;
            pathtmp = strstr(localfiletmp->path,base_path) + base_path_len;
            //printf("pathtmp = %s\n",pathtmp);
            if ((cmp = strcmp(hreftmp,pathtmp)) == 0){     //just compared file's name
                time = compareTime(filetmp->modtime,localfiletmp->path);
                //printf("time = %d\n",time);
                if(time == 0){
                    //down_head = add_sync_item(DOWNLOAD,localfiletmp->path,down_head);
                    ret = Download(filetmp);
                    if (ret != NE_OK){
                        //remove_tmp(filetmp);
                        //free_LocalFolder_item(LocalFolderList);
                        //free_LocalFile_item(LocalFileList);
                    }
                    else{
                        //down_head = del_sync_item(DOWNLOAD,localfiletmp->path,down_head);
                        ChangeFile_modtime(localfiletmp->path,filetmp->modtime);
                    }
                    if(ret == NE_WEBDAV_QUIT){
                        return NE_WEBDAV_QUIT;
                    }

                }
                /*else if(time == 1){
					//char fullname[256];
					//memset(fullname,0,sizeof(fullname));
					//sprintf(fullname,"%s%s",base_path,pathtmp);
					up_head = add_sync_item(UPLOAD,localfiletmp->path,up_head);
					ret = Upload(localfiletmp->path);
					if(ret != NE_WEBDAV_QUIT)
					{
						up_head = del_sync_item(UPLOAD,localfiletmp->path,up_head);
					}
					char URL[512];
					int status;
					memset(URL,0,sizeof(URL));
					get_file_parent(filetmp->href,URL);
					Upload_ChangeFile_modtime(URL,localfiletmp->path);
				}*/
                else{
                    //no action
                }
                //localfiletmp = LocalFileList->next;
                break;
            }
            else {
                localfiletmp = localfiletmp->next;
            }
        }
        if (cmp != 0){
            //down_head = add_sync_item(DOWNLOAD,localfiletmp->path,down_head);
            printf("cmp = %d\n",cmp);
            ret = Download(filetmp);
            if (ret != NE_OK){
                //remove_tmp(filetmp);
                //free_LocalFolder_item(LocalFolderList);
                //free_LocalFile_item(LocalFileList);;
            }
            else{
                //down_head = del_sync_item(DOWNLOAD,localfiletmp->path,down_head);
                char localfilepath[512];
                memset(localfilepath,0,sizeof(localfilepath));
                sprintf(localfilepath,"%s%s",base_path,hreftmp);
                ChangeFile_modtime(localfilepath,filetmp->modtime);
            }
            if(ret == NE_WEBDAV_QUIT){
                return NE_WEBDAV_QUIT;
            }

        }
        filetmp = filetmp->next;
        localfiletmp = LocalFileList->next;
    }
    filetmp = OldFileList->next;
    localfiletmp = LocalFileList->next;

    /*LocalList's files compared with ServerList's files*/
    while (localfiletmp->path != NULL){
        //printf("localfiletmp->path = %s \n",localfiletmp->path);
        cmp = 1;
        char *pathtmp;
        pathtmp = strstr(localfiletmp->path,base_path) + base_path_len;
        while (filetmp->href != NULL){
            char *hreftmp;
            hreftmp = strstr(filetmp->href,ROOTFOLDER) + rootfolder_len;
            hreftmp = oauth_url_unescape(hreftmp,NULL);
            //printf("pathtmp = %s \n",pathtmp);
            //printf("hreftmp = %s \n",hreftmp);
            if ((cmp = strcmp(hreftmp,pathtmp)) == 0){
                //filetmp = OldFileList->next;
                break;
            }
            else {
                filetmp = filetmp->next;
            }
        }
        if (cmp != 0){
            //printf("unlink %s \n",pathtmp);
            unlink(localfiletmp->path);
        }
        localfiletmp = localfiletmp->next;
        filetmp = OldFileList->next;
    }


    return 0;
}
#endif

int the_same_name_compare(LocalFile *localfiletmp,CloudFile *filetmp,int index){

    int ret = 0;
    int newer_file_ret = 0;

    if(asus_cfg.prule[index]->rule == 1)
    {
        newer_file_ret = newer_file(localfiletmp->path,index);
        if(newer_file_ret != 2 && newer_file_ret != -1)
        {
            char *newname;
            newname = change_local_same_name(localfiletmp->path);
            rename(localfiletmp->path,newname);
            //add_action_item("rename",newname,server_action_list);
            printf("newname = %s\n",newname);
            free(newname);

            char *localpath;
            localpath = serverpath_to_localpath(filetmp->href,index);

            action_item *item;
            item = get_action_item("download",filetmp->href,g_pSyncList[index]->unfinished_list,index);

            if(is_local_space_enough(filetmp,index))
            {
                if(wait_handle_socket(index))
                {
                    return HAVE_LOCAL_SOCKET;
                }
                add_action_item("createfile",localpath,g_pSyncList[index]->server_action_list);
                ret = Download(filetmp->href,index);
                if (ret == NE_OK)
                {
                    //char *localpath;
                    //localpath = serverpath_to_localpath(filetmp->href);
                    //ChangeFile_modtime(localpath,filetmp->modtime);
                    time_t modtime;
                    modtime = Getmodtime(filetmp->href,index);
                    if(modtime != -1)
                        ChangeFile_modtime(localpath,modtime);
                    else
                    {
                        printf("ChangeFile_modtime failed!\n");
                    }
                    if(item != NULL)
                    {
                        del_action_item("download",filetmp->href,g_pSyncList[index]->unfinished_list);
                    }
                }
                else
                {
                    free(localpath);
                    //free(serverpathtmp);
                    return ret;
                }
            }
            else
            {
                write_log(S_ERROR,"local space is not enough!","",index);
                if(item == NULL)
                {
                    add_action_item("download",filetmp->href,g_pSyncList[index]->unfinished_list);
                }

            }
            free(localpath);
        }
#ifdef COMPARE_FILE_SIZE
        else
        {
            if((compare_file_size(filetmp)) != 2)
            {
                char *localpath;
                localpath = serverpath_to_localpath(filetmp->href);

                action_item *item;
                item = get_action_item("download",filetmp->href,unfinished_list);

                if(is_local_space_enough(filetmp))
                {
                    wait_handle_socket();
                    add_action_item("createfile",localpath,server_action_list);
                    ret = Download(filetmp->href);
                    if (ret == NE_OK)
                    {
                        //char *localpath;
                        //localpath = serverpath_to_localpath(filetmp->href);
                        //ChangeFile_modtime(localpath,filetmp->modtime);
                        time_t modtime;
                        modtime = Getmodtime(filetmp->href);
                        if(modtime != -1)
                            ChangeFile_modtime(localpath,modtime);
                        else
                        {
                            printf("ChangeFile_modtime failed!\n");
                        }
                        if(item != NULL)
                        {
                            del_action_item("download",filetmp->href,unfinished_list);
                        }
                    }
                    else
                    {
                        free(localpath);
                        //free(serverpathtmp);
                        return ret;
                    }
                }
                else
                {
                    write_log(S_ERROR,"local space is not enough!","");
                    if(item == NULL)
                    {
                        add_action_item("download",filetmp->href,unfinished_list);
                    }

                }
                free(localpath);
            }
        }
#endif
    }
    else if(asus_cfg.prule[index]->rule == 2)     //初始化的时候会用到
    {
#if 0
        if((newer_file(localfiletmp->path,index)) != 2)
        {
            if(wait_handle_socket(index))
            {
                return HAVE_LOCAL_SOCKET;
            }
            char *newname;
            newname = change_server_same_name(localfiletmp->path,index);
            Move(localfiletmp->path,newname,index);
            printf("newname = %s\n",newname);
            free(newname);

            ret = Upload(localfiletmp->path,index);
            if(ret == NE_OK || ret == SERVER_SPACE_NOT_ENOUGH || ret == LOCAL_FILE_LOST)
            {
                if(ret == NE_OK)
                {
                    char *serverpath;
                    serverpath = localpath_to_serverpath(localfiletmp->path,index);
                    //printf("serverpath = %s\n",serverpath);
                    time_t modtime;
                    modtime = Getmodtime(serverpath,index);
                    if(modtime != -1)
                        ChangeFile_modtime(localfiletmp->path,modtime);
                    else
                    {
                        printf("ChangeFile_modtime failed!\n");
                    }
                    free(serverpath);
                }
            }
            else
            {
                return ret;
            }
        }
#endif
        newer_file_ret = newer_file(localfiletmp->path,index);
        if(newer_file_ret != 2 && newer_file_ret != -1)
        {
            if(wait_handle_socket(index))
            {
                return HAVE_LOCAL_SOCKET;
            }
            /*char *newname;
            newname = change_server_same_name(localfiletmp->path,index);
            Move(localfiletmp->path,newname,index);
            printf("newname = %s\n",newname);
            free(newname);*/

            ret = Upload(localfiletmp->path,index);
            if(ret == NE_OK || ret == SERVER_SPACE_NOT_ENOUGH || ret == LOCAL_FILE_LOST)
            {
                if(ret == NE_OK)
                {
                    char *serverpath;
                    serverpath = localpath_to_serverpath(localfiletmp->path,index);
                    //printf("serverpath = %s\n",serverpath);
                    time_t modtime;
                    modtime = Getmodtime(serverpath,index);
                    if(modtime != -1)
                        ChangeFile_modtime(localfiletmp->path,modtime);
                    else
                    {
                        printf("ChangeFile_modtime failed!\n");
                    }
                    free(serverpath);
                }
            }
            else
            {
                return ret;
            }

        }
    }
    else
    {
        newer_file_ret = newer_file(localfiletmp->path,index);
        if(newer_file_ret == 1)
        {
            char *localpath;
            localpath = serverpath_to_localpath(filetmp->href,index);

            action_item *item;
            item = get_action_item("download",filetmp->href,g_pSyncList[index]->unfinished_list,index);

            if(is_local_space_enough(filetmp,index))
            {
                if(wait_handle_socket(index))
                {
                    return HAVE_LOCAL_SOCKET;
                }
                add_action_item("createfile",localpath,g_pSyncList[index]->server_action_list);
                ret = Download(filetmp->href,index);
                if (ret == NE_OK)
                {
                    //char *localpath;
                    //localpath = serverpath_to_localpath(filetmp->href);
                    //ChangeFile_modtime(localpath,filetmp->modtime);
                    time_t modtime;
                    modtime = Getmodtime(filetmp->href,index);
                    if(modtime != -1)
                        ChangeFile_modtime(localpath,modtime);
                    else
                    {
                        printf("ChangeFile_modtime failed!\n");
                    }
                    if(item != NULL)
                    {
                        del_action_item("download",filetmp->href,g_pSyncList[index]->unfinished_list);
                    }
                }
                else
                {
                    free(localpath);
                    //free(serverpathtmp);
                    return ret;
                }
            }
            else
            {
                write_log(S_ERROR,"local space is not enough!","",index);
                if(item == NULL)
                {
                    add_action_item("download",filetmp->href,g_pSyncList[index]->unfinished_list);
                }

            }
            free(localpath);
        }

        else if(newer_file_ret == 0 && asus_cfg.prule[index]->rule != 1)
        {
            if(wait_handle_socket(index))
            {
                return HAVE_LOCAL_SOCKET;
            }
            add_action_item("createfile",localfiletmp->path,g_pSyncList[index]->server_action_list);  //??
            ret = Upload(localfiletmp->path,index);
            if(ret == NE_OK || ret == SERVER_SPACE_NOT_ENOUGH || ret == LOCAL_FILE_LOST)
            {
                if(ret == NE_OK)
                {
                    char *serverpath;
                    serverpath = localpath_to_serverpath(localfiletmp->path,index);
                    //printf("serverpath = %s\n",serverpath);
                    time_t modtime;
                    modtime = Getmodtime(serverpath,index);
                    if(modtime != -1)
                        ChangeFile_modtime(localfiletmp->path,modtime);
                    else
                    {
                        printf("ChangeFile_modtime failed!\n");
                    }
                    free(serverpath);
                }
            }
            else
            {
                return ret;
            }

        }
#ifdef COMPARE_FILE_SIZE
        else if((newer_file(localfiletmp->path)) == 2)
        {
            if((compare_file_size(filetmp)) != 2)
            {
                char *localpath;
                localpath = serverpath_to_localpath(filetmp->href);

                action_item *item;
                item = get_action_item("download",filetmp->href,unfinished_list);

                if(is_local_space_enough(filetmp))
                {
                    wait_handle_socket();
                    add_action_item("createfile",localpath,server_action_list);
                    ret = Download(filetmp->href);
                    if (ret == NE_OK)
                    {
                        //char *localpath;
                        //localpath = serverpath_to_localpath(filetmp->href);
                        //ChangeFile_modtime(localpath,filetmp->modtime);
                        time_t modtime;
                        modtime = Getmodtime(filetmp->href);
                        if(modtime != -1)
                            ChangeFile_modtime(localpath,modtime);
                        else
                        {
                            printf("ChangeFile_modtime failed!\n");
                        }
                        if(item != NULL)
                        {
                            del_action_item("download",filetmp->href,unfinished_list);
                        }
                    }
                    else
                    {
                        free(localpath);
                        //free(serverpathtmp);
                        return ret;
                    }
                }
                else
                {
                    write_log(S_ERROR,"local space is not enough!","");
                    if(item == NULL)
                    {
                        add_action_item("download",filetmp->href,unfinished_list);
                    }

                }
                free(localpath);
            }
        }
#endif
    }

    return ret;
}

int sync_server_to_local_init(Browse *perform_br,Local *perform_lo,int index){

    if(perform_br == NULL || perform_lo == NULL)
    {
        return 0;
    }

    CloudFile *foldertmp = NULL;
    CloudFile *filetmp = NULL;
    LocalFolder *localfoldertmp;
    LocalFile *localfiletmp;
    int ret = 0;

    if(perform_br->foldernumber > 0)
        foldertmp = perform_br->folderlist->next;
    if(perform_br->filenumber > 0)
        filetmp = perform_br->filelist->next;

    localfoldertmp = perform_lo->folderlist->next;
    localfiletmp = perform_lo->filelist->next;

    /****************handle files****************/
    if(perform_br->filenumber == 0 && perform_lo->filenumber != 0)
    {
        while(localfiletmp != NULL)
        {
            if(asus_cfg.prule[index]->rule != 1)
            {
                /*while(is_file_copying(localfiletmp->path))
                {
                    usleep(1000*100);
                }*/

                if(wait_handle_socket(index))
                {
                    return HAVE_LOCAL_SOCKET;
                }

                add_action_item("createfile",localfiletmp->path,g_pSyncList[index]->server_action_list);
                ret = Upload(localfiletmp->path,index);
                if(ret == NE_OK || ret == SERVER_SPACE_NOT_ENOUGH || ret == LOCAL_FILE_LOST)
                {
                    if(ret == NE_OK)
                    {
                        char *serverpath;
                        serverpath = localpath_to_serverpath(localfiletmp->path,index);
                        //printf("serverpath = %s\n",serverpath);
                        time_t modtime;
                        modtime = Getmodtime(serverpath,index);
                        if(modtime != -1)
                            ChangeFile_modtime(localfiletmp->path,modtime);
                        else
                        {
                            printf("ChangeFile_modtime failed!\n");
                        }
                        free(serverpath);
                    }
                }
                else
                {
                    return ret;
                }
            }
            else
            {
                if(wait_handle_socket(index))
                {
                    return HAVE_LOCAL_SOCKET;
                }
                add_action_item("createfile",localfiletmp->path,g_pSyncList[index]->download_only_socket_head);
            }

            localfiletmp = localfiletmp->next;
        }

    }
    else if(perform_br->filenumber != 0 && perform_lo->filenumber == 0)
    {
        if(asus_cfg.prule[index]->rule != 2)
        {
            while(filetmp != NULL)
            {
                if(is_local_space_enough(filetmp,index))
                {
                    char *localpath;
                    if(wait_handle_socket(index))
                    {
                        return HAVE_LOCAL_SOCKET;
                    }
                    localpath = serverpath_to_localpath(filetmp->href,index);
                    add_action_item("createfile",localpath,g_pSyncList[index]->server_action_list);

                    //wait_handle_socket(index);

                    ret = Download(filetmp->href,index);
                    if (ret == NE_OK)
                    {
                        //ChangeFile_modtime(localpath,filetmp->modtime);
                        time_t modtime;
                        modtime = Getmodtime(filetmp->href,index);
                        if(modtime != -1)
                            ChangeFile_modtime(localpath,modtime);
                        else
                        {
                            printf("ChangeFile_modtime failed!\n");
                        }
                        free(localpath);
                    }
                    else
                    {
                        free(localpath);
                        return ret;
                    }
                }
                else
                {
                    write_log(S_ERROR,"local space is not enough!","",index);
                    add_action_item("download",filetmp->href,g_pSyncList[index]->unfinished_list);
                }

                filetmp = filetmp->next;
            }
        }
    }
    else if(perform_br->filenumber != 0 && perform_lo->filenumber != 0)
    {
        while(localfiletmp != NULL)
        {
            int cmp = 1;
            char *localpathtmp;
            localpathtmp = strstr(localfiletmp->path,asus_cfg.prule[index]->base_path) + asus_cfg.prule[index]->base_path_len;
            while(filetmp != NULL)
            {
                char *serverpathtmp;
                serverpathtmp = strstr(filetmp->href,asus_cfg.prule[index]->rootfolder) + asus_cfg.prule[index]->rootfolder_length;
                serverpathtmp = oauth_url_unescape(serverpathtmp,NULL);
                if ((cmp = strcmp(localpathtmp,serverpathtmp)) == 0)
                {
                    //filetmp = OldFileList->next;
                    free(serverpathtmp);
                    break;
                }
                else
                {
                    filetmp = filetmp->next;
                }
                free(serverpathtmp);
            }
            if (cmp != 0)
            {
                if(asus_cfg.prule[index]->rule != 1)
                {
                    /*while(is_file_copying(localfiletmp->path))
                    {
                        usleep(1000*100);
                    }*/

                    if(wait_handle_socket(index))
                    {
                        return HAVE_LOCAL_SOCKET;
                    }

                    add_action_item("createfile",localfiletmp->path,g_pSyncList[index]->server_action_list);
                    ret = Upload(localfiletmp->path,index);
                    if(ret == NE_OK || ret == SERVER_SPACE_NOT_ENOUGH || ret == LOCAL_FILE_LOST)
                    {
                        if(ret == NE_OK)
                        {
                            char *serverpath;
                            serverpath = localpath_to_serverpath(localfiletmp->path,index);
                            //printf("serverpath = %s\n",serverpath);
                            time_t modtime;
                            modtime = Getmodtime(serverpath,index);
                            if(modtime != -1)
                                ChangeFile_modtime(localfiletmp->path,modtime);
                            else
                            {
                                printf("ChangeFile_modtime failed!\n");
                            }
                            free(serverpath);
                        }
                    }
                    else
                    {
                        return ret;
                    }
                }
                else
                {
                    if(wait_handle_socket(index))
                    {
                        return HAVE_LOCAL_SOCKET;
                    }
                    add_action_item("createfile",localfiletmp->path,g_pSyncList[index]->download_only_socket_head);
                }
            }
            else
            {
                if((ret = the_same_name_compare(localfiletmp,filetmp,index)) != 0)
                {
                    return ret;
                }
            }
            filetmp = perform_br->filelist->next;
            localfiletmp = localfiletmp->next;
        }


        filetmp = perform_br->filelist->next;
        localfiletmp = perform_lo->filelist->next;
        while(filetmp != NULL)
        {
            int cmp = 1;
            char *serverpathtmp;
            serverpathtmp = strstr(filetmp->href,asus_cfg.prule[index]->rootfolder) + asus_cfg.prule[index]->rootfolder_length;
            serverpathtmp = oauth_url_unescape(serverpathtmp,NULL);
            while(localfiletmp != NULL)
            {
                char *localpathtmp;
                localpathtmp = strstr(localfiletmp->path,asus_cfg.prule[index]->base_path) + asus_cfg.prule[index]->base_path_len;
                if ((cmp = strcmp(localpathtmp,serverpathtmp)) == 0)
                {
                    //filetmp = OldFileList->next;
                    break;
                }
                else
                {
                    localfiletmp = localfiletmp->next;
                }
            }
            if (cmp != 0)
            {
                if(asus_cfg.prule[index]->rule != 2)
                {
                    if(is_local_space_enough(filetmp,index))
                    {
                        char *localpath;
                        localpath = serverpath_to_localpath(filetmp->href,index);

                        if(wait_handle_socket(index))
                        {
                            return HAVE_LOCAL_SOCKET;
                        }

                        add_action_item("createfile",localpath,g_pSyncList[index]->server_action_list);
                        ret = Download(filetmp->href,index);
                        //printf("XXXXXXXXXXXXXret3 = %d\n",ret);
                        if (ret == NE_OK)
                        {
                            //ChangeFile_modtime(localpath,filetmp->modtime);
                            time_t modtime;
                            modtime = Getmodtime(filetmp->href,index);
                            if(modtime != -1)
                                ChangeFile_modtime(localpath,modtime);
                            else
                            {
                                printf("ChangeFile_modtime failed!\n");
                            }
                            free(localpath);
                        }
                        else
                        {
                            free(localpath);
                            free(serverpathtmp);
                            return ret;
                        }
                    }
                    else
                    {
                        write_log(S_ERROR,"local space is not enough!","",index);
                        add_action_item("download",filetmp->href,g_pSyncList[index]->unfinished_list);
                    }
                }
            }
            free(serverpathtmp);
            filetmp = filetmp->next;
            localfiletmp = perform_lo->filelist->next;
        }
    }



    /*************handle folders**************/
    if(perform_br->foldernumber == 0 && perform_lo->foldernumber != 0)
    {
        while(localfoldertmp != NULL)
        {
            if(wait_handle_socket(index))
            {
                return HAVE_LOCAL_SOCKET;
            }
            if(asus_cfg.prule[index]->rule != 1)
            {
                add_action_item("createfolder",localfoldertmp->path,g_pSyncList[index]->server_action_list);
                ret = Mkcol(localfoldertmp->path,index);
                if(ret != NE_OK && ret != -1)
                {
                    return ret;
                }
            }
            else
            {
                if(wait_handle_socket(index))
                {
                    return HAVE_LOCAL_SOCKET;
                }
                add_action_item("createfolder",localfoldertmp->path,g_pSyncList[index]->download_only_socket_head);
            }
            localfoldertmp = localfoldertmp->next;
        }
    }
    else if(perform_br->foldernumber != 0 && perform_lo->foldernumber == 0)
    {
        if(asus_cfg.prule[index]->rule != 2)
        {
            while(foldertmp != NULL)
            {
                if(wait_handle_socket(index))
                {
                    return HAVE_LOCAL_SOCKET;
                }
                char *localpath;
                localpath = serverpath_to_localpath(foldertmp->href,index);
                if(NULL == opendir(localpath))
                {
                    add_action_item("createfolder",localpath,g_pSyncList[index]->server_action_list);
                    mkdir(localpath,0777);
                }
                free(localpath);
                foldertmp = foldertmp->next;
            }
        }
    }
    else if(perform_br->foldernumber != 0 && perform_lo->foldernumber != 0)
    {
        while(localfoldertmp != NULL)
        {
            int cmp = 1;
            char *localpathtmp;
            localpathtmp = strstr(localfoldertmp->path,asus_cfg.prule[index]->base_path) + asus_cfg.prule[index]->base_path_len;
            while(foldertmp != NULL)
            {
                char *serverpathtmp;
                serverpathtmp = strstr(foldertmp->href,asus_cfg.prule[index]->rootfolder) + asus_cfg.prule[index]->rootfolder_length;
                serverpathtmp = oauth_url_unescape(serverpathtmp,NULL);
                if ((cmp = strcmp(localpathtmp,serverpathtmp)) == 0)
                {
                    //filetmp = OldFileList->next;
                    free(serverpathtmp);
                    break;
                }
                else
                {
                    foldertmp = foldertmp->next;
                }
                free(serverpathtmp);
            }
            if (cmp != 0)
            {
                if(asus_cfg.prule[index]->rule != 1)
                {
                    if(wait_handle_socket(index))
                    {
                        return HAVE_LOCAL_SOCKET;
                    }
                    add_action_item("createfolder",localfoldertmp->path,g_pSyncList[index]->server_action_list);
                    ret = Mkcol(localfoldertmp->path,index);
                    if(ret != NE_OK && ret != -1)
                    {
                        return ret;
                    }
                }
                else
                {
                    if(wait_handle_socket(index))
                    {
                        return HAVE_LOCAL_SOCKET;
                    }
                    add_action_item("createfolder",localfoldertmp->path,g_pSyncList[index]->download_only_socket_head);
                }
            }
            foldertmp = perform_br->folderlist->next;
            localfoldertmp = localfoldertmp->next;
        }

        foldertmp = perform_br->folderlist->next;
        localfoldertmp = perform_lo->folderlist->next;
        while(foldertmp != NULL)
        {
            int cmp = 1;
            char *serverpathtmp;
            serverpathtmp = strstr(foldertmp->href,asus_cfg.prule[index]->rootfolder) + asus_cfg.prule[index]->rootfolder_length;
            serverpathtmp = oauth_url_unescape(serverpathtmp,NULL);
            while(localfoldertmp != NULL)
            {
                char *localpathtmp;
                localpathtmp = strstr(localfoldertmp->path,asus_cfg.prule[index]->base_path) + asus_cfg.prule[index]->base_path_len;
                if ((cmp = strcmp(localpathtmp,serverpathtmp)) == 0)
                {
                    //filetmp = OldFileList->next;
                    break;
                }
                else
                {
                    localfoldertmp = localfoldertmp->next;
                }
            }
            if (cmp != 0)
            {
                if(asus_cfg.prule[index]->rule != 2)
                {
                    if(wait_handle_socket(index))
                    {
                        return HAVE_LOCAL_SOCKET;
                    }
                    char *localpath;
                    localpath = serverpath_to_localpath(foldertmp->href,index);
                    if(NULL == opendir(localpath))
                    {
                        add_action_item("createfolder",localpath,g_pSyncList[index]->server_action_list);
                        mkdir(localpath,0777);
                    }
                    free(localpath);
                }
            }
            free(serverpathtmp);
            foldertmp = foldertmp->next;
            localfoldertmp = perform_lo->folderlist->next;
        }
    }

    return ret;
}

int sync_server_to_local_perform(Browse *perform_br,Local *perform_lo,int index){

    if(perform_br == NULL || perform_lo == NULL)
    {
        return 0;
    }

    CloudFile *foldertmp = NULL;
    CloudFile *filetmp = NULL;
    LocalFolder *localfoldertmp;
    LocalFile *localfiletmp;
    int ret = 0;
    int wait_ret = 0;

    if(perform_br->foldernumber > 0)
        foldertmp = perform_br->folderlist->next;
    if(perform_br->filenumber > 0)
        filetmp = perform_br->filelist->next;

    localfoldertmp = perform_lo->folderlist->next; 
    localfiletmp = perform_lo->filelist->next;

    /****************handle files****************/
    //printf("##########handle files\n");
    if(perform_br->filenumber == 0 && perform_lo->filenumber != 0)
    {
        while(localfiletmp != NULL)
        {
            if(asus_cfg.prule[index]->rule == 1)
            {
                action_item *item;
                item = get_action_item("download_only",localfiletmp->path,
                                       g_pSyncList[index]->download_only_socket_head,index);
                if(item != NULL)
                {
                    localfiletmp = localfiletmp->next;
                    continue;
                }
            }
            if(wait_handle_socket(index))
            {
                return HAVE_LOCAL_SOCKET;
            }
            add_action_item("remove",localfiletmp->path,g_pSyncList[index]->server_action_list);

            action_item *pp;
            pp = get_action_item("upload",localfiletmp->path,
                                 g_pSyncList[index]->up_space_not_enough_list,index);
            if(pp == NULL)
            {
                unlink(localfiletmp->path);
            }
            localfiletmp = localfiletmp->next;
        }
    }
    else if(perform_br->filenumber != 0 && perform_lo->filenumber == 0)
    {
        while(filetmp != NULL)
        {
            if(wait_handle_socket(index))
            {
                return HAVE_LOCAL_SOCKET;
            }
            char *localpath;
            localpath = serverpath_to_localpath(filetmp->href,index);

            action_item *item;
            item = get_action_item("download",filetmp->href,g_pSyncList[index]->unfinished_list,index);

            if(is_local_space_enough(filetmp,index))
            {
                add_action_item("createfile",localpath,g_pSyncList[index]->server_action_list);
                ret = Download(filetmp->href,index);
                if (ret == NE_OK)
                {
                    //ChangeFile_modtime(localpath,filetmp->modtime);
                    time_t modtime;
                    modtime = Getmodtime(filetmp->href,index);
                    if(modtime != -1)
                        ChangeFile_modtime(localpath,modtime);
                    else
                    {
                        printf("ChangeFile_modtime failed!\n");
                    }
                    if(item != NULL)
                    {
                        del_action_item("download",filetmp->href,g_pSyncList[index]->unfinished_list);
                    }
                }
                else
                {
                    free(localpath);
                    return ret;
                }
            }
            else
            {
                write_log(S_ERROR,"local space is not enough!","",index);
                if(item == NULL)
                {
                    add_action_item("download",filetmp->href,g_pSyncList[index]->unfinished_list);
                }
            }
            free(localpath);
            filetmp = filetmp->next;
        }
    }
    else if(perform_br->filenumber != 0 && perform_lo->filenumber != 0)
    {
        //printf("remove local file while\n");
        while(localfiletmp != NULL)
        {
            int cmp = 1;
            char *localpathtmp;
            localpathtmp = strstr(localfiletmp->path,
                                  asus_cfg.prule[index]->base_path) + asus_cfg.prule[index]->base_path_len;
            //printf("localpathtmp = %s\n",localpathtmp);
            while(filetmp != NULL)
            {
                char *serverpathtmp;
                serverpathtmp = strstr(filetmp->href,
                                       asus_cfg.prule[index]->rootfolder) + asus_cfg.prule[index]->rootfolder_length;
                serverpathtmp = oauth_url_unescape(serverpathtmp,NULL);
                //printf("serverpathtmp = %s\n",serverpathtmp);
                if ((cmp = strcmp(localpathtmp,serverpathtmp)) == 0)
                {
                    //filetmp = OldFileList->next;
                    free(serverpathtmp);
                    break;
                }
                else
                {
                    filetmp = filetmp->next;
                }
                free(serverpathtmp);
            }
            if (cmp != 0)
            {
                if(wait_handle_socket(index))
                {
                    return HAVE_LOCAL_SOCKET;
                }
                if(asus_cfg.prule[index]->rule == 1)
                {
                    action_item *item;
                    item = get_action_item("download_only",localfiletmp->path,
                                           g_pSyncList[index]->download_only_socket_head,index);
                    if(item != NULL)
                    {
                        filetmp = perform_br->filelist->next;
                        localfiletmp = localfiletmp->next;
                        continue;
                    }
                }
                action_item *pp;
                pp = get_action_item("upload",localfiletmp->path,
                                     g_pSyncList[index]->up_space_not_enough_list,index);
                if(pp == NULL)
                {
                    unlink(localfiletmp->path);
                    add_action_item("remove",localfiletmp->path,g_pSyncList[index]->server_action_list);
                }
                //add_action_item("remove",localfiletmp->path,g_pSyncList[index]->server_action_list);

                //unlink(localfiletmp->path);
            }
            else
            {
                if((ret = the_same_name_compare(localfiletmp,filetmp,index)) != 0)
                {
                    return ret;
                }
            }
            filetmp = perform_br->filelist->next;
            localfiletmp = localfiletmp->next;
        }


        filetmp = perform_br->filelist->next;
        localfiletmp = perform_lo->filelist->next;

        //printf("download server file while\n");
        while(filetmp != NULL)
        {
            int cmp = 1;
            char *serverpathtmp;
            serverpathtmp = strstr(filetmp->href,
                                   asus_cfg.prule[index]->rootfolder) + asus_cfg.prule[index]->rootfolder_length;
            serverpathtmp = oauth_url_unescape(serverpathtmp,NULL);
            while(localfiletmp != NULL)
            {
                char *localpathtmp;
                localpathtmp = strstr(localfiletmp->path,
                                      asus_cfg.prule[index]->base_path) + asus_cfg.prule[index]->base_path_len;
                if ((cmp = strcmp(localpathtmp,serverpathtmp)) == 0)
                {
                    //filetmp = OldFileList->next;
                    break;
                }
                else
                {
                    localfiletmp = localfiletmp->next;
                }
            }
            free(serverpathtmp);
            if (cmp != 0)
            {
                if(wait_handle_socket(index))
                {
                    return HAVE_LOCAL_SOCKET;
                }
                char *localpath;
                localpath = serverpath_to_localpath(filetmp->href,index);

                action_item *item;
                item = get_action_item("download",
                                       filetmp->href,g_pSyncList[index]->unfinished_list,index);

                if(is_local_space_enough(filetmp,index))
                {
                    add_action_item("createfile",localpath,g_pSyncList[index]->server_action_list);
                    ret = Download(filetmp->href,index);
                    if (ret == NE_OK)
                    {
                        //char *localpath;
                        //localpath = serverpath_to_localpath(filetmp->href);
                        //ChangeFile_modtime(localpath,filetmp->modtime);
                        time_t modtime;
                        modtime = Getmodtime(filetmp->href,index);
                        if(modtime != -1)
                            ChangeFile_modtime(localpath,modtime);
                        else
                        {
                            printf("ChangeFile_modtime failed!\n");
                        }
                        if(item != NULL)
                        {
                            del_action_item("download",filetmp->href,g_pSyncList[index]->unfinished_list);
                        }
                    }
                    else
                    {
                        free(localpath);
                        //free(serverpathtmp);
                        return ret;
                    }
                }
                else
                {
                    write_log(S_ERROR,"local space is not enough!","",index);
                    if(item == NULL)
                    {
                        add_action_item("download",filetmp->href,g_pSyncList[index]->unfinished_list);
                    }

                }
                free(localpath);
            }
            //free(serverpathtmp);
            filetmp = filetmp->next;
            localfiletmp = perform_lo->filelist->next;
        }
    }



    /*************handle folders**************/
    //printf("##########handle folders\n");
    if(perform_br->foldernumber == 0 && perform_lo->foldernumber != 0)
    {
        while(localfoldertmp != NULL)
        {
            if(asus_cfg.prule[index]->rule == 1)
            {
                action_item *item;
                item = get_action_item("download_only",
                                       localfoldertmp->path,g_pSyncList[index]->download_only_socket_head,index);
                if(item != NULL)
                {
                    localfoldertmp = localfoldertmp->next;
                    continue;
                }
            }
            if(wait_handle_socket(index))
            {
                return HAVE_LOCAL_SOCKET;
            }
            del_all_items(localfoldertmp->path,index);
            localfoldertmp = localfoldertmp->next;
        }
    }
    else if(perform_br->foldernumber != 0 && perform_lo->foldernumber == 0)
    {
        while(foldertmp != NULL)
        {
            if(wait_handle_socket(index))
            {
                return HAVE_LOCAL_SOCKET;
            }
            char *localpath;
            localpath = serverpath_to_localpath(foldertmp->href,index);
            if(NULL == opendir(localpath))
            {
                add_action_item("createfolder",localpath,g_pSyncList[index]->server_action_list);
                mkdir(localpath,0777);
            }
            free(localpath);
            foldertmp = foldertmp->next;
        }

    }
    else if(perform_br->foldernumber != 0 && perform_lo->foldernumber != 0)
    {
        while(localfoldertmp != NULL)
        {
            int cmp = 1;
            char *localpathtmp;
            localpathtmp = strstr(localfoldertmp->path,
                                  asus_cfg.prule[index]->base_path) + asus_cfg.prule[index]->base_path_len;
            while(foldertmp != NULL)
            {
                char *serverpathtmp;
                serverpathtmp = strstr(foldertmp->href,
                                       asus_cfg.prule[index]->rootfolder) + asus_cfg.prule[index]->rootfolder_length;
                serverpathtmp = oauth_url_unescape(serverpathtmp,NULL);
                if ((cmp = strcmp(localpathtmp,serverpathtmp)) == 0)
                {
                    //filetmp = OldFileList->next;
                    free(serverpathtmp);
                    break;
                }
                else
                {
                    foldertmp = foldertmp->next;
                }
                free(serverpathtmp);
            }
            if (cmp != 0)
            {
                if(asus_cfg.prule[index]->rule == 1)
                {
                    action_item *item;
                    item = get_action_item("download_only",
                                           localfoldertmp->path,g_pSyncList[index]->download_only_socket_head,index);
                    if(item != NULL)
                    {
                        foldertmp = perform_br->folderlist->next;
                        localfoldertmp = localfoldertmp->next;
                        continue;
                    }
                }
                if(wait_handle_socket(index))
                {
                    return HAVE_LOCAL_SOCKET;
                }
                del_all_items(localfoldertmp->path,index);
            }
            foldertmp = perform_br->folderlist->next;
            localfoldertmp = localfoldertmp->next;
        }

        foldertmp = perform_br->folderlist->next;
        localfoldertmp = perform_lo->folderlist->next;
        while(foldertmp != NULL)
        {
            int cmp = 1;
            char *serverpathtmp;
            serverpathtmp = strstr(foldertmp->href,
                                   asus_cfg.prule[index]->rootfolder) + asus_cfg.prule[index]->rootfolder_length;
            serverpathtmp = oauth_url_unescape(serverpathtmp,NULL);
            while(localfoldertmp != NULL)
            {
                char *localpathtmp;
                localpathtmp = strstr(localfoldertmp->path,
                                      asus_cfg.prule[index]->base_path) + asus_cfg.prule[index]->base_path_len;
                if ((cmp = strcmp(localpathtmp,serverpathtmp)) == 0)
                {
                    //filetmp = OldFileList->next;
                    break;
                }
                else
                {
                    localfoldertmp = localfoldertmp->next;
                }
            }
            free(serverpathtmp);
            if (cmp != 0)
            {
                if(wait_handle_socket(index))
                {
                    return HAVE_LOCAL_SOCKET;
                }
                char *localpath;
                localpath = serverpath_to_localpath(foldertmp->href,index);
                if(NULL == opendir(localpath))
                {
                    add_action_item("createfolder",localpath,
                                    g_pSyncList[index]->server_action_list);
                    mkdir(localpath,0777);
                }
                free(localpath);
            }
            //free(serverpathtmp);
            foldertmp = foldertmp->next;
            localfoldertmp = perform_lo->folderlist->next;
        }
    }

    return ret;

}

int sync_server_to_local(Server_TreeNode *treenode,int (*sync_fuc)(Browse*,Local*,int),int index){

    //printf("sync_server_to_local start!\n");
    if(treenode->parenthref == NULL)
    {
        return 0;
    }

    Local *localnode;
    char *hreftmp;
    char *localpath;
    int ret = 0;


    hreftmp = strstr(treenode->parenthref,asus_cfg.prule[index]->rootfolder)+asus_cfg.prule[index]->rootfolder_length;
    //printf("hreftmp = %s,hreftmplen = %d\n",hreftmp,strlen(hreftmp));
    if(strlen(hreftmp) != 0)
    {
        hreftmp = oauth_url_unescape(hreftmp,NULL);
        localpath = (char *)malloc(sizeof(char)*(asus_cfg.prule[index]->base_path_len+strlen(hreftmp)+1));
        memset(localpath,'\0',sizeof(localpath));
        sprintf(localpath,"%s%s",asus_cfg.prule[index]->base_path,hreftmp);
        free(hreftmp);
    }
    else
    {
        localpath = (char *)malloc(sizeof(char)*(asus_cfg.prule[index]->base_path_len+1));
        memset(localpath,'\0',sizeof(localpath));
        sprintf(localpath,"%s",asus_cfg.prule[index]->base_path);
    }


    //printf("sync_server_to_local hreftmp = %s\n",hreftmp);
    //printf("sync_server_to_local localpath = %s\n",localpath);

    localnode = Find_Floor_Dir(localpath);
    free(localpath);
    /*if(NULL == localnode)
    {
        return LOCAL_FILE_LOST;
    }*/

    /*LocalFile *localfloorfiletmp;
    LocalFolder *localfloorfoldertmp;
    localfloorfiletmp = localnode->filelist->next;
    localfloorfoldertmp = localnode->folderlist->next;
    while(localfloorfiletmp != NULL)
    {
        printf("sync_server_to_local localfloorfiletmp->path = %s\n",localfloorfiletmp->path);
        localfloorfiletmp = localfloorfiletmp->next;
    }
    while(localfloorfoldertmp != NULL)
    {
        printf("sync_server_to_local localfloorfoldertmp->path = %s\n",localfloorfoldertmp->path);
        localfloorfoldertmp = localfloorfoldertmp->next;
    }*/

    if(NULL != localnode)
    {
        ret = sync_fuc(treenode->browse,localnode,index);
        //printf("**********ret2 = %d\n",ret);
        if(ret == COULD_NOT_CONNECNT_TO_SERVER || ret == CONNECNTION_TIMED_OUT
           || ret == COULD_NOT_READ_RESPONSE_BODY || ret == HAVE_LOCAL_SOCKET
           || ret == NE_WEBDAV_QUIT)
        {
            printf("###############free localnode\n");
            free_localfloor_node(localnode);
            return ret;
        }


        //printf("###############free localnode\n");
        free_localfloor_node(localnode);
        //printf("###############localnode->filenumber = %d\n",localnode->filenumber);
    }

    if(treenode->Child != NULL)
    {
        //if(treenode->Child->browse->filenumber != 0
        //   || treenode->Child->browse->foldernumber != 0)
        //{
            ret = sync_server_to_local(treenode->Child,sync_fuc,index);
            if(ret != 0 && ret != SERVER_SPACE_NOT_ENOUGH
               && ret != LOCAL_FILE_LOST && ret != SERVER_FILE_DELETED)
            {
                return ret;
            }
        //}
    }
    if(treenode->NextBrother != NULL)
    {
        //if(treenode->NextBrother->browse->filenumber != 0
        //   || treenode->NextBrother->browse->foldernumber != 0)
        //{
            ret = sync_server_to_local(treenode->NextBrother,sync_fuc,index);
            if(ret != 0 && ret != SERVER_SPACE_NOT_ENOUGH
               && ret != LOCAL_FILE_LOST && ret != SERVER_FILE_DELETED)
            {
                return ret;
            }
        //}
    }

    return ret;
}

int is_server_have_localpath(char *path,Server_TreeNode *treenode,int index){

    if(treenode == NULL)
        return 0;

    char *hreftmp;
    char *localpath;
    int ret = 0;
    int cmp = 1;

    hreftmp = strstr(treenode->parenthref,asus_cfg.prule[index]->rootfolder)+asus_cfg.prule[index]->rootfolder_length;
    hreftmp = oauth_url_unescape(hreftmp,NULL);
    if(hreftmp == NULL)
    {
        localpath = (char *)malloc(sizeof(char)*(asus_cfg.prule[index]->base_path_len+1));
        memset(localpath,'\0',sizeof(localpath));
        sprintf(localpath,"%s",asus_cfg.prule[index]->base_path);
    }
    else
    {
        localpath = (char *)malloc(sizeof(char)*(asus_cfg.prule[index]->base_path_len+strlen(hreftmp)+1));
        memset(localpath,'\0',sizeof(localpath));
        sprintf(localpath,"%s%s",asus_cfg.prule[index]->base_path,hreftmp);
        free(hreftmp);
    }

    if((cmp = strcmp(localpath,path)) == 0)
    {
        ret = 1;
        free(localpath);
        return ret;
    }
    else
    {
        free(localpath);
    }

    if(treenode->Child != NULL)
    {
        ret = is_server_have_localpath(path,treenode->Child,index);
        if(ret == 1)
        {
            return ret;
        }
    }
    if(treenode->NextBrother != NULL)
    {
        ret = is_server_have_localpath(path,treenode->NextBrother,index);
        if(ret == 1)
        {
            return ret;
        }
    }

    return ret;
}

int sync_local_to_server_init(Local *perform_lo,int index){

    LocalFolder *localfoldertmp;
    LocalFile *localfiletmp;
    int ret = 0;

    localfoldertmp = perform_lo->folderlist->next;
    localfiletmp = perform_lo->filelist->next;

    /*handle files*/
    while(localfiletmp != NULL)
    {
        if(asus_cfg.prule[index]->rule != 1)
        {
            /*while(is_file_copying(localfiletmp->path))
            {
                usleep(1000*100);
            }*/
            if(wait_handle_socket(index))
            {
                return HAVE_LOCAL_SOCKET;
            }
            add_action_item("createfile",localfiletmp->path,g_pSyncList[index]->server_action_list);
            ret = Upload(localfiletmp->path,index);
            if(ret == NE_OK || ret == SERVER_SPACE_NOT_ENOUGH || ret == LOCAL_FILE_LOST)
            {
                if(ret == NE_OK)
                {
                    char *serverpath;
                    serverpath = localpath_to_serverpath(localfiletmp->path,index);
                    //printf("serverpath = %s\n",serverpath);
                    time_t modtime;
                    modtime = Getmodtime(serverpath,index);
                    if(modtime != -1)
                        ChangeFile_modtime(localfiletmp->path,modtime);
                    else
                    {
                        printf("ChangeFile_modtime failed!\n");
                    }
                    free(serverpath);
                }
            }
            else
            {
                return ret;
            }
        }
        else
        {
            if(wait_handle_socket(index))
            {
                return HAVE_LOCAL_SOCKET;
            }
            add_action_item("createfile",localfiletmp->path,g_pSyncList[index]->download_only_socket_head);
        }

        localfiletmp = localfiletmp->next;
    }

    /*handle folders*/
    while(localfoldertmp != NULL)
    {
        if(asus_cfg.prule[index]->rule != 1)
        {
            if(wait_handle_socket(index))
            {
                return HAVE_LOCAL_SOCKET;
            }
            add_action_item("createfolder",localfoldertmp->path,g_pSyncList[index]->server_action_list);
            ret = Mkcol(localfoldertmp->path,index);
            if(ret != NE_OK && ret != LOCAL_FILE_LOST)
            {
                return ret;
            }
        }
        else
        {
            if(wait_handle_socket(index))
            {
                return HAVE_LOCAL_SOCKET;
            }
            add_action_item("createfolder",localfoldertmp->path,g_pSyncList[index]->download_only_socket_head);
        }

        localfoldertmp = localfoldertmp->next;
    }

    return ret;
}

int sync_local_to_server(char *path,int index){

    printf("sync_local_to_server path = %s\n",path);

    Local *localnode;
    int ret = 0;
    int res = 0;
    LocalFolder *localfoldertmp;

    localnode = Find_Floor_Dir(path);

    if(localnode == NULL)
    {
        return 0;
    }

    ret = is_server_have_localpath(path,g_pSyncList[index]->ServerRootNode->Child,index);

    if(ret == 0)
    {
        res = sync_local_to_server_init(localnode,index);
        if(res != 0 && res != SERVER_SPACE_NOT_ENOUGH && res != LOCAL_FILE_LOST)
        {
            printf("##########res = %d\n",res);
            free_localfloor_node(localnode);
            return res;
        }
    }

    localfoldertmp = localnode->folderlist->next;
    while(localfoldertmp != NULL)
    {
        res = sync_local_to_server(localfoldertmp->path,index);
        if(res != 0 && res != SERVER_SPACE_NOT_ENOUGH && res != LOCAL_FILE_LOST)
        {
            printf("##########res = %d\n",res);
            free_localfloor_node(localnode);
            return res;
        }

        localfoldertmp = localfoldertmp->next;
    }

    free_localfloor_node(localnode);

    return res;
}

int compareLocalList(int index){
    printf("compareLocalList start!\n");
    int ret = 0;

    if(g_pSyncList[index]->ServerRootNode->Child != NULL)
    {
        //if(g_pSyncList[index]->ServerRootNode->Child->browse->filenumber != 0
        //   || g_pSyncList[index]->ServerRootNode->Child->browse->foldernumber != 0)
        //{
            printf("ServerRootNode->Child != NULL\n");
            ret = sync_server_to_local(g_pSyncList[index]->ServerRootNode->Child,sync_server_to_local_perform,index);
        //}
        /*else
        {
            printf("ServerRootNode->Child == NULL\n");
        }*/
    }
    else
    {
        printf("ServerRootNode->Child == NULL\n");
    }

    return ret;
}

#if 0
int wd_initial(){
    printf("**************wd_initial**************\n");

    //LocalFile *SavedLocalFileTmp;
    SavedLocalFileList = (LocalFile *)malloc(sizeof(LocalFile));
    memset(SavedLocalFileList,'\0',sizeof(LocalFile));
    if(access(all_local_item_file,0) == 0)
    {
        get_all_local_file_item(all_local_item_file,SavedLocalFileList);
        remove(all_local_item_file);
    }
    /*SavedLocalFileTmp = SavedLocalFileList->next;
	while(SavedLocalFileTmp != NULL){
		printf("SavedLocalFileTmp->path = %s\n",SavedLocalFileTmp->path);
		printf("SavedLocalFileTmp->lastwritetime = %s\n",SavedLocalFileTmp->lastwritetime);
		SavedLocalFileTmp = SavedLocalFileTmp->next;
	}*/

    CloudFile *foldertmp,*filetmp;
    LocalFolder *localfoldertmp;
    LocalFile *localfiletmp;
    int cmp;
    int ret,status;
    int changelist = 0;
    /******time = 0 ,download; time = 1 ,upload; time = 2 ,no action;time = 3,rename then upload;*********/
    int time;
    //char *hreftmp;
    //char *pathtmp;
    char createpath[256];
    char href_unescape[512];
    //char createhref[256];
    foldertmp = OldFolderList->next;
    localfoldertmp = LocalFolderList->next;
    filetmp = OldFileList->next;
    localfiletmp = LocalFileList->next;

    memset(createpath,0,sizeof(createpath));
    memset(href_unescape,0,sizeof(href_unescape));

    /*ServerList's folders compared with LocalList's folders*/
    while (foldertmp->href != NULL)
    {
        cmp = 1;
        char *hreftmp;
        //memset(href_unescape,0,sizeof(href_unescape));
        hreftmp = strstr(foldertmp->href,ROOTFOLDER)+rootfolder_len;
        hreftmp = oauth_url_unescape(hreftmp,NULL);
        //printf("hreftmp = %s\n",hreftmp);
        //printf("foldertmp->href = %s\n",foldertmp->href);
        while (localfoldertmp->path != NULL)
        {
            char *pathtmp;
            pathtmp = strstr(localfoldertmp->path,base_path) + base_path_len;
            if ((cmp = strcmp(hreftmp,pathtmp)) == 0){
                //localfoldertmp = LocalFolderList->next;
                break;
            }
            else {
                localfoldertmp = localfoldertmp->next;
            }
        }
        if (cmp != 0){
            //hreftmp = oauth_url_unescape(hreftmp,NULL);
            sprintf(createpath,"%s%s",base_path,hreftmp);
            if(NULL == opendir(createpath))
                mkdir(createpath,0777);
            memset(createpath,0,sizeof(createpath));
            changelist = 1;
        }
        foldertmp = foldertmp->next;
        localfoldertmp = LocalFolderList->next;
    }
    if(changelist)
    {
        free_LocalFolder_item(LocalFolderList);
        free_LocalFile_item(LocalFileList);
        getLocalList();
        localfoldertmp = LocalFolderList->next;
        localfiletmp = LocalFileList->next;
        changelist = 0;
    }
    foldertmp = OldFolderList->next;

    /*LocalList's folders compared with ServerList's folders*/
    while (localfoldertmp->path != NULL){
        cmp = 1;
        char *pathtmp;
        pathtmp = strstr(localfoldertmp->path,base_path) + base_path_len;
        while (foldertmp->href != NULL){
            char *hreftmp;
            hreftmp = strstr(foldertmp->href,ROOTFOLDER) + rootfolder_len;
            hreftmp = oauth_url_unescape(hreftmp,NULL);
            if ((cmp = strcmp(hreftmp,pathtmp)) == 0){
                //foldertmp = OldFolderList->next;
                break;
            }
            else {
                foldertmp = foldertmp->next;
            }
        }
        if (cmp != 0)
        {
            printf("localfoldertmp->path = %s\n",localfoldertmp->path);
            add_sync_item(MKCOL,localfoldertmp->path,mkcol_head);
            ret = Mkcol(localfoldertmp->path);
            if(ret == NE_OK || ret == -1)
            {
                del_sync_item(MKCOL,localfoldertmp->path,mkcol_head);
            }
            changelist = 1;
        }
        localfoldertmp = localfoldertmp->next;
        foldertmp = OldFolderList->next;
    }
    if(changelist){
        //printf("############changelist = %d\n",changelist);
        free_LocalFolder_item(LocalFolderList);
        free_LocalFile_item(LocalFileList);
        getLocalList();
        localfoldertmp = LocalFolderList->next;
        localfiletmp = LocalFileList->next;

        do{
            status = getServerList();
            if (status != NE_OK)
            {
                printf("get ServerList ERROR! \n");
                server_sync = 0;      //server sync finished
                sleep(2);
                continue;
            }
        }while(status != NE_OK);


        status = compareServerList();
        printf("status = %d\n",status);
        if (status == 0)
        {
            free_CloudFile_item(OldFolderList);
            free_CloudFile_item(OldFileList);
            OldFolderList = FolderList;
            OldFileList = FileList;
        }
        else
        {
            free_CloudFile_item(FolderList);
            free_CloudFile_item(FileList);
        }
        foldertmp = OldFolderList->next;
        filetmp = OldFileList->next;

        changelist = 0;
    }


    /*ServerList's files compared with LocalList's files*/
    while (filetmp->href != NULL){
        cmp = 1;
        char *hreftmp;
        hreftmp = strstr(filetmp->href,ROOTFOLDER) + rootfolder_len;
        hreftmp = oauth_url_unescape(hreftmp,NULL);
        printf("hreftmp = %s\n",hreftmp);
        while (localfiletmp->path != NULL){
            char *pathtmp;
            pathtmp = strstr(localfiletmp->path,base_path) + base_path_len;
            //printf("pathtmp = %s\n",pathtmp);
            if ((cmp = strcmp(hreftmp,pathtmp)) == 0){     //just compared file's name
                time = wd_initial_compareTime(filetmp->modtime,localfiletmp->path,SavedLocalFileList);
                //printf("time = %d\n",time);
                if(time == 0){
                    //down_head = add_sync_item(DOWNLOAD,localfiletmp->path,down_head);
                    ret = Download(filetmp);
                    if (ret != NE_OK){
                        //remove_tmp(filetmp);
                        //free_LocalFolder_item(LocalFolderList);
                        //free_LocalFile_item(LocalFileList);
                    }
                    else{
                        ChangeFile_modtime(localfiletmp->path,filetmp->modtime);
                        //down_head = del_sync_item(DOWNLOAD,localfiletmp->path,down_head);
                    }
                    if(ret == NE_WEBDAV_QUIT){
                        return NE_WEBDAV_QUIT;
                    }
                    changelist = 1;

                }
                else if(time == 1){
                    //char fullname[256];
                    //memset(fullname,0,sizeof(fullname));
                    //sprintf(fullname,"%s%s",base_path,pathtmp);
                    add_sync_item(UPLOAD,localfiletmp->path,up_head);
                    ret = Upload(localfiletmp->path);
                    if(ret == NE_OK || ret == -1)
                    {
                        del_sync_item(UPLOAD,localfiletmp->path,up_head);
                        if(ret == NE_OK)
                        {
                            char URL[512];
                            int status;
                            memset(URL,0,sizeof(URL));
                            get_file_parent(filetmp->href,URL);
                            Upload_ChangeFile_modtime(URL,localfiletmp->path);
                            changelist = 1;
                        }
                    }
                    else if(ret == NE_WEBDAV_QUIT){
                        return NE_WEBDAV_QUIT;
                    }

                }
                else if(time == 3){
                    char newfilepath[256];
                    memset(newfilepath,'\0',sizeof(newfilepath));
                    sprintf(newfilepath,"%s(%d)",localfiletmp->path,rand());
                    rename(localfiletmp->path,newfilepath);

                    /******upload new name file*****/
                    add_sync_item(UPLOAD,newfilepath,up_head);
                    ret = Upload(newfilepath);
                    if(ret == NE_OK || ret == -1)
                    {
                        del_sync_item(UPLOAD,newfilepath,up_head);
                        if(ret == NE_OK)
                        {
                            char URL[512];
                            int status;
                            memset(URL,0,sizeof(URL));
                            get_file_parent(filetmp->href,URL);
                            Upload_ChangeFile_modtime(URL,newfilepath);
                        }
                    }
                    else if(ret == NE_WEBDAV_QUIT)
                    {
                        return NE_WEBDAV_QUIT;
                    }


                    /******download old name file*****/
                    ret = Download(filetmp);
                    if (ret != NE_OK)
                    {

                    }
                    else{
                        ChangeFile_modtime(localfiletmp->path,filetmp->modtime);
                        //down_head = del_sync_item(DOWNLOAD,localfiletmp->path,down_head);
                    }
                    if(ret == NE_WEBDAV_QUIT)
                    {
                        return NE_WEBDAV_QUIT;
                    }
                    changelist = 1;

                }
                else{
                    //no action
                }
                //localfiletmp = LocalFileList->next;
                break;
            }
            else
            {
                localfiletmp = localfiletmp->next;
            }
        }
        if (cmp != 0){
            //down_head = add_sync_item(DOWNLOAD,localfiletmp->path,down_head);
            ret = Download(filetmp);
            if (ret != NE_OK){
                //remove_tmp(filetmp);
                //free_LocalFolder_item(LocalFolderList);
                //free_LocalFile_item(LocalFileList);
            }
            else{
                //down_head = del_sync_item(DOWNLOAD,localfiletmp->path,down_head);
                char localfilepath[512];
                memset(localfilepath,0,sizeof(localfilepath));
                sprintf(localfilepath,"%s%s",base_path,hreftmp);
                ChangeFile_modtime(localfilepath,filetmp->modtime);
            }
            if(ret == NE_WEBDAV_QUIT){
                return NE_WEBDAV_QUIT;
            }
            changelist = 1;
        }
        filetmp = filetmp->next;
        localfiletmp = LocalFileList->next;
    }
    if(changelist)
    {
        free_LocalFolder_item(LocalFolderList);
        free_LocalFile_item(LocalFileList);
        getLocalList();
        localfoldertmp = LocalFolderList->next;
        localfiletmp = LocalFileList->next;
        changelist = 0;
    }
    filetmp = OldFileList->next;

    /*LocalList's files compared with ServerList's files*/
    while (localfiletmp->path != NULL){
        //printf("localfiletmp->path = %s \n",localfiletmp->path);
        cmp = 1;
        char *pathtmp;
        pathtmp = strstr(localfiletmp->path,base_path) + base_path_len;
        while (filetmp->href != NULL){
            char *hreftmp;
            hreftmp = strstr(filetmp->href,ROOTFOLDER) + rootfolder_len;
            hreftmp = oauth_url_unescape(hreftmp,NULL);
            //printf("pathtmp = %s \n",pathtmp);
            //printf("hreftmp = %s \n",hreftmp);
            if ((cmp = strcmp(hreftmp,pathtmp)) == 0){
                //filetmp = OldFileList->next;
                break;
            }
            else {
                filetmp = filetmp->next;
            }
        }
        if (cmp != 0){
            add_sync_item(UPLOAD,localfiletmp->path,up_head);
            ret = Upload(localfiletmp->path);
            if(ret == NE_OK || ret == -1)
            {
                del_sync_item(UPLOAD,localfiletmp->path,up_head);
                if(ret == NE_OK)
                {
                    char escapepath_tmp[512];
                    char path_tmp[256];
                    memset(path_tmp,0,sizeof(path_tmp));
                    sprintf(path_tmp,"%s",localfiletmp->path);
                    memset(escapepath_tmp,0,sizeof(escapepath_tmp));
                    wd_escape(path_tmp,escapepath_tmp);
                    char URL[512];
                    memset(URL,0,sizeof(URL));
                    printf("escapepath_tmp = %s\n",escapepath_tmp);
                    get_file_parent_base_path(escapepath_tmp,URL);
                    //printf("URL = %s\n",URL);
                    Upload_ChangeFile_modtime(URL,localfiletmp->path);
                    changelist = 1;
                }
            }
            else if(ret == NE_WEBDAV_QUIT)
            {
                return NE_WEBDAV_QUIT;
            }

        }
        localfiletmp = localfiletmp->next;
        filetmp = OldFileList->next;
    }
    if(changelist)
    {
        free_LocalFolder_item(LocalFolderList);
        free_LocalFile_item(LocalFileList);
        getLocalList();
        changelist = 0;
    }
    return 0;
}
#endif

#if 1
/*
 *used for initial
 *if the local root folder is null,
 *it will download all the files from Server
*/
int wd_initial(){
    int ret;
    int status;
    int i;
    //struct timeval now;
    //struct timespec outtime;
    for(i=0;i<asus_cfg.dir_number;i++)
    {
        if(g_pSyncList[i]->init_completed)
            continue;
        write_log(S_INITIAL,"","",i);
        ret = 1;
        printf("it is %d init \n",i);
        //while(ret != 0 && ret != NE_WEBDAV_QUIT && exit_loop == 0)
        if(exit_loop == 0)
        {
            printf("wd_initial ret = %d\n",ret);
            free_server_tree(g_pSyncList[i]->ServerRootNode);
            g_pSyncList[i]->ServerRootNode = NULL;

#if LIST_TEST
            //do{
            g_pSyncList[i]->ServerRootNode = create_server_treeroot();
            status = browse_to_tree(asus_cfg.prule[i]->rooturl,g_pSyncList[i]->ServerRootNode,i);
            //SearchServerTree(ServerRootNode);
            //sleep(2);
            usleep(1000*200);
            if(status != NE_OK)
                continue;
            //usleep(1000*200);
            //}while(status != NE_OK && exit_loop == 0);
#endif

            if(exit_loop == 0)
            {
                if(test_if_dir_empty(asus_cfg.prule[i]->base_path))
                {
                    printf("base_path is blank\n");
                    if(asus_cfg.prule[i]->rule != 2)
                    {
                        if(g_pSyncList[i]->ServerRootNode->Child != NULL)
                        {
                            printf("######## init sync folder,please wait......#######\n");
                            ret = initMyLocalFolder(g_pSyncList[i]->ServerRootNode->Child,i);
                            if(ret != 0 && ret != NE_WEBDAV_QUIT)
                                continue;
                            else if(ret == NE_WEBDAV_QUIT)
                                break;
                            g_pSyncList[i]->init_completed = 1;
                            g_pSyncList[i]->OldServerRootNode = g_pSyncList[i]->ServerRootNode;
                            printf("######## init sync folder end#######\n");
                        }
                    }
                    else
                    {
                        g_pSyncList[i]->init_completed = 1;
                        g_pSyncList[i]->OldServerRootNode = g_pSyncList[i]->ServerRootNode;
                        ret = 0;
                    }
                }
                else
                {
                    //if(sync_up == 1)
                    //{
                    printf("######## init sync folder(have files),please wait......#######\n");
                    if(g_pSyncList[i]->ServerRootNode->Child != NULL)
                        ret = sync_server_to_local(g_pSyncList[i]->ServerRootNode->Child,sync_server_to_local_init,i);
                    //printf("**********ret1 = %d\n",ret);
                    if(ret != 0 && ret != NE_WEBDAV_QUIT)
                        continue;
                    else if(ret == NE_WEBDAV_QUIT)
                        break;
                    //if(sync_down == 1)
                    //{
                    ret = sync_local_to_server(asus_cfg.prule[i]->base_path,i);
                    printf("#########ret = %d\n",ret);
                    //}
                    if(ret != 0 && ret != NE_WEBDAV_QUIT)
                        continue;
                    else if(ret == NE_WEBDAV_QUIT)
                        break;
                    printf("######## init sync folder end#######\n");
                    //}
                    g_pSyncList[i]->init_completed = 1;
                    g_pSyncList[i]->OldServerRootNode = g_pSyncList[i]->ServerRootNode;
                }
            }
        }
        if(ret == 0)
            write_log(S_SYNC,"","",i);
        /*pthread_mutex_lock(&mutex);
        gettimeofday(&now, NULL);
        outtime.tv_sec = now.tv_sec + 3;
        outtime.tv_nsec = now.tv_usec * 1000;
        pthread_cond_timedwait(&cond, &mutex, &outtime);
        pthread_mutex_unlock(&mutex);*/
    }


    return ret;
}
#endif

#if 0
int wd_initial(){
    printf("**************wd_initial**************\n");

    CloudFile *foldertmp,*filetmp;
    LocalFolder *localfoldertmp;
    LocalFile *localfiletmp;
    int cmp;
    int ret;
    int changelist = 0;
    /******time = 0 ,download; time = 1 ,upload; time = 2 ,no action*********/
    int time;
    //char *hreftmp;
    //char *pathtmp;
    char createpath[256];
    char href_unescape[512];
    //char createhref[256];
    foldertmp = OldFolderList->next;
    localfoldertmp = LocalFolderList->next;
    filetmp = OldFileList->next;
    localfiletmp = LocalFileList->next;

    memset(createpath,0,sizeof(createpath));
    memset(href_unescape,0,sizeof(href_unescape));

    /*ServerList's folders compared with LocalList's folders*/
    while (foldertmp->href != NULL){
        cmp = 1;
        char *hreftmp;
        //memset(href_unescape,0,sizeof(href_unescape));
        hreftmp = strstr(foldertmp->href,ROOTFOLDER)+rootfolder_len;
        hreftmp = oauth_url_unescape(hreftmp,NULL);
        //printf("hreftmp = %s\n",hreftmp);
        //printf("foldertmp->href = %s\n",foldertmp->href);
        while (localfoldertmp->path != NULL){
            char *pathtmp;
            pathtmp = strstr(localfoldertmp->path,base_path) + base_path_len;
            if ((cmp = strcmp(hreftmp,pathtmp)) == 0){
                //localfoldertmp = LocalFolderList->next;
                break;
            }
            else {
                localfoldertmp = localfoldertmp->next;
            }
        }
        if (cmp != 0){
            //hreftmp = oauth_url_unescape(hreftmp,NULL);
            sprintf(createpath,"%s%s",base_path,hreftmp);
            if(NULL == opendir(createpath))
                mkdir(createpath,0777);
            memset(createpath,0,sizeof(createpath));
            changelist = 1;
        }
        foldertmp = foldertmp->next;
        localfoldertmp = LocalFolderList->next;
    }

    foldertmp = OldFolderList->next;
    localfoldertmp = LocalFolderList->next;

    /*LocalList's folders compared with ServerList's folders*/
    while (localfoldertmp->path != NULL){
        cmp = 1;
        char *pathtmp;
        pathtmp = strstr(localfoldertmp->path,base_path) + base_path_len;
        while (foldertmp->href != NULL){
            char *hreftmp;
            hreftmp = strstr(foldertmp->href,ROOTFOLDER) + rootfolder_len;
            hreftmp = oauth_url_unescape(hreftmp,NULL);
            if ((cmp = strcmp(hreftmp,pathtmp)) == 0){
                //foldertmp = OldFolderList->next;
                break;
            }
            else {
                foldertmp = foldertmp->next;
            }
        }
        if (cmp != 0){
            printf("localfoldertmp->path = %s\n",localfoldertmp->path);
            del_all_items(localfoldertmp->path);
            changelist = 1;
        }
        localfoldertmp = localfoldertmp->next;
        foldertmp = OldFolderList->next;
    }
    if(changelist){
        free_LocalFolder_item(LocalFolderList);
        free_LocalFile_item(LocalFileList);
        getLocalList();
    }


    /*ServerList's files compared with LocalList's files*/
    while (filetmp->href != NULL){
        cmp = 1;
        char *hreftmp;
        hreftmp = strstr(filetmp->href,ROOTFOLDER) + rootfolder_len;
        hreftmp = oauth_url_unescape(hreftmp,NULL);
        //printf("hreftmp = %s\n",hreftmp);
        while (localfiletmp->path != NULL){
            char *pathtmp;
            pathtmp = strstr(localfiletmp->path,base_path) + base_path_len;
            //printf("pathtmp = %s\n",pathtmp);
            if ((cmp = strcmp(hreftmp,pathtmp)) == 0){     //just compared file's name
                time = compareTime(filetmp->modtime,localfiletmp->path);
                //printf("time = %d\n",time);
                if(time == 0){
                    //down_head = add_sync_item(DOWNLOAD,localfiletmp->path,down_head);
                    ret = Download(filetmp);
                    if (ret != NE_OK){
                        remove_tmp(filetmp);
                        //free_LocalFolder_item(LocalFolderList);
                        //free_LocalFile_item(LocalFileList);
                    }
                    else{
                        ChangeFile_modtime(localfiletmp->path,filetmp->modtime);
                        //down_head = del_sync_item(DOWNLOAD,localfiletmp->path,down_head);
                    }
                    if(ret == NE_WEBDAV_QUIT){
                        return NE_WEBDAV_QUIT;
                    }

                }
                else if(time == 1){
                    //char fullname[256];
                    //memset(fullname,0,sizeof(fullname));
                    //sprintf(fullname,"%s%s",base_path,pathtmp);
                    up_head = add_sync_item(UPLOAD,localfiletmp->path,up_head);
                    ret = Upload(localfiletmp->path);
                    if(ret == NE_OK)
                    {
                        up_head = del_sync_item(UPLOAD,localfiletmp->path,up_head);
                    }
                    else if(ret == NE_WEBDAV_QUIT){
                        return NE_WEBDAV_QUIT;
                    }
                    char URL[512];
                    int status;
                    memset(URL,0,sizeof(URL));
                    get_file_parent(filetmp->href,URL);
                    Upload_ChangeFile_modtime(URL,localfiletmp->path);
                }
                else{
                    //no action
                }
                //localfiletmp = LocalFileList->next;
                break;
            }
            else {
                localfiletmp = localfiletmp->next;
            }
        }
        if (cmp != 0){
            //down_head = add_sync_item(DOWNLOAD,localfiletmp->path,down_head);
            ret = Download(filetmp);
            if (ret != NE_OK){
                remove_tmp(filetmp);
                //free_LocalFolder_item(LocalFolderList);
                //free_LocalFile_item(LocalFileList);
            }
            else{
                //down_head = del_sync_item(DOWNLOAD,localfiletmp->path,down_head);
                char localfilepath[512];
                memset(localfilepath,0,sizeof(localfilepath));
                sprintf(localfilepath,"%s%s",base_path,hreftmp);
                ChangeFile_modtime(localfilepath,filetmp->modtime);
            }
            if(ret == NE_WEBDAV_QUIT){
                return NE_WEBDAV_QUIT;
            }

        }
        filetmp = filetmp->next;
        localfiletmp = LocalFileList->next;
    }
    filetmp = OldFileList->next;
    localfiletmp = LocalFileList->next;

    /*LocalList's files compared with ServerList's files*/
    while (localfiletmp->path != NULL){
        //printf("localfiletmp->path = %s \n",localfiletmp->path);
        cmp = 1;
        char *pathtmp;
        pathtmp = strstr(localfiletmp->path,base_path) + base_path_len;
        while (filetmp->href != NULL){
            char *hreftmp;
            hreftmp = strstr(filetmp->href,ROOTFOLDER) + rootfolder_len;
            hreftmp = oauth_url_unescape(hreftmp,NULL);
            //printf("pathtmp = %s \n",pathtmp);
            //printf("hreftmp = %s \n",hreftmp);
            if ((cmp = strcmp(hreftmp,pathtmp)) == 0){
                //filetmp = OldFileList->next;
                break;
            }
            else {
                filetmp = filetmp->next;
            }
        }
        if (cmp != 0){
            //printf("unlink %s \n",pathtmp);
            unlink(localfiletmp->path);
        }
        localfiletmp = localfiletmp->next;
        filetmp = OldFileList->next;
    }

    return 0;
}
#endif


#if 0
int getLocalInfo(char *dir)
{
    //printf("dir = %s\n",dir);
    //printf("**************getLocalInfo***************\n");
    struct dirent* ent = NULL;
    DIR *pDir;
    pDir=opendir(dir);

    if(pDir != NULL )
    {
        while (NULL != (ent=readdir(pDir)))
        {

            //if(ent->d_name[0] == '.')          //hidden file and folder
            //continue;
            if(!strcmp(ent->d_name,".") || !strcmp(ent->d_name,".."))
                continue;
            if(test_if_download_temp_file(ent->d_name))     //download temp files
                continue;

            char *fullname;
            fullname = (char *)malloc(sizeof(char)*(strlen(dir)+strlen(ent->d_name)+2));
            memset(fullname,'\0',sizeof(fullname));
            sprintf(fullname,"%s/%s",dir,ent->d_name);
            //printf("fullname = %s\n",fullname);
            //printf("name is %s,d_type is %u\n",ent->d_name,ent->d_type);

            if( test_if_dir(fullname) == 1 )
            {
            	LocalFolderTmp = (LocalFolder *)malloc(sizeof(LocalFolder));
            	memset(LocalFolderTmp,0,sizeof(LocalFolder));
                LocalFolderTmp->path = (char *)malloc(sizeof(char)*(strlen(fullname)+1));
                memset(LocalFolderTmp->path,'\0',sizeof(LocalFolderTmp->path));
                //printf("folder name is %s,folder num is %d\n",ent->d_name,local->foldernum);
                //strcpy(local->folderlist[foldernum].name,ent->d_name);
                sprintf(LocalFolderTmp->name,"%s",ent->d_name);
                sprintf(LocalFolderTmp->path,"%s",fullname);

                //printf("LocalFolderTmp->path = %s\n",LocalFolderTmp->path);

                LocalFolderTail->next = LocalFolderTmp;
                LocalFolderTail = LocalFolderTmp;
                LocalFolderTail->next = NULL;
            }
            else
            {
                struct stat buf;

                if( stat(fullname,&buf) == -1)
                {
                    perror("stat:");
                    continue;
                }
                LocalFileTmp = (LocalFile *)malloc(sizeof(LocalFile));
                memset(LocalFileTmp,0,sizeof(LocalFile));
                LocalFileTmp->path = (char *)malloc(sizeof(char)*(strlen(fullname)+1));
                memset(LocalFileTmp->path,'\0',sizeof(LocalFileTmp->path));

                unsigned long asec = buf.st_atime;
                unsigned long msec = buf.st_mtime;
                unsigned long csec = buf.st_ctime;
                //time_t msec = buf.st_mtime;
                //time_t csec = buf.st_ctime;

                sprintf(LocalFileTmp->lastaccesstime,"%lu",asec);
                sprintf(LocalFileTmp->creationtime,"%lu",csec);
                sprintf(LocalFileTmp->lastwritetime,"%lu",msec);
                //printf("creationtime = %lu\n",csec);
                //printf("lastwritetime = %lu\n",msec);
                //printf("lastwritetime = %s\n",ctime(&msec));
                //printf("lastwritetime = %s\n",ctime(&msec));

                //strcpy(local->filelist[filenum].name,ent->d_name);
                //printf("@@@@@@@@ file is %s @@@@@@@\n ",ent->d_name);
                sprintf(LocalFileTmp->name,"%s",ent->d_name);
                sprintf(LocalFileTmp->path,"%s",fullname);
                LocalFileTmp->size = buf.st_size;

                //printf("LocalFileTmp->path = %s\n",LocalFileTmp->path);

                LocalFileTail->next = LocalFileTmp;
                LocalFileTail = LocalFileTmp;
                LocalFileTail->next = NULL;
            }
            free(fullname);
            //printf("per end\n");

        }
        closedir(pDir);
        return 0;
    }
    else{
    	printf("open %s fail \n",dir);
    	return -1;
    }

}
#endif

/*
int getLocalList(){
    printf("**********getLocalList****************\n");

    int status;
    LocalFolderList = (LocalFolder *)malloc(sizeof(LocalFolder));
    memset(LocalFolderList,0,sizeof(LocalFolder));
    LocalFileList = (LocalFile *)malloc(sizeof(LocalFile));
    memset(LocalFileList,0,sizeof(LocalFile));

    LocalFolderList->path = NULL;
    LocalFileList->path = NULL;

    LocalFolderTail = LocalFolderList;
    LocalFileTail = LocalFileList;

    status = getLocalInfo(base_path);
    if(status != 0)
    {
        printf("get Local Info ERROR! \n");
        return -1;
    }

    LocalFolderCurrent = LocalFolderList->next;
    while(LocalFolderCurrent != NULL){
        //printf("LocalFolderCurrent->path = %s\n",LocalFolderCurrent->path);
        status = getLocalInfo(LocalFolderCurrent->path);
        if(status != 0)
        {
            printf("get Local Info ERROR! \n");
            return -1;
        }
        LocalFolderCurrent = LocalFolderCurrent->next;
    }
#if 0
    LocalFolder *de_foldercurrent;
    LocalFile *de_filecurrent;
    de_foldercurrent = LocalFolderList->next;
    de_filecurrent = LocalFileList->next;
    while(de_foldercurrent->path != NULL){
        printf("localfolder->path = %s\n",de_foldercurrent->path);
        de_foldercurrent = de_foldercurrent->next;
    }
    while(de_filecurrent->path != NULL){
        printf("localfile->path = %s\n",de_filecurrent->path);
        de_filecurrent = de_filecurrent->next;
    }

#endif

    return 0;
}*/

void *SyncServer(){
    struct timeval now;
    struct timespec outtime;
    pthread_mutex_lock(&mutex);

    int status;
    int i;

    while (!exit_loop)
    {
        //int testint;
        //testint = ne_getrouterinfo(parseRouterInfo);

        printf("***************SyncServer start**************\n");
        for(i=0;i<asus_cfg.dir_number;i++)
        {
            status = 0;
            printf("the %d SyncServer\n",i);

            //memset(now,'\0',sizeof(now));
            //memset(outtime,'\0',sizeof(outtime));
            //pthread_mutex_lock(&mutex_sync);
            while (local_sync == 1 && exit_loop == 0){
                //printf("local_sync = %d\n",local_sync);
                //sleep(2);
                usleep(1000*10);
                //server_sync = 0;
            }
            server_sync = 1;      //server sync starting
            //pthread_mutex_unlock(&mutex_sync);

            if(exit_loop)
                break;

            if(g_pSyncList[i]->no_local_root)
            {
                my_mkdir_r(asus_cfg.prule[i]->base_path);
                send_action(1,asus_cfg.prule[i]->base_path);
                usleep(1000*10);
                g_pSyncList[i]->no_local_root = 0;
                g_pSyncList[i]->init_completed = 0;
            }

            status = do_unfinished(i);
            printf("do_unfinished completed!\n");
            //dounfinish = 0;
            if(status != 0 && status != -1)
            {
                server_sync = 0;      //server sync finished
                //sleep(2);
                usleep(1000*200);
                break;
            }

            if(g_pSyncList[i]->init_completed == 0)
                status = wd_initial();

            if(status != 0)
                continue;

            if(asus_cfg.prule[i]->rule == 2)
            {
                //write_log(S_SYNC,"","",i);
                continue;
            }
            
            if(exit_loop == 0)
            {
                //server_sync = 1;      //server sync starting
#if LIST_TEST
                printf("get ServerRootNode\n");
                g_pSyncList[i]->ServerRootNode = create_server_treeroot();
                status = browse_to_tree(asus_cfg.prule[i]->rooturl,g_pSyncList[i]->ServerRootNode,i);
                //SearchServerTree(ServerRootNode);
                if (status != NE_OK)
                {
                    printf("get ServerList ERROR! \n");
                    //server_sync = 0;      //server sync finished
                    g_pSyncList[i]->first_sync = 1;
                    //sleep(2);
                    usleep(1000*20);
                    continue;
                    //break;
                }
                //sleep(2);
                //usleep(1000*200);
#endif
                /*status = getServerList();
                if (status != NE_OK)
                {
                    printf("get ServerList ERROR! \n");
                    server_sync = 0;      //server sync finished
                    sleep(2);
                    continue;
                }*/

                /****data:2012/07/02****/
                /*status = compareServerList();
                if (status == 0)
                {
                    free_CloudFile_item(OldFolderList);
                    free_CloudFile_item(OldFileList);
                    OldFolderList = FolderList;
                    OldFileList = FileList;
                }
                else
                {
                    free_CloudFile_item(FolderList);
                    free_CloudFile_item(FileList);
                }*/

                //dounfinish = 1;
                /*status = do_unfinished(i);
                printf("do_unfinished completed!\n");
                //dounfinish = 0;
                if(status != 0 && status != -1)
                {
                    server_sync = 0;      //server sync finished
                    //sleep(2);
                    usleep(1000*200);
                    break;
                }*/

                //free_LocalFolder_item(LocalFolderList);
                //free_LocalFile_item(LocalFileList);
                //free_CloudFile_item(FolderList);
                //free_CloudFile_item(FileList);
                //getLocalList();

                /*******data:2012/07/09*********/
                /*status = getServerList();
                if (status != NE_OK)
                {
                    printf("get ServerList ERROR! \n");
                    server_sync = 0;      //server sync finished
                    sleep(2);
                    continue;
                }*/
                if(g_pSyncList[i]->unfinished_list->next != NULL)
                {
                    continue;
                }



                //printf("status = %d\n",status);
                if(asus_cfg.prule[i]->rule == 0)
                {
                    status = compareServerList(i);
                }

                if(g_pSyncList[i]->first_sync)
                {
                    printf("first sync!\n");
                    g_pSyncList[i]->first_sync = 0;
                    //free_CloudFile_item(OldFolderList);
                    //free_CloudFile_item(OldFileList);
                    //OldFolderList = FolderList;
                    //OldFileList = FileList;
                    //FolderList = NULL;
                    //FileList = NULL;
#if LIST_TEST
                    free_server_tree(g_pSyncList[i]->OldServerRootNode);
                    g_pSyncList[i]->OldServerRootNode = g_pSyncList[i]->ServerRootNode;
#endif
                    //getLocalList();
                    status = compareLocalList(i);
                    //free_LocalFolder_item(LocalFolderList);
                    //free_LocalFile_item(LocalFileList);
                    //getLocalList();
                }
                else
                {
                    //serverList different or download only
                    if (status == 0 || asus_cfg.prule[i]->rule == 1)
                    {
                        //printf("ServerList is changed !\n");
                        //free_CloudFile_item(OldFolderList);
                        //free_CloudFile_item(OldFileList);
                        //OldFolderList = FolderList;
                        //OldFileList = FileList;
                        //FolderList = NULL;
                        //FileList = NULL;
#if LIST_TEST
                        free_server_tree(g_pSyncList[i]->OldServerRootNode);
                        g_pSyncList[i]->OldServerRootNode = g_pSyncList[i]->ServerRootNode;
#endif
                        //getLocalList();
                        status = compareLocalList(i);
                        //free_LocalFolder_item(LocalFolderList);
                        //free_LocalFile_item(LocalFileList);
                        //getLocalList();
                    }
                    else
                    {//serverList same
                        //free_CloudFile_item(FolderList);
                        //free_CloudFile_item(FileList);

                        //FolderList = NULL;
                        //FileList = NULL;
#if LIST_TEST
                        free_server_tree(g_pSyncList[i]->ServerRootNode);
                        g_pSyncList[i]->ServerRootNode = NULL;
#endif
                    }
                }


            }
            write_log(S_SYNC,"","",i);
            /*gettimeofday(&now, NULL);
            outtime.tv_sec = now.tv_sec + 3;
            outtime.tv_nsec = now.tv_usec * 1000;
            pthread_cond_timedwait(&cond, &mutex, &outtime);*/
        }
        //write_log(S_SYNC,"","");
        server_sync = 0;      //server sync finished
        if(!exit_loop)
        {
            gettimeofday(&now, NULL);
            outtime.tv_sec = now.tv_sec + 10;
            outtime.tv_nsec = now.tv_usec * 1000;
            pthread_cond_timedwait(&cond, &mutex, &outtime);
        }
    }
    pthread_mutex_unlock(&mutex);
    printf("stop WebDAV server sync\n");

    stop_up = 1;

}

void* sigmgr_thread(){
    sigset_t   waitset;
    //siginfo_t  info;
    int        sig;
    int        rc;
    pthread_t  ppid = pthread_self();

    pthread_detach(ppid);

    sigemptyset(&waitset);
    sigaddset(&waitset,SIGUSR1);
    sigaddset(&waitset,SIGUSR2);
    sigaddset(&waitset,SIGTERM);

    while (1)  {
        rc = sigwait(&waitset, &sig);
        if (rc != -1) {
            printf("sigwait() fetch the signal - %d\n", sig);
            sig_handler(sig);
        } else {
            printf("sigwaitinfo() returned err: %d; %s\n", errno, strerror(errno));
        }
    }
}

void sig_handler (int signum)
{
    //Getmysyncfolder *gf;
    //printf("sig_handler !\n");
    //sleep(5);

    switch (signum)
    {
    case SIGTERM:
        printf("signal is SIGTERM\n");
        //signal(SIGINT, SIG_DFL);
        //stop_sync_server = 1;
        sync_up = 0;
        sync_down = 0;
        stop_progress = 1;
        exit_loop = 1;
        //pthread_mutex_lock(&mutex);
        pthread_cond_signal(&cond);
        pthread_cond_signal(&cond_socket);
        pthread_cond_signal(&cond_log);
        //pthread_mutex_unlock(&mutex);
        break;
    case SIGUSR1:  // add user
        printf("signal is SIGUSER1\n");
        //signal(SIGINT, SIG_DFL);
        //no_config = 0;
        exit_loop = 1;
        read_config();
        //restart_run = 1;
        //run();
        break;
    case SIGUSR2:  // modify user
        printf("signal is SIGUSR2\n");
        //signal(SIGINT, SIG_DFL);
        //no_config = 0;
        exit_loop = 1;
        sync_up = 0;
        sync_down = 0;
        break;
#if 0
    case SIGPIPE:  // delete user
        printf("signal is SIGPIPE\n");

        signal(SIGPIPE, SIG_IGN);
        //no_config = 0;
        //stop_progress = 1;
        //exit_loop = 1;
        //sync_up = 0;
        //sync_down = 0;
        //restart_run = 1;
        //run();

        break;
#endif
    default:
        break;
    }
}

//just add unfinished upload files
void add_unfinished_list_to_file(int index){

    //printf("enter add_unfinished_list_to_file\n");

    if(g_pSyncList[index]->unfinished_list->next != NULL)
    {
        action_item *p;
        char *localpath;
        p = g_pSyncList[index]->unfinished_list->next;
        printf("p->path = %s\n",p->path);
        while(p != NULL)
        {
            if(!(strcmp(p->action,"upload")))
            {
                localpath = strstr(p->path,asus_cfg.prule[index]->base_path) + asus_cfg.prule[index]->base_path_len;
                FILE *fp;
                //printf("localpath = %s\n",localpath);
                //printf("g_pSyncList[index]->up_item_file = %s\n",g_pSyncList[index]->up_item_file);
                fp = fopen(g_pSyncList[index]->up_item_file,"w");
                //printf("open %s OK!\n",g_pSyncList[index]->up_item_file);
                if(fp == NULL)
                {
                    printf("open %s error\n",g_pSyncList[index]->up_item_file);
                    //return NULL;
                }
                fprintf(fp,"%s",localpath);
                fclose(fp);
                //printf("close fp!\n");
                return ;
            }
            p = p->next;
        }
    }
}

void clean_up()
{
    printf("enter clean up\n");
    int i;

    pthread_cond_destroy(&cond);
    pthread_cond_destroy(&cond_socket);
    pthread_cond_destroy(&cond_log);

    //add_sync_item("upload,12345","up_test",up_head);
    //add_sync_item("download,23456,256","down_test",down_head);
#if 0
    if(up_head->next != NULL)
    {
        /*if( !item_empty(up_head))
        {
            printf("write uncomplete upload item to file\n");
            print_sync_item(up_head,UPLOAD);
        }
        else
            printf("upload all finish\n");*/
    	struct sync_item *p1;
    	p1 = up_head->next;
    	while(p1 != NULL)
    	{
            printf("up_head->href = %s\n",p1->href);
            p1=p1->next;
    	}

        free_sync_item(up_head);
    }
    else
    {
        free_sync_item(up_head);
    }


    if(mkcol_head->next != NULL)
    {
    	struct sync_item *p1;
    	p1 = mkcol_head->next;
    	while(p1 != NULL)
    	{
            printf("mkcol_head->href = %s\n",p1->href);
            p1=p1->next;
    	}
    	free_sync_item(mkcol_head);
    }
    else
    {
        free_sync_item(mkcol_head);
    }

    free_sync_item(mkcol_parent_head);
#endif
    /*if(mkcol_head->next != NULL)
    {
       	struct sync_item *p1;
       	p1 = mkcol_head->next;
       	while(p1 != NULL)
       	{
       		printf("mkcol_head->href = %s\n",p1->href);
       		p1=p1->next;
       	}
       	free_sync_item(mkcol_head);
   }*/

    /*if(LocalActionList->next != NULL){
    	print_LocalAction_item();
    	free_LocalAction_item(LocalActionList);
    }*/

    /*if(down_head != NULL)
    {
        if( !item_empty(down_head))
        {
            printf("write uncomplete download item to file\n");
            print_sync_item(down_head,DOWNLOAD);
        }
        else
            printf("download all finish\n");

        free_sync_item(down_head);
     }
     else
    {
         printf("down head is NULL\n");
     }*/

    //free_sync_item(from_server_sync_head);

    //printf("print all local item to file\n");


    //queue_destroy(queue_download);
    //queue_destroy(queue_upload);
    remove(general_log);
    //ne_session_destroy(sess);

    //printf("remove %s success!\n",general_log);


    for(i=0;i<asus_cfg.dir_number;i++)
    {
        add_unfinished_list_to_file(i);

        queue_destroy(g_pSyncList[i]->SocketActionList);
        ne_session_destroy(asus_cfg.prule[i]->sess);

        //printf("clean %d sess success!\n",i);

        //printf("the pointer g_pSyncList[i]->ServerRootNode = %p\n",g_pSyncList[i]->ServerRootNode);
        if(g_pSyncList[i]->ServerRootNode == g_pSyncList[i]->OldServerRootNode)
        {
            //printf("the same Pointer!\n");
            if(g_pSyncList[i]->ServerRootNode != NULL)
                free_server_tree(g_pSyncList[i]->ServerRootNode);
        }
        else
        {
            if(g_pSyncList[i]->ServerRootNode != NULL)
                free_server_tree(g_pSyncList[i]->ServerRootNode);

            //printf("clean %d ServerRootNode success!\n",i);

            if(g_pSyncList[i]->OldServerRootNode != NULL)
                free_server_tree(g_pSyncList[i]->OldServerRootNode);

            //printf("clean %d OldServerRootNode success!\n",i);
        }

        free_action_item(g_pSyncList[i]->server_action_list);
        free_action_item(g_pSyncList[i]->dragfolder_action_list);
        free_action_item(g_pSyncList[i]->unfinished_list);
        free_action_item(g_pSyncList[i]->copy_file_list);
        free_action_item(g_pSyncList[i]->up_space_not_enough_list);
        //free(g_pSyncList[i]->up_item_file);
        //free(g_pSyncList[i]->down_item_file);
        if(asus_cfg.prule[i]->rule == 1)
        {
            free_action_item(g_pSyncList[i]->download_only_socket_head);
        }
        //printf("clean %d up_space_not_enough_list success!\n",i);

    }


    /*FILE *fp ;
    fp = fopen(all_local_item_file,"w");
    if( NULL == fp)
    {
        printf("create %s file error\n",all_local_item_file);
        return;
    }
    fclose(fp);
    fp = fopen(all_local_item_folder,"w");
    if( NULL == fp)
    {
    	printf("create %s file error\n",all_local_item_folder);
        return;
    }
    fclose(fp);


    print_all_local_item(all_local_item_file,0);
    print_all_local_item(all_local_item_folder,1);*/

    //free_LocalFolder_item(LocalFolderList);
    //free_LocalFile_item(LocalFileList);




    //printf("\n\n\n");


#if 0
    if(LocalRootNode != NULL)
        free_tree_node(LocalRootNode);
    else
        printf("LocalRootNode is NULL!\n");
#endif

    /*  CloudFile *de_filecurrent;
    de_filecurrent = FileList->next;
    while(de_filecurrent != NULL)
    {
        printf("serverfile->href = %s\n",de_filecurrent->href);
        de_filecurrent = de_filecurrent->next;
    }
    printf("\n\n\n");*/

    //if(FolderList != NULL)
    //free_CloudFile_item(FolderList);
    //printf("\n\n\n");

    //if(FileList != NULL)
    //free_CloudFile_item(FileList);
    //printf("\n\n\n");

    //if(OldFolderList != NULL)
    //free_CloudFile_item(OldFolderList);

    //if(OldFileList != NULL)
    //free_CloudFile_item(OldFileList);


    //pthread_mutex_destroy(&mutex);

    printf("clean up end !!!\n");
}

#ifdef NVRAM_
int convert_nvram_to_file(char *file)
{
    FILE *fp;
    char *nv, *nvp, *b;
    char *type, *url ,*user,*pwd,*s_path,*rule, *enable;
    int len;

    fp=fopen(file, "w");

    if (fp==NULL) return -1;

    nv = nvp = strdup(nvram_safe_get("Wclient_sync"));

    if(nv) {
        while ((b = strsep(&nvp, "<")) != NULL) {
            if((vstrsep(b, ">", &type, &user,&pwd,&url,&rule,&s_path,&enable)!=7)) continue;
            if(strlen(user)==0||strlen(pwd)==0) continue;
            printf("%s,%s,%s,%s,%s,%s,%s\n",type,enable,user,pwd,url,rule,s_path);
            len = strlen(s_path);
            if(s_path[len-1] == '/')
                s_path[len-1] = '\0';
            fprintf(fp, "%s\n%s\n%s\n%s\n%s\n%s\n%s\n",type,enable,user,pwd,url,rule,s_path);
        }
        free(nv);
    }
    fclose(fp);
}
#endif

#ifdef NVRAM_
/*Type>Desc>URL>Rule>capacha>LocalFolder*/
int convert_nvram_to_file_mutidir(char *file,struct asus_config *config)
{
    printf("enter convert_nvram_to_file_mutidir function\n");
    FILE *fp;
    char *nv, *nvp, *b;
    //struct asus_config config;
    int i;
    int j = 0;
    //int status;
    char *p;
    char *buffer;
    char *buf;

    fp=fopen(file, "w");

    if (fp==NULL) return -1;

    nv = nvp = strdup(nvram_safe_get("cloud_sync"));

    //printf("otain nvram end\n");

    if(nv)
    {
        while ((b = strsep(&nvp, "<")) != NULL)
        {
            i = 0;
            buf = buffer = strdup(b);
            printf("buffer = %s\n",buffer);
            while((p = strsep(&buffer,">")) != NULL)
            {
                printf("p = %s\n",p);
                if (*p == 0)
                {
                    fprintf(fp,"\n");
                    i++;
                    continue;
                }
                if(i == 0)
                {
                    if(atoi(p) != 1)
                        break;
                    if(j > 0)
                    {
                        fprintf(fp,"\n%s",p);
                    }
                    else
                    {
                        fprintf(fp,"%s",p);
                    }
                    j++;
                }
                else
                {
                    fprintf(fp,"\n%s",p);
                }
                i++;
            }
            free(buf);
            //status = parse_nvram(b,&config);
            /*if(status == -1)
            {
                printf("%s:%d parse nvram fail\n",__FILE__,__LINE__);
                return -1;
            }
            else if(config.type != 0)
                continue;*/
            /*else
            {
                fprintf(fp, "%d,%s,%s,%s,%d,%d,%d,",
                        config.type,config.user,config.pwd,config.url,config.enable,
                        config.ismuti,config.dir_number);
                for(i=0;i<config.dir_number;i++)
                {
                    if( i == config.dir_number - 1)
                       fprintf(fp,"%d,%s",config.prule[i]->rule,config.prule[i]->path);
                    else
                       fprintf(fp,"%d,%s,",config.prule[i]->rule,config.prule[i]->path);
                }

                fprintf(fp,"\n");

                for(i=0;i<config.dir_number;i++)
                {
                    my_free(config.prule[i]);
                }
                my_free(config.prule);

                break;
            }*/

        }
        printf("j = %d\n",j);
        free(nv);
        config->dir_number = j;
    }
    else
        printf("get nvram fail\n");
    fclose(fp);

    printf("end convert_nvram_to_file_mutidir function\n");

    return 0;
}
#else
int get_dir_number(char *file,struct asus_config *config)
{
    int i = 0;
    FILE *fp;
    char buffer[256];

    if(access(file,F_OK) == 0)
    {
        if((fp = fopen(file,"rb")) == NULL)
        {
            fprintf(stderr,"read Cloud error");
            return -1;
        }
        while(fgets(buffer,255,fp) != NULL)
        {
            i++;
        }
        fclose(fp);
        config->dir_number = i/6;
        return 0;
    }
    return -1;
}
#endif

void read_config()
{
    int i = 0;
    printf("####read_config####\n");
#ifdef NVRAM_
    /*#ifndef MUTI_DIR
    if(convert_nvram_to_file(CONFIG_PATH) == -1)
    {
        printf("convert_nvram_to_file fail\n");
        return;
    }

#else*/
    if(convert_nvram_to_file_mutidir(CONFIG_PATH,&asus_cfg) == -1)
    {
        printf("convert_nvram_to_file fail\n");
        return;
    }
#else
    if(get_dir_number(CONFIG_PATH,&asus_cfg) == -1)
    {
        printf("get_dir_number fail\n");
        return;
    }
    //#endif
#endif
    //#ifdef MUTI_DIR
    //#ifndef NVRAM_
    //asus_cfg.dir_number = 3;    //test for PC
    //get_dir_number();
    //#endif
    parse_config_mutidir(CONFIG_PATH,&asus_cfg);
    //exit(0);
    /*#else
    parse_config(CONFIG_PATH);
#endif*/
    //if( strlen(wd_username) == 0 )
    while(i < asus_cfg.dir_number)
    {
        if( strlen(asus_cfg.prule[i]->rooturl) == 0 )
        {
#if DEBUG
            //printf("username is blank ,please input your username and passwrod\n");
            printf("Read config error!\n");
            write_log(S_ERROR,"Read config error!","",i);
#endif
            no_config = 1;
            break;
        }
        else
        {
            no_config = 0;
        }
        i++;
    }


    while(no_config == 1 && exit_loop != 1 )
    {
        //sleep(1);
        usleep(1000*100);
    }

    if(!no_config)
    {
        //no_config = 0 ;
        exit_loop = 0;
    }

}

void send_to_inotify(){

    int i;

    for(i=0;i<asus_cfg.dir_number;i++)
    {
        printf("send_action base_path = %s\n",asus_cfg.prule[i]->base_path);
        send_action(1,asus_cfg.prule[i]->base_path);
        usleep(1000*10);
    }
}

void run(){
    int status;
    int i;
    int need_server_thid = 0;
    int create_thid1 = 0;
    int create_thid2 = 0;
    int create_thid3 = 0;
    //down_head = NULL;
    /*up_head = (struct sync_item *)malloc(sizeof(struct sync_item));
    memset(up_head,'\0',sizeof(struct sync_item));
    up_head->next = NULL;
    mkcol_head = (struct sync_item *)malloc(sizeof(struct sync_item));
    memset(mkcol_head,'\0',sizeof(struct sync_item));
    mkcol_head->next = NULL;
    mkcol_parent_head = (struct sync_item *)malloc(sizeof(struct sync_item));
    memset(mkcol_parent_head,'\0',sizeof(struct sync_item));
    mkcol_parent_head->next = NULL;*/
    /*LocalActionList = (Local_Action *)malloc(sizeof(Local_Action));
    memset(LocalActionList,'\0',sizeof(Local_Action));
    LocalActionList->next = NULL;*/

    //my_base64_encode();
    for(i=0;i<asus_cfg.dir_number;i++)
    {
        status = wd_create_session(i);
    }

    init_global_var();
#if LIST_TEST
    /*do{
        ServerRootNode = create_server_treeroot();
        status = browse_to_tree(ROOTURL,ServerRootNode);
        //SearchServerTree(ServerRootNode);
        sleep(2);
        if(status != NE_OK)
            sleep(5);
    }while(status != NE_OK && exit_loop == 0);*/
#endif
    /*do{
        status = getServerList();
        if(status != NE_OK)
            sleep(5);
    }while(status != NE_OK && exit_loop == 0);*/

    //if(sync_down == 1)
    handle_quit_upload();

    if(exit_loop == 0)
    {

        //OldFolderList = FolderList;
        //OldFileList = FileList;


        //printf("up_item_file = %s\n",up_item_file);

        /*if(access(up_item_file,0) == 0)
		{
			parse_trans_item(up_item_file,UPLOAD);
			remove(up_item_file);
		}*/

        //status = getLocalList();  //delete data:2012-07-11.Reason:wd_initial() changed

#if 0
        LocalRootNode = create_tree_rootnode(base_path);
        FindDir(LocalRootNode,base_path);
        SearchTree(LocalRootNode);

        if(status != 0)
        {
            printf("get Local List ERROR! \n");
        }
#endif
        send_to_inotify();

        if(exit_loop == 0)
        {
            printf("create newthid2\n");
            if( pthread_create(&newthid2,NULL,(void *)SyncLocal,NULL) != 0)
            {
                printf("thread creation failder\n");
                exit(1);
            }
            usleep(1000*500);
            create_thid2 = 1;
        }

        if(exit_loop == 0)
        {
            printf("create newthid3\n");
            if( pthread_create(&newthid3,NULL,(void *)Socket_Parser,NULL) != 0)
            {
                printf("thread creation failder\n");
                exit(-1);
            }
            usleep(1000*500);
            create_thid3 = 1;
        }




        //write_log(S_INITIAL,"","");

        //finished_initial = 0;
        //server_sync = 1;
        status = wd_initial();          //used for init
        //printf("###########finished initial\n");

        //delete data:2012-07-11.Reason:wd_initial() changed
        /*free_LocalFolder_item(LocalFolderList);
        free_LocalFile_item(LocalFileList);
        free_LocalFile_item(SavedLocalFileList);*/
        //server_sync = 0;
#if 0
        free_tree_node(LocalRootNode);
        LocalRootNode = NULL;
#endif
        //getLocalList();
#if 0
        LocalRootNode = create_tree_rootnode(base_path);
        FindDir(LocalRootNode,base_path);
        SearchTree(LocalRootNode);
#endif
        finished_initial = 1;
        //write_log(S_SYNC,"","");

        /*if(upload_only == 1)     //upload only 规则时，处理未完成的上传动作
        {
            action_item *p;
            p = unfinished_list->next;
            while(p != NULL && exit_loop == 0)
            {
                do_unfinished();
                p = unfinished_list->next;
                sleep(2);
            }
        }*/

#if LIST_TEST
        /*int num;
        for(num=0;num<asus_cfg.dir_number;num++)
        {
            g_pSyncList[num]->OldServerRootNode = g_pSyncList[num]->ServerRootNode;
        }*/
#endif

        //sync_down = 1;


        need_server_thid = get_create_threads_state();

        if(need_server_thid && exit_loop == 0)
        {
            printf("create newthid1\n");
            if( pthread_create(&newthid1,NULL,(void *)SyncServer,NULL) != 0)
            {
                printf("thread creation failder\n");
                exit(1);
            }
            create_thid1 = 1;
        }
        else
        {
            server_sync = 0;
        }

        if(create_thid1)
        {
            /*pthread_mutex_lock(&mutex);
            pthread_cond_signal(&cond);
            pthread_mutex_unlock(&mutex);*/

            pthread_join(newthid1,NULL);
            printf("newthid1 has stoped!\n");
        }

        //pthread_cond_destroy(&cond);
        if(create_thid3)
        {
            pthread_join(newthid3,NULL);
            printf("newthid3 has stoped!\n");
        }

        if(create_thid2)
        {
            pthread_join(newthid2,NULL);
            printf("newthid2 has stoped!\n");
        }



        //free_CloudFile_item(FolderList);
        //free_CloudFile_item(FileList);
    }

    //sleep(2);
    usleep(1000);
    clean_up();

    if(stop_progress != 1)
    {
        exit_loop = 0;
        read_config();
        run();
    }
}

int main(int argc, char *argv[]){
#if 0
    setenv("MALLOC_TRACE","memlog",1);
    mtrace();
#endif
    exit_loop = 0;
    stop_progress = 0;

    sigset_t bset,oset;
    pthread_t sig_thread;

    sigemptyset(&bset);
    sigaddset(&bset,SIGUSR1);
    sigaddset(&bset,SIGUSR2);
    sigaddset(&bset,SIGTERM);

    if( pthread_sigmask(SIG_BLOCK,&bset,&oset) == -1)
        printf("!! Set pthread mask failed\n");

    if( pthread_create(&sig_thread,NULL,(void *)sigmgr_thread,NULL) != 0)
    {
        printf("thread creation failder\n");
        exit(-1);
    }


    clear_global_var();
    //sleep(40);

#ifndef NVRAM_
    strcpy(log_base_path,"/home/alan/tool");   //just for test for pc
    //strcpy(log_base_path,argv[1]);
    //strcpy(log_base_path,"/tmp");               //for N16
#else
    strcpy(log_base_path,"/tmp");
#endif
    //rootfolder_len = strlen(ROOTFOLDER);
    sprintf(log_path,"%s/Cloud/log",log_base_path);
    //sprintf(temp_path,"%s/Cloud/temp",mount_path);
    sprintf(general_log,"%s/WebDAV",log_path);
    //my_mkdir_r(temp_path);
    my_mkdir_r(log_path);
    //printf("log_path = %s\ntemp_path = %s\ngeneral_log = %s\n",log_path,temp_path,general_log);
    printf("log_path = %s\ngeneral_log = %s\n",log_path,general_log);
    //printf("mount_path = %s\n",mount_path);

    //signal(SIGUSR1,sig_handler); //add    user
    //signal(SIGUSR2,sig_handler); //modify user
    //signal(SIGPIPE,sig_handler); //delete user
    //signal(SIGINT, sig_handler);

    read_config();
    //printf("HOST = %s\n",HOST);

    if(no_config == 0)
        run();

    printf("stop WebDAV end\n");

    return 1;
}

