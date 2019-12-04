#ifndef PARTITIONS_H
#define PARTITIONS_H

#include <stdbool.h>
#include "filesystems/fat/fat.h"
#include "partition.h"
#include "klibrary/kvector.h"
#include "mbr.h"
#include "drivers/floppy/floppy.h"
#include "drivers/harddisk/harddisk.h"

void partitions_init();
void partitions_init_floppy();
void partitions_init_harddisks(HARDDISK_ATA_MASTER_SLAVE type, HARDDISK_ATA_BUS_TYPE bus);
partition *partitions_get(char *path);
int partitions_get_count();
void partitions_get_symbols(char *symbol_array);

#endif