#include "drivers/vga/vga.h"
#include "cpu/pic/pic.h"
#include "drivers/keyboard/keyboard.h"
#include "cpu/idt/idt.h"
#include "drivers/keyboard/keyboard.h"
#include "drivers/floppy/floppy.h"
#include "drivers/vga/vga_gmode.h"
#include "drivers/pcspeaker/pc_speaker.h"
#include "drivers/pci/pci.h"
#include "init/startup_music.h"
#include "cpu/timer/timer.h"
#include "logger/logger.h"
#include "cpu/gdt/gdt.h"
#include "cpu/paging/paging.h"
#include "memory/map/memory_map.h"
#include "memory/physical/physical_memory_manager.h"
#include "filesystems/filesystem.h"
#include "cpu/panic/panic_screen.h"
#include "process/elf/parser/elf_parser.h"
#include "process/elf/loader/elf_loader.h"
#include "process/manager/process_manager.h"
#include "process/syscalls/syscalls_manager.h"
#include "process/signals/signals_manager.h"
#include "cpu/tss/tss.h"
#include "drivers/dal/videocard/videocard.h"
#include "drivers/vga/genericvga.h"
#include "cpu/dma/dma.h"
#include "drivers/harddisk/harddisk.h"
#include "drivers/harddisk/ata/harddisk_ata.h"
#include "drivers/harddisk/harddisk_identify_device_data.h"
#include "filesystems/partitions/partitions.h"
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include "terminal/terminal_manager.h"
#include "cpu/cpuid/cpuid.h"

typedef struct _linesStruct
{
    uint16_t ax;
    uint16_t bx;
    uint16_t ay;
    uint16_t by;
    uint8_t color;
} linesStruct;

char buff[50];
linesStruct ssBuffer[64];

//! Prints processor details.
/*! Used during boot to print informations about print processor.
 */
void print_processor_status()
{
    char vendor_string_buffer[13];
    logger_log_info(cpuid_get_vendor_string(vendor_string_buffer));

    char processor_brand_buffer[48];
    logger_log_info(__cpuid_get_processor_brand(processor_brand_buffer));

    char buff[20];
    char buff2[100];
    uint8_t cores = cpuid_number_of_physical_processors_cores();
    itoa(cores, buff, 20);
    strcpy(buff2, "Number of physical cores: ");
    strcat(buff2, buff);
    logger_log_info(buff2);

    if(cores > 1)
    {
        logger_log_warning("But only one core is used :v");
        logger_log_info("For future SKNI members: add support for more cores");
    }

    for(int i = 0; i < cpuid_get_valid_number_cache_entries(); i++)
    {
        cpuid_cache_struct cache = cpuid_get_cache_data(i);

        strcpy(buff2, "Cache level: ");
        itoa(cache.level, buff, 10);
        strcat(buff2, buff);

        switch(cache.type)
        {
            case DATA_CACHE:
                strcat(buff2, ", type: data, size: ");
                break;
            case INSTRUCTION_CACHE:
                strcat(buff2, ", type: instruction, size: ");
                break;
            case UNIFIED_CACHE:
                strcat(buff2, ", type: unified, size: ");
                break;
        }

        uint32_t size = cache.size / 1024;
        if(size < 1024)
        {
            itoa(size, buff, 10);
            strcat(buff2, buff);
            strcat(buff2, " KiB");
        }
        else
        {
            itoa(size / 1024, buff, 10);
            strcat(buff2, buff);
            strcat(buff2, " MiB");
        }

        logger_log_info(buff2);
    }
}

//! Prints hard disk detail.
/*! Used during boot to print informations about hard disk.
    \param type Type of hard disk.
    \param bus Type of bus for hard disk.
    \param name Name for hard disk eg. "Primary Master", that is printed during boot to specify disk.
 */
void print_harddisk_details(HARDDISK_ATA_MASTER_SLAVE type, HARDDISK_ATA_BUS_TYPE bus, char* name)
{
    char buff[50];
    char buff2[100];
    HARDDISK_STATE state = harddisk_get_state(type, bus);

    if(state == HARDDISK_ATA_PRESENT)
    {
        strcpy(buff2, name);
        strcat(buff2, ": ATA device");
        logger_log_info(buff2);

        harddisk_get_disk_model_number_terminated(type, bus, buff);
        strcpy(buff2, "Model name: ");
        strcat(buff2, buff);
        logger_log_info(buff2);

        harddisk_get_disk_firmware_version_terminated(type, bus, buff);
        strcpy(buff2, "Firmware version: ");
        strcat(buff2, buff);
        logger_log_info(buff2);

        harddisk_get_disk_serial_number_terminated(type, bus, buff);
        strcpy(buff2, "Serial number: ");
        strcat(buff2, buff);
        logger_log_info(buff2);

        itoa(harddisk_get_user_addressable_sectors(type, bus), buff, 10);
        strcpy(buff2, "Total number of user addressable sectors: ");
        strcat(buff2, buff);
        logger_log_info(buff2);

        itoa(harddisk_get_disk_space(type, bus) / (1024 * 1024), buff, 10);
        strcpy(buff2, "Total number of megabytes: ");
        strcat(buff2, buff);
        strcat(buff2, " MB");
        logger_log_info(buff2);

        if(harddisk_get_is_removable_media_device(type, bus) == true)
        {
            logger_log_info("Removable media: true");
        }
        else
        {
            logger_log_info("Removable media: false");
        }
    }
    else if(state == HARDDISK_ATAPI_PRESENT)
    {
        strcpy(buff2, name);
        strcat(buff2, ": ATAPI device");
        logger_log_info(buff2);

        harddisk_get_disk_model_number_terminated(type, bus, buff);
        strcpy(buff2, "Model name: ");
        strcat(buff2, buff);
        logger_log_info(buff2);

        harddisk_get_disk_firmware_version_terminated(type, bus, buff);
        strcpy(buff2, "Firmware version: ");
        strcat(buff2, buff);
        logger_log_info(buff2);

        harddisk_get_disk_serial_number_terminated(type, bus, buff);
        strcpy(buff2, "Serial number: ");
        strcat(buff2, buff);
        logger_log_info(buff2);

        if(harddisk_get_is_removable_media_device(type, bus) == true)
        {
            logger_log_info("Removable media: true");
        }
        else
        {
            logger_log_info("Removable media: false");
        }
    }
    else if(state == HARDDISK_NOT_PRESENT)
    {
        strcpy(buff2, name);
        strcat(buff2, ": not detected");
        logger_log_info(buff2);
    }
    else
    {
        strcpy(buff2, name);
        strcat(buff2, ": error or not supported device");
        logger_log_info(buff2);
    }
}

//! Prints hard disks details.
/*! Used during boot to print informations about all hard disks.
 */
void print_harddisks_status()
{
    print_harddisk_details(HARDDISK_ATA_MASTER, HARDDISK_ATA_PRIMARY_BUS, "Primary Master");
    print_harddisk_details(HARDDISK_ATA_SLAVE, HARDDISK_ATA_PRIMARY_BUS, "Primary Slave");
    print_harddisk_details(HARDDISK_ATA_MASTER, HARDDISK_ATA_SECONDARY_BUS, "Secondary Master");
    print_harddisk_details(HARDDISK_ATA_SLAVE, HARDDISK_ATA_SECONDARY_BUS, "Secondary Slave");
}

void startup()
{
    // Must be done before any VGA operation
    gdt_init();
    paging_init();

    //Don't use VGA before calling VGA init
    vga_init(VGA_MODE_03H);
    logger_log_info("MicrOS is starting...");
    logger_log_ok("BASIC TEXT VGA Driver");

    cpuid_init();
    logger_log_ok("Procesor");
    print_processor_status();
    
    //Loading Generic VGA Driver
    generic_vga_driver_init();
    logger_log_ok("Loaded DAL, and Generic VGA Driver");

    physical_memory_init();
    logger_log_ok("Physical Memory");

    heap_set_kernel_heap((void *)((uint32_t)774 * 1024 * 1024 * 4));
    heap_init_kernel_heap();
    logger_log_ok("Virtual Memory");

    pic_init();
    logger_log_ok("Programmable Interrupt Controller");

    idt_init();
    logger_log_ok("Interrupt Descriptor Table");

    timer_init();
    logger_log_ok("Timer");

    dma_init(0xc0000500);
    logger_log_ok("DMA");

    // NOTE: it doesn't work well, so assume for now that floppy controller is always present
    // if(fdc_is_present())
    {
        fdc_init();
        logger_log_ok("Floppy Disc Controller");
    }
    
    harddisk_init();
    logger_log_ok("Hard Disks");
    print_harddisks_status();
    
    partitions_init();
    logger_log_ok("Partitions");

    keyboard_init();
    logger_log_ok("Keyboard");

    timer_init();
    logger_log_ok("Timer");

    tss_init();
    logger_log_ok("TSS");

    syscalls_manager_init();
    logger_log_ok("Syscalls manager");

    signals_manager_init();
    logger_log_ok("Signals manager");

    /*pci_init();
    logger_log_ok("PCI");
    logger_log_info("Number of devices: ");
    uint8_t nd = pci_get_number_of_devices();
    logger_log_info(itoa(nd, buff, 10));
    logger_log_info("Devices: ");
    vga_color col;
    col.color_without_blink.background = VGA_COLOR_BLACK;
    col.color_without_blink.letter = VGA_COLOR_BLUE;
    vga_printstring_color("vendor_id ", &col);
    col.color_without_blink.letter = VGA_COLOR_GREEN;
    vga_printstring_color("device_id ", &col);
    col.color_without_blink.letter = VGA_COLOR_LIGHT_BLUE;
    vga_printstring_color("header_type ", &col);
    col.color_without_blink.letter = VGA_COLOR_LIGHT_RED;
    vga_printstring_color("class_code ", &col);
    col.color_without_blink.letter = VGA_COLOR_YELLOW;
    vga_printstring_color("subclass ", &col);
    col.color_without_blink.letter = VGA_COLOR_MAGENTA;
    vga_printstring_color("prog_if\n", &col);
    for (int i = 0; i < pci_get_number_of_devices(); i++)
    {
        pci_device *dev = pci_get_device(i);
        col.color_without_blink.letter = VGA_COLOR_BLUE;
        vga_printstring_color(itoa(dev->vendor_id, buff, 16), &col);
        vga_printchar(' ');
        col.color_without_blink.letter = VGA_COLOR_GREEN;
        vga_printstring_color(itoa(dev->device_id, buff, 16), &col);
        vga_printchar(' ');
        col.color_without_blink.letter = VGA_COLOR_LIGHT_BLUE;
        vga_printstring_color(itoa(dev->header_type, buff, 16), &col);
        vga_printchar(' ');
        col.color_without_blink.letter = VGA_COLOR_LIGHT_RED;
        vga_printstring_color(itoa(dev->class_code, buff, 16), &col);
        vga_printchar(' ');
        col.color_without_blink.letter = VGA_COLOR_YELLOW;
        vga_printstring_color(itoa(dev->subclass, buff, 16), &col);
        vga_printchar(' ');
        col.color_without_blink.letter = VGA_COLOR_MAGENTA;
        vga_printstring_color(itoa(dev->prog_if, buff, 16), &col);
        vga_printchar('\n');
    }
    pci_dev* dev = get_device(0);
    log_info(itoa(dev->vendor_id, buff, 16));
    log_info(itoa(dev->header_type, buff, 16));
    log_info(itoa(dev->class_code, buff, 16));
    log_info(itoa(dev->subclass, buff, 16));
    log_info(itoa(dev->prog_if, buff, 16));*/
    //fat_init();
    //logger_log_ok("FAT12");
    
    init_terminal_manager();
    logger_log_ok("Terminal manager");

    process_manager_init();
    logger_log_ok("Process manager");

    logger_log_info("MicrOS ready");
    logger_log_info("Created by Application Section of SKNI KOD");
    logger_log_info("Development version");
}

void clear_bss()
{
    extern uint32_t BSS_SECTION_START;
    extern uint32_t BSS_SECTION_END;

    void *bss_start_addr = &BSS_SECTION_START;
    void *bss_end_addr = &BSS_SECTION_END;
    uint32_t bss_length = bss_end_addr - bss_start_addr;

    memset(bss_start_addr, 0, bss_length);
}

int kmain()
{
    clear_bss();

    startup();
    logger_log_info("Hello, World!");
    //startup_music_play();
    logger_log_ok("READY.");
    
    //while (1);
    
    logger_log_ok("Loading shells...");
    
    // create_terminal(&d);
    // create_terminal(&d);
    
    uint32_t d = 0;
    for (int i = 0; i < 4; i++)
    {
        char args[16];
        itoa(i, args, 10);
        
        uint32_t p = process_manager_create_process("A:/ENV/SHELL.ELF", args, 0, false);
        create_terminal(&d);
    
        uint32_t terminal_number = i;
        const terminal_struct* ts = get_terminals(&terminal_number);
        attach_process_to_terminal(ts[i].terminal_id, process_manager_get_process(p));
    }
    
    vga_clear_screen();
    switch_active_terminal(0);
    
    // terminal_manager_print_string(p, "CIASTKO");
    // p = process_manager_create_process("A:/ENV/SHELL.ELF", "", 1000, false);
    // attach_process_to_terminal(ts[0].terminal_id, process_manager_get_process(p));
    // terminal_manager_print_string(p, "KARMEL");
    // p = process_manager_create_process("A:/ENV/SHELL.ELF", "", 1000, false);
    // attach_process_to_terminal(ts[1].terminal_id, process_manager_get_process(p));
    // terminal_manager_print_string(p, "CZEKOLAAAAAAAAADA!");
    //terminal_manager_print_string(ts[1].active_process->id, "KARMAEL");
    //terminal_manager_print_string(ts[2].active_process->id, "CZEKOLADA!");
    process_manager_run();
    //destroy_active_terminal();

    while (1);
    // {
    //     sleep(5000);
    //     next_terminal();
    //     
    // }
    //    ;
    /*char buff[50];
    video_mode *currentMode;
    srand(clock());
    char shouldDrawLines = 0;
    char screenSaver = 0;
    while (1)
    {
        if (!keyboard_is_buffer_empty())
        {
            keyboard_scan_ascii_pair c;
            keyboard_get_key_from_buffer(&c);
            if (c.scancode == 59) //F1
            {
                video_card_set_video_mode(0x3);
                logger_log_ok("UDALO SIE WSKOCZYC na 0x03\n");
                currentMode = video_card_get_current_video_mode();
                vga_printstring("Szerokosc Trybu: ");
                itoa(currentMode->width, buff, 10);
                vga_printstring(buff);
                vga_newline();
                vga_printstring("Wysokosc Trybu: ");
                itoa(currentMode->height, buff, 10);
                vga_printstring(buff);
                vga_newline();
                vga_printstring("Ilosc Kolorow: ");
                itoa(currentMode->colors, buff, 10);
                vga_printstring(buff);
                vga_newline();
                vga_printstring("Monochromatyczny: ");
                itoa(currentMode->monochrome, buff, 10);
                vga_printstring(buff);
                vga_newline();
                vga_printstring("Pamiec platowa: ");
                itoa(currentMode->planar, buff, 10);
                vga_printstring(buff);
                vga_newline();
                vga_printstring("-------TESTY INNYCH FUNKCJI-------:\n");
                vga_printstring("Czy tekstowy: ");
                itoa(video_card_is_text_mode(), buff, 10);
                vga_printstring(buff);
                vga_newline();
            }
            else if (c.scancode == 60) //F2
            {
                video_card_set_video_mode(0x03);
                currentMode = video_card_get_current_video_mode();
                for (uint32_t x = 0; x < currentMode->width; x++)
                    for (uint32_t y = 0; y < currentMode->height; y++)
                        video_card_draw_pixel(0x01, x, y);
            }
            else if (c.scancode == 2) //1
            {
                video_card_set_video_mode(0x04);
                currentMode = video_card_get_current_video_mode();
                video_card_turn_on_buffer();
                for (uint32_t x = 0; x < currentMode->width; x++)
                    for (uint32_t y = 0; y < currentMode->height; y++)
                        video_card_draw_pixel(0x02, x, y);
                video_card_swap_buffers();
            }
            else if (c.scancode == 61) //F3
            {
                video_card_set_video_mode(0x05);
                currentMode = video_card_get_current_video_mode();
                for (uint32_t x = 0; x < currentMode->width; x++)
                    for (uint32_t y = 0; y < currentMode->height; y++)
                        video_card_draw_pixel(0x03, x, y);
            }
            else if (c.scancode == 3) //2
            {
                video_card_set_video_mode(0x05);
                currentMode = video_card_get_current_video_mode();
                video_card_turn_on_buffer();
                for (uint32_t x = 0; x < currentMode->width; x++)
                    for (uint32_t y = 0; y < currentMode->height; y++)
                        video_card_draw_pixel(0x01, x, y);
                video_card_swap_buffers();
            }
            else if (c.scancode == 62) //F4
            {
                video_card_set_video_mode(0x06);
                currentMode = video_card_get_current_video_mode();
                for (uint32_t x = 0; x < currentMode->width; x++)
                    for (uint32_t y = 0; y < currentMode->height; y++)
                        video_card_draw_pixel(0x01, x, y);
            }
            else if (c.scancode == 4) //3
            {
                video_card_set_video_mode(0x06);
                currentMode = video_card_get_current_video_mode();
                video_card_turn_on_buffer();
                for (uint32_t x = 0; x < currentMode->width; x++)
                    for (uint32_t y = 0; y < currentMode->height; y++)
                        video_card_draw_pixel(0x01, x, y);
                video_card_swap_buffers();
            }
            else if (c.scancode == 63) //F5
            {
                video_card_set_video_mode(0x0d);
                currentMode = video_card_get_current_video_mode();
                for (uint32_t x = 0; x < currentMode->width; x++)
                    for (uint32_t y = 0; y < currentMode->height; y++)
                        video_card_draw_pixel(0x04, x, y);
            }
            else if (c.scancode == 5) //4
            {
                video_card_set_video_mode(0x0d);
                currentMode = video_card_get_current_video_mode();
                video_card_turn_on_buffer();
                for (uint32_t x = 0; x < currentMode->width; x++)
                    for (uint32_t y = 0; y < currentMode->height; y++)
                        video_card_draw_pixel(0x05, x, y);
                video_card_swap_buffers();
            }
            else if (c.scancode == 64) //F6
            {
                video_card_set_video_mode(0x0e);
                currentMode = video_card_get_current_video_mode();
                for (uint32_t x = 0; x < currentMode->width; x++)
                    for (uint32_t y = 0; y < currentMode->height; y++)
                        video_card_draw_pixel(0x06, x, y);
            }
            else if (c.scancode == 6) //5
            {
                video_card_set_video_mode(0x0e);
                currentMode = video_card_get_current_video_mode();
                video_card_turn_on_buffer();
                for (uint32_t x = 0; x < currentMode->width; x++)
                    for (uint32_t y = 0; y < currentMode->height; y++)
                        video_card_draw_pixel(0x07, x, y);
                video_card_swap_buffers();
            }
            else if (c.scancode == 65) //F7
            {
                video_card_set_video_mode(0x0f);
                currentMode = video_card_get_current_video_mode();
                for (uint32_t x = 0; x < currentMode->width; x++)
                    for (uint32_t y = 0; y < currentMode->height; y++)
                        video_card_draw_pixel(0x01, x, y);
            }
            else if (c.scancode == 7) //6
            {
                video_card_set_video_mode(0x0f);
                currentMode = video_card_get_current_video_mode();
                video_card_turn_on_buffer();
                for (uint32_t x = 0; x < currentMode->width; x++)
                    for (uint32_t y = 0; y < currentMode->height; y++)
                        video_card_draw_pixel(0x02, x, y);
                video_card_swap_buffers();
            }
            else if (c.scancode == 66) //F8
            {
                video_card_set_video_mode(0x10);
                currentMode = video_card_get_current_video_mode();
                for (uint32_t x = 0; x < currentMode->width; x++)
                    for (uint32_t y = 0; y < currentMode->height; y++)
                        video_card_draw_pixel(0x08, x, y);
            }
            else if (c.scancode == 8) //7
            {
                video_card_set_video_mode(0x10);
                currentMode = video_card_get_current_video_mode();
                video_card_turn_on_buffer();
                for (uint32_t x = 0; x < currentMode->width; x++)
                    for (uint32_t y = 0; y < currentMode->height; y++)
                        video_card_draw_pixel(0x09, x, y);
                video_card_swap_buffers();
            }
            else if (c.scancode == 67) //F9
            {
                video_card_set_video_mode(0x11);
                currentMode = video_card_get_current_video_mode();
                for (uint32_t x = 0; x < currentMode->width; x++)
                    for (uint32_t y = 0; y < currentMode->height; y++)
                        video_card_draw_pixel(0x01, x, y);
            }
            else if (c.scancode == 9) //8
            {
                video_card_set_video_mode(0x11);
                currentMode = video_card_get_current_video_mode();
                video_card_turn_on_buffer();
                for (uint32_t x = 0; x < currentMode->width; x++)
                    for (uint32_t y = 0; y < currentMode->height; y++)
                        video_card_draw_pixel(0x01, x, y);
                video_card_swap_buffers();
            }
            else if (c.scancode == 68) //F10
            {
                video_card_set_video_mode(0x12);
                currentMode = video_card_get_current_video_mode();
                for (uint32_t x = 0; x < currentMode->width; x++)
                    for (uint32_t y = 0; y < currentMode->height; y++)
                        video_card_draw_pixel(0x0A, x, y);
            }
            else if (c.scancode == 10) //9
            {
                video_card_set_video_mode(0x12);
                currentMode = video_card_get_current_video_mode();
                video_card_turn_on_buffer();
                for (uint32_t x = 0; x < currentMode->width; x++)
                    for (uint32_t y = 0; y < currentMode->height; y++)
                        video_card_draw_pixel(0x0B, x, y);
                video_card_swap_buffers();
            }
            else if (c.scancode == 87) //F11
            {
                video_card_set_video_mode(0x13);
                currentMode = video_card_get_current_video_mode();
                for (uint32_t x = 0; x < currentMode->width; x++)
                    for (uint32_t y = 0; y < currentMode->height; y++)
                        video_card_draw_pixel(0x10, x, y);
            }
            else if (c.scancode == 11) //0
            {
                video_card_set_video_mode(0x13);
                currentMode = video_card_get_current_video_mode();
                video_card_turn_on_buffer();
                for (uint32_t x = 0; x < currentMode->width; x++)
                    for (uint32_t y = 0; y < currentMode->height; y++)
                        video_card_draw_pixel(0x25, x, y);
                video_card_swap_buffers();
            }
            else if (c.scancode == 88) //F12
            {
                video_card_set_video_mode(0x69);
                currentMode = video_card_get_current_video_mode();
                for (uint32_t x = 0; x < currentMode->width; x++)
                    for (uint32_t y = 0; y < currentMode->height; y++)
                        video_card_draw_pixel(0x45, x, y);
            }
            else if (c.scancode == 12) //-
            {
                video_card_set_video_mode(0x69);
                currentMode = video_card_get_current_video_mode();
                video_card_turn_on_buffer();
                for (uint32_t x = 0; x < currentMode->width; x++)
                    for (uint32_t y = 0; y < currentMode->height; y++)
                        video_card_draw_pixel(0x8, x, y);
                video_card_swap_buffers();
            }
            else if (c.scancode == 01) //ESC
            {
                video_card_clear_screen();
                if (video_card_is_buffer_on())
                    video_card_swap_buffers();
            }
            else if (c.scancode == 15) // TAB
            {
                currentMode = video_card_get_current_video_mode();
                uint8_t color = (rand() % (currentMode->colors - 1) + 1);
                for (uint32_t x = 0; x < currentMode->width; x++)
                    for (uint32_t y = 0; y < currentMode->height; y++)
                        video_card_draw_pixel(color, x, y);
                if (video_card_is_buffer_on())
                    video_card_swap_buffers();
            }
            else if (c.scancode == 26) //[
            {
                video_card_turn_on_buffer();
            }
            else if (c.scancode == 27) //]
            {
                video_card_turn_off_buffer();
            }
            else if (c.scancode == 41) //`
            {
                screenSaver = 0;
                shouldDrawLines = !shouldDrawLines;
            }
            else if (c.scancode == 13) //=
            {
                shouldDrawLines = 0;
                screenSaver = !screenSaver;
            }
            else
            {
                if (video_card_is_text_mode())
                    vga_printchar(c.ascii);
            }
        }
        if (shouldDrawLines)
        {
            currentMode = video_card_get_current_video_mode();
            uint8_t color = (rand() % (currentMode->colors - 1) + 1);
            uint16_t ax = (rand() % (currentMode->width));
            uint16_t ay = (rand() % (currentMode->height));
            uint16_t bx = (rand() % (currentMode->width));
            uint16_t by = (rand() % (currentMode->height));
            video_card_draw_line(color, ax, ay, bx, by);
            if (video_card_is_buffer_on())
                video_card_swap_buffers();
        }
        if (screenSaver)
        {
            if (video_card_is_buffer_on())
            {
                linesStruct s;
                currentMode = video_card_get_current_video_mode();
                s.color = (rand() % (currentMode->colors - 1) + 1);
                //s.ax = (rand() % (currentMode->width));
                //s.ay = (rand() % (currentMode->height));
                s.ax = ssBuffer[63].bx;
                s.ay = ssBuffer[63].by;
                s.bx = (rand() % (currentMode->width));
                s.by = (rand() % (currentMode->height));
                video_card_clear_screen();
                for (int i = 0; i < 64; i++)
                {
                    video_card_draw_line(ssBuffer[i].color, ssBuffer[i].ax, ssBuffer[i].ay, ssBuffer[i].bx, ssBuffer[i].by);
                    if (i)
                        ssBuffer[i - 1] = ssBuffer[i];
                }
                ssBuffer[63] = s;
                //video_card_draw_line(color, ax, ay, bx, by);
                video_card_swap_buffers();
            }
        }
    }
    */
    return 0;
}
