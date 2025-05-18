#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#define BUF_SIZE 1024

void handle_read(int data_fd, int results_fd, int start, int end, off_t file_size)
{
    if (start < 0 || end < 0 || start > end || start >= file_size)
    {
        return;
    }

    if (end >= file_size)
    {
        end = file_size - 1;
    }

    int len = end - start + 1;
    char *buf = malloc(len + 1);
    if (!buf)
    {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    lseek(data_fd, start, SEEK_SET);
    read(data_fd, buf, len);
    buf[len] = '\n';

    write(results_fd, buf, len + 1);
    free(buf);
}

void handle_write(int data_fd, int offset, char *text, off_t file_size)
{
    if (offset < 0 || offset > file_size)
    {
        return;
    }

    int text_len = strlen(text);
    off_t tail_len = file_size - offset;

    char *tail = NULL;

    if (offset < file_size)
    {
        tail = malloc(tail_len);
        if (!tail)
        {
            perror("malloc");
            exit(EXIT_FAILURE);
        }

        lseek(data_fd, offset, SEEK_SET);
        read(data_fd, tail, tail_len);
    }

    lseek(data_fd, offset, SEEK_SET);
    write(data_fd, text, text_len);

    if (tail)
    {
        write(data_fd, tail, tail_len);
        free(tail);
    }
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s data.txt requests.txt\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int data_fd = open(argv[1], O_RDWR);
    if (data_fd < 0)
    {
        perror("data.txt");
        exit(EXIT_FAILURE);
    }

    int req_fd = open(argv[2], O_RDONLY);
    if (req_fd < 0)
    {
        perror("requests.txt");
        close(data_fd);
        exit(EXIT_FAILURE);
    }

    int results_fd = open("read_results.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (results_fd < 0)
    {
        perror("read_results.txt");
        close(data_fd);
        close(req_fd);
        exit(EXIT_FAILURE);
    }

    FILE *req_file = fdopen(req_fd, "r");
    char line[BUF_SIZE];

    while (fgets(line, BUF_SIZE, req_file))
    {
        if (line[0] == 'Q') break;

        off_t file_size = lseek(data_fd, 0, SEEK_END);

        if (line[0] == 'R')
        {
            int start, end;
            if (sscanf(line, "R %d %d", &start, &end) == 2)
            {
                handle_read(data_fd, results_fd, start, end, file_size);
            }
        }
        else if (line[0] == 'W')
        {
            int offset;
            char *text = strchr(line, ' ');
            if (text)
            {
                text++;
                char *offset_str = text;
                text = strchr(text, ' ');
                if (text)
                {
                    *text = '\0';
                    text++;
                    offset = atoi(offset_str);
                    char *newline = strchr(text, '\n');
                    if (newline) *newline = '\0';
                    handle_write(data_fd, offset, text, file_size);
                }
            }
        }
    }

    close(data_fd);
    fclose(req_file);
    close(results_fd);
    return 0;
}
