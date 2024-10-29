#define _POSIX_C_SOURCE 200809L

#include <dirent.h>
#include <err.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#define FAIL 1
#define PASS 0

int lsdir(const char *path)
{
    DIR *dp;
    struct dirent *dt;
    struct stat sb;
    char subpath[PATH_MAX];

    int errn = PASS;
    size_t path_len = strlen(path);

    if (lstat(path, &sb))
    {
        warn("lsdir: cannot stat '%s'", path);
        return FAIL;
    }

    printf("%s\n", path);

    if (!S_ISDIR(sb.st_mode))
        return PASS;

    if (!(dp = opendir(path)))
    {
        warn("lsdir: cannot open directory '%s'", path);
        return FAIL;
    }

    while ((dt = readdir(dp)))
    {
        if (strcmp(dt->d_name, ".") == 0 || strcmp(dt->d_name, "..") == 0)
            continue;

        snprintf(subpath, sizeof(subpath), "%s%s%s", path,
                 (path[path_len - 1] == '/') ? "" : "/", dt->d_name);

        if (lsdir(subpath) != PASS)
            errn = FAIL;
    }

    closedir(dp);

    return errn;
}

int main(int argc, char **argv)
{
    char *path;
    int errn = PASS;

    if (argc > 2)
    {
        warnx("Expressions are not implemented yet");
        errn = FAIL;
    }

    path = argc == 1 ? "." : argv[1];
    if (lsdir(path) != PASS)
        errn = FAIL;

    return errn;
}
