#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <dirent.h>
#include <limits.h>
#include <utime.h>

void create_hard_link(const char *src, const char *dst) {
    if (link(src, dst) == -1) {
        perror("Failed to create hard link");
        exit(EXIT_FAILURE);
    }
}

// similar to StartsWith in C#
int prefix(const char *pre, const char *str)
{
    return strncmp(pre, str, strlen(pre)) == 0;
}

// used in order to get a portion of the string (starting at idx start, and ending at idx end)
// primarily when we would like to get the path relative to srcDir:
// srcDir/.../..../... => .../..../...
void slice(const char* str, char* result, int start, int end) {
    strncpy(result, str + start, end - start);
}


// most of the complication comes from the case that the directory src included a symlink to itself.
// in this case, we get the path to which the symlink points that is relative to the src dir
// and we then create a new symlink to the corresponding location in dst
// there is no need to fear the case in which the file was not yet created in dst, as symlinks can
// point to a non-existent file, and by the time the function finished, the symlink will be pointing to an existing file
// (we are to created it by then)
void copy_symlink(const char *src, const char *dst, const char* src_dir_path, const char* dst_dir_path) {
    char target[PATH_MAX];
    char targetAbsolute[PATH_MAX];
    char targetPathRelativeToSrc[PATH_MAX];
    char targetPathInDst[PATH_MAX];

    // GETTING SYMLINK ABSOLUTE PATH
    ssize_t len = readlink(src, target, sizeof(target) - 1);
    if (len == -1) {
        perror("Failed to read symlink");
        exit(EXIT_FAILURE);
    }
    target[len] = '\0';

    realpath(target, targetAbsolute);

    // CHECKING IF THE SYMLINK TARGET IS INSIDE SRC
    if (prefix(src_dir_path, targetAbsolute))
    {
        // ORIGINAL: ...srcPath/.../file BACKUP: ...dstPath/.../file
        slice(targetAbsolute, targetPathRelativeToSrc, strlen(src_dir_path), strlen(targetAbsolute));
        strcpy(targetPathInDst, dst_dir_path);
        strcat(targetPathInDst, targetPathRelativeToSrc);

        if (symlink(targetPathInDst, dst) == -1) {
            perror("Failed to create symlink");
            exit(EXIT_FAILURE);
        }
        return;
    }

    if (symlink(target, dst) == -1) {
        perror("Failed to create symlink");
        exit(EXIT_FAILURE);
    }
}

void copy_directory(const char *src, const char *dst) {
    DIR *dir = opendir(src);
    if (!dir) {
        perror("Failed to open source directory");
        exit(EXIT_FAILURE);
    }

    if (mkdir(dst, 0777) == -1) {
        perror("Failed to create backup directory");
        exit(EXIT_FAILURE);
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        char src_path[PATH_MAX];
        char dst_path[PATH_MAX];

        char src_dir_path[PATH_MAX];
        char dst_dir_path[PATH_MAX];

        snprintf(src_path, PATH_MAX, "%s/%s", src, entry->d_name);
        snprintf(dst_path, PATH_MAX, "%s/%s", dst, entry->d_name);

        realpath(src, src_dir_path);
        realpath(dst, dst_dir_path);

        struct stat st;
        if (lstat(src_path, &st) == -1) {
            perror("Failed to stat source entry");
            exit(EXIT_FAILURE);
        }

        if (S_ISREG(st.st_mode)) {
            create_hard_link(src_path, dst_path);
        } else if (S_ISLNK(st.st_mode)) {
            copy_symlink(src_path, dst_path,
                    src_dir_path, dst_dir_path);
        } else if (S_ISDIR(st.st_mode)) {
            copy_directory(src_path, dst_path);
        }
        chmod(dst_path, st.st_mode);
        if (lchown(dst_path, st.st_uid, st.st_gid) == -1) {
            perror("Failed to chown");
            exit(EXIT_FAILURE);
        }
        struct utimbuf times;
        times.actime = st.st_atime;
        times.modtime = st.st_mtime;
        if (utime(dst_path, &times) == -1) {
            perror("Failed to set timestamps");
            exit(EXIT_FAILURE);
        }
    }

    closedir(dir);

    // Preserve permissions
    struct stat st;
    if (stat(src, &st) == 0) {
        chmod(dst, st.st_mode);
    }
    if (lchown(dst, st.st_uid, st.st_gid) == -1) {
        perror("Failed to chown");
        exit(EXIT_FAILURE);
    }
    struct utimbuf times;
    times.actime = st.st_atime;
    times.modtime = st.st_mtime;
    if (utime(dst, &times) == -1) {
        perror("Failed to set timestamps");
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <source_directory> <backup_directory>\n", argv[0]);
        return EXIT_FAILURE;
    }

    struct stat src_stat, dst_stat;
    if (stat(argv[1], &src_stat) == -1) {
        perror("src dir");
        return EXIT_FAILURE;
    }

    if (stat(argv[2], &dst_stat) == 0) {
        perror("backup dir");
        return EXIT_FAILURE;
    }

    copy_directory(argv[1], argv[2]);

    return EXIT_SUCCESS;
}

