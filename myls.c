#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>

#define BUF_SIZE 512
char str[BUF_SIZE];

char *make_lstat_str(struct stat buf)
{
    int mode;
    char auth[20];
    struct passwd *pwd;
    struct group *grp;
    struct tm *time;
    memset(str, '\0', BUF_SIZE);
    memset(auth, '\0', 20);

    S_ISREG(buf.st_mode) ? strcat(auth, "-") : strcat(auth, "");
    S_ISDIR(buf.st_mode) ? strcat(auth, "d") : strcat(auth, "");

    mode = buf.st_mode & S_IRUSR;
    (mode == S_IRUSR) ? strcat(auth, "r") : strcat(auth, "-");
    mode = buf.st_mode & S_IWUSR;
    (mode == S_IWUSR) ? strcat(auth, "w") : strcat(auth, "-");
    mode = buf.st_mode & S_IXUSR;
    (mode == S_IXUSR) ? strcat(auth, "x") : strcat(auth, "-");

    mode = buf.st_mode & S_IRGRP;
    (mode == S_IRGRP) ? strcat(auth, "r") : strcat(auth, "-");
    mode = buf.st_mode & S_IWGRP;
    (mode == S_IWGRP) ? strcat(auth, "w") : strcat(auth, "-");
    mode = buf.st_mode & S_IXGRP;
    (mode == S_IXGRP) ? strcat(auth, "x") : strcat(auth, "-");

    mode = buf.st_mode & S_IROTH;
    (mode == S_IROTH) ? strcat(auth, "r") : strcat(auth, "-");
    mode = buf.st_mode & S_IWOTH;
    (mode == S_IWOTH) ? strcat(auth, "w") : strcat(auth, "-");
    mode = buf.st_mode & S_IXOTH;
    (mode == S_IXOTH) ? strcat(auth, "x") : strcat(auth, "-");

    pwd = getpwuid(buf.st_uid);
    grp = getgrgid(buf.st_gid);

    time = localtime(&buf.st_atime);

    sprintf(str, "%s %2d %s %s %5d %02d월 %02d %02d:%02d", auth, buf.st_nlink, pwd->pw_name, grp->gr_name, buf.st_size, time->tm_mon + 1, time->tm_mday, time->tm_hour, time->tm_min);

    return str;
}

void do_ls(char *name, int a_flag, int l_flag)
{
    DIR *dirptr;
    struct dirent *direntp;
    struct stat buf;

    if (lstat(name, &buf) == -1)
    {
        fprintf(stderr, "File stat error (%s)", name);
        return;
    }

    if (S_ISREG(buf.st_mode))
    {
        if (l_flag == 1)
        {
            char *lstat_str;
            if (lstat(name, &buf) == -1)
            {
                fprintf(stderr, "File stat error (%s)", name);
                return;
            }
            lstat_str = make_lstat_str(buf);
            printf("%s %s\n", lstat_str, name);
        }
        else
        {
            printf("%s\n", name);
        }
    }
    else if (S_ISDIR(buf.st_mode))
    {

        dirptr = opendir(name);
        if (dirptr == NULL)
        {
            fprintf(stderr, "Fail to open %s", name);
            return;
        }
        else
        {
            if (a_flag == 1 && l_flag == 1)
            {
                direntp = readdir(dirptr);
                while (direntp != NULL)
                {
                    char *lstat_str;
                    char wd[BUF_SIZE];
                    memset(wd, '\0', BUF_SIZE);
                    sprintf(wd, "%s/%s", name, direntp->d_name);
                    if (lstat(wd, &buf) == -1)
                    {
                        fprintf(stderr, "File stat error (%s)", direntp->d_name);
                        return;
                    }
                    lstat_str = make_lstat_str(buf);
                    printf("%s %s\n", lstat_str, direntp->d_name);
                    direntp = readdir(dirptr);
                }
                closedir(dirptr);
            }
            else if (a_flag == 1 && l_flag == 0)
            {
                direntp = readdir(dirptr);
                while (direntp != NULL)
                {
                    printf("%s\n", direntp->d_name);
                    direntp = readdir(dirptr);
                }
                closedir(dirptr);
            }
            else if (a_flag == 0 && l_flag == 1)
            {
                direntp = readdir(dirptr);
                while (direntp != NULL)
                {
                    if (strcmp(direntp->d_name, ".") == 0 || strcmp(direntp->d_name, "..") == 0 || direntp->d_name[0] == '.')
                    {
                        direntp = readdir(dirptr);
                        continue;
                    }

                    struct stat buf;
                    char *lstat_str;
                    char wd[BUF_SIZE];
                    memset(wd, '\0', BUF_SIZE);
                    sprintf(wd, "%s/%s", name, direntp->d_name);
                    if (lstat(wd, &buf) == -1)
                    {
                        fprintf(stderr, "File stat error (%s)", direntp->d_name);
                        return;
                    }
                    lstat_str = make_lstat_str(buf);
                    printf("%s %s\n", lstat_str, direntp->d_name);
                    direntp = readdir(dirptr);
                }
                closedir(dirptr);
            }
            else if (a_flag == 0 && l_flag == 0)
            {
                direntp = readdir(dirptr);
                while (direntp != NULL)
                {
                    if (strcmp(direntp->d_name, ".") == 0 || strcmp(direntp->d_name, "..") == 0 || direntp->d_name[0] == '.')
                    {
                        direntp = readdir(dirptr);
                        continue;
                    }

                    printf("%s\n", direntp->d_name);
                    direntp = readdir(dirptr);
                }
                closedir(dirptr);
            }
        }
    }
}

int check_flag(char *str, char flag)
{
    for (int i = 0; i < strlen(str); i++)
    {
        if (str[i] == flag)
            return 1;
    }
    return 0;
}

int main(int argc, char *argv[])
{
    int a_flag = 0, l_flag = 0;
    int size = 0;
    char **dir_name = (char **)malloc(sizeof(char *) * argc);
    if (argc == 1)
    {
        do_ls(".", 0, 0);
    }
    else
    {
        for (int i = 1; i < argc; i++)
        {
            if (argv[i][0] == '-')
            {
                if (a_flag != 1)
                {
                    a_flag = check_flag(argv[i], 'a');
                }
                if (l_flag != 1)
                {
                    l_flag = check_flag(argv[i], 'l');
                }
            }
            else
            {
                dir_name[size++] = argv[i];
            }
        }
        if (size == 0)
        {
            do_ls(".", a_flag, l_flag);
        }
        else
        {
            do_ls(dir_name[0], a_flag, l_flag);
        }
    }

    return 0;
}
