#ifndef FILE_H
#define FILE_H

typedef enum file_buffering_mode
{
    file_buffering_mode_none,
    file_buffering_mode_line,
    file_buffering_mode_full
} file_buffering_mode;

typedef struct file
{
    char *buffer;
    uint32_t pos;
    uint32_t size;
    file_buffering_mode buffering_mode;

    int (*fetch)(struct file *file);
    void (*flush)(struct file *file);
} FILE;

#endif