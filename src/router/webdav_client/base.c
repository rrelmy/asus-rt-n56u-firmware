#include "base.h"

char *my_str_malloc(size_t len){

    //printf("len = %d\n",len);

    char *s;
    s = (char *)malloc(sizeof(char)*len);
    if(s == NULL)
    {
        printf("Out of memory.\n");
        exit(1);
    }

    memset(s,'\0',sizeof(s));
    return s;
}

static void *xmalloc_fatal(size_t size) {
    if (size==0) return NULL;
    fprintf(stderr, "Out of memory.");
    exit(1);
}

void *xmalloc (size_t size) {
    void *ptr = malloc (size);
    if (ptr == NULL) return xmalloc_fatal(size);
    return ptr;
}

void *xrealloc (void *ptr, size_t size) {
    void *p = realloc (ptr, size);
    if (p == NULL) return xmalloc_fatal(size);
    return p;
}

char *xstrdup (const char *s) {
    void *ptr = xmalloc(strlen(s)+1);
    strcpy (ptr, s);
    return (char*) ptr;
}

/**
 * Escape 'string' according to RFC3986 and
 * http://oauth.net/core/1.0/#encoding_parameters.
 *
 * @param string The data to be encoded
 * @return encoded string otherwise NULL
 * The caller must free the returned string.
 */
char *oauth_url_escape(const char *string) {
    size_t alloc, newlen;
    char *ns = NULL, *testing_ptr = NULL;
    unsigned char in;
    size_t strindex=0;
    size_t length;

    if (!string) return xstrdup("");

    alloc = strlen(string)+1;
    newlen = alloc;

    ns = (char*) xmalloc(alloc);

    length = alloc-1;
    while(length--) {
        in = *string;

        switch(in){
        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9':
        case 'a': case 'b': case 'c': case 'd': case 'e':
        case 'f': case 'g': case 'h': case 'i': case 'j':
        case 'k': case 'l': case 'm': case 'n': case 'o':
        case 'p': case 'q': case 'r': case 's': case 't':
        case 'u': case 'v': case 'w': case 'x': case 'y': case 'z':
        case 'A': case 'B': case 'C': case 'D': case 'E':
        case 'F': case 'G': case 'H': case 'I': case 'J':
        case 'K': case 'L': case 'M': case 'N': case 'O':
        case 'P': case 'Q': case 'R': case 'S': case 'T':
        case 'U': case 'V': case 'W': case 'X': case 'Y': case 'Z':
        case '_': case '~': case '.': case '-':
            ns[strindex++]=in;
            break;
        default:
            newlen += 2; /* this'll become a %XX */
            if(newlen > alloc) {
                alloc *= 2;
                testing_ptr = (char*) xrealloc(ns, alloc);
                ns = testing_ptr;
            }
            //snprintf(&ns[strindex], 4, "%%%02X", in);
            snprintf(&ns[strindex], 4, "%%%02x", in);
            strindex+=3;
            break;
        }
        string++;
    }
    ns[strindex]=0;
    return ns;
}

#ifndef ISXDIGIT
# define ISXDIGIT(x) (isxdigit((int) ((unsigned char)x)))
#endif

/**
 * Parse RFC3986 encoded 'string' back to  unescaped version.
 *
 * @param string The data to be unescaped
 * @param olen unless NULL the length of the returned string is stored there.
 * @return decoded string or NULL
 * The caller must free the returned string.
 */
char *oauth_url_unescape(const char *string, size_t *olen) {
    size_t alloc, strindex=0;
    char *ns = NULL;
    unsigned char in;
    long hex;

    if (!string) return NULL;
    alloc = strlen(string)+1;
    ns = (char*) xmalloc(alloc);

    while(--alloc > 0) {
        in = *string;
        if(('%' == in) && ISXDIGIT(string[1]) && ISXDIGIT(string[2])) {
            char hexstr[3]; // '%XX'
            hexstr[0] = string[1];
            hexstr[1] = string[2];
            hexstr[2] = 0;
            hex = strtol(hexstr, NULL, 16);
            in = (unsigned char)hex; /* hex is always < 256 */
            string+=2;
            alloc-=2;
        }
        ns[strindex++] = in;
        string++;
    }
    ns[strindex]=0;
    if(olen) *olen = strindex;
    return ns;
}


/*void clean_fun_session(void *arg){
	ne_session_destroy(arg);
}

void clean_fun_fd(int *arg){
	close(*arg);
}*/

/*
int my_auth(void *userdata, const char *realm, int attempt,char *username, char *password){
    strncpy(username,wd_username,NE_ABUFSIZ);
    strncpy(password,wd_password,NE_ABUFSIZ);
    return attempt;
}*/

static void s_progress(void *userdata, ne_off_t prog, ne_off_t total){

    //printf("prog = %ld\n",prog);
    //printf("total = %ld\n",total);

    if(prog_total == 0)
    {
        time_t t;
        t = time(NULL);
        printf("progress1 = %.2f\n",(float)(1));
        prog_time = t;
    }
#if 0
    if(prog_total != -1 && prog_total != 0)
    {
        time_t t;
        t = time(NULL);
        if((t-prog_time >= 5) || total == prog_total)
        {
            //printf("time t = %ld,prog_time = %ld\n",t,prog_time);
            //printf("total = %ld,prog_total=%ld\n",total,prog_total);
            printf("progress2 = %.2f\n",(float)total/prog_total);
            prog_time = t;
        }
    }
#else
    if(prog_total != -1 && prog_total != 0)
    {
        time_t t;
        t = time(NULL);
        if((t-prog_time >= 5) || prog == prog_total)
        {
            //printf("time t = %ld,prog_time = %ld\n",t,prog_time);
            //printf("total = %ld,prog_total=%ld\n",total,prog_total);
            printf("progress2 = %.2f\n",(float)prog/prog_total);
            prog_time = t;
        }
    }
#endif

    /*if(exit_loop == 1){
		pthread_exit(0);
		pthread_cancel(newthid2);
		pthread_cancel(newthid1);
	}*/
}

#if 0
void my_base64_encode(){

    char *tmp_auth;
    char *name_pw;
    //char *return_auth;

    name_pw = (char *)malloc(sizeof(char)*128);
    //tmp_auth = (char *)malloc(sizeof(char)*128);
    memset(name_pw,'\0',sizeof(name_pw));
    //memset(tmp_auth,'\0',sizeof(tmp_auth));
    sprintf(name_pw,"%s:%s",wd_username,wd_password);

    tmp_auth = ne_base64(name_pw,strlen(name_pw));

    //return_auth = (char *)malloc(sizeof(char)*(strlen(tmp_auth)+7));
    //memset(return_auth,'\0',sizeof(return_auth));

    sprintf(base64_auth,"Basic %s",tmp_auth);

    free(name_pw);
    free(tmp_auth);

    printf("\nAuthorization:%s\n",base64_auth);

    //return base64_auth;
}
#endif

static int my_verify(void *userdata,int failures,const ne_ssl_certificate *cert)
{
    return 0;
}

/*Used for create Session*/
int wd_create_session(int j){
    if (ne_sock_init())
    {
        printf(" Socket library initalization failed.\n");
        return -1;
    }
    ne_uri uri = {0};

    if (ne_uri_parse(asus_cfg.prule[j]->host, &uri) || uri.host==NULL || uri.path==NULL)
    {
        printf("Could not parse url %s/n", asus_cfg.prule[j]->host);
        return -1;
    }
    asus_cfg.prule[j]->sess = ne_session_create(uri.scheme, uri.host, uri.port);

    if(!strcmp(uri.scheme,"https"))
    {
        printf("https session\n");
        ne_ssl_set_verify(asus_cfg.prule[j]->sess,my_verify,uri.host);
    }
    //ne_set_server_auth(sess,my_auth,NULL);

    ne_set_connect_timeout(asus_cfg.prule[j]->sess,30);
    ne_set_read_timeout(asus_cfg.prule[j]->sess,30);
    ne_set_progress(asus_cfg.prule[j]->sess, s_progress, NULL);

    ne_uri_free(&uri);

    return 0;
}

int is_local_space_enough(CloudFile *do_file,int index){

    //printf("************is_local_space_enough start*************\n");

    long long int freespace;
    freespace = get_local_freespace(index);
    printf("do_file->getcontentlength = %s\n",do_file->getcontentlength);
    printf("freespace = %lld,do_file->getcontentlength = %lld\n",freespace,atoll(do_file->getcontentlength));
    if(freespace <= atoll(do_file->getcontentlength)){
        printf("local freespace is not enough!\n");
        return 0;
    }
    else
    {
        //prog_total = atoll(do_file->getcontentlength);
        printf("local freespace is enough!\n");
        return 1;
    }
}

int is_server_space_enough(const char *localfilepath,int index){
    //return 1;   //for test

    int status;
    long long int filesize;
    status = ne_getrouterinfo(parseRouterInfo,index);

    filesize = (long long int)stat_file(localfilepath);

    printf("DiskAvailable = %lld,filesize = %lld\n",DiskAvailable,filesize);

    if(status == 0)
    {
        if(DiskAvailable > filesize)
        {
            printf("server freespace is enough!\n");
            return 1;
        }
        else
        {
            printf("server freespace is not enough!\n");
            return 0;
        }
    }
    return status;

}

int get_filename_length(char *filepath){

    const char split = '/';
    char *p;
    int len;

    p = strrchr(filepath,split);

    if(p)
    {
        p++;
        len = strlen(p);
        return len;
    }

    return 0;
}

char *parse_name_from_path(const char *path)
{
    char *name;
    char *p;

    name = (char *)malloc(sizeof(char)*512);
    memset(name,0,sizeof(char)*512);

    p = strrchr(path,'/');

    if( p == NULL)
    {
        free(name);
        return NULL;
    }

    p++;

    strcpy(name,p);

    return name;
}

char *change_local_same_name(char *fullname)
{
    int i = 1;
    char *temp_name = NULL;
    //char *temp_suffix = ".asus.td";
    int len = 0;
    char *path;
    char newfilename[256];

    char *fullname_tmp = NULL;
    fullname_tmp = my_str_malloc(strlen(fullname)+1);
    sprintf(fullname_tmp,"%s",fullname);

    char *filename = parse_name_from_path(fullname_tmp);
    len = strlen(filename);
    printf("filename len is %d\n",len);
    //my_free(filename);
    path = my_str_malloc((size_t)(strlen(fullname)-len+1));
    printf("fullname = %s\n",fullname);
    //strncpy(path,fullname,strlen(fullname)-len-1);
    snprintf(path,strlen(fullname)-len+1,"%s",fullname);
    printf("path = %s\n",path);
    //memset(path,'\0',sizeof(path));
    free(fullname_tmp);
    /*if(access(filepath,F_OK) != 0)
    {
        printf("Local has no %s\n",filepath);
        return LOCAL_FILE_LOST;
    }*/

    /*do{
        int n = i;
        int j = 0;
        while(n=(n/10))
        {
            j++;
        }
        memset(newfilename,'\0',sizeof(newfilename));
        snprintf(newfilename,252-j,"%s",filename);
        sprintf(newfilename,"%s(%d)",newfilename,i);
        printf("newfilename = %s\n",newfilename);
        i++;
    }while(access(newfilename,F_OK) == 0);*/

    while(1)
    {
        int n = i;
        int j = 0;
        while((n=(n/10)))
        {
            j++;
        }
        memset(newfilename,'\0',sizeof(newfilename));
        snprintf(newfilename,252-j,"%s",filename);
        sprintf(newfilename,"%s(%d)",newfilename,i);
        printf("newfilename = %s\n",newfilename);
        i++;

        temp_name = my_str_malloc((size_t)(strlen(path)+strlen(newfilename)+1));
        sprintf(temp_name,"%s%s",path,newfilename);

        if(access(temp_name,F_OK) != 0)
            break;
        else
            free(temp_name);
    }


    //temp_name = my_str_malloc((size_t)(strlen(path)+strlen("/")+strlen(newfilename)+1));
    //sprintf(temp_name,"%s/%s",path,newfilename);
    /*if(len > 252)
    {
        strncpy(path,fullname,strlen(fullname)-len-1);
        strncpy(newfilename,filename,247);
        //printf("newfilename len is %d\n",strlen(newfilename));
        temp_name = (char *)malloc(sizeof(char)*(strlen(path)+strlen("/")+
                                                 strlen(newfilename)+strlen(temp_suffix)+1));
        memset(temp_name,0,sizeof(temp_name));
        sprintf(temp_name,"%s/%s%s",path,newfilename,temp_suffix);
    }
    else
    {
        temp_name = (char *)malloc(sizeof(char)*(strlen(fullname)+strlen(temp_suffix)+1));
        memset(temp_name,0,sizeof(temp_name));
        sprintf(temp_name,"%s%s",fullname,temp_suffix);
    }*/

    free(path);
    free(filename);
    return temp_name;
}

char *change_server_same_name(char *fullname,int index){

    int i = 1;
    int exist;
    char *filename = NULL;
    char *temp_name = NULL;
    //char *temp_suffix = ".asus.td";
    int len = 0;
    char *path;
    char newfilename[512];
    int exit = 1;

    char *fullname_tmp = NULL;
    fullname_tmp = my_str_malloc(strlen(fullname)+1);
    sprintf(fullname_tmp,"%s",fullname);


    filename = parse_name_from_path(fullname_tmp);
    len = strlen(filename);
    //len = 6;
    printf("filename len is %d\n",len);
    path = my_str_malloc((size_t)(strlen(fullname)-len+1));
    printf("fullname = %s\n",fullname);
    snprintf(path,strlen(fullname)-len+1,"%s",fullname);
    printf("path = %s\n",path);

    free(fullname_tmp);

    while(exit)
    {
        int n = i;
        int j = 0;
        while((n=(n/10)))
        {
            j++;
        }
        memset(newfilename,'\0',sizeof(newfilename));
        snprintf(newfilename,252-j,"%s",filename);
        sprintf(newfilename,"%s(%d)",newfilename,i);
        printf("newfilename = %s\n",newfilename);
        i++;

        temp_name = my_str_malloc((size_t)(strlen(path)+strlen(newfilename)+1));
        sprintf(temp_name,"%s%s",path,newfilename);

        //char *serverpath;
        //serverpath = localpath_to_serverpath(temp_name);

        //do{
        printf("temp_name = %s\n",temp_name);

        exist = is_server_exist(path,temp_name,index);
        //}while(exist == -2);

        if(exist)
        {
            free(temp_name);
        }
        else
        {
            exit = 0;
        }

    }

    free(path);
    free(filename);
    return temp_name;

}

#if 0
char *change_server_same_name_root(char *filename){

    int i = 1;
    int exist;
    char *temp_name = NULL;
    int len = 0;
    char newfilename[512];
    char *newhref;
    int exit = 1;

    len = strlen(filename);

    while(exit)
    {
        int n = i;
        int j = 0;
        while((n=(n/10)))
        {
            j++;
        }
        memset(newfilename,'\0',sizeof(newfilename));
        snprintf(newfilename,252-j,"%s",filename);
        sprintf(newfilename,"%s(%d)",newfilename,i);
        printf("newfilename = %s\n",newfilename);
        i++;

        temp_name = oauth_url_escape(newfilename);
        newhref = my_str_malloc(strlen(temp_name)+strlen(ROOTURL)+2);
        sprintf(newhref,"%s/%s",ROOTURL,temp_name);

        //sprintf(temp_name,"%s%s",path,newfilename);

        //char *serverpath;
        //serverpath = localpath_to_serverpath(temp_name);

        //do{
        //printf("temp_name = %s\n",temp_name);

        exist = is_server_exist_with_type(newhref);
        //}while(exist == -2);

        if(exist)
        {
            free(temp_name);
            free(newhref);
        }
        else
        {
            exit = 0;
        }

    }

    char *return_href;
    return_href = my_str_malloc(strlen(temp_name)+strlen(ROOTFOLDER)+2);
    sprintf(return_href,"%s/%s",ROOTFOLDER,temp_name);
    free(temp_name);
    free(newhref);
    return return_href;

}
#endif

char *get_temp_name(char *fullname)
{
    char *temp_name = NULL;
    char *temp_suffix = ".asus.td";
    int len = 0;
    char *path;
    char newfilename[256];

    memset(newfilename,0,sizeof(newfilename));
    char *filename = parse_name_from_path(fullname);
    len = strlen(filename);
    printf("filename len is %d\n",len);
    //my_free(filename);
    path = my_str_malloc((size_t)(strlen(fullname)-len));
    //memset(path,0,sizeof(path));
    if(len > 247)
    {
        strncpy(path,fullname,strlen(fullname)-len-1);
        strncpy(newfilename,filename,247);
        //printf("newfilename len is %d\n",strlen(newfilename));
        temp_name = (char *)malloc(sizeof(char)*(strlen(path)+strlen("/")+
                                                 strlen(newfilename)+strlen(temp_suffix)+1));
        memset(temp_name,0,sizeof(temp_name));
        sprintf(temp_name,"%s/%s%s",path,newfilename,temp_suffix);
    }
    else
    {
        temp_name = (char *)malloc(sizeof(char)*(strlen(fullname)+strlen(temp_suffix)+1));
        memset(temp_name,0,sizeof(temp_name));
        sprintf(temp_name,"%s%s",fullname,temp_suffix);
    }

    free(path);
    free(filename);
    return temp_name;
}

int Download(char *dofile_href,int index){
    printf("**********Download****************\n");
    char *LocalFilePath;
    char *LocalFilePath_temp;
    char *path;
    //int len;
    //long long int freespace;
    path = strstr(dofile_href,asus_cfg.prule[index]->rootfolder) + asus_cfg.prule[index]->rootfolder_length;
    path = oauth_url_unescape(path,NULL);
    LocalFilePath = my_str_malloc((size_t)(strlen(path)+asus_cfg.prule[index]->base_path_len+1));
    sprintf(LocalFilePath,"%s%s",asus_cfg.prule[index]->base_path,path);
    LocalFilePath_temp = get_temp_name(LocalFilePath);
    printf("LocalFilePath_temp = %s\n",LocalFilePath_temp);
    //sprintf(LocalFilePath_temp,"%s.asus.td",LocalFilePath);
    free(path);
#if 0
    freespace = get_local_freespace();
    printf("freespace = %lld,do_file->getcontentlength = %d\n",freespace,atoi(do_file->getcontentlength));
    if(freespace <= atoi(do_file->getcontentlength)){
        printf("freespace is not enough!\n");
        return -1;
    }
    else if(freespace > atoi(do_file->getcontentlength)){
        printf("freespace is enough!\n");
    }
#endif

    //ne_request *req;
    //req = ne_request_create(sess,"PUT","/usbdisk/asd/6.txt");

    if(access(LocalFilePath,F_OK) == 0)
    {
        printf("Local has %s\n",LocalFilePath);
        unlink(LocalFilePath);
        add_action_item("remove",LocalFilePath,g_pSyncList[index]->server_action_list);
    }

    //len = get_filename_length(LocalFilePath);
    char *serverPath;
    int cp = 0;
    serverPath = strstr(dofile_href,asus_cfg.prule[index]->rootfolder);
    printf("serverPath = %s\n",serverPath);

    do{
        cp = is_Server_Copying(serverPath,index);
    }while(cp == 1);

    if(cp != 0)
    {
        free(LocalFilePath);
        free(LocalFilePath_temp);
        if(cp == -1)
            return SERVER_FILE_DELETED;
        return cp;
    }

    int ret,fd;
    //if(len <= 247)
    //{
    //FILE *fp = fopen(LocalFilePath_temp,"w");
    //fclose(fp);
    fd = open(LocalFilePath_temp,O_WRONLY | O_CREAT,S_IRWXU | S_IRWXG | S_IRWXO);
    //}
    //else
    //{
    //   FILE *fp = fopen(LocalFilePath,"w");
    //   fclose(fp);
    //   fd = open(LocalFilePath,O_WRONLY);
    // }

    //pthread_cleanup_push(clean_fun_fd,(int *)&fd);
    //if(finished_initial)
    write_log(S_DOWNLOAD,"",LocalFilePath,index);
    ret = ne_get(asus_cfg.prule[index]->sess,serverPath,fd);

    //pthread_cleanup_pop(0);
    prog_total = -1;

    if(ret != 0){
        printf("get file error!\n");
        //remove_tmp(dofile_href);           // deleted temporary
        char error_info[100];
        memset(error_info,0,sizeof(error_info));
        sprintf(error_info,"%s",ne_get_error(asus_cfg.prule[index]->sess));
        printf("%s\n",error_info);
        //ne_sock_exit();
        close(fd);
        free(LocalFilePath);
        free(LocalFilePath_temp);

        if(strstr(error_info,"Could not connect to server") != NULL)
        {
            write_log(S_ERROR,"Could not connect to server!","",index);
            return COULD_NOT_CONNECNT_TO_SERVER;
            //return -1;
        }
        if(strstr(error_info,"Connection timed out") != NULL || strstr(error_info,"connection timed out") != NULL)
        {
            write_log(S_ERROR,"Could not connect to server!","",index);
            return CONNECNTION_TIMED_OUT;
            //return -1;
        }
        if(strstr(error_info,"Could not read response body") != NULL)
        {
            write_log(S_ERROR,"Could not read response body!","",index);
            return COULD_NOT_READ_RESPONSE_BODY;
        }


        return ret;
    }

    //ne_sock_exit();
    close(fd);

    //if(len <= 247)
    //{
    rename(LocalFilePath_temp,LocalFilePath);
    //}

    free(LocalFilePath);
    free(LocalFilePath_temp);

    if(finished_initial)
        write_log(S_SYNC,"","",index);
    else
        write_log(S_INITIAL,"","",index);
    return ret;
}

/*
void remove_tmp(char *dofile_href){
    printf("^^^^^^^^^^^^^^remove_tmp^^^^^^^^^^^\n");
    char *LocalFilePath;
    char *path;

    path = strstr(dofile_href,ROOTFOLDER) + rootfolder_len;
    path = oauth_url_unescape(path,NULL);

    LocalFilePath = (char *)malloc(sizeof(char)*(strlen(path)+base_path_len+1));

    memset(LocalFilePath,'\0',sizeof(LocalFilePath));
    sprintf(LocalFilePath,"%s%s",base_path,path);

    if(access(LocalFilePath,0) == 0)
        unlink(LocalFilePath);

    free(path);
    free(LocalFilePath);
}*/

int strnicmp(const char *s1, const char *s2, size_t len){

    /* Yes, Virginia, it had better be unsigned */

    unsigned char c1, c2;



    c1 = c2 = 0;

    if (len) {

        do {

            c1 = *s1;

            c2 = *s2;

            s1++;

            s2++;

            if (!c1)

                break;

            if (!c2)

                break;

            if (c1 == c2)

                continue;

            c1 = tolower(c1);

            c2 = tolower(c2);

            if (c1 != c2)

                break;

        } while (--len);

    }

    return (int)c1 - (int)c2;

}
#if 0
int is_have_parent(char *leafpath,char *serverpath){

    int have_parent,cmp;
    char *hreftmp;
    have_parent = 1;
    cmp = 0;
    CloudFile *foldertmp;

    char URL[512];
    char son_tmp[512];
    char localURL[512];
    char localson_tmp[512];
    memset(localURL,'\0',sizeof(localURL));
    memset(URL,'\0',sizeof(URL));
    memset(son_tmp,'\0',sizeof(son_tmp));
    memset(localson_tmp,'\0',sizeof(localson_tmp));
    sprintf(son_tmp,"%s",serverpath);
    sprintf(localson_tmp,"%s",leafpath);
    do
    {
        get_file_parent(son_tmp,URL);
        //printf("URL = %s\n",URL);

        foldertmp = OldFolderList->next;
        while (foldertmp->href != NULL)
        {
            hreftmp = foldertmp->href;
            //printf("foldertmp->href = %s\n",foldertmp->href);
            if ((cmp = strnicmp(URL,hreftmp,strlen(URL))) == 0){
                break;
            }
            else
                foldertmp = foldertmp->next;
        }
        printf("cmp = %d\n",cmp);
        if(cmp != 0)
        {
            printf("localson_tmp = %s\n",localson_tmp);
            get_file_parent_local(localson_tmp,localURL);
            printf("localURL = %s\n",localURL);
            add_sync_item(MKCOL_PARENT,localURL,mkcol_parent_head);
            have_parent = 0;
        }
        memset(son_tmp,'\0',sizeof(son_tmp));
        memset(localson_tmp,'\0',sizeof(localson_tmp));
        sprintf(son_tmp,"%s",URL);
        sprintf(localson_tmp,"%s",localURL);
        memset(localURL,'\0',sizeof(localURL));
        memset(URL,'\0',sizeof(URL));
    }while(cmp != 0);

    return have_parent;

}
#endif

/*upload the file*/
int auth_put(const char *uri, const char *buf,long int bufsize,long int start,long int stop,long int filesize,int index)
{
    ne_request *req;
    int ret;
    char range[128];
    sprintf(range,"bytes %ld-%ld/%ld",start,stop,filesize);

    req = ne_request_create(asus_cfg.prule[index]->sess,"PUT",uri);

    //if (get_bool_option (opt_expect100) != 0)
    //ne_set_request_flag(req, NE_REQFLAG_EXPECT100, 1);

    //ne_add_request_header(req,"Authorization",base64_auth);
    ne_add_request_header(req,"Content-Range",range);

    //ne_lock_using_resource(req, uri, 0);
    //ne_lock_using_parent(req, uri);

    ne_set_request_body_buffer(req, buf,bufsize);

    //printf("upload start---\n");
    ret = ne_request_dispatch(req);
    //printf("ret = %d\n",ret);

    if (ret == NE_OK && ne_get_status(req)->klass != 2)
        ret = NE_ERROR;

    ne_request_destroy(req);

    if(ret == NE_OK)
    {
        time_t t;
        t = time(NULL);
        if((t-prog_time >= 5) || ((stop+1) == filesize))
        {
            if(filesize == 0)
            {
                printf("progress = %.2f\n",(float)(1));
                prog_time = t;
            }
            else
            {
                //printf("total = %ld,prog_total=%ld\n",stop+1,filesize);
                printf("progress = %.2f\n",(float)(stop+1)/filesize);
                prog_time = t;
            }
        }
    }

    return ret;
}

/*cut the upload file*/
int my_put(char *filepath,char *serverpath,int index){
    int fd;
    FILE *fp;
    int ret;
    long int max_buf_size = 51200;
    long int file_size;
    long int left_file_size;
    long int range_start = 0;
    long int range_stop = 0;
    int i=0;
    char buffer[51200];

    fd = open(filepath,O_RDONLY);

    struct stat filestat;

    if(fstat(fd,&filestat)==-1)
    {
        return NE_ERROR;
    }

    file_size = filestat.st_size;
    left_file_size = filestat.st_size;

    close(fd);

    if((fp = fopen(filepath,"r")) == NULL)
    {
        printf("open file fail\n");
        return -1;
    }

    if(file_size <= max_buf_size)
    {
        range_stop = file_size -1;
        fread(buffer,5120,10,fp);
        ret = auth_put(serverpath,buffer,file_size,range_start,range_stop,file_size,index);
    }

    else
    {
        while(left_file_size > 0)
        {
            if(left_file_size <= max_buf_size)
            {
                range_start = max_buf_size*i;
                range_stop = file_size -1;
                fread(buffer,5120,10,fp);
                ret = auth_put(serverpath,buffer,left_file_size,range_start,range_stop,file_size,index);
                if(ret != 0)
                {
                    printf("Put file error!\n");
                    //close(fd);
                    fclose(fp);
                    return ret;
                }
                left_file_size = 0;
            }
            else
            {
                range_start = max_buf_size*i;
                range_stop = range_start + max_buf_size -1;
                left_file_size = file_size - range_stop - 1;
                fread(buffer,5120,10,fp);
                ret = auth_put(serverpath,buffer,max_buf_size,range_start,range_stop,file_size,index);
                ++i;
                if(ret != 0)
                {
                    printf("Put file error!\n");
                    //close(fd);
                    fclose(fp);
                    return ret;
                }
            }

        }
    }


    printf("ret = %d\n",ret);
    if(ret != 0)
    {
        printf("Put file error!\n");
        return ret;
    }

    fclose(fp);
    return ret;
}

int Upload(char *filepath,int index){
    printf("*****************Upload***************\n");
    int server_enough;
    if(access(filepath,F_OK) != 0)
    {
        printf("Local has no %s\n",filepath);
        return LOCAL_FILE_LOST;
    }

    server_enough = is_server_space_enough(filepath,index);
    if(server_enough != 1)
    {
        if(server_enough == 0)
        {
            write_log(S_ERROR,"server space is not enough!","",index);
            action_item *item;
            item = get_action_item("upload",filepath,g_pSyncList[index]->up_space_not_enough_list,index);
            if(item == NULL)
            {
                add_action_item("upload",filepath,g_pSyncList[index]->up_space_not_enough_list);
            }
            return SERVER_SPACE_NOT_ENOUGH;
        }
        else
        {
            return server_enough;
        }
    }

    char *localpath;
    localpath = strstr(filepath,asus_cfg.prule[index]->base_path) + asus_cfg.prule[index]->base_path_len;
#if 0
    char *hreftmp;
    int cmp,have_parent;
    CloudFile *filetmp;
    filetmp = OldFileList->next;
    while (filetmp->href != NULL){
        //printf("foldertmp->href = %s\n",foldertmp->href);
        hreftmp = strstr(filetmp->href,ROOTFOLDER) + rootfolder_len;
        hreftmp = oauth_url_unescape(hreftmp,NULL);
        //printf("foldertmp->href = %s\n",foldertmp->href);
        if ((cmp = strcmp(hreftmp,localpath)) == 0){
            if((cmp = compareTime(filetmp->modtime,filepath)) == 2){
                printf("%s has existed\n",localpath);
                return -1;
            }
            else
                break;
        }
        else
            filetmp = filetmp->next;
    }
#endif
    //printf("Upload -> filepath = %s\n",filepath);
    int ret;
    //int fd;

    //如果本地同名文件比Server小，那么就无法覆盖这个Server上的文件，因为上传是一段一段上传的，所以先删除Server上的同名文件
    //Delete(filepath);

    char *serverpath;
    serverpath = localpath_to_serverpath(filepath,index);
    printf("Upload -> serverpath = %s\n",serverpath);

    /*have_parent = is_have_parent(filepath,serverpath);

    if(!have_parent)
    {
        ret = Mkcol_parent();
        if(ret == NE_WEBDAV_QUIT)
        {
            return NE_WEBDAV_QUIT;
        }
        else if(ret == NE_OK || ret == -1)
        {}
        else
        {
            return NO_PARENT;
        }
    }*/

    //fd = open(filepath,O_RDONLY);

    //pthread_cleanup_push(clean_fun_fd,(int *)&fd);
    //pthread_cleanup_push(clean_fun_session,sess);

    //if(finished_initial)
    write_log(S_UPLOAD,"",filepath,index);
    //ret = ne_put(sess,serverpath,fd);
    ret = my_put(filepath,serverpath,index);

    //free(base64_auth);
    free(serverpath);

    //pthread_cleanup_pop(0);
    //pthread_cleanup_pop(0);
    if(ret != 0)
    {
        printf("Put file error! ret = %d\n",ret);
        char error_info[200];
        memset(error_info,0,sizeof(error_info));
        sprintf(error_info,"%s",ne_get_error(asus_cfg.prule[index]->sess));
        printf("%s\n",error_info);

        //Delete(filepath,index);
        //close(fd);
        if(strstr(error_info,"507") != NULL)
        {
            Delete(filepath,index);
            write_log(S_ERROR,"server space is not enough!","",index);
            action_item *item;
            item = get_action_item("upload",filepath,g_pSyncList[index]->up_space_not_enough_list,index);
            if(item == NULL)
            {
                add_action_item("upload",filepath,g_pSyncList[index]->up_space_not_enough_list);
            }
            return SERVER_SPACE_NOT_ENOUGH;
        }
        if(strstr(error_info,"Could not connect to server") != NULL)
        {
            Delete(filepath,index);
            //if(!finished_initial)
            //{
            action_item *item;
            item = get_action_item("upload",filepath,g_pSyncList[index]->unfinished_list,index);
            if(item == NULL)
            {
                add_action_item("upload",filepath,g_pSyncList[index]->unfinished_list);
            }
            //}
            write_log(S_ERROR,"Could not connect to server!","",index);
            return COULD_NOT_CONNECNT_TO_SERVER;
            //return -1;
        }
        if(strstr(error_info,"Connection timed out") != NULL || strstr(error_info,"connection timed out") != NULL)
        {
            Delete(filepath,index);
            //if(!finished_initial)
            //{
            action_item *item;
            item = get_action_item("upload",filepath,g_pSyncList[index]->unfinished_list,index);
            if(item == NULL)
            {
                add_action_item("upload",filepath,g_pSyncList[index]->unfinished_list);
            }
            //}
            write_log(S_ERROR,"Could not connect to server!","",index);
            return CONNECNTION_TIMED_OUT;
            //return -1;
        }
        if(strstr(error_info,"Invalid argument") != NULL)
        {
            Delete(filepath,index);
            //if(!finished_initial)
            //{
            action_item *item;
            item = get_action_item("upload",filepath,g_pSyncList[index]->unfinished_list,index);
            if(item == NULL)
            {
                add_action_item("upload",filepath,g_pSyncList[index]->unfinished_list);
            }
            //}
            return INVALID_ARGUMENT;
            //return -1;
        }
        if(strstr(error_info,"427") != NULL)
        {
            Delete(filepath,index);
            char *filename = parse_name_from_path(filepath);
            //free(fullname_tmp);

            int len = strlen(filename);
            printf("filename = %s\nlen = %d\n",filename,len);
            char *path = my_str_malloc((size_t)(strlen(filepath)-len+1));

            snprintf(path,strlen(filepath)-len,"%s",filepath);
            printf("path = %s\n",path);

            ret = Mkcol(path,index);

            free(path);
            free(filename);
            return ret;

        }
        if(ret == NE_WEBDAV_QUIT)
        {
            FILE *fp;
            fp = fopen(g_pSyncList[index]->up_item_file,"w");
            if(fp == NULL)
            {
                printf("open %s error\n",g_pSyncList[index]->up_item_file);
                return ret;
            }
            fprintf(fp,"%s",localpath);
            fclose(fp);
            return ret;
        }
        return ret;
    }

    //if(!finished_initial)
    //{
    del_action_item("upload",filepath,g_pSyncList[index]->unfinished_list);
    del_action_item("upload",filepath,g_pSyncList[index]->up_space_not_enough_list);
    //}
    //close(fd);
    if(finished_initial)
        write_log(S_SYNC,"","",index);
    else
        write_log(S_INITIAL,"","",index);
    //sleep(1);
    usleep(1000*100);
    return ret;
}

#if 0
int Mkcol_parent(){
    printf("*************Mkcol_parent*****************\n");

    int ret;
    int have_unfinished = 0;
    struct sync_item *mkcol_parent_tmp;
    struct sync_item *p;

    mkcol_parent_tmp = mkcol_parent_head->next;

    while(mkcol_parent_tmp != NULL)
    {
        printf("mkcol_parent_tmp->href = %s\n",mkcol_parent_tmp->href);
        ret = Mkcol(mkcol_parent_tmp->href);
        if(ret == NE_OK || ret == -1)
        {
            p = mkcol_parent_tmp->next;
            del_sync_item(MKCOL_PARENT,mkcol_parent_tmp->href,mkcol_parent_head);
            mkcol_parent_tmp = p;
        }
        else if(exit_loop == 1)
        {
            free_sync_item(mkcol_parent_head);
            return NE_WEBDAV_QUIT;
        }
        else
        {
            have_unfinished = ret;
            mkcol_parent_tmp = mkcol_parent_tmp->next;
        }

    }
    free_sync_item(mkcol_parent_head);
    return have_unfinished;

}
#endif

int Delete(char *filepath,int index){
    printf("*************Delete*****************\n");

    //char *localpath;

    //localpath = strstr(filepath,base_path) + base_path_len;

    //printf("foldertmp->href = %s\n",foldertmp->href);
    //printf("serverpath_tmp = %s\n",serverpath_tmp);
#if 0
    char *hreftmp;
    int cmp;
    CloudFile *foldertmp;
    CloudFile *filetmp;
    foldertmp = OldFolderList->next;
    filetmp = OldFileList->next;
    while (foldertmp->href != NULL){
        //printf("foldertmp->href = %s\n",foldertmp->href);
        hreftmp = strstr(foldertmp->href,ROOTFOLDER) + rootfolder_len;
        hreftmp = oauth_url_unescape(hreftmp,NULL);
        //printf("hreftmp = %s\n",hreftmp);
        if ((cmp = strcmp(hreftmp,localpath)) == 0){
            //printf("%s has deleted!\n",serverpath);
            break;
        }
        else
            foldertmp = foldertmp->next;
    }
    if (cmp != 0){
        while (filetmp->href != NULL){
            //printf("foldertmp->href = %s\n",foldertmp->href);
            hreftmp = strstr(filetmp->href,ROOTFOLDER) + rootfolder_len;
            hreftmp = oauth_url_unescape(hreftmp,NULL);
            //printf("hreftmp = %s\n",hreftmp);
            //printf("foldertmp->href = %s\n",foldertmp->href);
            if ((cmp = strcmp(hreftmp,localpath)) == 0){
                //printf("%s has deleted!\n",serverpath);
                break;
            }
            else
                filetmp = filetmp->next;
        }
    }
    if (cmp != 0){
        printf("%s has deleted!\n",localpath);
        return 0;
    }
#endif
    int ret;
    char *serverpath;
    serverpath = localpath_to_serverpath(filepath,index);
    printf("serverpath = %s\n",serverpath);

    ret = ne_delete(asus_cfg.prule[index]->sess,serverpath);

    free(serverpath);

    if(ret != 0){
        printf("Delete file error!\n");
        char error_info[100];
        memset(error_info,0,sizeof(error_info));
        sprintf(error_info,"%s",ne_get_error(asus_cfg.prule[index]->sess));
        printf("%s\n",error_info);
        if(strstr(error_info,"404") != NULL)
        {
            return SERVER_FILE_DELETED;
        }
        write_log(S_ERROR,error_info,"",index);
        return ret;
    }

    return 0;
}

int test_if_dir(const char *dir){
    DIR *dp = opendir(dir);

    if(dp == NULL)
        return 0;

    closedir(dp);
    return 1;
}

int test_if_dir_empty(char *dir)
{
    struct dirent* ent = NULL;
    DIR *pDir;
    int i = 0;
    pDir=opendir(dir);

    if(pDir != NULL )
    {
        while (NULL != (ent=readdir(pDir)))
        {

            if(ent->d_name[0] == '.')
                continue;
            if(!strcmp(ent->d_name,".") || !strcmp(ent->d_name,".."))
                continue;
            i++;

        }
        closedir(pDir);
    }

    return  (i == 0) ? 1 : 0;
}

unsigned long stat_file(const char *filename)
{
    //unsigned long size;
    struct stat filestat;
    if( stat(filename,&filestat) == -1)
    {
        perror("stat:");
        return -1;
        //exit(1);
    }
    return  filestat.st_size;

    //return size;

}

/*
int wait_file_copying(char *filename)
{
    unsigned long size1,size2;
    unsigned long tmp = (unsigned long)0;
    size1 = stat_file(filename);
    usleep(1000*100);
    size2 = stat_file(filename);
    printf("@@@@@@@@@@size1 = %lu,size2 = %lu\n",size1,size2);
    while((size2 - size1) != tmp){
    	size1 = stat_file(filename);
    	usleep(1000*500);
    	size2 = stat_file(filename);
    	printf("size1 = %lu,size2 = %lu\n",size1,size2);
    }
    return 0;
}
*/

int is_file_copying(char *filename)
{
    unsigned long size1,size2;
    size1 = stat_file(filename);
    usleep(1000*100);
    size2 = stat_file(filename);

    return (size2 - size1);
}

int createFolder(char *dir,int index)
{
    printf("dir = %s\n",dir);
    int status;

    struct dirent *ent = NULL;
    DIR *pDir;
    pDir=opendir(dir);
    //char fullnametmp[256];
    //char escapepath[512];

    if(pDir != NULL )
    {
        status = Mkcol(dir,index);
        if(status != 0)
        {
#if DEBUG
            printf("Create %s failed\n",dir);
            //write_system_log("error","uploadfile fail");
#endif
            //return -1;
            closedir(pDir);
            return status;
        }
        add_action_item("createfolder",dir,g_pSyncList[index]->dragfolder_action_list);
        //如果能返回文件或文件夹已存在信息，这边可以加入对create_folder_cmp是否为0的判断

        while ((ent=readdir(pDir)) != NULL)
        {
            //usleep(1000*100);
            //printf("ent->d_name = %s\n",ent->d_name);
#if 1
            if(ent->d_name[0] == '.')
                continue;
            if(!strcmp(ent->d_name,".") || !strcmp(ent->d_name,".."))
                continue;

            char *fullname;
            fullname = (char *)malloc(sizeof(char)*(strlen(dir)+strlen(ent->d_name)+2));

            memset(fullname,'\0',sizeof(fullname));
            //memset(escapepath,0,sizeof(escapepath));
            //memset(fullnametmp,0,sizeof(fullnametmp));
            //memset(foldername,0,sizeof(foldername));
            //sprintf(foldername,"%s",dir,ent->d_name);
            //foldernametmp = foldername;
            //foldernametmp = oauth_url_escape(foldernametmp);
            sprintf(fullname,"%s/%s",dir,ent->d_name);
            //sprintf(fullnametmp,"%s/%s",dir,ent->d_name);
            //wd_escape(fullnametmp,escapepath);
            //printf("name is %s,d_type is %u\n",ent->d_name,ent->d_type);
            //printf("fullname = %s\n",fullname);
            if(test_if_dir(fullname) == 1)
            {
                //create_folder_cmp++;
            	//printf("fullname = %s\n",fullname);
                status = createFolder(fullname,index);
            	if(status != 0)
            	{
                    printf("CreateFolder %s failed\n",fullname);
                    //return -1;
                    free(fullname);
                    closedir(pDir);
                    return status;
            	}
            }
            else
            {
            	//printf("upload\n");
                //create_folder_cmp++;
            	//printf("fullname = %s\n",fullname);
            	//int a;
                //if( (a=wait_file_copying(fullname)) == 0)
                // {
                //printf("upload file %s\n",fullname);
                status = Upload(fullname,index);
                //sleep(2);
                //}

                if(status == 0)
                {
                    char *serverpath;
                    serverpath = localpath_to_serverpath(fullname,index);
                    time_t modtime;
                    modtime = Getmodtime(serverpath,index);
                    if(modtime != -1)
                        ChangeFile_modtime(fullname,modtime);
                    else
                    {
                        printf("ChangeFile_modtime failed!\n");
                    }
                    free(serverpath);
                    add_action_item("createfile",fullname,g_pSyncList[index]->dragfolder_action_list);
                }
                else if(status == SERVER_SPACE_NOT_ENOUGH)
                {
                    printf("upload %s failed,server space is not enough!\n",fullname);
                    //return -1;
                }
                else
                {
                    printf("upload %s failed\n",fullname);
                    free(fullname);
                    closedir(pDir);
                    return status;
                }         
            }
            free(fullname);
#endif
            //printf("per end\n");

        }
        closedir(pDir);
        return 0;
    }
    else{
        printf("open %s fail \n",dir);
        return LOCAL_FILE_LOST;
    }
}


int Mkcol(char *folderpath,int index){
    printf("****************Mkcol****************\n");

    int ret;

    if(access(folderpath,0) != 0)
    {
        printf("Local has no %s\n",folderpath);
        return LOCAL_FILE_LOST;
    }

    if(strcmp(folderpath,asus_cfg.prule[index]->base_path) == 0)
    {
        /*ret = ne_mkcol(asus_cfg.prule[index]->sess,asus_cfg.prule[index]->rootfolder);
        if(ret != 0)
        {
            printf("Create Folder error!\n");
            printf("ERROR info: %s\n",ne_get_error(asus_cfg.prule[index]->sess));
            return ret;
        }
        g_pSyncList[index]->init_completed = 0;
        return 0;*/
        write_log(S_ERROR,"Server Deleted Sync Folder!","",index);
        return SERVER_ROOT_DELETED;
    }

    int exist = 0;

    //char *fullname_tmp = NULL;
    //fullname_tmp = my_str_malloc(strlen(folderpath)+1);
    //sprintf(fullname_tmp,"%s",folderpath);

    char *filename = parse_name_from_path(folderpath);
    //free(fullname_tmp);

    int len = strlen(filename);
    printf("filename = %s\nlen = %d\n",filename,len);
    char *path = my_str_malloc((size_t)(strlen(folderpath)-len+1));

    snprintf(path,strlen(folderpath)-len,"%s",folderpath);
    printf("path = %s\n",path);
    exist = is_server_exist(path,folderpath,index);
    free(path);
    free(filename);

    if(exist)
    {
        char *newname;
        newname = change_server_same_name(folderpath,index);
        printf("newname = %s\n",newname);
        int status = Move(folderpath,newname,index);

        free(newname);
        if(status != 0)
            return status;
    }

    /*char *localpath;

    localpath = strstr(folderpath,base_path) + base_path_len;

    printf("localpath = %s\n",localpath);*/
#if 0
    char *hreftmp;
    int cmp,have_parent;
    CloudFile *foldertmp;
    foldertmp = OldFolderList->next;
    while (foldertmp->href != NULL){
        //printf("foldertmp->href = %s\n",foldertmp->href);
        hreftmp = strstr(foldertmp->href,ROOTFOLDER) + rootfolder_len;
        hreftmp = oauth_url_unescape(hreftmp,NULL);
        //printf("foldertmp->href = %s\n",foldertmp->href);
        if ((cmp = strcmp(hreftmp,localpath)) == 0){
            printf("%s has existed\n",localpath);
            return -1;
        }
        else
            foldertmp = foldertmp->next;
    }
#endif
    //int ret;

    char *serverpath;
    serverpath = localpath_to_serverpath(folderpath,index);
    printf("serverpath = %s\n",serverpath);

    /*have_parent = is_have_parent(folderpath,serverpath);

    if(!have_parent)
    {
        ret = Mkcol_parent();
        if(ret == NE_WEBDAV_QUIT)
        {
            return NE_WEBDAV_QUIT;
        }
        else if(ret == NE_OK || ret == -1)
        {}
        else
        {
            return NO_PARENT;
        }
    }*/


    ret = ne_mkcol(asus_cfg.prule[index]->sess,serverpath);

    free(serverpath);

    if(ret != 0){
        printf("Create Folder error!\n");
        printf("ERROR info: %s\n",ne_get_error(asus_cfg.prule[index]->sess));
        return ret;
    }

    //char URL[512];
    //memset(URL,0,sizeof(URL));
    //sprintf(URL,"%s%s",HOST,serverpath);
    //add_FolderList_item(FolderList,URL);
    //add_FolderList_item(OldFolderList,URL);

    return 0;
}

#if 0
int Mkcol_Root(char *serverpath){
    printf("****************Mkcol_Root****************\n");

    /*if(access(folderpath,0) != 0)
    {
        printf("Local has no %s\n",folderpath);
        return LOCAL_FILE_LOST;
    }*/

    int ret;

    /*char *serverpath;
    serverpath = localpath_to_serverpath(folderpath);*/
    printf("serverpath = %s\n",serverpath);

    ret = ne_mkcol(sess,serverpath);

    //free(serverpath);

    if(ret != 0){
        printf("Create Folder error!\n");
        printf("ERROR info: %s\n",ne_get_error(sess));
        return ret;
    }

    return 0;
}
#endif

#if 0
int is_newname_existed(char *serverpath,char *server_newpath_tmp){
    char URL[512];
    memset(URL,0,sizeof(URL));
    get_file_parent(serverpath,URL);
    FileList_one = (CloudFile *)malloc(sizeof(CloudFile));
    memset(FileList_one,0,sizeof(CloudFile));

    FileList_one->href = NULL;

    FileTail_one = FileList_one;
    //printf("URL = %s\n",URL);
    int status;
    status = getCloudInfo(URL,parseCloudInfo_one);
    if(status != 0)
    {
        printf("get Cloud Info ERROR! \n");
    }
    //char *hreftmp1;
    char *hreftmp2;
    int cmp = 1;
    //hreftmp1 = strstr(localpath,base_path) + base_path_len;
    CloudFile *FileCurrent_one;
    FileCurrent_one = FileList_one->next;
    //printf("FileCurrent_one->href = %s\n",FileCurrent_one->href);
    while(FileCurrent_one->href != NULL){
        hreftmp2 = strstr(FileCurrent_one->href,ROOTFOLDER) + rootfolder_len;
        hreftmp2 = oauth_url_unescape(hreftmp2,NULL);
        //printf("hreftmp1 = %s\n",hreftmp1);
        //printf("hreftmp2 = %s\n",hreftmp2);
        if((cmp = strcmp(server_newpath_tmp,hreftmp2)) == 0){
            free_CloudFile_item(FileList_one);
            return 1;
        }
        else
            FileCurrent_one = FileCurrent_one->next;
    }
    if(cmp != 0){
        free_CloudFile_item(FileList_one);
        return 0;
    }

}
#endif

int Move(char *oldpath,char *newpath,int index){
    printf("****************Move****************\n");
    //char *server_oldpath_tmp;
    //char *server_newpath_tmp;
    //char old_tmp[512];
    //char new_tmp[512];
    char *server_oldpath;
    char *server_newpath;

    server_oldpath = localpath_to_serverpath(oldpath,index);
    server_newpath = localpath_to_serverpath(newpath,index);

    //memset(old_tmp,0,sizeof(old_tmp));
    //memset(new_tmp,0,sizeof(new_tmp));
    //memset(server_oldpath,0,sizeof(server_oldpath));
    //memset(server_newpath,0,sizeof(server_newpath));

    //server_oldpath_tmp = strstr(oldpath,base_path) + base_path_len;
    //server_newpath_tmp = strstr(newpath,base_path) + base_path_len;

    //sprintf(old_tmp,"%s%s",ROOTFOLDER,server_oldpath_tmp);
    //sprintf(new_tmp,"%s%s",ROOTFOLDER,server_newpath_tmp);

    //wd_escape(old_tmp,server_oldpath);
    //wd_escape(new_tmp,server_newpath);

#if 0
    int exist;
    exist = is_newname_existed(server_newpath,server_newpath_tmp);
    if(exist == 1){
        printf("%s has existed!\n",server_newpath_tmp);
        return -1;
    }
#endif
    int ret;
    printf("server_oldpath = %s\nserver_newpath = %s\n",server_oldpath,server_newpath);

    //printf("%d %d %d %s\n",sess->connected,sess->persisted,sess->is_http11,sess->scheme);
    ret = ne_move(asus_cfg.prule[index]->sess,1,server_oldpath,server_newpath);

    printf("move ret = %d\n",ret);

    if(ret != 0){
        printf("move/rename %s to %s ERROR!\n",server_oldpath,server_newpath);
        char error_info[200];
        memset(error_info,0,sizeof(error_info));
        sprintf(error_info,"%s",ne_get_error(asus_cfg.prule[index]->sess));
        printf("ret = %d\nerror_info = %s\n",ret,error_info);
        if(strstr(error_info,"404") != NULL)
        {
            if(test_if_dir(newpath))
            {
                printf("it is folder\n");
                ret = createFolder(newpath,index);
                if(ret != 0)
                {
#if DEBUG
                    printf("Create %s failed\n",newpath);
                    //write_system_log("error","uploadfile fail");
#endif
                    //return -1;
                    free(server_oldpath);
                    free(server_newpath);
                    return ret;
                }
            }
            else
            {
                printf("it is file\n");
                ret = Upload(newpath,index);
                if(ret != 0)
                {
                    free(server_oldpath);
                    free(server_newpath);
                    return ret;
                }
                else
                {

                    time_t modtime;
                    modtime = Getmodtime(server_newpath,index);
                    if(modtime != -1)
                        ChangeFile_modtime(newpath,modtime);
                    else
                    {
                        printf("ChangeFile_modtime failed!\n");
                    }
                }
            }
        }
        if(strstr(error_info,"427") != NULL)
        {
            char *filename = parse_name_from_path(newpath);
            //free(fullname_tmp);

            int len = strlen(filename);
            printf("filename = %s\nlen = %d\n",filename,len);
            char *path = my_str_malloc((size_t)(strlen(newpath)-len+1));

            snprintf(path,strlen(newpath)-len,"%s",newpath);
            printf("path = %s\n",path);

            Mkcol(path,index);

            free(path);
            free(filename);
            free(server_oldpath);
            free(server_newpath);
            return ret;

        }
    }

    free(server_oldpath);
    free(server_newpath);
    return 0;
}

#if 0
int Move_Root(char *oldpath,char *newpath){
    printf("****************Move_Root****************\n");

    int ret;
    printf("oldpath = %s\nnewpath = %s\n",oldpath,newpath);

    //printf("%d %d %d %s\n",sess->connected,sess->persisted,sess->is_http11,sess->scheme);
    ret = ne_move(sess,1,oldpath,newpath);

    printf("move ret = %d\n",ret);

    if(ret != 0){
        printf("move/rename %s to %s ERROR!\n",oldpath,newpath);
        char error_info[200];
        memset(error_info,0,sizeof(error_info));
        sprintf(error_info,"%s",ne_get_error(sess));
        printf("ret = %d\nerror_info = %s\n",ret,error_info);
        return ret;

    }
    return ret;
}
#endif

time_t Getmodtime(char *serverhref,int index){

    //printf("Getmodtime serverhref = %s\n",serverhref);
    time_t modtime;
    int ret;

    ret = ne_getmodtime(asus_cfg.prule[index]->sess,serverhref,&modtime);

    //printf("ret = %d,modtime = %ld\n",ret,modtime);

    if(ret != 0){
        printf("getmodtime of %s ERROR!\n",serverhref);
        printf("ERROR info: %s\n",ne_get_error(asus_cfg.prule[index]->sess));
        return (time_t)-1;
    }
    return modtime;
}

int ne_getContentLength(ne_session *ne_sess, const char *uri,long long int *length)
{
    ne_request *req = ne_request_create(ne_sess, "HEAD", uri);
    const char *value;
    int ret;

    ret = ne_request_dispatch(req);

    value = ne_get_response_header(req, "Content-Length");

    if (ret == NE_OK && ne_get_status(req)->klass != 2) {
        //*modtime = -1;
        *length = -1;
        ret = NE_ERROR;
    }
    else if (value) {
        printf("value = %s\n",value);
        //*modtime = ne_httpdate_parse(value);
        *length = atoll(value);
    }
    else {
        //*modtime = -1;
        *length = 0;
    }

    ne_request_destroy(req);

    return ret;
}

long long int GetContentLength(char *serverhref,int index){

    long long int length;

    printf("GetContentLength serverhref = %s\n",serverhref);
    int ret;

    ret = ne_getContentLength(asus_cfg.prule[index]->sess,serverhref,&length);

    printf("ret = %d,length = %lld\n",ret,length);

    if(ret != 0){
        char error_info[100];
        memset(error_info,0,sizeof(error_info));
        sprintf(error_info,"%s",ne_get_error(asus_cfg.prule[index]->sess));
        printf("%s\n",error_info);
        printf("GetContentLength     of %s ERROR!\n",serverhref);

        if(strstr(error_info,"Could not connect to server") != NULL)
        {
            write_log(S_ERROR,"Could not connect to server!","",index);
            return (long long int)-2;
            //return -1;
        }
        if(strstr(error_info,"Connection timed out") != NULL || strstr(error_info,"connection timed out") != NULL)
        {
            write_log(S_ERROR,"Could not connect to server!","",index);
            return (long long int)-2;
            //return -1;
        }
        if(strstr(error_info,"404") != NULL)
        {
            return (long long int)-3;
        }

        return (long long int)-1;
    }
    return length;

}

int is_Server_Copying(char *serverhref,int index){

    long long int old_length;
    long long int new_length;
    long long int d_value;

    old_length = GetContentLength(serverhref,index);
    if(old_length == -1)
    {
        prog_total = -1;
        return -1;
    }
    if(old_length == -2)
    {
        prog_total = -1;
        return COULD_NOT_CONNECNT_TO_SERVER;
    }
    if(old_length == -3)
    {
        prog_total = -1;
        return SERVER_FILE_DELETED;
    }

    usleep(1000*1000);

    new_length = GetContentLength(serverhref,index);
    if(new_length == -1)
    {
        prog_total = -1;
        return -1;
    }
    if(new_length == -2)
    {
        prog_total = -1;
        return COULD_NOT_CONNECNT_TO_SERVER;
    }
    if(new_length == -3)
    {
        prog_total = -1;
        return SERVER_FILE_DELETED;
    }

    d_value = new_length - old_length;

    if(d_value == 0)
    {
        prog_total = new_length;
        return 0;
    }
    else
        return 1;

}

int ChangeFile_modtime(char *filepath,time_t servermodtime){
    printf("**************ChangeFile_modtime**********\n");
    //char *localfilepath_tmp;
    //char localfilepath[256];
    struct utimbuf *ub;
    ub = (struct utimbuf *)malloc(sizeof(struct utimbuf));
    //memset(localfilepath,0,sizeof(localfilepath));

    printf("servermodtime = %lu\n",servermodtime);

    if(servermodtime == -1){
        printf("ChangeFile_modtime ERROR!\n");
        return -1;
    }

    ub->actime = servermodtime;
    ub->modtime = servermodtime;

    //localfilepath_tmp = strstr(filepath,"/RT-N16/");
    //sprintf(localfilepath,"%s%s",base_path,localfilepath_tmp);
    utime(filepath,ub);

    /*struct stat buf;

		if( stat(localfilepath,&buf) == -1)
		{
			perror("stat:");
		}

		                //unsigned long asec = buf.st_atime;
		                unsigned long msec = buf.st_mtime;
		                //unsigned long csec = buf.st_ctime;

		                //printf("accesstime = %lu\n",asec);
		                //printf("creationtime = %lu\n",csec);
		                printf("lastwritetime = %lu\n",msec);
		                printf("servermodtime = %lu\n",servermodtime);*/

    free(ub);
    return 0;
}

int compareTime(time_t servermodtime ,char *localpath){
    printf("************compareTime*************\n");
    //time_t servermodtime;
    //char *localfilepath_tmp;
    //char localfilepath[256];
    struct stat buf;

    //servermodtime = Getmodtime(serverfilepath);
    if(servermodtime == -1){
        printf("compareTime ERROR!\n");
        return -1;
    }

    //memset(localfilepath,0,sizeof(localfilepath));

    //localfilepath_tmp = strstr(localpath,"/RT-N16/");
    //sprintf(localfilepath,"%s%s",base_path,localfilepath_tmp);
    printf("localpath = %s\n",localpath);

    if( stat(localpath,&buf) == -1)
    {
        perror("stat:");
    }
    time_t msec = buf.st_mtime;
    printf("msec = %lu,servermodtime = %lu\n",msec,servermodtime);

    if(servermodtime > msec)
        return 0;
    else if(servermodtime < msec)
        return 1;
    else
        return 2;
}

#if 0
int wd_initial_compareTime(time_t servermodtime ,char *localpath,LocalFile *head){
    printf("************wd_initial_compareTime*************\n");
    //time_t servermodtime;
    //char *localfilepath_tmp;
    //char localfilepath[256];
    LocalFile *SavedLocalFileTmp;
    SavedLocalFileTmp = head->next;
    int is_saved = 0;
    int cmp;
    while(SavedLocalFileTmp != NULL)
    {
        if((cmp = strcmp(SavedLocalFileTmp->path,localpath)) == 0)
        {
            is_saved = 1;
            break;
        }
        else
        {
            SavedLocalFileTmp = SavedLocalFileTmp->next;
        }
    }

    struct stat buf;
    //servermodtime = Getmodtime(serverfilepath);
    if(servermodtime == -1){
        printf("wd_initial_compareTime ERROR!\n");
        return -1;
    }

    //memset(localfilepath,0,sizeof(localfilepath));

    printf("localpath = %s\n",localpath);

    if( stat(localpath,&buf) == -1)
    {
        perror("stat:");
    }
    time_t msec = buf.st_mtime;

    if(is_saved){
        time_t savedmodtime = atoi(SavedLocalFileTmp->lastwritetime);
        printf("msec = %lu,servermodtime = %lu,savedmodtime = %lu\n",msec,servermodtime,savedmodtime);
        if(msec != savedmodtime && servermodtime == savedmodtime)
        {
            return 1;
        }
        else if(msec == savedmodtime && servermodtime != savedmodtime)
        {
            return 0;
        }
        else if(msec != savedmodtime && servermodtime != savedmodtime)
        {
            return 3;
        }
        else
        {
            return 2;
        }
    }
    else
    {
        if(msec != servermodtime)
        {
            return 3;
        }
        else
        {
            return 2;
        }

    }

    return 2;

}
#endif

void wd_escape(char *unescapepath,char *escapepath){
    char *p;
    const char *split = "/";
    unescapepath = unescapepath+1;

    p=strtok(unescapepath,split);
    while(p!=NULL)
    {
        p = oauth_url_escape(p);
        sprintf(escapepath,"%s/%s",escapepath,p);

        free(p);
        p=strtok(NULL,split);
    }

}

/*
void get_file_parent(char *filehref,char *URL){
    char *p;
    const char split = '/';
    char filehref_tmp[512];
    memset(filehref_tmp,0,sizeof(filehref_tmp));
    sprintf(filehref_tmp,"%s",filehref);
    char *get_file_tmp;
    get_file_tmp = strstr(filehref_tmp,ROOTFOLDER);
    printf("get_file_tmp = %s\n",get_file_tmp);

    p=strrchr(get_file_tmp, split);
    if(p!=NULL)
    {
        *p = '\0';
    }
    printf("get_file_parent URL = %s\n",get_file_tmp);
    sprintf(URL,"%s%s",HOST,get_file_tmp);
}

void get_file_parent_base_path(char *filehref,char *URL){
    char *p;
    const char split = '/';
    char filehref_tmp[512];
    memset(filehref_tmp,0,sizeof(filehref_tmp));
    sprintf(filehref_tmp,"%s",filehref);
    char *get_file_tmp;
    get_file_tmp = strstr(filehref_tmp,base_path) + base_path_len;

    p=strrchr(get_file_tmp, split);
    if(p!=NULL)
    {
        *p = '\0';
    }
    printf("get_file_parent = %s\n",get_file_tmp);
    sprintf(URL,"%s%s%s",HOST,ROOTFOLDER,get_file_tmp);
}

void get_file_parent_local(char *filehref,char *URL){
    char *p;
    const char split = '/';
    char filehref_tmp[512];
    memset(filehref_tmp,0,sizeof(filehref_tmp));
    sprintf(filehref_tmp,"%s",filehref);
    char *get_file_tmp;
    get_file_tmp = strstr(filehref_tmp,base_path) + base_path_len;

    p=strrchr(get_file_tmp, split);
    if(p!=NULL)
    {
        *p = '\0';
    }
    printf("get_file_parent = %s\n",get_file_tmp);
    sprintf(URL,"%s%s",base_path,get_file_tmp);
}
*/

void del_all_items(char *dir,int index)
{
    struct dirent* ent = NULL;
    DIR *pDir;
    pDir=opendir(dir);

    if(pDir != NULL )
    {
        while (NULL != (ent=readdir(pDir)))
        {
            //if(ent->d_name[0] == '.')
            //continue;
            if(!strcmp(ent->d_name,".") || !strcmp(ent->d_name,".."))
                continue;

            char *fullname;
            size_t len;
            len = strlen(dir)+strlen(ent->d_name)+2;
            fullname = my_str_malloc(len);
            sprintf(fullname,"%s/%s",dir,ent->d_name);

            if(test_if_dir(fullname) == 1)
            {
                /*if(receve_socket)
                {
                    server_sync = 0;
                    while(receve_socket)
                    {
                        usleep(1000*100);
                    }
                    server_sync = 1;
                }*/
                wait_handle_socket(index);
                del_all_items(fullname,index);
            }
            else
            {
                /*if(receve_socket)
                {
                    server_sync = 0;
                    while(receve_socket)
                    {
                        usleep(1000*100);
                    }
                    server_sync = 1;
                }*/
                wait_handle_socket(index);
                add_action_item("remove",fullname,g_pSyncList[index]->server_action_list);
                remove(fullname);
                //add_sync_item("remove",fullname,head);
                //write_system_log("remove",ent->d_name);
            }

            free(fullname);
            //remove(fullname);
            //rmdir()

        }
        closedir(pDir);

        add_action_item("remove",dir,g_pSyncList[index]->server_action_list);
        remove(dir);
        //add_sync_item("remove",dir,head);
        //write_system_log("remove",dir);
    }
    else
        printf("open %s fail \n",dir);
}

#if 0
void Upload_ChangeFile_modtime(char *URL,char *localpath){
    FileList_one = (CloudFile *)malloc(sizeof(CloudFile));
    memset(FileList_one,0,sizeof(CloudFile));

    FileList_one->href = NULL;

    FileTail_one = FileList_one;
    //printf("URL = %s\n",URL);
    int status;
    status = getCloudInfo(URL,parseCloudInfo_one);
    if(status != 0)
    {
        printf("get Cloud Info ERROR! \n");
    }
    char *hreftmp1;
    char *hreftmp2;
    int cmp;
    hreftmp1 = strstr(localpath,base_path) + base_path_len;
    CloudFile *FileCurrent_one;
    FileCurrent_one = FileList_one->next;
    //printf("FileCurrent_one->href = %s\n",FileCurrent_one->href);
    while(FileCurrent_one->href != NULL){
        hreftmp2 = strstr(FileCurrent_one->href,ROOTFOLDER) + rootfolder_len;
	hreftmp2 = oauth_url_unescape(hreftmp2,NULL);
	//printf("hreftmp1 = %s\n",hreftmp1);
	//printf("hreftmp2 = %s\n",hreftmp2);
	if((cmp = strcmp(hreftmp1,hreftmp2)) == 0){
            ChangeFile_modtime(localpath,FileCurrent_one->modtime);
	    break;
	}
	else
            FileCurrent_one = FileCurrent_one->next;
    }
    free_CloudFile_item(FileList_one);

}
#endif

/*
time_t GetServer_modtime(char *URL,char *localpath){
    FileList_one = (CloudFile *)malloc(sizeof(CloudFile));
    memset(FileList_one,0,sizeof(CloudFile));

    FileList_one->href = NULL;

    FileTail_one = FileList_one;
    //printf("URL = %s\n",URL);
    int status;
    status = getCloudInfo(URL,parseCloudInfo_one);
    if(status != 0)
    {
        printf("get Cloud Info ERROR! \n");
    }
    char *hreftmp1;
    char *hreftmp2;
    int cmp = 1;
    hreftmp1 = strstr(localpath,base_path) + base_path_len;
    CloudFile *FileCurrent_one;
    FileCurrent_one = FileList_one->next;
    //printf("FileCurrent_one->href = %s\n",FileCurrent_one->href);
    while(FileCurrent_one->href != NULL){
        hreftmp2 = strstr(FileCurrent_one->href,ROOTFOLDER) + rootfolder_len;
        hreftmp2 = oauth_url_unescape(hreftmp2,NULL);
        //printf("hreftmp1 = %s\n",hreftmp1);
        //printf("hreftmp2 = %s\n",hreftmp2);
        if((cmp = strcmp(hreftmp1,hreftmp2)) == 0){
            free_CloudFile_item(FileList_one);
            free(hreftmp2);
            return FileCurrent_one->modtime;
        }
        else
        {
            free(hreftmp2);
            FileCurrent_one = FileCurrent_one->next;
        }
    }
    if(cmp != 0){
        free_CloudFile_item(FileList_one);
        return 0;
    }
}*/

/*time_t GetList_modtime(char *localpath){
    char *hreftmp1;
    char *hreftmp2;
    int cmp = 1;
    hreftmp1 = strstr(localpath,base_path) + base_path_len;
    CloudFile *FileCurrent;
    FileCurrent = OldFileList->next;
    //printf("FileCurrent_one->href = %s\n",FileCurrent_one->href);
    while(FileCurrent->href != NULL){
        hreftmp2 = strstr(FileCurrent->href,ROOTFOLDER) + rootfolder_len;
        hreftmp2 = oauth_url_unescape(hreftmp2,NULL);
        //printf("hreftmp1 = %s\n",hreftmp1);
        //printf("hreftmp2 = %s\n",hreftmp2);
        if((cmp = strcmp(hreftmp1,hreftmp2)) == 0){
            free(hreftmp2);
            return FileCurrent->modtime;
            break;
        }
        else
        {
            free(hreftmp2);
            FileCurrent = FileCurrent->next;
        }
    }
    if(cmp != 0)
        return 0;
}*/

time_t GetFile_modtime(char *localpath){
    struct stat buf;
    if( stat(localpath,&buf) == -1)
    {
        perror("stat:");
        return 0;
    }
    time_t msec = buf.st_mtime;
    return msec;
}

long long int get_local_freespace(int index){
    /*************unit is B************/
    printf("***********get %s freespace!***********\n",asus_cfg.prule[index]->base_path);
    long long int freespace = 0;
    struct statvfs diskdata;
    //long long totalspace = 0;
    if(!statvfs(asus_cfg.prule[index]->base_path,&diskdata))
    {
        //printf("aaaaaaaaaaaaaaaaaa\n");
        freespace = (long long)diskdata.f_bsize * (long long)diskdata.f_bavail;
        //totalspace = (((long long)disk_statfs.f_bsize * (long long)disk_statfs.f_blocks));
        //printf("freespace = %lld\n",freespace);
        //printf("totalspace = %lld\n",totalspace);
        return freespace;
    }
    else
    {
        return 0;
    }
}

void free_CloudFile_item(CloudFile *head)
{
    //printf("***************free_CloudFile_item*********************\n");

    CloudFile *p = head;
    while(p != NULL)
    {
        head = head->next;
        if(p->href != NULL)
        {
            //printf("free CloudFile %s\n",p->href);
            free(p->href);
        }
        free(p);
        p = head;
    }

    //printf("free list ok\n");
    /*CloudFile *point;

    while(head->next != NULL)
    {
        point = head;
        head = head->next;
        if(point->href != NULL)
        {
            //printf("free CloudFile %s\n",point->href);
            free(point->href);
        }
        free(point);
    }
    if(head->href != NULL)
    {
        //printf("free CloudFile %s\n",head->href);
        free(head->href);
    }
    free(head);*/
}

void free_LocalFolder_item(LocalFolder *head)
{
    LocalFolder *p = head;
    while(p != NULL)
    {
        head = head->next;
        if(p->path != NULL)
        {
            //printf("free LocalFolder %s\n",point->path);
            free(p->path);
        }
        free(p);
        p = head;
    }

    //printf("free list ok\n");

    /*while(p2->next != NULL)
    {
        point = p2;
        //p2 = head;
        p2 = p2->next;
        if(point->path != NULL)
        {
            //printf("free LocalFile %s\n",point->path);
            free(point->path);
        }
        free(point);
    }
    if(p2->path != NULL)
    {
        //printf("free LocalFile %s\n",head->path);
        free(p2->path);
    }
    free(p2);*/
}

void free_LocalFile_item(LocalFile *head)
{
    LocalFile *p = head;
    while(p != NULL)
    {
        head = head->next;
        if(p->path != NULL)
        {
            //printf("free LocalFile %s\n",point->path);
            free(p->path);
        }
        free(p);
        p = head;
    }

    //printf("free list ok\n");

    /*while(p2->next != NULL)
    {
        point = p2;
        //p2 = head;
        p2 = p2->next;
        if(point->path != NULL)
        {
            //printf("free LocalFile %s\n",point->path);
            free(point->path);
        }
        free(point);
    }
    if(p2->path != NULL)
    {
        //printf("free LocalFile %s\n",head->path);
        free(p2->path);
    }
    free(p2);*/
}

/*void free_LocalAction_item(Local_Action *head)
{
    Local_Action *point;

    while(head->next != NULL)
    {
        point = head;
        head = head->next;
        free(point);
    }
}*/

void free_sync_item(struct sync_item *head)
{
    printf("^^^^^^^^^^free_sync_item^^^^^^^^^^^^\n");
    struct sync_item *point;

    while(head->next != NULL)
    {
        point = head;
        head = head->next;
        free(point);
    }
    free(head);
}

/*int write_log(int status, char *message)
{
    Log_struc log_s;
    FILE *fp;

    memset(&log_s,0,LOG_SIZE);

    log_s.status = status;

    if(log_s.status== S_ERROR)
    {
        strcpy(log_s.error,message);
    }

    fp = fopen(general_log,"wb");

    if(fp == NULL)
    {
        printf("open %s error\n",general_log);
        return -1;
    }

    fwrite(&log_s,LOG_SIZE,1,fp);
    fclose(fp);
}*/

int write_log(int status, char *message, char *filename,int index)
{
    if(exit_loop)
    {
        return 0;
    }
    //printf("write log status = %d\n",status);
    pthread_mutex_lock(&mutex_log);
    Log_struc log_s;
    FILE *fp;
    int mount_path_length;
    int ret;
    struct timeval now;
    struct timespec outtime;


    if(status == S_SYNC && exit_loop ==0)
    {
        ret = ne_getrouterinfo(parseRouterInfo,index);
    }

    long long int totalspace = PreDiskAvailableShow+PreDiskUsedShow;

    //mount_path_length = strlen(mount_path);
    mount_path_length = strlen(asus_cfg.prule[index]->mount_path);

    memset(&log_s,0,LOG_SIZE);

    log_s.status = status;

    fp = fopen(general_log,"w");

    if(fp == NULL)
    {
        printf("open %s error\n",general_log);
        pthread_mutex_unlock(&mutex_log);
        return -1;
    }

    if(log_s.status == S_ERROR)
    {
        printf("******** status is ERROR *******\n");
        strcpy(log_s.error,message);
        fprintf(fp,"STATUS:%d\nERR_MSG:%s\nTOTAL_SPACE:%lld\nUSED_SPACE:%lld\nRULENUM:%d\n",
                log_s.status,log_s.error,totalspace,PreDiskUsedShow,index);
    }
    else if(log_s.status == S_DOWNLOAD)
    {
        printf("******** status is DOWNLOAD *******\n");
        strcpy(log_s.path,filename);
        fprintf(fp,"STATUS:%d\nMOUNT_PATH:%s\nFILENAME:%s\nTOTAL_SPACE:%lld\nUSED_SPACE:%lld\nRULENUM:%d\n",
                log_s.status,asus_cfg.prule[index]->mount_path,log_s.path+mount_path_length,totalspace,PreDiskUsedShow,index);
    }
    else if(log_s.status == S_UPLOAD)
    {
        printf("******** status is UPLOAD *******\n");
        strcpy(log_s.path,filename);
        fprintf(fp,"STATUS:%d\nMOUNT_PATH:%s\nFILENAME:%s\nTOTAL_SPACE:%lld\nUSED_SPACE:%lld\nRULENUM:%d\n",
                log_s.status,asus_cfg.prule[index]->mount_path,log_s.path+mount_path_length,totalspace,PreDiskUsedShow,index);
    }
    else
    {
        //printf("write log status2 = %d\n",status);
        if (log_s.status == S_INITIAL)
            printf("******** other status is INIT *******\n");
        else
            printf("******** other status is SYNC *******\n");

        fprintf(fp,"STATUS:%d\nTOTAL_SPACE:%lld\nUSED_SPACE:%lld\nRULENUM:%d\n",
                log_s.status,totalspace,PreDiskUsedShow,index);
        //fprintf(fp,"%d\n",log_s.status);
    }

    fclose(fp);

    if(!exit_loop)
    {
        gettimeofday(&now, NULL);
        outtime.tv_sec = now.tv_sec + 3;
        outtime.tv_nsec = now.tv_usec * 1000;
        pthread_cond_timedwait(&cond_socket, &mutex_log, &outtime);
    }

    pthread_mutex_unlock(&mutex_log);
    return 0;
}

int add_sync_item(int action,char *item_href,struct sync_item *head)
{
    //pthread_mutex_lock(&mutex);

    printf("***************add_sync_item*****************\n");

    struct sync_item *p;

    p = head->next;



    struct sync_item *q = (struct sync_item *)malloc(sizeof(struct sync_item));
    memset(q, '\0', sizeof(struct sync_item));
    if(q == NULL)
        exit(-1);


    q->action = action;
    printf("item_href = %s\n",item_href);
    sprintf(q->href,"%s",item_href);



    //struct sync_item *item = create_sync_item(action,name);

    printf("q->action = %d\n",q->action);

    if(p == NULL)
    {
    	head->next = q;
        q->next = NULL;
    }
    else
    {
    	q->next = p;
    	head->next = q;
    }

    //pthread_mutex_unlock(&mutex);
    return 0;
}

int add_FolderList_item(CloudFile *head,char *item_href){
    printf("***************add_FolderList_item*****************\n");

    CloudFile *p;
    p = head->next;
    CloudFile *q = (CloudFile *)malloc(sizeof(CloudFile));
    memset(q, 0, sizeof(CloudFile));

    q->href = (char *)malloc(sizeof(char)*(strlen(item_href)+1));
    memset(q->href,'\0',sizeof(q->href));

    if(q == NULL)
        exit(-1);

    q->isFolder = 1;
    sprintf(q->href,"%s",item_href);

    if(p == NULL)
    {
        head->next = q;
        q->next = NULL;
    }
    else
    {
        q->next = p;
        head->next = q;
    }

    //pthread_mutex_unlock(&mutex);
    return 0;

}

int del_sync_item(int action,char *item_href,struct sync_item *head)
{
    //pthread_mutex_lock(&mutex);
    int cmp;
    struct sync_item *p1,*p2;

    p1 = head->next;
    p2 = head;



    if( NULL == p1 )
    {
        printf("sync item list is null\n");
        //pthread_mutex_unlock(&mutex);
        return -1 ;
    }

    if(p1->next == NULL)
    {
    	free(p1);
    	head->next = NULL;
    	return 1;
    }

    while(p1 != NULL)
    {
    	if((cmp = strcmp(p1->href,item_href)) ==0 )
    	{
            p2->next = p1->next;
            free(p1);
            return 1;
    	}
    	p2 = p1;
    	p1 = p1->next;
    }


    //pthread_mutex_unlock(&mutex);

    return 0;
}

/*int del_action_buf(char *action_buf){
    Local_Action *p1,*p2;
    int cmp;

    p1 = LocalActionList->next;
    p2 = LocalActionList;

    if( NULL == p1 )
    {
        printf("LocalAction list is null\n");
        //pthread_mutex_unlock(&mutex);
        return -1 ;
    }


    if(p1->next == NULL)
    {
        free(p1);
        LocalActionList->next = NULL;
        LocalActionTail = LocalActionList;
        return 1;
    }

    while(p1 != NULL)
    {
        if((cmp = strcmp(p1->buf,action_buf)) ==0 )
        {
            p2->next = p1->next;
            free(p1);
            return 1;
        }
        p2 = p1;
        p1 = p1->next;
    }

    return 0;
}
*/
int item_empty(struct sync_item *phead)
{
    //return phead->next == NULL;
    //return phead == NULL ;
    return (phead == NULL) ? 1 : 0 ;
}

#if 0
void print_sync_item(struct sync_item *phead,int type)
{

    FILE *fp;
    char filename[256];

    memset(filename,0,sizeof(filename));

    if(type == UPLOAD) //upload
        strcpy(filename,up_item_file);
    else
        strcpy(filename,down_item_file);

    fp = fopen(filename,"w");

    if(NULL == fp)
        return;

    /*
     if(NULL == phead)
     {
         printf(" head is null \n");
         return ;
     }
     */

    struct sync_item *point;
    point = phead;
    while(point != NULL)
    {
        printf("action = %d\n", point->action);
        printf("item_href = %s\n", point->href);
        fprintf(fp,"%s\n",point->href);
        point = point->next;
    }
    //struct sync_item* p1 = phead;

    /*
     do
     {
         printf("action = %s\n", p1->action);
         printf("name = %s\n", p1->name);
         fprintf(fp,"%s,%s\n",p1->action,p1->name);
         p1 = p1->next;
     }while(p1 != NULL);
     */

    //fprintf(fp,"%s,%s\n",p1->action,p1->name);

    fclose(fp);

}
#endif

#if 0
void print_LocalAction_item()
{
    printf("#############print_LocalAction_item##############\n");

    FILE *fp;
    char filename[256];

    memset(filename,0,sizeof(filename));

    strcpy(filename,local_action_file);

    fp = fopen(filename,"w");

    if(NULL == fp)
        return;

    /*
     if(NULL == phead)
     {
         printf(" head is null \n");
         return ;
     }
     */
    Local_Action *action_tmp;
    action_tmp = LocalActionList->next;
    while(action_tmp->buf != NULL)
    {
    	printf("action_tmp->buf = %s\n", action_tmp->buf);
    	fprintf(fp,"%s\n",action_tmp->buf);
    	action_tmp = action_tmp->next;
    }
    /*struct sync_item *point;
    point = phead;
    while(point != NULL)
    {
        printf("action = %d\n", point->action);
        printf("item_href = %s\n", point->href);
        fprintf(fp,"%d,%s\n",point->action,point->href);
        point = point->next;
    }*/
    //struct sync_item* p1 = phead;

    /*
     do
     {
         printf("action = %s\n", p1->action);
         printf("name = %s\n", p1->name);
         fprintf(fp,"%s,%s\n",p1->action,p1->name);
         p1 = p1->next;
     }while(p1 != NULL);
     */

    //fprintf(fp,"%s,%s\n",p1->action,p1->name);

    fclose(fp);

}
#endif

#if 0
void print_all_local_item(char *sync_item_filename,int type)
{
    //printf("dis is %s,filename is %s\n",dir,sync_item_filename);

    /*struct dirent* ent = NULL;
    DIR *pDir;
    pDir = opendir(dir);
    FILE *fp;

    if(NULL == pDir)
    {
        printf("open %s fail \n",dir);
        return;
    }*/
    FILE *fp;
    fp = fopen(sync_item_filename,"a");

    if(NULL == fp)
    {
        printf("create %s file error\n",sync_item_filename);
        return;
    }

    //printf("dis is %s 2\n",dir);
    if(type == 0){
    	LocalFileTmp = LocalFileList->next;
        while(LocalFileTmp != NULL)
    	{
            fprintf(fp,"%s/%s\n",LocalFileTmp->path,LocalFileTmp->lastwritetime);
            LocalFileTmp = LocalFileTmp->next;
    	}
    }
    else
    {
    	LocalFolderTmp = LocalFolderList->next;
        while(LocalFolderTmp != NULL)
    	{
            fprintf(fp,"%s\n",LocalFolderTmp->path);
            LocalFolderTmp = LocalFolderTmp->next;
    	}
    }

    fclose(fp);
    //fclose(fp);
}
#endif

#if 0
int parse_trans_item(char *path,int type)
{

    FILE *fp;

    char buffer[256];
    int ret;

    memset(buffer, '\0', sizeof(buffer));

    if (access(path,0) != 0)
        return -1;

    if(( fp = fopen(path,"rb"))==NULL)
    {
        fprintf(stderr,"read Cloud error");
    }
    while(fgets(buffer,256,fp)!=NULL)
    {
    	if(strlen(buffer)-1 > base_path_len){
            if( buffer[ strlen(buffer)-1 ] == '\n' )
                buffer[ strlen(buffer)-1 ] = '\0';
            printf("buffer = %s\n",buffer);
            if(access(buffer,0) == 0)
            {
                char escapepath_tmp[512];
                char path_tmp[256];
                memset(path_tmp,0,sizeof(path_tmp));
                sprintf(path_tmp,"%s",buffer);
                memset(escapepath_tmp,0,sizeof(escapepath_tmp));
                wd_escape(path_tmp,escapepath_tmp);
                char URL[512];
                memset(URL,0,sizeof(URL));
                printf("escapepath_tmp = %s\n",escapepath_tmp);
                get_file_parent_base_path(escapepath_tmp,URL);
                //printf("URL = %s\n",URL);

                FileList_one = (CloudFile *)malloc(sizeof(CloudFile));
                memset(FileList_one,0,sizeof(CloudFile));

                FileList_one->href = NULL;

                FileTail_one = FileList_one;

                ret = getCloudInfo(URL,parseCloudInfo_one);
                printf("ret = %d\n",ret);
                if(FileList_one->next != NULL)
                {
                    free_CloudFile_item(FileList_one);
                    add_sync_item(UPLOAD,buffer,up_head);
                    ret = Upload(buffer);
                    if(ret == NE_OK)
                    {
                        del_sync_item(UPLOAD,buffer,up_head);
                        Upload_ChangeFile_modtime(URL,buffer);
                    }
                    else if(ret == NE_WEBDAV_QUIT)
                    {
                        return NE_WEBDAV_QUIT;
                    }
                }
                else
                {
                    printf("Server has no %s\n",URL);
                }

            }
            else
            {
                printf("Local has no %s\n",buffer);
            }
    	}
    	memset(buffer, '\0', sizeof(buffer));
    }

    fclose(fp);


    return 0;

}
#endif

#if 0
int get_all_local_file_item(char *path,LocalFile *head)
{
    LocalFile *SavedLocalFileTmp;
    LocalFile *SavedLocalFileTail;
    SavedLocalFileTmp = (LocalFile *)malloc(sizeof(LocalFile));
    memset(SavedLocalFileTmp,'\0',sizeof(LocalFile));
    SavedLocalFileTail = head;

    FILE *fp;

    char buffer[256];
    char *p;
    const char split = '/';
    int ret;

    memset(buffer, '\0', sizeof(buffer));

    if (access(path,0) != 0)
        return -1;

    if(( fp = fopen(path,"rb"))==NULL)
    {
        fprintf(stderr,"read Cloud error");
    }
    while(fgets(buffer,256,fp)!=NULL)
    {
    	if(strlen(buffer)-1 > base_path_len){
            if( buffer[ strlen(buffer)-1 ] == '\n' )
            {
                buffer[ strlen(buffer)-1 ] = '\0';
            }
            //printf("buffer = %s\n",buffer);
            p=strrchr(buffer, split);
            //printf("p_len = %d\n",strlen(p));
            strncpy(SavedLocalFileTmp->path,buffer,strlen(buffer)-strlen(p));
            strncpy(SavedLocalFileTmp->lastwritetime,p+1,strlen(p)-1);
            //printf("SavedLocalFileTmp->path = %s\n",SavedLocalFileTmp->path);
            //printf("SavedLocalFileTmp->lastwritetime = %s\n",SavedLocalFileTmp->lastwritetime);

            SavedLocalFileTail->next = SavedLocalFileTmp;
            SavedLocalFileTail = SavedLocalFileTmp;
            SavedLocalFileTail->next = NULL;
            SavedLocalFileTmp = (LocalFile *)malloc(sizeof(LocalFile));
            memset(SavedLocalFileTmp,'\0',sizeof(LocalFile));
    	}
    	memset(buffer, '\0', sizeof(buffer));
    }

    fclose(fp);

    return 0;

}
#endif

/*void chang_filename(char *newpath,char *oldpath){
	char *p;
	char buffer[256];
	memset(buffer,'\0',sizeof(buffer));
	const char split = '/';

	sprintf(buffer,"%s",oldpath);

	p=strrchr(buffer, split);

	strncpy(newpath,buffer,strlen(buffer)-strlen(p));

}*/

#if 0
int do_unfinished(){

    printf("*************do_unfinished*****************\n");

    int ret;
    int have_unfinished = 0;
    struct sync_item *mkcol_tmp;
    struct sync_item *up_tmp;
    struct sync_item *p;
    //Local_Action *action_tmp;
    //Local_Action *q;

    mkcol_tmp = mkcol_head->next;
    up_tmp = up_head->next;
    //action_tmp = LocalActionList->next;

    while(mkcol_tmp != NULL)
    {
        printf("mkcol_tmp->href = %s\n",mkcol_tmp->href);
        ret = Mkcol(mkcol_tmp->href);
        if(ret == NE_OK || ret == -1)
        {
            p = mkcol_tmp->next;
            del_sync_item(MKCOL,mkcol_tmp->href,mkcol_head);
            mkcol_tmp = p;
        }
        else if(exit_loop == 1)
        {
            return NE_WEBDAV_QUIT;
        }
        else
        {
            have_unfinished = ret;
            mkcol_tmp = mkcol_tmp->next;
        }

    }

    while(up_tmp != NULL)
    {
        printf("up_tmp->href = %s\n",up_tmp->href);
        ret = Upload(up_tmp->href);
        if(ret == NE_OK || ret == -1)
        {
            char escapepath_tmp[512];
            char path_tmp[256];
            memset(path_tmp,0,sizeof(path_tmp));
            sprintf(path_tmp,"%s",up_tmp->href);
            memset(escapepath_tmp,0,sizeof(escapepath_tmp));
            wd_escape(path_tmp,escapepath_tmp);
            char URL[512];
            memset(URL,0,sizeof(URL));
            printf("escapepath_tmp = %s\n",escapepath_tmp);
            get_file_parent_base_path(escapepath_tmp,URL);
            //printf("URL = %s\n",URL);
            Upload_ChangeFile_modtime(URL,up_tmp->href);
            p = up_tmp->next;
            del_sync_item(UPLOAD,up_tmp->href,up_head);
            up_tmp = p;

        }
        else if(ret == NE_WEBDAV_QUIT || exit_loop == 1)
        {
            return NE_WEBDAV_QUIT;
        }
        else
        {
            have_unfinished = ret;
            up_tmp = up_tmp->next;
        }

    }

    /*while(action_tmp != NULL)
    {
        printf("action_tmp->buf = %s\n",action_tmp->buf);
        ret = cmd_parser(action_tmp->buf);
        //printf("ret = %d\n",ret);
        if(ret == NE_OK || ret == -1)
        {
            //printf("1111111111\n");
            q = action_tmp->next;
            del_action_buf(action_tmp->buf);
            action_tmp = q;
        }
        else if(ret == NE_WEBDAV_QUIT || exit_loop == 1)
        {
            //printf("2222222222222\n");
            return NE_WEBDAV_QUIT;
        }
        else
        {
            //printf("333333333333333\n");
            have_unfinished = ret;
            action_tmp = action_tmp->next;
        }
    }*/

    return have_unfinished;
}
#endif

int do_unfinished(int index){
    if(exit_loop)
    {
        return 0;
    }

    printf("*************do_unfinished*****************\n");

    action_item *p,*p1;
    p = g_pSyncList[index]->unfinished_list->next;
    int ret;

    while(p != NULL)
    {
        //printf("unfinished_list\n");
        //printf("p->path = %s\n",p->path);
        //printf("p->action = %s\n",p->action);
        if(!strcmp(p->action,"download"))
        {
            CloudFile *filetmp = NULL;
            filetmp = get_CloudFile_node(g_pSyncList[index]->ServerRootNode,p->path,0x2);
            //printf("filetmp->href = %s\n",filetmp->href);
            if(filetmp == NULL)   //if filetmp == NULL,it means server has deleted filetmp
            {
                printf("filetmp is NULL\n");
                p1 = p->next;
                del_action_item("download",p->path,g_pSyncList[index]->unfinished_list);
                p = p1;
                continue;
            }            
            char *localpath;
            localpath = serverpath_to_localpath(filetmp->href,index);
            printf("localpath = %s\n",localpath);
            if(is_local_space_enough(filetmp,index))
            {
                add_action_item("createfile",localpath,g_pSyncList[index]->server_action_list);
                ret = Download(filetmp->href,index);
                if (ret == NE_OK)
                {
                    ChangeFile_modtime(localpath,filetmp->modtime);
                    p1 = p->next;
                    del_action_item("download",filetmp->href,g_pSyncList[index]->unfinished_list);
                    p = p1;
                }
                else
                {
                    printf("download %s failed",filetmp->href);
                    p = p->next;
                    //return ret;
                }
            }
            else
            {
                write_log(S_ERROR,"local space is not enough!","",index);
                p = p->next;
            }
            free(localpath);
        }
        else if(!strcmp(p->action,"upload"))
        {
            p1 = p->next;
            char *serverpath;
            serverpath = localpath_to_serverpath(p->path,index);
            char *path_temp;
            path_temp = my_str_malloc(strlen(p->path)+1);
            sprintf(path_temp,"%s",p->path);
            ret = Upload(p->path,index);
            printf("********* uploadret = %d\n",ret);
            if(ret == NE_OK)
            {
                //char *serverpath;
                //serverpath = localpath_to_serverpath(p->path,index);
                //printf("serverpath = %s\n",serverpath);
                time_t modtime;
                modtime = Getmodtime(serverpath,index);
                if(modtime != -1)
                    ChangeFile_modtime(path_temp,modtime);
                else
                {
                    printf("ChangeFile_modtime failed!\n");
                }
                //free(serverpath);
                //p1 = p->next;
                //del_action_item("upload",p->path,g_pSyncList[index]->unfinished_list);
                p = p1;
            }
            else if(ret == LOCAL_FILE_LOST)
            {
                //p1 = p->next;
                del_action_item("upload",p->path,g_pSyncList[index]->unfinished_list);
                p = p1;
            }
            else
            {
                printf("upload %s failed",p->path);
                p = p->next;
            }
            free(serverpath);
            free(path_temp);
        }
        else
        {
            p = p->next;
        }
    }

    p = g_pSyncList[index]->up_space_not_enough_list->next;
    while(p != NULL)
    {
        p1 = p->next;
        printf("up_space_not_enough_list\n");
        char *serverpath;
        serverpath = localpath_to_serverpath(p->path,index);
        char *path_temp;
        path_temp = my_str_malloc(strlen(p->path)+1);
        sprintf(path_temp,"%s",p->path);
        ret = Upload(p->path,index);
        printf("########### uploadret = %d\n",ret);
        if(ret == NE_OK)
        {
            //char *serverpath;
            //serverpath = localpath_to_serverpath(p->path,index);
            printf("serverpath = %s\n",serverpath);
            time_t modtime;
            modtime = Getmodtime(serverpath,index);
            if(modtime != -1)
                ChangeFile_modtime(path_temp,modtime);
            else
            {
                printf("ChangeFile_modtime failed!\n");
            }
            //free(serverpath);
            //p1 = p->next;
            //del_action_item("upload",p->path,g_pSyncList[index]->up_space_not_enough_list);
            p = p1;
        }
        else if(ret == LOCAL_FILE_LOST)
        {
            //p1 = p->next;
            del_action_item("upload",p->path,g_pSyncList[index]->up_space_not_enough_list);
            p = p1;
        }
        else
        {
            printf("upload %s failed",p->path);
            p = p->next;
        }
        free(serverpath);
        free(path_temp);
    }
    return 0;
}

/*queue function*/
int queue_empty (queue_t q)
{
    return q->head == NULL;
}

queue_t queue_create ()
{
    queue_t q;
    q = malloc (sizeof (struct queue_struct));
    if (q == NULL)
        exit (-1);

    q->head = q->tail = NULL;
    return q;
}

void queue_enqueue (queue_entry_t d, queue_t q)
{
    d->next_ptr = NULL;
    if (q->tail)
    {
        q->tail->next_ptr = d;
        q->tail = d;
    }
    else
    {
        q->head = q->tail = d;
    }
}

queue_entry_t  queue_dequeue (queue_t q)
{
    queue_entry_t first = q->head;

    if (first)
    {
        q->head = first->next_ptr;
        if (q->head == NULL)
        {
            q->tail = NULL;
        }
        first->next_ptr = NULL;
    }
    return first;
}

void queue_destroy (queue_t q)
{
    if (q != NULL)
    {
        while (q->head != NULL)
        {
            queue_entry_t next = q->head;
            q->head = next->next_ptr;
            next->next_ptr = NULL;
            free (next);
        }
        q->head = q->tail = NULL;
        free (q);
    }
}

int is_copying_finished(char *filename)
{
    unsigned long size1,size2;
    size1 = stat_file(filename);
    sleep(1);
    size2 = stat_file(filename);

    return (size2 - size1);
}

/*changed from serverpath to localpath*/
char *serverpath_to_localpath(char *from_serverpath,int index){
    char *hreftmp;
    char *to_localpath;

    hreftmp = strstr(from_serverpath,asus_cfg.prule[index]->rootfolder)+asus_cfg.prule[index]->rootfolder_length;
    hreftmp = oauth_url_unescape(hreftmp,NULL);

    to_localpath = (char *)malloc(sizeof(char)*(strlen(hreftmp)+asus_cfg.prule[index]->base_path_len+1));
    memset(to_localpath,'\0',sizeof(to_localpath));

    sprintf(to_localpath,"%s%s",asus_cfg.prule[index]->base_path,hreftmp);
    free(hreftmp);

    printf("serverpath_to_localpath to_localpath = %s\n",to_localpath);

    return to_localpath;
}

/*大写转小写  is not useful*/
/*void block_letter_to_lower_case(char *block_str){

    char *p;
    p = block_str;
    while(*p != '\0')
    {
        if((*p>='a') && (*p<='z'))
            *p += 32;
        p++;
    }
}*/

/*changed from localpath to serverpath
 *to_serverpath without http://xxx.xx.xx.xx:xx
 */
char *localpath_to_serverpath(char *from_localpath,int index){
    //printf("localpath_to_serverpath from_localpath = %s\n",from_localpath);
    char *to_serverpath = NULL;
    char *pathtmp;
    char *tmp;
    char *p;
    const char *split = "/";
    char *strtok_tmp;
    int n;
    int len;

    len = strlen(from_localpath);

    if(len == asus_cfg.prule[index]->base_path_len)
    {
        tmp = my_str_malloc((size_t)asus_cfg.prule[index]->rootfolder_length+2);
        //printf("tmp = %s\n",tmp);
        sprintf(tmp,"%s",asus_cfg.prule[index]->rootfolder);
        //printf("tmp = %s\n",tmp);
        return tmp;
    }

    to_serverpath = (char *)malloc(sizeof(char));
    memset(to_serverpath,'\0',sizeof(to_serverpath));

    pathtmp = strstr(from_localpath,asus_cfg.prule[index]->base_path)+asus_cfg.prule[index]->base_path_len;
    //tmp = (char *)malloc(sizeof(char)*(strlen(pathtmp)+g_pSyncList[index]->rootfolder_length+1));
    //memset(tmp,'\0',sizeof(tmp));
    //sprintf(tmp,"%s%s",g_pSyncList[index]->rootfolder,pathtmp);
    //printf("pathtmp = %s\n",pathtmp);
    pathtmp = pathtmp+1;
    //printf("pathtmp+1 = %s\n",pathtmp);
    strtok_tmp = my_str_malloc(strlen(pathtmp)+1);
    sprintf(strtok_tmp,"%s",pathtmp);

    //printf("strtok_tmp = %s\n",strtok_tmp);

    p=strtok(strtok_tmp,split);
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
    tmp = my_str_malloc((size_t)strlen(to_serverpath)+asus_cfg.prule[index]->rootfolder_length+2);
    //printf("tmp = %s\n",tmp);
    sprintf(tmp,"%s%s",asus_cfg.prule[index]->rootfolder,to_serverpath);
    //free(tmp);
    free(to_serverpath);
    free(strtok_tmp);
    //printf("tmp1 = %s\n",tmp);
    return tmp;
}

/*used for initial,local syncfolder is NULL*/
int initMyLocalFolder(Server_TreeNode *servertreenode,int index)
{
    int res = 0;

    if(servertreenode->browse != NULL)
    {
        CloudFile *init_foldercurrent = NULL;
        CloudFile *init_filecurrent = NULL;
        if(servertreenode->browse->foldernumber > 0)
            init_foldercurrent = servertreenode->browse->folderlist->next;
        if(servertreenode->browse->filenumber > 0)
            init_filecurrent = servertreenode->browse->filelist->next;
        int ret;

        while(init_foldercurrent != NULL)
        {
            char *createpath;
            createpath = serverpath_to_localpath(init_foldercurrent->href,index);
            if(NULL == opendir(createpath))
            {
                if(wait_handle_socket(index))
                {
                    return HAVE_LOCAL_SOCKET;
                }
                //add_action_item("createfolder",createpath,g_pSyncList[index]->server_action_list);
                //mkdir(createpath,0777);
                //usleep(1000*200);
                if(-1 == mkdir(createpath,0777))
                {
                    printf("mkdir %s fail",createpath);
                    //sprintf(error_message,"mkdir %s fail",path);
                    //handle_error(S_MKDIR_FAIL,error_message);
                    exit(-1);
                }
                else
                {
                    add_action_item("createfolder",createpath,g_pSyncList[index]->server_action_list);
                }
            }
            free(createpath);
            init_foldercurrent = init_foldercurrent->next;
        }

        while(init_filecurrent != NULL)
        {
            if(is_local_space_enough(init_filecurrent,index))
            {
                char *createpath;
                createpath = serverpath_to_localpath(init_filecurrent->href,index);
                if(wait_handle_socket(index))
                {
                    return HAVE_LOCAL_SOCKET;
                }
                add_action_item("createfile",createpath,g_pSyncList[index]->server_action_list);
                ret = Download(init_filecurrent->href,index);
                if (ret == NE_OK)
                {
                    ChangeFile_modtime(createpath,init_filecurrent->modtime);
                    free(createpath);
                }
                else
                {
                    free(createpath);
                    return ret;
                }
            }
            else
            {
                write_log(S_ERROR,"local space is not enough!","",index);
                add_action_item("download",init_filecurrent->href,g_pSyncList[index]->unfinished_list);
            }
            init_filecurrent = init_filecurrent->next;
        }
    }

    if(servertreenode->Child != NULL)
    {
        res = initMyLocalFolder(servertreenode->Child,index);
        if(res != 0)
        {
            return res;
        }
    }

    if(servertreenode->NextBrother != NULL)
    {
        res = initMyLocalFolder(servertreenode->NextBrother,index);
        if(res != 0)
        {
            return res;
        }
    }

    return res;
}

int add_action_item(const char *action,const char *path,action_item *head){

    printf("add_action_item,action = %s,path = %s\n",action,path);

    action_item *p1,*p2;

    p1 = head;

    p2 = (action_item *)malloc(sizeof(action_item));
    memset(p2,'\0',sizeof(action_item));
    p2->action = (char *)malloc(sizeof(char)*(strlen(action)+1));
    p2->path = (char *)malloc(sizeof(char)*(strlen(path)+1));
    memset(p2->action,'\0',sizeof(p2->action));
    memset(p2->path,'\0',sizeof(p2->path));

    sprintf(p2->action,"%s",action);
    sprintf(p2->path,"%s",path);

    while(p1->next != NULL)
        p1 = p1->next;

    p1->next = p2;
    p2->next = NULL;

    printf("add action item OK!\n");

    return 0;
}

int del_action_item(const char *action,const char *path,action_item *head){

    action_item *p1,*p2;
    p1 = head->next;
    p2 = head;

    while(p1 != NULL)
    {
        if( !strcmp(p1->action,action) && !strcmp(p1->path,path))
        {
            p2->next = p1->next;
            free(p1->action);
            free(p1->path);
            free(p1);
            return 0;
        }
        p2 = p1;
        p1 = p1->next;
    }

    printf("can not find action item\n");
    return 1;
}

void del_download_only_action_item(const char *action,const char *path,action_item *head)
{
    //printf("del_sync_item action=%s,path=%s\n",action,path);
    if(head == NULL)
    {
        return;
    }
    action_item *p1, *p2;
    char *cmp_name;
    char *p1_cmp_name;
    p1 = head->next;
    p2 = head;

    cmp_name = my_str_malloc((size_t)(strlen(path)+2));
    sprintf(cmp_name,"%s/",path);    //add for delete folder and subfolder in download only socket list

    while(p1 != NULL)
    {
        p1_cmp_name = my_str_malloc((size_t)(strlen(p1->path)+2));
        sprintf(p1_cmp_name,"%s/",p1->path);      //add for delete folder and subfolder in download only socket list
        //printf("del_download_only_sync_item  p1->name = %s\n",p1->name);
        //printf("del_download_only_sync_item  cmp_name = %s\n",cmp_name);
        if(strstr(p1_cmp_name,cmp_name) != NULL)
        {
            p2->next = p1->next;
            free(p1->action);
            free(p1->path);
            free(p1);
            //printf("del sync item ok\n");
            //break;
            p1 = p2->next;
        }
        else
        {
            p2 = p1;
            p1 = p1->next;
        }
        free(p1_cmp_name);
    }

    free(cmp_name);
    //printf("del sync item fail\n");
}

int add_all_download_only_socket_list(char *cmd,const char *dir,int index)
{
    struct dirent* ent = NULL;
    char *fullname;
    int fail_flag = 0;
    //char error_message[256];

    DIR *dp = opendir(dir);

    if(dp == NULL)
    {
        printf("opendir %s fail",dir);
        //sprintf(error_message,"opendir %s fail",dir);
        //handle_error(S_OPENDIR_FAIL,error_message);
        fail_flag = 1;
        return -1;
    }

    add_action_item(cmd,dir,g_pSyncList[index]->download_only_socket_head);

    while (NULL != (ent=readdir(dp)))
    {

        if(ent->d_name[0] == '.')
            continue;
        if(!strcmp(ent->d_name,".") || !strcmp(ent->d_name,".."))
            continue;

        fullname = my_str_malloc((size_t)(strlen(dir)+strlen(ent->d_name)+2));
        //memset(error_message,0,sizeof(error_message));
        //memset(&createfolder,0,sizeof(Createfolder));

        sprintf(fullname,"%s/%s",dir,ent->d_name);

        if( test_if_dir(fullname) == 1)
        {
            add_all_download_only_socket_list("createfolder",fullname,index);
        }
        else
        {
            add_action_item("createfile",fullname,g_pSyncList[index]->download_only_socket_head);
        }
        free(fullname);
    }

    closedir(dp);

    return (fail_flag == 1) ? -1 : 0;
}

int add_all_download_only_dragfolder_socket_list(const char *dir,int index)
{
    struct dirent* ent = NULL;
    char *fullname;
    int fail_flag = 0;
    //char error_message[256];

    DIR *dp = opendir(dir);

    if(dp == NULL)
    {
        printf("opendir %s fail",dir);
        //sprintf(error_message,"opendir %s fail",dir);
        //handle_error(S_OPENDIR_FAIL,error_message);
        fail_flag = 1;
        return -1;
    }

    while (NULL != (ent=readdir(dp)))
    {

        if(ent->d_name[0] == '.')
            continue;
        if(!strcmp(ent->d_name,".") || !strcmp(ent->d_name,".."))
            continue;

        fullname = my_str_malloc((size_t)(strlen(dir)+strlen(ent->d_name)+2));
        //memset(error_message,0,sizeof(error_message));
        //memset(&createfolder,0,sizeof(Createfolder));

        sprintf(fullname,"%s/%s",dir,ent->d_name);

        if( test_if_dir(fullname) == 1)
        {
            add_action_item("createfolder",fullname,g_pSyncList[index]->dragfolder_action_list);
            add_action_item("createfolder",fullname,g_pSyncList[index]->download_only_socket_head);
            add_all_download_only_dragfolder_socket_list(fullname,index);
        }
        else
        {
            add_action_item("createfile",fullname,g_pSyncList[index]->dragfolder_action_list);
            add_action_item("createfile",fullname,g_pSyncList[index]->download_only_socket_head);
        }
        free(fullname);
    }

    closedir(dp);

    return (fail_flag == 1) ? -1 : 0;
}

action_item *get_action_item(const char *action,const char *path,action_item *head,int index){

    action_item *p;
    p = head->next;

    while(p != NULL)
    {
        if(asus_cfg.prule[index]->rule == 1)
        {
            if(!strcmp(p->path,path))
            {
                return p;
            }
        }
        else
        {
            if(!strcmp(p->action,action) && !strcmp(p->path,path))
            {
                return p;
            }
        }
        p = p->next;
    }

    printf("can not find action item\n");
    return NULL;
}

void free_action_item(action_item *head){

    action_item *point;
    point = head->next;

    while(point != NULL)
    {
        head->next = point->next;
        free(point->action);
        free(point->path);
        free(point);
        point = head->next;
    }
    free(head);
}

action_item *create_action_item_head(){

    action_item *head;

    head = (action_item *)malloc(sizeof(action_item));
    if(head == NULL)
    {
        printf("create memory error!\n");
        exit(-1);
    }
    memset(head,'\0',sizeof(action_item));
    head->next = NULL;

    return head;
}

int test_if_download_temp_file(char *filename)
{
    char file_suffix[9];
    char *temp_suffix = ".asus.td";
    memset(file_suffix,0,sizeof(file_suffix));
    char *p = filename;

    if(strstr(filename,temp_suffix))
    {
        strcpy(file_suffix,p+(strlen(filename)-strlen(temp_suffix)));

        //printf(" %s file_suffix is %s\n",filename,file_suffix);

        if(!strcmp(file_suffix,temp_suffix))
            return 1;
    }

    return 0;

}

void handle_quit_upload()
{
    printf("###handle_quit_upload###\n");
    int i;
    for(i=0;i<asus_cfg.dir_number;i++)
    {
        if(asus_cfg.prule[i]->rule != 1)
        {
            FILE *fp;
            unsigned long filesize;
            char *buf;
            char *filepath;
            int status;
            if(access(g_pSyncList[i]->up_item_file,0) == 0)
            {
                filesize = stat_file(g_pSyncList[i]->up_item_file);
                fp = fopen(g_pSyncList[i]->up_item_file,"r");
                if(fp == NULL)
                {
                    printf("open %s error\n",up_item_file);
                    return;
                }
                buf = my_str_malloc((size_t)(filesize+1));
                //filepath = my_str_malloc((size_t)(filesize+asus_cfg.prule[i]->base_path_len+1));
                fscanf(fp,"%s",buf);
                fclose(fp);

                if((strlen(buf)) <=1 )
                {
                    return ;
                }

                unlink(g_pSyncList[i]->up_item_file);

                filepath = my_str_malloc((size_t)(filesize+asus_cfg.prule[i]->base_path_len+strlen(buf)+1));
                sprintf(filepath,"%s%s",asus_cfg.prule[i]->base_path,buf);
                free(buf);
                Delete(filepath,i);
                status = Upload(filepath,i);

                if(status != 0)
                {
#if DEBUG
                    printf("upload %s failed\n",filepath);
                    //write_system_log("error","uploadfile fail");
#endif
                }
                else
                {
                    char *serverpath;
                    serverpath = localpath_to_serverpath(filepath,i);
                    //printf("serverpath = %s\n",serverpath);
                    time_t modtime;
                    modtime = Getmodtime(serverpath,i);
                    if(modtime != -1)
                        ChangeFile_modtime(filepath,modtime);
                    else
                    {
                        printf("ChangeFile_modtime failed!\n");
                    }
                    free(serverpath);
                }

                free(filepath);
            }
        }
    }
}

void my_mkdir(char *path)
{
    //char error_message[256];
    DIR *dir;
    if(NULL == (dir = opendir(path)))
    {
        if(-1 == mkdir(path,0777))
        {
            printf("mkdir %s fail",path);
            //sprintf(error_message,"mkdir %s fail",path);
            //handle_error(S_MKDIR_FAIL,error_message);
            exit(-1);
        }
    }
    else
        closedir(dir);
}

void my_mkdir_r(char *path)
{
    int i,len;
    char str[512];

    strncpy(str,path,512);
    len = strlen(str);
    for(i=0; i < len ; i++)
    {
        if(str[i] == '/' && i != 0)
        {
            str[i] = '\0';
            if(access(str,F_OK) != 0)
            {
                my_mkdir(str);
            }
            str[i] = '/';
        }
    }

    if(len > 0 && access(str,F_OK) != 0)
    {
        my_mkdir(str);
    }

}

/*0,local file newer
 *1,server file newer
 *2,local time == server time
 *-1,get server modtime failed
**/
int newer_file(char *localpath,int index){

    char *serverpath;
    serverpath = localpath_to_serverpath(localpath,index);
    //printf("serverpath = %s\n",serverpath);
    time_t modtime1,modtime2;
    modtime1 = Getmodtime(serverpath,index);
    free(serverpath);
    if(modtime1 == -1)
    {
        printf("newer_file Getmodtime failed!\n");

        return -1;
    }

    struct stat buf;

    //printf("localpath = %s\n",localpath);

    if( stat(localpath,&buf) == -1)
    {
        perror("stat:");
    }
    modtime2 = buf.st_mtime;
    //printf("modtime2 = %lu,modtime1 = %lu\n",modtime2,modtime1);

    if(modtime2 > modtime1)
        return 0;
    else if(modtime2 == modtime1)
        return 2;
    else
        return 1;

}

unsigned long get_file_size(const char *path)
{
    unsigned long filesize = -1;
    struct stat statbuff;
    if(stat(path, &statbuff) < 0){
        return filesize;
    }else{
        filesize = statbuff.st_size;
    }
    return filesize;
}

/*0,local file larger
 *1,server file larger
 *2,local size == server size
**/
/*
int compare_file_size(CloudFile *filetmp){

    char *localpath;
    localpath = serverpath_to_localpath(filetmp->href);

    long long int local_file_size;
    long long int server_file_size;

    server_file_size = atoll(filetmp->getcontentlength);

    local_file_size = (long long int)get_file_size(localpath);

    free(localpath);

    if(local_file_size > server_file_size)
    {
        return 0;
    }
    else if(local_file_size < server_file_size)
    {
        return 1;
    }
    else
    {
        return 2;
    }

}
*/

/*
 *find 'chr' in 'str' at the 'n' times,'n' need to >=1;
 *返回'chr'在'str'中第'n'次出现的位置
*/
char *my_nstrchr(const char chr,char *str,int n){

    if(n<1)
    {
        printf("my_nstrchr need n>=1\n");
        return NULL;
    }

    char *p1,*p2;
    int i = 1;
    p1 = str;

    do{
        p2 = strchr(p1,chr);
        p1 = p2;
        p1++;
        i++;
    }while(p2!=NULL && i<=n);

    if(i<n)
    {
        return NULL;
    }

    return p2;
}

/*
 *0,no local socket
 *1,local socket
*/
int wait_handle_socket(int index){

    //if(receve_socket)
    if(g_pSyncList[index]->receve_socket)
    {
        server_sync = 0;
        //while(receve_socket)
        while(g_pSyncList[index]->receve_socket || local_sync)
        {
            usleep(1000*100);
        }
        server_sync = 1;
        if(g_pSyncList[index]->have_local_socket)
        {
            g_pSyncList[index]->have_local_socket = 0;
            g_pSyncList[index]->first_sync = 1;
            return 1;
        }
        else
        {
            return 0;
        }
    }
    return 0;
}

int moveFolder(char *old_dir,char *new_dir,int index)
{
    //printf("dir = %s\n",new_dir);
    int status;

    struct dirent *ent = NULL;
    DIR *pDir;
    pDir=opendir(new_dir);
    //char fullnametmp[256];
    //char escapepath[512];

    if(pDir != NULL )
    {
        status = Mkcol(new_dir,index);
        if(status != 0)
        {
#if DEBUG
            printf("Create %s failed\n",new_dir);
            //write_system_log("error","uploadfile fail");
#endif
            //return -1;
            closedir(pDir);
            return status;
        }
        //add_action_item("createfolder",dir,dragfolder_action_list);


        while ((ent=readdir(pDir)) != NULL)
        {
            //usleep(1000*100);
            //printf("ent->d_name = %s\n",ent->d_name);
#if 1
            //if(ent->d_name[0] == '.')
            //continue;
            if(!strcmp(ent->d_name,".") || !strcmp(ent->d_name,".."))
                continue;

            char *old_fullname;
            char *new_fullname;
            new_fullname = my_str_malloc(strlen(new_dir)+strlen(ent->d_name)+2);
            old_fullname = my_str_malloc(strlen(old_dir)+strlen(ent->d_name)+2);

            //memset(new_fullname,'\0',sizeof(new_fullname));
            //memset(escapepath,0,sizeof(escapepath));
            //memset(fullnametmp,0,sizeof(fullnametmp));
            //memset(foldername,0,sizeof(foldername));
            //sprintf(foldername,"%s",dir,ent->d_name);
            //foldernametmp = foldername;
            //foldernametmp = oauth_url_escape(foldernametmp);
            sprintf(new_fullname,"%s/%s",new_dir,ent->d_name);
            sprintf(old_fullname,"%s/%s",old_dir,ent->d_name);
            //sprintf(fullnametmp,"%s/%s",dir,ent->d_name);
            //wd_escape(fullnametmp,escapepath);
            //printf("name is %s,d_type is %u\n",ent->d_name,ent->d_type);
            //printf("fullname = %s\n",fullname);
            if(test_if_dir(new_fullname) == 1)
            {
                //create_folder_cmp++;
                //printf("fullname = %s\n",fullname);
                status = moveFolder(old_fullname,new_fullname,index);
                if(status != 0)
                {
                    printf("CreateFolder %s failed\n",new_fullname);
                    //return -1;
                    free(new_fullname);
                    free(old_fullname);
                    closedir(pDir);
                    return status;
                }
            }
            else
            {
                //printf("upload\n");
                //create_folder_cmp++;
                //printf("fullname = %s\n",fullname);
                //int a;
                //if( (a=wait_file_copying(fullname)) == 0)
                // {
                //printf("upload file %s\n",fullname);
                //status = Upload(fullname);
                status = Move(old_fullname,new_fullname,index);
                //sleep(2);
                //}

                if(status != 0)
                {
                    printf("move %s failed\n",new_fullname);
                    free(new_fullname);
                    free(old_fullname);
                    closedir(pDir);
                    return status;
                }
                /*if(status == 0)
                {
                    char *serverpath;
                    serverpath = localpath_to_serverpath(fullname);
                    time_t modtime;
                    modtime = Getmodtime(serverpath);
                    if(modtime != -1)
                        ChangeFile_modtime(fullname,modtime);
                    else
                    {
                        printf("ChangeFile_modtime failed!\n");
                    }
                    free(serverpath);
                    add_action_item("createfile",fullname,dragfolder_action_list);
                }
                else if(status == SERVER_SPACE_NOT_ENOUGH)
                {
                    printf("upload %s failed,server space is not enough!\n",fullname);
                    //return -1;
                }
                else
                {
                    printf("upload %s failed\n",fullname);
                    free(fullname);
                    closedir(pDir);
                    return status;
                }*/
            }
            free(new_fullname);
            free(old_fullname);
#endif
            //printf("per end\n");

        }
        closedir(pDir);
        return 0;
    }
    else{
        printf("open %s fail \n",new_dir);
        return LOCAL_FILE_LOST;
    }
}


/*
 *1,server has the same file
 *0,server has no the same file
*/
int is_server_exist(char *parentpath,char *filepath,int index){

    int status;
    char *server_parenthref;
    char *url;
    char *file_url;
    FileList_one = (CloudFile *)malloc(sizeof(CloudFile));
    memset(FileList_one,0,sizeof(CloudFile));

    FileList_one->href = NULL;

    FileTail_one = FileList_one;
    FileTail_one->next = NULL;

    printf("parentpath = %s\n",parentpath);
    server_parenthref = localpath_to_serverpath(parentpath,index);
    printf("server_parenthref = %s\n",server_parenthref);
    url = my_str_malloc(strlen(asus_cfg.prule[index]->host)+strlen(server_parenthref)+1);
    sprintf(url,"%s%s",asus_cfg.prule[index]->host,server_parenthref);
    free(server_parenthref);
    printf("url = %s\n",url);

    do{
        status = getCloudInfo(url,parseCloudInfo_one,index);
        usleep(1000*500);
    }while(status == COULD_NOT_CONNECNT_TO_SERVER && exit_loop == 0);

    free(url);

    if(status != 0)
    {
        free_CloudFile_item(FileList_one);
        FileList_one = NULL;
        printf("get Cloud Info One ERROR! \n");
        return 0;
    }

    char *server_path;
    char *m_temp;
    const char m_chr = ':';
    m_temp = my_nstrchr(m_chr,asus_cfg.prule[index]->host,2);
    server_path = localpath_to_serverpath(filepath,index);
    file_url = my_str_malloc(strlen(asus_cfg.prule[index]->host)+strlen(server_path)+1);

    snprintf(file_url,strlen(asus_cfg.prule[index]->host)-strlen(m_temp)+1,"%s",
             asus_cfg.prule[index]->host);

    sprintf(file_url,"%s%s",file_url,server_path);
    free(server_path);
    printf("file_url = %s\n",file_url);

    CloudFile *de_filecurrent;
    de_filecurrent = FileList_one->next;
    while(de_filecurrent != NULL)
    {
        if(de_filecurrent->href != NULL)
        {
            //printf("de_filecurrent->href = %s\n",de_filecurrent->href);
            if(!(strcmp(de_filecurrent->href,file_url)))
            {
                printf("get it\n");
                free(file_url);
                free_CloudFile_item(FileList_one);
                return 1;
            }
        }
        de_filecurrent = de_filecurrent->next;
    }

    free(file_url);
    free_CloudFile_item(FileList_one);

    return 0;
}

/*
 *1,server has the same name file
 *0,server has no the same name
 *2,server has the same name folder
*/
#if 0
int is_server_exist_with_type(char *filepath){

    int status;
    FileList_one = (CloudFile *)malloc(sizeof(CloudFile));
    memset(FileList_one,0,sizeof(CloudFile));

    FileList_one->href = NULL;

    FileTail_one = FileList_one;
    FileTail_one->next = NULL;

    //printf("before while filepath = %s\n",filepath);
    do{
        status = getCloudInfo(ROOTURL,parseCloudInfo_one);
        sleep(2);
    }while(status == COULD_NOT_CONNECNT_TO_SERVER && exit_loop == 0);
    //printf("after while filepath = %s\n",filepath);

    //free(url);

    if(status != 0)
    {
        free_CloudFile_item(FileList_one);
        FileList_one = NULL;
        printf("get Cloud Info One ERROR! \n");
        return 0;
    }

    //char *server_path;
    //server_path = localpath_to_serverpath(filepath);
    //file_url = my_str_malloc(strlen(HOST)+strlen(server_path)+1);
    //sprintf(file_url,"%s%s",HOST,server_path);
    //free(server_path);
    printf("filepath = %s\n",filepath);

    CloudFile *de_filecurrent;
    de_filecurrent = FileList_one->next;
    while(de_filecurrent != NULL)
    {
        if(de_filecurrent->href != NULL)
        {
            //printf("de_filecurrent->href = %s\n",de_filecurrent->href);
            if(!(strcmp(de_filecurrent->href,filepath)))
            {
                printf("get it\n");
                //free(file_url);
                if(de_filecurrent->isFolder == 0)
                {
                    free_CloudFile_item(FileList_one);
                    return 1;
                }
                else
                {
                    free_CloudFile_item(FileList_one);
                    return 2;
                }

            }
        }
        de_filecurrent = de_filecurrent->next;
    }

    //free(file_url);
    free_CloudFile_item(FileList_one);

    return 0;
}
#endif

int get_create_threads_state()
{
    int i;
    for(i=0;i<asus_cfg.dir_number;i++)
    {
        if(asus_cfg.prule[i]->rule != 2)
        {
            return 1;
        }
    }

    return 0;
}


