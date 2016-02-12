#include "serial.h"
#include <inc/stdio.h>
#include <inc/stdarg.h>

static void
putchar(char ch, int *count)
{
    serial_putchar(ch);
    *count += 1;
}

int
serial_printf(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    int n = 0;
    vprintfmt((void *)putchar, &n, fmt, args);  // ......
    return n;
}
