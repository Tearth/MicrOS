#include <micros.h>
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char *argv[])
{
    micros_process_set_current_process_name("LS");
    if(argc < 2)
    {
        return -1;
    }
    
    char *path = argc >= 4 ? argv[2] : argv[1];
    
    if(!micros_filesystem_directory_exists(path))
    {
        printf("Directory not found!\n");
        return -1;
    }
    
    int count = micros_filesystem_get_entries_count_in_directory(path);
    char **entries = malloc(sizeof(char *) * count);
    
    micros_filesystem_get_entries_in_directory(path, entries);
    for(int i=0; i<count; i++)
    {
        printf("%s\n", entries[i]);
    }
    
    return 0;
}