#ifndef KERNEL_DEBUG_H
#define KERNEL_DEBUG_H

int serial_printf(const char *fmt, ...);

#define STR(x) # x

#define END         "\e[0m"
#define LOG_COLOR   "\e[38;5;046m"
#define PANIC_COLOR "\e[38;5;160m"
#define WARN_COLOR  "\e[38;5;166m"

#define LOG(s, ...)\
    serial_printf(LOG_COLOR "[%s#%s:%d] " s END "\n", __FILE__, __func__, __LINE__, ## __VA_ARGS__)

#define PANIC(s, ...)\
    do {\
        serial_printf(PANIC_COLOR "[%s#%s:%d] " s END "\n", __FILE__, __func__, __LINE__, ## __VA_ARGS__);\
        for(;;);\
    } while (0)

#define WARN(s, ...)\
    do {\
        serial_printf(WARN_COLOR "[%s#%s:%d] " s END "\n", __FILE__, __func__, __LINE__, ## __VA_ARGS__);\
    } while (0)

#define TEST(expr, s, ...)\
    do {\
        if (!(expr)) {\
            PANIC("Test on " STR(expr) " failed: " s, ## __VA_ARGS__);\
        }\
    } while (0)

#endif  // KERNEL_DEBUG_H
