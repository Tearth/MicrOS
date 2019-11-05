#include "harddisk_ata.h"

//! Current states of all hard drives.
harddisk_ata_states current_states;


harddisk_ata_states harddisk_ata_get_states()
{
    return current_states;
}

HARDDISK_ATA_STATE harddisk_ata_get_state(HARDDISK_ATA_MASTER_SLAVE type, HARDDISK_ATA_BUS_TYPE bus)
{
    const HARDDISK_ATA_STATE *state;
    const harddisk_identify_device_data *data;
    __harddisk_ata_get_pointers(type, bus, &state, &data);

    return *state;
}

char* harddisk_ata_get_disk_serial_number_terminated(HARDDISK_ATA_MASTER_SLAVE type, HARDDISK_ATA_BUS_TYPE bus, char *buffer)
{
    const HARDDISK_ATA_STATE *state;
    const harddisk_identify_device_data *data;
    __harddisk_ata_get_pointers(type, bus, &state, &data);

    // Copy serial number to buffer
    for(int i = 0; i < HARDDISK_SERIAL_NUMBER_LENGTH; i += 2)
    {
        buffer[i] = data->fields.serial_number[i + 1];
        buffer[i + 1] = data->fields.serial_number[i];
    }
    buffer[HARDDISK_SERIAL_NUMBER_LENGTH] = '\0';
    return buffer;
}

char* harddisk_ata_get_disk_firmware_version_terminated(HARDDISK_ATA_MASTER_SLAVE type, HARDDISK_ATA_BUS_TYPE bus, char *buffer)
{
    const HARDDISK_ATA_STATE *state;
    const harddisk_identify_device_data *data;
    __harddisk_ata_get_pointers(type, bus, &state, &data);

    // Copy firmware version to buffer
    for(int i = 0; i < HARDDISK_FIRMWARE_VERSION_LENGTH; i += 2)
    {
        buffer[i] = data->fields.firmware_version[i + 1];
        buffer[i + 1] = data->fields.firmware_version[i];
    }
    buffer[HARDDISK_FIRMWARE_VERSION_LENGTH] = '\0';
    return buffer;
}

char* harddisk_ata_get_disk_model_number_terminated(HARDDISK_ATA_MASTER_SLAVE type, HARDDISK_ATA_BUS_TYPE bus, char *buffer)
{
    const HARDDISK_ATA_STATE *state;
    const harddisk_identify_device_data *data;
    __harddisk_ata_get_pointers(type, bus, &state, &data);

    // Copy model number to buffer
    for(int i = 0; i < HARDDISK_MODEL_NUMBER_LENGTH; i += 2)
    {
        buffer[i] = data->fields.model_number[i + 1];
        buffer[i + 1] = data->fields.model_number[i];
    }
    buffer[HARDDISK_MODEL_NUMBER_LENGTH] = '\0';
    return buffer;
}

uint32_t harddisk_ata_get_user_addressable_sectors(HARDDISK_ATA_MASTER_SLAVE type, HARDDISK_ATA_BUS_TYPE bus)
{
    const HARDDISK_ATA_STATE *state;
    const harddisk_identify_device_data *data;
    __harddisk_ata_get_pointers(type, bus, &state, &data);
    
    return data->fields.total_number_of_user_addressable_sectors;
}

uint32_t harddisk_ata_get_disk_space(HARDDISK_ATA_MASTER_SLAVE type, HARDDISK_ATA_BUS_TYPE bus)
{
    const HARDDISK_ATA_STATE *state;
    const harddisk_identify_device_data *data;
    __harddisk_ata_get_pointers(type, bus, &state, &data);
    
    // Multiply total number of user addressable sectors by number of bytes per sector (currently hard coded).
    return data->fields.total_number_of_user_addressable_sectors * 512;
}

int8_t harddisk_ata_read_sector(HARDDISK_ATA_MASTER_SLAVE type, HARDDISK_ATA_BUS_TYPE bus, uint32_t high_lba, uint32_t low_lba, uint16_t *buffer)
{
    if(buffer == NULL) return -2;
    uint16_t io_port = 0;
    harddisk_io_drive_head_register message_to_drive = {.value = 0};

    // Set port of drive
    if (bus == HARDDISK_ATA_PRIMARY_BUS) io_port = HARDDISK_ATA_PRIMARY_BUS_IO_PORT;
    else if(bus == HARDDISK_ATA_SECONDARY_BUS) io_port = HARDDISK_ATA_SECONDARY_BUS_IO_PORT;
    else return -2;

    // Set drive
    switch (type)
    {
    case HARDDISK_ATA_MASTER:
        // For master set it to 0x40. Choose to use LBA.
        message_to_drive.fields.chs_head_lba_block_number = 1;
        break;
    case HARDDISK_ATA_SLAVE:
        // For slave set it to 0x50. Choose to use LBA and drive 1.
        message_to_drive.fields.chs_head_lba_block_number = 1;
        message_to_drive.fields.drive_number = 1;
        break;
    default:
        return -2;
    }

    // Send message to drive
    io_out_byte(io_port + HARDDISK_IO_DRIVE_HEAD_REGISTER_OFFSET, message_to_drive.value);

    // Send what to read
    io_out_byte(io_port + HARDDISK_IO_SECTOR_COUNT_REGISTER_OFFSET, 0);
    io_out_byte(io_port + HARDDISK_IO_SECTOR_NUMBER_REGISTER_OFFSET, (uint8_t)high_lba);
    io_out_byte(io_port + HARDDISK_IO_CYLINDER_LOW_REGISTER_OFFSET, (uint8_t)(high_lba >> 8));
    io_out_byte(io_port + HARDDISK_IO_CYLINDER_HIGH_REGISTER_OFFSET, (uint8_t)(high_lba >> 16));
    io_out_byte(io_port + HARDDISK_IO_SECTOR_COUNT_REGISTER_OFFSET, 1);
    io_out_byte(io_port + HARDDISK_IO_SECTOR_NUMBER_REGISTER_OFFSET, (uint8_t)low_lba);
    io_out_byte(io_port + HARDDISK_IO_CYLINDER_LOW_REGISTER_OFFSET, (uint8_t)(low_lba >> 8));
    io_out_byte(io_port + HARDDISK_IO_CYLINDER_HIGH_REGISTER_OFFSET, (uint8_t)(low_lba >> 16));

    // Send the READ SECTORS EX" command to command register of I/O port.
    io_out_byte(io_port + HARDDISK_IO_COMMAND_REGISTER_OFFSET, HARDDISK_READ_SECTORS_EXT_COMMAND);

    __harddisk_ata_400ns_delay(io_port);

    // For any other value: poll the Status port until bit 7 (BSY, value = 0x80) clears.
    int8_t pooling_result = __harddisk_ata_poll(io_port);
    if(pooling_result == 1)
    {
        for(int i = 0; i < 256; i++)
        {
            // Read 256 16-bit values, and store them.
            buffer[i] = io_in_word(io_port);
        }
        return 1;
    }
    else
    {
        // Error occured
        return -1;
    }

}

int8_t harddisk_ata_write_sector(HARDDISK_ATA_MASTER_SLAVE type, HARDDISK_ATA_BUS_TYPE bus, uint32_t high_lba, uint32_t low_lba, uint16_t *buffer)
{
    if(buffer == NULL) return -2;
    uint16_t io_port = 0;
    harddisk_io_drive_head_register message_to_drive = {.value = 0};

    // Set port of drive
    if (bus == HARDDISK_ATA_PRIMARY_BUS) io_port = HARDDISK_ATA_PRIMARY_BUS_IO_PORT;
    else if(bus == HARDDISK_ATA_SECONDARY_BUS) io_port = HARDDISK_ATA_SECONDARY_BUS_IO_PORT;
    else return -2;

    // Set drive
    switch (type)
    {
    case HARDDISK_ATA_MASTER:
        // For master set it to 0x40. Choose to use LBA.
        message_to_drive.fields.chs_head_lba_block_number = 1;
        break;
    case HARDDISK_ATA_SLAVE:
        // For slave set it to 0x50. Choose to use LBA and drive 1.
        message_to_drive.fields.chs_head_lba_block_number = 1;
        message_to_drive.fields.drive_number = 1;
        break;
    default:
        return -2;
    }

    // Send message to drive
    io_out_byte(io_port + HARDDISK_IO_DRIVE_HEAD_REGISTER_OFFSET, message_to_drive.value);

    // Send what to write
    io_out_byte(io_port + HARDDISK_IO_SECTOR_COUNT_REGISTER_OFFSET, 0);
    io_out_byte(io_port + HARDDISK_IO_SECTOR_NUMBER_REGISTER_OFFSET, (uint8_t)high_lba);
    io_out_byte(io_port + HARDDISK_IO_CYLINDER_LOW_REGISTER_OFFSET, (uint8_t)(high_lba >> 8));
    io_out_byte(io_port + HARDDISK_IO_CYLINDER_HIGH_REGISTER_OFFSET, (uint8_t)(high_lba >> 16));
    io_out_byte(io_port + HARDDISK_IO_SECTOR_COUNT_REGISTER_OFFSET, 1);
    io_out_byte(io_port + HARDDISK_IO_SECTOR_NUMBER_REGISTER_OFFSET, (uint8_t)low_lba);
    io_out_byte(io_port + HARDDISK_IO_CYLINDER_LOW_REGISTER_OFFSET, (uint8_t)(low_lba >> 8));
    io_out_byte(io_port + HARDDISK_IO_CYLINDER_HIGH_REGISTER_OFFSET, (uint8_t)(low_lba >> 16));

    // Send the WRITE SECTORS EXT command to command register of I/O port.
    io_out_byte(io_port + HARDDISK_IO_COMMAND_REGISTER_OFFSET, HARDDISK_WRITE_SECTORS_EXT_COMMAND);

    __harddisk_ata_400ns_delay(io_port);

    // For any other value: poll the Status port until bit 7 (BSY, value = 0x80) clears.
    int8_t pooling_result = __harddisk_ata_poll(io_port);
    if(pooling_result == 1)
    {
        for(int i = 0; i < 256; i++)
        {
            // Write 256 16-bit values.
            io_out_word(io_port, buffer[i]);
            // Cache flush.
            io_out_byte(io_port + HARDDISK_IO_COMMAND_REGISTER_OFFSET, 0xE7);
        }
        return 1;
    }
    else
    {
        // Error occured
        return -1;
    }

}

void harddisk_ata_init()
{
    current_states.primary_master = (HARDDISK_ATA_STATE) __harddisk_ata_check_presence(HARDDISK_ATA_MASTER, HARDDISK_ATA_PRIMARY_BUS);
    current_states.primary_slave = (HARDDISK_ATA_STATE) __harddisk_ata_check_presence(HARDDISK_ATA_SLAVE, HARDDISK_ATA_PRIMARY_BUS);
    current_states.secondary_master = (HARDDISK_ATA_STATE) __harddisk_ata_check_presence(HARDDISK_ATA_MASTER, HARDDISK_ATA_SECONDARY_BUS);
    current_states.secondary_slave = (HARDDISK_ATA_STATE) __harddisk_ata_check_presence(HARDDISK_ATA_SLAVE, HARDDISK_ATA_SECONDARY_BUS);
}

void __harddisk_ata_get_pointers(HARDDISK_ATA_MASTER_SLAVE type, HARDDISK_ATA_BUS_TYPE bus, const HARDDISK_ATA_STATE **state, const harddisk_identify_device_data **data)
{
    *state = NULL;
    *data = NULL;
    switch(type)
    {
    case HARDDISK_ATA_MASTER:
        switch(bus)
        {
            case HARDDISK_ATA_PRIMARY_BUS:
                // Primary master
                *state = &current_states.primary_master;
                *data = &current_states.primary_master_data;
                break;
            case HARDDISK_ATA_SECONDARY_BUS:
                // Secondary master
                *state = &current_states.secondary_master;
                *data = &current_states.secondary_master_data;
                break;
            default:
                return;
        }
        break;
    case HARDDISK_ATA_SLAVE:
        switch(bus)
        {
            case HARDDISK_ATA_PRIMARY_BUS:
                // Primary slave
                *state = &current_states.primary_slave;
                *data = &current_states.primary_slave_data;
                break;
            case HARDDISK_ATA_SECONDARY_BUS:
                // Secondary slave
                *state = &current_states.secondary_slave;
                *data = &current_states.secondary_slave_data;
                break;
            default:
                return;
        }
        break;
    default:
        return;      
    }
}

int8_t __harddisk_ata_check_presence(HARDDISK_ATA_MASTER_SLAVE type, HARDDISK_ATA_BUS_TYPE bus)
{
    uint16_t io_port = 0;
    harddisk_io_drive_head_register message_to_drive = {.value = 0};

    // Set port of drive
    if (bus == HARDDISK_ATA_PRIMARY_BUS) io_port = HARDDISK_ATA_PRIMARY_BUS_IO_PORT;
    else if(bus == HARDDISK_ATA_SECONDARY_BUS) io_port = HARDDISK_ATA_SECONDARY_BUS_IO_PORT;
    else return -2;

    // Set drive
    switch (type)
    {
    case HARDDISK_ATA_MASTER:
        // For master set it to 0xA0. We set 2 bits that should be always 1.
        message_to_drive.fields.always_set_field_1 = 1;
        message_to_drive.fields.always_set_field_2 = 1;
        break;
    case HARDDISK_ATA_SLAVE:
        // For slave set it to 0xB0. We set 2 bits that should be always 1 and drive number to 1.
        message_to_drive.fields.always_set_field_1 = 1;
        message_to_drive.fields.always_set_field_2 = 1;
        message_to_drive.fields.drive_number = 1;
        break;
    default:
        return -2;
    }

    harddisk_identify_device_data *data;
    if(type == HARDDISK_ATA_MASTER && bus == HARDDISK_ATA_PRIMARY_BUS) data = &current_states.primary_master_data;
    else if(type == HARDDISK_ATA_SLAVE && bus == HARDDISK_ATA_PRIMARY_BUS) data = &current_states.primary_slave_data;
    else if(type == HARDDISK_ATA_MASTER && bus == HARDDISK_ATA_SECONDARY_BUS) data = &current_states.secondary_master_data;
    else if(type == HARDDISK_ATA_SLAVE && bus == HARDDISK_ATA_SECONDARY_BUS) data = &current_states.secondary_slave_data;

    // Send message to drive
    io_out_byte(io_port + HARDDISK_IO_DRIVE_HEAD_REGISTER_OFFSET, message_to_drive.value);

    // Make 400ns delay
    __harddisk_ata_400ns_delay(io_port);

    // Set the Sectorcount, LBAlo, LBAmid, and LBAhi IO ports to 0
    io_out_word(io_port + HARDDISK_IO_SECTOR_COUNT_REGISTER_OFFSET, 0);
    io_out_word(io_port + HARDDISK_IO_SECTOR_NUMBER_REGISTER_OFFSET, 0);
    io_out_word(io_port + HARDDISK_IO_CYLINDER_LOW_REGISTER_OFFSET, 0);
    io_out_word(io_port + HARDDISK_IO_CYLINDER_HIGH_REGISTER_OFFSET, 0);

    // Send the IDENTIFY command (0xEC) to the Command IO port.
    io_out_byte(io_port + HARDDISK_IO_COMMAND_REGISTER_OFFSET, HARDDISK_IDENTIFY_DEVICE_COMMAND);

    // Read the Status port again.
    harddisk_io_control_status_register result;
    result.value = io_in_byte(io_port + HARDDISK_IO_STATUS_REGISTER_OFFSET);

    // If the value read is 0, the drive does not exist.
    if(result.value == 0)
    {
        return 0;
    }  
    else
    {   // For any other value: poll the Status port until bit 7 (BSY, value = 0x80) clears.
        for(;;)
        {
            if(result.fields.busy == 0)
            {
                // Otherwise, continue polling one of the Status ports until bit 3 (DRQ, value = 8) sets, or until bit 0 (ERR, value = 1) sets.
                for(;;)
                {
                    result.value = io_in_byte(io_port + HARDDISK_IO_STATUS_REGISTER_OFFSET);
                    if(result.fields.has_pio_data_to_transfer_or_ready_to_accept_pio_data == 1)
                    {
                        for(int i = 0; i < 256; i++)
                        {
                            //  Read 256 16-bit values, and store them.
                            data->values[i] = io_in_word(io_port);
                        }
                        return 1;
                    }
                    else if(result.fields.error_occurred == 1)
                    {
                        return -1;
                    }
                }
            }
            else
            {
                // Because of some ATAPI drives that do not follow spec, at this point you need to check the LBAmid and LBAhi ports (0x1F4 and 0x1F5)
                // to see if they are non-zero. If so, the drive is not ATA, and you should stop polling.
                if(io_in_word(io_port + HARDDISK_IO_CYLINDER_LOW_REGISTER_OFFSET) != 0 || io_in_word(io_port + HARDDISK_IO_CYLINDER_HIGH_REGISTER_OFFSET) != 0)
                {
                    return 0;
                }
            }
            result.value = io_in_byte(io_port + HARDDISK_IO_STATUS_REGISTER_OFFSET);
        }
    }
}

int8_t __harddisk_ata_poll(uint16_t io_port)
{
    harddisk_io_control_status_register result;
    for(;;)
    {
        result.value = io_in_byte(io_port + HARDDISK_IO_STATUS_REGISTER_OFFSET);
        if(result.fields.busy == 0)
        {
            // Otherwise, continue polling one of the Status ports until bit 3 (DRQ, value = 8) sets, or until bit 0 (ERR, value = 1) sets.
            for(;;)
            {
                result.value = io_in_byte(io_port + HARDDISK_IO_STATUS_REGISTER_OFFSET);
                if(result.fields.has_pio_data_to_transfer_or_ready_to_accept_pio_data == 1)
                {
                    return 1;
                }
                else if(result.fields.error_occurred == 1)
                {
                    return -1;
                }
            }
        }
 
    }
}

void __harddisk_ata_400ns_delay(uint16_t port)
{
    for(int i = 0; i < 4; ++i)
    {
        io_in_byte(port + HARDDISK_IO_STATUS_REGISTER_OFFSET);
    }
}