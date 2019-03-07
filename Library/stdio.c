#include "stdio.h"

int fclose(FILE * stream)
{
    fflush(stream);
    free(stream);
}

int fflush(FILE* stream)
{
    stream->flush(stream);
    stream->pos = 0;
}

int fputc(int character, FILE* stream)
{
    fwrite(&character, 1, 1, stream);
    return character;
}

int fputs(const char* str, FILE* stream)
{
    return fwrite(str, strlen(str), 1, stream);
}

int putc(int character, FILE* stream)
{
    return fputc(character, stream);
}

int putchar(int character)
{
    return fputc(character, stdout);
}

int puts(const char* str)
{
    return fputs(str, stdout);
}

int ungetc(int character, FILE* stream)
{
    if(stream->pos > 0)
    {
        stream->pos++;
    }
    
    return stream->buffer[stream->pos - 1];
}

size_t fwrite(const void* ptr, size_t size, size_t count, FILE* stream)
{
    uint32_t total_size = size * count;
    
    if(stream->pos + total_size > BUFSIZ)
    {
        fflush(stream);
    }
    
    switch(stream->buffering_mode)
    {
        case file_buffering_mode_none:
        {
            memcpy(stream->buffer + stream->pos, ptr, total_size);
            stream->pos += total_size;
            
            fflush(stream);
            
            break;
        }
        
        case file_buffering_mode_line:
        {
            uint32_t origin = 0;
            for(int i=origin; i<total_size; i++)
            {
                char c = ((const char*)ptr)[i];
                if(c == '\n')
                {
                    uint32_t bytes_to_copy = i - origin + 1;
                
                    memcpy(stream->buffer + stream->pos, ptr + origin, bytes_to_copy);
                    fflush(stream);
                    
                    origin = i + 1;
                }
            }
                
            if(origin != total_size)
            {
                uint32_t bytes_to_copy = total_size - origin - 1;

                memcpy(stream->buffer + stream->pos, ptr + origin, bytes_to_copy);
                stream->pos += bytes_to_copy;
            }
            
            break;
        }
        
        case file_buffering_mode_full:
        {
            memcpy(stream->buffer + stream->pos, ptr, total_size);
            stream->pos += total_size;
            
            break;
        }
    }
    
    return total_size;
}

FILE* streams_create_stream()
{
    FILE* stream = malloc(sizeof(FILE));
    stream->buffer = malloc(BUFSIZ);
    stream->pos = 0;
    stream->buffering_mode = file_buffering_mode_line;
    
    return stream;
}