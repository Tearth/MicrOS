#ifndef LOCALE_H
#define LOCALE_H

#include <limits.h>
#include "stdlib.h"
#include "string.h"

//! The entire locale.
#define LC_ALL 0

//! Affects the behavior of strcoll and strxfrm.
#define LC_COLLATE 1

//! Affects character handling functions.
#define LC_CTYPE 2

//! Affects monetary formatting information returned by localeconv.
#define LC_MONETARY 3

//! Affects the decimal-point character in formatted input/output operations and string formatting functions.
#define LC_NUMERIC 4

//! Affects the behavior of strftime.
#define LC_TIME 5

//! Formatting info for numeric values.
typedef struct lconv
{
    //! Decimal-point separator used for non-monetary quantities.
    char *decimal_point;

    //! Separators used to delimit groups of digits to the left of the decimal point for non-monetary quantities.
    char *thousands_sep;

    //! Specifies the amount of digits that form each of the groups to be separated by thousands_sep separator for non-monetary quantities.
    char *grouping;

    //! International currency symbol.
    char *int_curr_symbol;

    //! Local currency symbol.
    char *currency_symbol;

    //! Decimal-point separator used for monetary quantities.
    char *mon_decimal_point;

    //! Separators used to delimit groups of digits to the left of the decimal point for monetary quantities.
    char *mon_thousands_sep;

    //! Specifies the amount of digits that form each of the groups to be separated by mon_thousands_sep separator for monetary quantities.
    char *mon_grouping;

    //! Sign to be used for nonnegative (positive or zero) monetary quantities.
    char *positive_sign;

    //! Sign to be used for negative monetary quantities.
    char *negative_sign;

    //! Amount of fractional digits to the right of the decimal point for monetary quantities in the local format.
    char frac_digits;

    //! Whether the currency symbol should precede nonnegative (positive or zero) monetary quantities.
    char p_cs_precedes;

    //! Whether the currency symbol should precede negative monetary quantities.
    char n_cs_precedes;

    //! Whether a space should appear between the currency symbol and nonnegative (positive or zero) monetary quantities.
    char p_sep_by_space;

    //! Whether a space should appear between the currency symbol and negative monetary quantities.
    char n_sep_by_space;

    //! Position of the sign for nonnegative (positive or zero) monetary quantities.
    char p_sign_posn;

    //! Position of the sign for negative monetary quantities.
    char n_sign_posn;

    //! Same as frac_digits, but for the international format.
    char int_frac_digits;

    //! Same as p_cs_precedes, but for the international format.
    char int_p_cs_precedes;

    //! Same as n_cs_precedes, but for the international format.
    char int_n_cs_precedes;

    //! Same as p_sep_by_space, but for the international format.
    char int_p_sep_by_space;

    //! Same as n_sep_by_space, but for the international format.
    char int_n_sep_by_space;

    //! Same as p_sign_posn, but for the international format.
    char int_p_sign_posn;

    //! Same as n_sign_posn, but for the international format.
    char int_n_sign_posn;
} lconv;

//! Current locale state.
lconv *locale_state;

//! Current locale name.
char *locale_name;

#ifdef __cplusplus
extern "C" {
#endif

//! Set or retrieve locale.
/*!
    Sets locale information to be used by the current program, either changing the entire locale or portions of it. The function can also be used to retrieve the current locale's name by passing NULL as the value for argument locale.
    \param category Portion of the locale affected (LC_ALL, LC_COLLATE, LC_CTYPE, LC_MONETARY, LC_NUMERIC, LC_TIME).
    \param locale C string containing the name of a C locale.
    \return On success, A pointer to a C string identifying the locale currently set for the category. If category is LC_ALL and different portions of the locale are set to different values, the string returned gives this information in a format which may vary between library implementations. If the function failed to set a new locale, this is not modified and a null pointer is returned.
*/
char *setlocale(int category, const char *locale);

//! Get locale formatting parameters for quantities.
/*!
    Returns the length of the C string str. The length of a C string is determined by the terminating null-character.
    \return A pointer to a structure object of the structure type lconv with the corresponding values for the current locale filled in. 
*/
lconv *localeconv();

//! Delete (free) field's value.
/*!
    Delete (free) field's value if it has been initialized previously.
    \param field Field's value to remove.
*/
void __locale_delete_char_field_value(char *field);

//! Set field's value.
/*!
    Allocs memory for the specified field and returns it.
    \param value Newly created value.
*/
char *__locale_set_char_field_value(const char *value);

//! Set locale state to the neutral value.
/*!
    Every field in the locale state will have default value (mostly "" or CHAR_MAX).
    \param category Portion of the locale affected (LC_ALL, LC_COLLATE, LC_CTYPE, LC_MONETARY, LC_NUMERIC, LC_TIME).
*/
void __locale_set_neutral_locale(int category);

#ifdef __cplusplus
}
#endif

#endif