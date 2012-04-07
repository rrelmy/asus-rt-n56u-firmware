struct group *brcm_getgrent(int grp_fd);
struct group *brcm_getgrgid(gid_t gid);
struct group *brcm_getgrnam(const char *name);
struct passwd *brcm__getpwent(int pwd_fd);
struct passwd *brcm_getpwent(void);
struct passwd *brcm_getpwnam(const char *name);
struct passwd *brcm_getpwuid(uid_t uid);
void brcm_endpwent(void);
void brcm_setpwent(void);
