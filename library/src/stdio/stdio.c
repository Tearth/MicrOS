#include "../stdio.h"

FILE *__stdio_create_stream()
{
    FILE *stream = malloc(sizeof(FILE));
    stream->buffer = malloc(BUFSIZ);
    stream->base = 0;
    stream->pos = 0;
    stream->size = 0;
    stream->limit = 0;
    stream->buffering_mode = file_buffering_mode_line;

    return stream;
}

file_mode __stdio_get_file_mode(const char *str_mode)
{
    if (strcmp(str_mode, "r") == 0)
        return file_mode_read;
    if (strcmp(str_mode, "w") == 0)
        return file_mode_write;
    if (strcmp(str_mode, "a") == 0)
        return file_mode_append;
    if (strcmp(str_mode, "r+") == 0)
        return file_mode_read_and_update;
    if (strcmp(str_mode, "w+") == 0)
        return file_mode_write_and_update;
    if (strcmp(str_mode, "a+") == 0)
        return file_mode_append_and_update;

    return file_mode_none;
}