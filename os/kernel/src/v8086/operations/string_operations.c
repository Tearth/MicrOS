#include <v8086/mod_rm_parsing.h>
#include <stddef.h>
#include <v8086/memory_operations.h>
#include "string_operations.h"

uint16_t perform_movs(v8086 *machine, uint8_t width) {
    uint16_t* source_segment;
    uint16_t* dest_segment = select_segment_register(machine, ES);
    if(machine->internal_state.segment_reg_select == DEFAULT) source_segment = select_segment_register(machine, DS);
    else source_segment = select_segment_register(machine, machine->internal_state.segment_reg_select);
    if(source_segment == NULL) return UNDEFINED_SEGMENT_REGISTER;
    if(dest_segment == NULL) return UNDEFINED_SEGMENT_REGISTER;

    //if repeat and number of repats == 0 -> dont copy anything
    if(machine->internal_state.rep_prefix == REP_REPE && machine->regs.w.cx == 0) return OK;

    do{
        void* source = NULL;
        if(machine->internal_state.address_32_bit)
            source = get_variable_length_pointer(machine->Memory, get_absolute_address(*source_segment, machine->regs.d.esi), width);
        else
            source = get_variable_length_pointer(machine->Memory, get_absolute_address(*source_segment, machine->regs.w.si), width);
        void* dest = NULL;
        if(machine->internal_state.address_32_bit)
            dest = get_variable_length_pointer(machine->Memory, get_absolute_address(*dest_segment, machine->regs.d.edi), width);
        else
            dest = get_variable_length_pointer(machine->Memory, get_absolute_address(*dest_segment, machine->regs.w.di), width);
        if(width == 8)
            *((uint8_t*)dest) = *((uint8_t*) source);
        else if(width == 16)
            *((uint16_t*)dest) = *((uint16_t*) source);
        else if(width == 32)
            *((uint32_t*)dest) = *((uint32_t*) source);
        else
            return BAD_WIDTH;
        int8_t offset = width / 8;
        if(machine->internal_state.address_32_bit){
            machine->regs.d.esi += bit_get(machine->regs.w.flags, 1u << DIRECTION_FLAG_BIT) ? -offset : offset;
            machine->regs.d.edi += bit_get(machine->regs.w.flags, 1u << DIRECTION_FLAG_BIT) ? -offset : offset;
        }
        else {
            machine->regs.w.si += bit_get(machine->regs.w.flags, 1u << DIRECTION_FLAG_BIT) ? -offset : offset;
            machine->regs.w.di += bit_get(machine->regs.w.flags, 1u << DIRECTION_FLAG_BIT) ? -offset : offset;
        }
    } while(machine->internal_state.rep_prefix == REP_REPE && --(machine->regs.w.cx));

    return OK;
}

uint16_t perform_stos(v8086* machine, uint8_t width)
{
    uint16_t* segment = select_segment_register(machine, ES);
    void* source;
    void* dest;
    if(segment == NULL) return UNDEFINED_SEGMENT_REGISTER;
    //if repeat and number of repats == 0 -> dont copy anything
    if(machine->internal_state.rep_prefix == REP_REPE && machine->regs.w.cx == 0) return OK;

    do{
        if(machine->internal_state.address_32_bit)
            dest = get_variable_length_pointer(machine->Memory, get_absolute_address(*segment, machine->regs.d.edi), width);
        else
            dest = get_variable_length_pointer(machine->Memory, get_absolute_address(*segment, machine->regs.w.di), width);
        source = get_variable_length_register(machine, AL, width);
        if(source == NULL) return UNDEFINED_REGISTER;
        if(width == 8) *((uint8_t*) dest) = *((uint8_t*) source);
        else if(width == 16) *((uint16_t*) dest) = *((uint16_t*) source);
        else if(width == 32) *((uint32_t*) dest) = *((uint32_t*) source);
        else return -1;

        if(machine->internal_state.address_32_bit)
            machine->regs.d.edi += bit_get(machine->regs.w.flags, 1u << DIRECTION_FLAG_BIT) ? -(width/8) : (width/8);
        else
            machine->regs.w.di += bit_get(machine->regs.w.flags, 1u << DIRECTION_FLAG_BIT) ? -(width/8) : (width/8);
    } while(machine->internal_state.rep_prefix == REP_REPE && --(machine->regs.w.cx));

    return OK;
}

uint16_t perform_cmps(v8086* machine, uint8_t width)
{
    uint16_t* source_segment;
    uint16_t* dest_segment = select_segment_register(machine, ES);
    void* source;
    void* dest;
    uint32_t dest_before; //for overflow flag checking
    uint32_t source_before;
    uint64_t result;

    if(machine->internal_state.segment_reg_select == DEFAULT) source_segment = select_segment_register(machine, DS);
    else source_segment = select_segment_register(machine, machine->internal_state.segment_reg_select);
    if(source_segment == NULL) return UNDEFINED_SEGMENT_REGISTER;
    if(dest_segment == NULL) return UNDEFINED_SEGMENT_REGISTER;
    //if repeat and number of repats == 0 -> dont copy anything
    if(machine->internal_state.rep_prefix != NONE && machine->regs.w.cx == 0) return OK;

    do{
        if(machine->internal_state.address_32_bit)
            source = get_variable_length_pointer(machine->Memory, get_absolute_address(*source_segment, machine->regs.d.esi), width);
        else
            source = get_variable_length_pointer(machine->Memory, get_absolute_address(*source_segment, machine->regs.w.si), width);
        if(machine->internal_state.address_32_bit)
            dest = get_variable_length_pointer(machine->Memory, get_absolute_address(*dest_segment, machine->regs.d.edi), width);
        else
            dest = get_variable_length_pointer(machine->Memory, get_absolute_address(*dest_segment, machine->regs.w.di), width);
        if(width == 8){
            dest_before = *((uint8_t*)dest);
            source_before = *((uint8_t*)source);
        } else if(width == 16){
            dest_before = *((uint16_t*)dest);
            source_before = *((uint16_t*)source);
        } else if(width == 32){
            dest_before = *((uint32_t*)dest);
            source_before = *((uint32_t*)source);
        } else return BAD_WIDTH;

        result = dest_before - source_before;
        bit_write(machine->regs.d.eflags, 1u <<CARRY_FLAG_BIT, (result >> width) ? 1 : 0); // CARRY FLAG
        bit_write(machine->regs.d.eflags, 1u <<AUX_CARRY_FLAG_BIT, (((dest_before & 0xfu) - (source_before & 0xfu)) >> 4u) ? 1: 0); //AUX CARRY FLAG
        uint8_t parrity = result & 1u;
        for(uint8_t i = 1; i < 8; i++) parrity ^= (result >> i) & 1u;
        bit_write(machine->regs.d.eflags, 1u <<PARITY_FLAG_BIT, (parrity) ? 1: 0); //PARRITY FLAG
        bit_write(machine-> regs.d.eflags, 1u <<ZERO_FLAG_BIT, result == 0); //ZERO FLAG
        bit_write(machine->regs.d.eflags, 1u <<SIGN_FLAG_BIT, result >> (width - 1u)); //SIGN FLAG
        bit_write(machine->regs.d.eflags, 1u <<OVERFLOW_FLAG_BIT, ((result >> (width - 1u)) != (dest_before >> (width - 1u)))); //OVERFLOW FLAG

        if(machine->internal_state.address_32_bit){
            machine->regs.d.esi += bit_get(machine->regs.w.flags, 1u << DIRECTION_FLAG_BIT) ? -(width/8) : (width/8);
            machine->regs.d.edi += bit_get(machine->regs.w.flags, 1u << DIRECTION_FLAG_BIT) ? -(width/8) : (width/8);
        }
        else {
            machine->regs.w.si += bit_get(machine->regs.w.flags, 1u << DIRECTION_FLAG_BIT) ? -(width/8) : (width/8);
            machine->regs.w.di += bit_get(machine->regs.w.flags, 1u << DIRECTION_FLAG_BIT) ? -(width/8) : (width/8);
        }

        if((machine->internal_state.rep_prefix == REP_REPE && !bit_get(machine->regs.w.flags, ZERO_FLAG_BIT)) ||
          (machine->internal_state.rep_prefix == REPNE && bit_get(machine->regs.w.flags, ZERO_FLAG_BIT))) break;
    } while(machine->internal_state.rep_prefix != NONE && --(machine->regs.w.cx));

    return OK;
}

uint16_t perform_lods(v8086* machine, uint8_t width)
{
    uint16_t* segment;
    void* source;
    void* dest;

    if(machine->internal_state.segment_reg_select == DEFAULT) segment = select_segment_register(machine, DS);
    else segment = select_segment_register(machine, machine->internal_state.segment_reg_select);
    if(segment == NULL) return UNDEFINED_SEGMENT_REGISTER;
    //if repeat and number of repats == 0 -> dont copy anything
    if(machine->internal_state.rep_prefix == REP_REPE && machine->regs.w.cx == 0) return OK;

    do{
        if(machine->internal_state.address_32_bit)
            source = get_variable_length_pointer(machine->Memory, get_absolute_address(*segment, machine->regs.d.esi), width);
        else
            source = get_variable_length_pointer(machine->Memory, get_absolute_address(*segment, machine->regs.w.si), width);
        dest = get_variable_length_register(machine, AL, width);
        if(dest == NULL) return UNDEFINED_REGISTER;
        if(width == 8) *((uint8_t*) dest) = *((uint8_t*) source);
        else if(width == 16) *((uint16_t*) dest) = *((uint16_t*) source);
        else if(width == 32) *((uint32_t*) dest) = *((uint32_t*) source);
        else return BAD_WIDTH;

        if(machine->internal_state.address_32_bit)
            machine->regs.d.esi += bit_get(machine->regs.w.flags, 1u << DIRECTION_FLAG_BIT) ? -(width/8) : (width/8);
        else
            machine->regs.w.si += bit_get(machine->regs.w.flags, 1u << DIRECTION_FLAG_BIT) ? -(width/8) : (width/8);
    }while(machine->internal_state.rep_prefix == REP_REPE && --(machine->regs.w.cx));
    return OK;
}

uint16_t perform_scas(v8086* machine, uint8_t width)
{
    uint16_t* source_segment = select_segment_register(machine, ES);
    void* source;
    void* dest;
    uint32_t dest_before; //for overflow flag checking
    uint32_t source_before;
    uint64_t result;

    if(source_segment == NULL) return UNDEFINED_SEGMENT_REGISTER;
    //if repeat and number of repats == 0 -> dont copy anything
    if(machine->internal_state.rep_prefix != NONE && machine->regs.w.cx == 0) return OK;

    do{
        dest = get_variable_length_register(machine, AX, width);
        if(machine->internal_state.address_32_bit)
            source = get_variable_length_pointer(machine->Memory, get_absolute_address(*source_segment, machine->regs.d.edi), width);
        else
            source = get_variable_length_pointer(machine->Memory, get_absolute_address(*source_segment, machine->regs.w.di), width);
        if(dest == NULL) return UNDEFINED_REGISTER;
        if(width == 8){
            dest_before = *((uint8_t*)dest);
            source_before = *((uint8_t*)source);
        } else if(width == 16){
            dest_before = *((uint16_t*)dest);
            source_before = *((uint16_t*)source);
        } else if(width == 32){
            dest_before = *((uint32_t*)dest);
            source_before = *((uint32_t*)source);
        } else return BAD_WIDTH;

        result = dest_before - source_before;
        bit_write(machine->regs.d.eflags, 1u <<CARRY_FLAG_BIT, (result >> width) ? 1 : 0); // CARRY FLAG
        bit_write(machine->regs.d.eflags, 1u <<AUX_CARRY_FLAG_BIT, (((dest_before & 0xfu) - (source_before & 0xfu)) >> 4u) ? 1: 0); //AUX CARRY FLAG
        uint8_t parrity = result & 1u;
        for(uint8_t i = 1; i < 8; i++) parrity ^= (result >> i) & 1u;
        bit_write(machine->regs.d.eflags, 1u <<PARITY_FLAG_BIT, (parrity) ? 1: 0); //PARRITY FLAG
        bit_write(machine-> regs.d.eflags, 1u <<ZERO_FLAG_BIT, result == 0); //ZERO FLAG
        bit_write(machine->regs.d.eflags, 1u <<SIGN_FLAG_BIT, result >> (width - 1u)); //SIGN FLAG
        bit_write(machine->regs.d.eflags, 1u <<OVERFLOW_FLAG_BIT, ((result >> (width - 1u)) != (dest_before >> (width - 1u)))); //OVERFLOW FLAG

        if(machine->internal_state.address_32_bit)
            machine->regs.d.edi += bit_get(machine->regs.w.flags, 1u << DIRECTION_FLAG_BIT) ? -(width/8) : (width/8);
        else
            machine->regs.w.di += bit_get(machine->regs.w.flags, 1u << DIRECTION_FLAG_BIT) ? -(width/8) : (width/8);

        if((machine->internal_state.rep_prefix == REP_REPE && !bit_get(machine->regs.w.flags, ZERO_FLAG_BIT)) ||
           (machine->internal_state.rep_prefix == REPNE && bit_get(machine->regs.w.flags, ZERO_FLAG_BIT))) break;

    } while(machine->internal_state.rep_prefix != NONE && --(machine->regs.w.cx));
    return OK;
}