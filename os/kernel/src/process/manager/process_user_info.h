#ifndef PROCESS_USER_INFO_H
#define PROCESS_USER_INFO_H

typedef struct process_user_info
{
    uint32_t id;
    uint32_t parent_id;
    char name[32];

    uint32_t status;
    float cpu_usage;
    uint32_t memory_usage;
    bool is_thread;
} __attribute__((packed)) process_user_info;

#endif