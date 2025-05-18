#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <dirent.h>
#include <limits.h>

void create_hard_link(const char *src, const char *dst) {
    if (link(src, dst) == -1) {
        perror("Failed to create hard link");
        exit(EXIT_FAILURE);
    }
}

void copy_symlink(const char *src, const char *dst) {
    char target[PATH_MAX];
    ssize_t len = readlink(src, target, sizeof(target) - 1);
    if (len == -1) {
        perror("Failed to read symlink");
        exit(EXIT_FAILURE);
    }
    target[len] = '\0';
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
        // Skip "." and ".."
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        char src_path[PATH_MAX];
        char dst_path[PATH_MAX];

        snprintf(src_path, PATH_MAX, "%s/%s", src, entry->d_name);
        snprintf(dst_path, PATH_MAX, "%s/%s", dst, entry->d_name);

        struct stat st;
        if (lstat(src_path, &st) == -1) {
            perror("Failed to stat source entry");
            exit(EXIT_FAILURE);
        }

        if (S_ISREG(st.st_mode)) {
            create_hard_link(src_path, dst_path);
        } else if (S_ISLNK(st.st_mode)) {
            copy_symlink(src_path, dst_path);
        } else if (S_ISDIR(st.st_mode)) {
            copy_directory(src_path, dst_path);
        }
    }

    closedir(dir);

    // Preserve permissions
    struct stat st;
    if (stat(src, &st) == 0) {
        chmod(dst, st.st_mode);
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

