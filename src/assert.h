#ifndef ASSERT_H
#define ASSERT_H
#include <stdbool.h>

void assert_equali(int expected, int actual);
void assert_equalp(const void *const expected, const void *const actual);

void assert_null(const void *const a);

void assert_not_null(const void *const a);

void assert_that(bool expected);

#endif
