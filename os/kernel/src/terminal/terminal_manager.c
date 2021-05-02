#include "terminal_manager.h"
#include "memory/heap/heap.h"
#include "drivers/dal/videocard/videocard.h"
#include "process/manager/process_manager.h"

uint32_t active_terminal_id = 0;
uint32_t next_terminal_id = 0;
terminal_struct* terminals_array = NULL;
uint32_t _terminal_number = 0;

terminal_struct* find_terminal_for_process(uint32_t process_id)
{
    for(uint32_t i=0; i<_terminal_number; i++)
        for(uint32_t j=0; j<terminals_array[i].process_number; j++)
            if(terminals_array[i].attached_processes[j]->id == process_id) return terminals_array + i;
    return NULL;
}

int8_t attach_process_to_terminal(uint32_t terminal_id, process_info* p_info)
{
    for(uint32_t i=0; i<_terminal_number; i++)
    {
        if(terminals_array[i].terminal_id == terminal_id)
        {
            process_info** ptr; 
            
            if(terminals_array[i].attached_processes == NULL)
                ptr = heap_kernel_alloc(sizeof(process_info*), 0);
            else
                ptr = heap_kernel_realloc(
                    terminals_array[i].attached_processes, 
                    sizeof(process_info*) * (terminals_array[i].process_number + 1), 0);
            if(ptr == NULL) return -1;
            terminals_array[i].attached_processes = ptr;
            terminals_array[i].attached_processes[terminals_array[i].process_number] = p_info;
            terminals_array[i].process_number += 1;
            terminals_array[i].active_process = p_info;
            p_info->terminal_id = terminal_id;
            return 0;
        }
    }
    return -1;
}

int8_t dettached_process_from_terminal(process_info* p_info)
{
    bool move = false;
    uint32_t terminal_index;
    for(uint32_t i=0; i<_terminal_number; i++)
    {
        uint32_t ti = i;
        for(uint32_t j=0; j<terminals_array[ti].process_number; j++)
        {
            if(terminals_array[ti].attached_processes[j] == p_info)
            {
                terminal_index = ti; 
                move = true;
                i = _terminal_number;
                
                continue;
            }
            if(move)
                terminals_array[ti].attached_processes[j-1] = terminals_array[ti].attached_processes[j];
        }
    }
     
    if(terminals_array[terminal_index].active_process == p_info)
    {
        uint32_t x = terminals_array[terminal_index].process_number;
        if(x > 1)
            terminals_array[terminal_index].active_process = terminals_array[terminal_index].attached_processes[x-2];
        else
            terminals_array[terminal_index].active_process = NULL;
    }
    terminals_array[terminal_index].attached_processes = heap_kernel_realloc(terminals_array[terminal_index].attached_processes, 
        sizeof(process_info*) * (terminals_array[terminal_index].process_number - 1), 0);
    terminals_array[terminal_index].process_number -= 1;
    return 0;
}

const terminal_struct* get_terminals(uint32_t* terminal_number)
{
    *terminal_number = _terminal_number;
    return terminals_array;
}

void init_terminal_manager()
{
    idt_attach_interrupt_handler(1, terminal_manager_keyboard_interrupt_handler);
}

int8_t create_terminal(uint32_t* terminal_id)
{
    void *ptr;
    if(terminals_array == NULL)
        ptr = heap_kernel_alloc(sizeof(terminal_struct), 0);
    else
        ptr = heap_kernel_realloc(terminals_array, (_terminal_number + 1) * sizeof(terminal_struct), 0);
    if(ptr == NULL) return -1;
    terminals_array = ptr;
    terminals_array[_terminal_number].terminal_id = next_terminal_id;
    terminals_array[_terminal_number].cursor = true;
    terminals_array[_terminal_number].cursor_position_x = 0;
    terminals_array[_terminal_number].cursor_position_y = 0;
    terminals_array[_terminal_number].screen_mode = 0x03;
    terminals_array[_terminal_number].screen_buffer = video_card_create_external_buffer(0x03);
    terminals_array[_terminal_number].active_process = NULL;
    terminals_array[_terminal_number].process_number = 0;
    terminals_array[_terminal_number].attached_processes = NULL;
    active_terminal_id = next_terminal_id;
    *terminal_id = active_terminal_id;
    _terminal_number++;
    next_terminal_id++;
    return 0;
}

int8_t destroy_terminal(uint32_t terminal_id)
{
    bool move = false;
    terminal_struct s;
    for(uint32_t i = 0; i < _terminal_number; i++)
    {
        if(terminals_array[i].terminal_id == terminal_id)
        {
            move = true;
            s = terminals_array[i];
        }
        if(move)
            terminals_array[i-1] = terminals_array[i];
    }

    for(uint32_t i; i<s.process_number; i++)
    {
        process_manager_close_process(s.attached_processes[i]->id, s.attached_processes[i]->is_thread, true);
    }

    heap_kernel_dealloc(s.attached_processes);

    if(_terminal_number == 1){
        heap_kernel_dealloc(terminals_array);
        terminals_array = NULL;
        active_terminal_id = 0;
    }
    else {
        heap_kernel_realloc(terminals_array, (_terminal_number - 1) * sizeof(terminal_struct), 0);
        active_terminal_id = terminals_array[0].terminal_id;
    }
    _terminal_number--;
    return 0;
}

int8_t destroy_active_terminal()
{
    return destroy_active_terminal(active_terminal_id);
}

int8_t switch_active_terminal(uint32_t terminal_id)
{
    for(uint32_t i = 0; i < _terminal_number; i++)
    {
        if(terminals_array[i].terminal_id == terminal_id){
            active_terminal_id = terminal_id;
            video_card_set_video_mode(terminals_array[i].screen_mode);
            video_card_swap_external_buffer(terminals_array[i].screen_buffer, terminals_array[i].screen_mode);
            if(video_card_is_text_mode())
            {
                video_card_set_cursor_pos(terminals_array[i].cursor_position_x, terminals_array[i].cursor_position_y);
                if(terminals_array[i].cursor)
                {
                    video_card_turn_cursor_on();
                }
                else
                {
                    video_card_turn_cursor_off();
                }
            }
            return 0;
        }
    }
    return -1;
}

int8_t next_terminal()
{
    for(uint32_t i = 0; i < _terminal_number; i++)
    {
        if(terminals_array[i].terminal_id == active_terminal_id){
            if(i+1 == _terminal_number)
            {
                active_terminal_id = terminals_array[0].terminal_id;
                video_card_set_video_mode(terminals_array[0].screen_mode);
                video_card_swap_external_buffer(terminals_array[0].screen_buffer, terminals_array[0].screen_mode);
                if(video_card_is_text_mode())
                {
                    video_card_set_cursor_pos(terminals_array[0].cursor_position_x, terminals_array[0].cursor_position_y);
                    if(terminals_array[i].cursor)
                {
                    video_card_turn_cursor_on();
                }
                else
                {
                    video_card_turn_cursor_off();
                }
                }
            }
            else{
                active_terminal_id = terminals_array[i+1].terminal_id;
                video_card_set_video_mode(terminals_array[i+1].screen_mode);
                video_card_swap_external_buffer(terminals_array[i+1].screen_buffer, terminals_array[i+1].screen_mode);
                if(video_card_is_text_mode())
                {
                    video_card_set_cursor_pos(terminals_array[i+1].cursor_position_x, terminals_array[i+1].cursor_position_y);
                    if(terminals_array[i].cursor)
                {
                    video_card_turn_cursor_on();
                }
                else
                {
                    video_card_turn_cursor_off();
                }
                }  
            }
            return 0;
        }
    }
    return -1;
}

int8_t terminal_manager_set_mode(uint32_t process_id, int8_t mode)
{
    terminal_struct* terminal = find_terminal_for_process(process_id);
    if(terminal == NULL) return -1;
    uint8_t* buffer = video_card_create_external_buffer(mode);
    if(buffer == NULL) return -1;
    video_card_destroy_external_buffer(terminal->screen_buffer);
    terminal->screen_buffer = buffer;
    terminal->screen_mode = mode;
    video_card_clear_screen_external_buffer(buffer, mode, &(terminal->cursor_position_x), &(terminal->cursor_position_y));
    if(terminal->terminal_id == active_terminal_id)
    {
        video_card_set_video_mode(mode);
        video_card_clear_screen();
    }
    return mode;
}

int8_t terminal_manager_print_char(uint32_t process_id, char character)
{
    terminal_struct* terminal = find_terminal_for_process(process_id);
    if(terminal == NULL) return -1;
    video_card_print_char_external_buffer(
        terminal->screen_buffer, 
        terminal->screen_mode, 
        &(terminal->cursor_position_x), 
        &(terminal->cursor_position_y), 
        character);
    if(terminal->terminal_id == active_terminal_id) video_card_print_char(character);
    return 0;
}

int8_t terminal_manager_print_char_color(uint32_t process_id, char character, uint8_t color)
{
    terminal_struct* terminal = find_terminal_for_process(process_id);
    if(terminal == NULL) return -1;
    video_card_print_char_color_external_buffer(
        terminal->screen_buffer, 
        terminal->screen_mode, 
        &(terminal->cursor_position_x), 
        &(terminal->cursor_position_y), 
        character, 
        color);
    if(terminal->terminal_id == active_terminal_id) video_card_print_char_color(character, color);
    return 0;
}

int8_t terminal_manager_print_string(uint32_t process_id, const char* string)
{
    terminal_struct* terminal = find_terminal_for_process(process_id);
    if(terminal == NULL) return -1;
    video_card_print_string_external_buffer(
        terminal->screen_buffer, 
        terminal->screen_mode, 
        &(terminal->cursor_position_x), 
        &(terminal->cursor_position_y), 
        string);
    if(terminal->terminal_id == active_terminal_id) video_card_print_string(string);
    return 0;
}

int8_t terminal_manager_print_string_color(uint32_t process_id, const char* string, uint8_t color)
{
    terminal_struct* terminal = find_terminal_for_process(process_id);
    if(terminal == NULL) return -1;
    video_card_print_string_color_external_buffer(
        terminal->screen_buffer, 
        terminal->screen_mode, 
        &(terminal->cursor_position_x), 
        &(terminal->cursor_position_y), 
        string,
        color);
    if(terminal->terminal_id == active_terminal_id) video_card_print_string_color(string, color);
    return 0;
}
int8_t terminal_manager_set_char(uint32_t process_id, uint16_t x, uint16_t y, char character)
{
    terminal_struct* terminal = find_terminal_for_process(process_id);
    if(terminal == NULL) return -1;
    video_card_set_char_external_buffer(
        terminal->screen_buffer, 
        terminal->screen_mode, x, y, character);
    if(terminal->terminal_id == active_terminal_id) video_card_set_char(x, y, character);
    return 0;
}

int8_t terminal_manager_get_char(uint32_t process_id, uint16_t x, uint16_t y, char* character)
{
    terminal_struct* terminal = find_terminal_for_process(process_id);
    if(terminal == NULL) return -1;
    video_card_get_char_external_buffer(
        terminal->screen_buffer,
        terminal->screen_mode,
        x, y, character
    );
    if(terminal->terminal_id == active_terminal_id) video_card_get_char(x, y, character);
    return 0;
}

int8_t terminal_manager_set_color(uint32_t process_id, uint16_t x, uint16_t y, uint8_t color)
{
    terminal_struct* terminal = find_terminal_for_process(process_id);
    if(terminal == NULL) return -1;
    video_card_set_color_external_buffer(
        terminal->screen_buffer,
        terminal->screen_mode,
        x, y, color
    );
    if(terminal->terminal_id == active_terminal_id) video_card_set_color(x, y, color);
    return 0;
}

int8_t terminal_manager_get_color(uint32_t process_id, uint16_t x, uint16_t y, uint8_t* color)
{
    terminal_struct* terminal = find_terminal_for_process(process_id);
    if(terminal == NULL) return -1;
    video_card_get_color_external_buffer(
        terminal->screen_buffer,
        terminal->screen_mode,
        x, y, color
    );
    if(terminal->terminal_id == active_terminal_id) video_card_get_color(x, y, color);
    return 0;
}

int8_t terminal_manager_set_char_and_color(uint32_t process_id, uint16_t x, uint16_t y, char character, uint8_t color)
{
    terminal_struct* terminal = find_terminal_for_process(process_id);
    if(terminal == NULL) return -1;
    video_card_set_char_and_color_external_buffer(
        terminal->screen_buffer,
        terminal->screen_mode,
        x, y, character, color
    );
    return 0;
}

int8_t terminal_manager_get_char_and_color(uint32_t process_id, uint16_t x, uint16_t y, char* character, uint8_t* color)
{
    terminal_struct* terminal = find_terminal_for_process(process_id);
    if(terminal == NULL) return -1;
    video_card_get_char_and_color_external_buffer(
        terminal->screen_buffer,
        terminal->screen_mode,
        x, y, character, color
    );
    if(terminal->terminal_id == active_terminal_id) video_card_get_char_and_color(x, y, character, color);
    return 0;
}

int8_t terminal_manager_set_cursor_pos(uint32_t process_id, uint16_t x, uint16_t y)
{
    terminal_struct* terminal = find_terminal_for_process(process_id);
    if(terminal == NULL) return -1;
    terminal->cursor_position_x = x;
    terminal->cursor_position_y = y;
    if(terminal->terminal_id == active_terminal_id) video_card_set_cursor_pos(x, y);
    return 0; 
}

int8_t terminal_manager_get_cursor_pos(uint32_t process_id, uint16_t* x, uint16_t* y)
{
    terminal_struct* terminal = find_terminal_for_process(process_id);
    if(terminal == NULL) return -1;
    *x = terminal->cursor_position_x;
    *y = terminal->cursor_position_y;
    if(terminal->terminal_id == active_terminal_id) video_card_get_cursor_pos(x, y);
    return 0;
}

int8_t terminal_manager_turn_cursor_on(uint32_t process_id)
{
    terminal_struct* terminal = find_terminal_for_process(process_id);
    if(terminal == NULL) return -1;
    terminal->cursor = true;
    if(terminal->terminal_id == active_terminal_id) video_card_turn_cursor_on();
    return 0;
}
int8_t terminal_manager_turn_cursor_off(uint32_t process_id)
{
    terminal_struct* terminal = find_terminal_for_process(process_id);
    if(terminal == NULL) return -1;
    terminal->cursor = false;
    if(terminal->terminal_id == active_terminal_id) video_card_turn_cursor_off();
    return 0;
}

int8_t terminal_manager_draw_pixel(uint32_t process_id, uint8_t color, uint16_t x, uint16_t y)
{
    terminal_struct* terminal = find_terminal_for_process(process_id);
    if(terminal == NULL) return -1;
    video_card_draw_pixel_external_buffer(
        terminal->screen_buffer,
        terminal->screen_mode,
        color, x, y
    );
    if(terminal->terminal_id == active_terminal_id) video_card_draw_pixel(color, x, y);
    return 0;
}

int8_t terminal_manager_draw_line(uint32_t process_id, uint8_t color, uint16_t ax, uint16_t ay, uint16_t bx, uint16_t by)
{
    terminal_struct* terminal = find_terminal_for_process(process_id);
    if(terminal == NULL) return -1;
    video_card_draw_line_external_buffer(
        terminal->screen_buffer,
        terminal->screen_mode,
        color, ax, ay, bx, by
    );
    if(terminal->terminal_id == active_terminal_id) video_card_draw_line(color, ax, ay, bx, by);
    return 0;
}

int8_t terminal_manager_draw_circle(uint32_t process_id, uint8_t color, uint16_t x, uint16_t y, uint16_t radius)
{
    terminal_struct* terminal = find_terminal_for_process(process_id);
    if(terminal == NULL) return -1;
    video_card_draw_circle_external_buffer(
        terminal->screen_buffer,
        terminal->screen_mode,
        color, x, y, radius
    );
    if(terminal->terminal_id == active_terminal_id) video_card_draw_circle(color, x, y, radius); 
    return 0;
}

int8_t terminal_manager_draw_rectangle(uint32_t process_id, uint8_t color, uint16_t ax, uint16_t ay, uint16_t bx, uint16_t by)
{
    terminal_struct* terminal = find_terminal_for_process(process_id);
    if(terminal == NULL) return -1;
    video_card_draw_rectangle_external_buffer(
        terminal->screen_buffer,
        terminal->screen_mode,
        color, ax, ay, bx, by
    );
    if(terminal->terminal_id == active_terminal_id) video_card_draw_rectangle(color, ax, ay, bx, by);
    return 0;
}

int8_t terminal_manager_clear_screen(uint32_t process_id)
{
    terminal_struct* terminal = find_terminal_for_process(process_id);
    if(terminal == NULL) return -1;
    video_card_clear_screen_external_buffer(terminal->screen_buffer, 
    terminal->screen_mode, &(terminal->cursor_position_x), &(terminal->cursor_position_y));
    if(terminal->terminal_id == active_terminal_id) video_card_clear_screen();
    return 0;
}

uint32_t terminal_manager_get_active_terminal_id()
{
    return active_terminal_id;
}

bool terminal_manager_keyboard_interrupt_handler(interrupt_state *state)
{
    if (!keyboard_is_buffer_empty())
    {
        keyboard_scan_ascii_pair pair;
        keyboard_get_key_from_buffer(&pair);
        keyboard_state_flags *flags = keyboard_get_state_flags();
        
        switch (pair.scancode)
        {
            // Tab
            case 15:
            {
                // if (flags->left_alt_pressed)
                if (flags->left_ctrl_pressed)
                {
                    next_terminal();
                }
                
                return true;
            }
            
            // F1, F2, F3, F4
            case 59:
            case 60:
            case 61:
            case 62:
            {
                switch_active_terminal(pair.scancode - 59);
                break;
            }
            
            default:
            {
                keyboard_put_key_to_buffer(pair.scancode, pair.ascii);
            }
        }
    }
    
    return false;
}

//TODO: Functions for writting to chars and pixels, changing modes, changing active terminal and get list of terminals