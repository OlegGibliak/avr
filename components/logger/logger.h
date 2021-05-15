#ifndef _LOGGER_H__
#define _LOGGER_H__

#define LOG_LEVEL_NO_LOG        (0)
#define LOG_LEVEL_ERROR         (1)
#define LOG_LEVEL_WARNING       (2)
#define LOG_LEVEL_INFO          (3)
#define LOG_LEVEL_DEBUG         (4)

#ifndef LOG_LEVEL
    #define LOG_LEVEL_DEFAULT   LOG_LEVEL_DEBUG
#endif

void logger_serial_print(uint8_t log_level, const char * format, ...);
void logger_serial_print_arr(uint8_t log_level, const char *p_str, uint8_t *p_data, uint8_t len);

#if LOG_LEVEL_DEFAULT == LOG_LEVEL_NO_LOG
    #define __LOG(log_level, ...)       (...)
#else
    #define __LOG(log_level, ...)       logger_serial_print(log_level, ##__VA_ARGS__)
#endif

#define __LOG_XB(log_level, p_str, p_data, len) \
	logger_serial_print_arr(log_level, p_str, p_data, len)
#endif /* _LOGGER_H__ */
