#include "../locale.h"

void __locale_delete_char_field_value(char *field)
{
    if (field != NULL)
    {
        free(field);
    }
}

char *__locale_set_char_field_value(const char *value)
{
    uint32_t length = strlen(value) + 1;
    char *field = malloc(length);

    memcpy(field, value, length);
    return field;
}

void __locale_set_neutral_locale(int category)
{
    __locale_delete_char_field_value(locale_state->decimal_point);
    locale_state->decimal_point = __locale_set_char_field_value(".");

    __locale_delete_char_field_value(locale_state->thousands_sep);
    locale_state->thousands_sep = __locale_set_char_field_value("");

    __locale_delete_char_field_value(locale_state->grouping);
    locale_state->grouping = __locale_set_char_field_value("");

    __locale_delete_char_field_value(locale_state->int_curr_symbol);
    locale_state->int_curr_symbol = __locale_set_char_field_value("");

    __locale_delete_char_field_value(locale_state->currency_symbol);
    locale_state->currency_symbol = __locale_set_char_field_value("");

    __locale_delete_char_field_value(locale_state->mon_decimal_point);
    locale_state->mon_decimal_point = __locale_set_char_field_value("");

    __locale_delete_char_field_value(locale_state->mon_thousands_sep);
    locale_state->mon_thousands_sep = __locale_set_char_field_value("");

    __locale_delete_char_field_value(locale_state->mon_grouping);
    locale_state->mon_grouping = __locale_set_char_field_value("");

    __locale_delete_char_field_value(locale_state->positive_sign);
    locale_state->positive_sign = __locale_set_char_field_value("");

    __locale_delete_char_field_value(locale_state->negative_sign);
    locale_state->negative_sign = __locale_set_char_field_value("");

    locale_state->frac_digits = CHAR_MAX;
    locale_state->p_cs_precedes = CHAR_MAX;
    locale_state->n_cs_precedes = CHAR_MAX;
    locale_state->p_sep_by_space = CHAR_MAX;
    locale_state->n_sep_by_space = CHAR_MAX;
    locale_state->p_sign_posn = CHAR_MAX;
    locale_state->n_sign_posn = CHAR_MAX;
    locale_state->int_frac_digits = CHAR_MAX;
    locale_state->int_p_cs_precedes = CHAR_MAX;
    locale_state->int_n_cs_precedes = CHAR_MAX;
    locale_state->int_p_sep_by_space = CHAR_MAX;
    locale_state->int_n_sep_by_space = CHAR_MAX;
    locale_state->int_p_sign_posn = CHAR_MAX;
    locale_state->int_n_sign_posn = CHAR_MAX;
}