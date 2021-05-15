#include <string.h>
#include <stdint.h>
#include <stdarg.h>
#include <stdio.h>
#include "logger.h"
#include "serial.h"

#define ERROR_LEVEL         "[ERR]"
#define WARNING_LEVEL       "[WRN]"
#define INFO_LEVEL          "[INF]"
#define DEBUG_LEVEL         "[DBG]"
#define UNKNOW_LEVEL        "[UNK]"

#define LOG_BUFF_SIZE       (256)
#define LOG_ERROR_DESC_SIZE (6)

 char* log_level_to_str(uint8_t log_level)
{
    switch(log_level)
    {
        case LOG_LEVEL_ERROR:   return ERROR_LEVEL;
        case LOG_LEVEL_WARNING: return WARNING_LEVEL;
        case LOG_LEVEL_INFO:    return INFO_LEVEL;
        case LOG_LEVEL_DEBUG:   return DEBUG_LEVEL;
        default:                return UNKNOW_LEVEL;
    }
}

void logger_serial_print(uint8_t log_level, const char * format, ...)
{
    if (log_level <= LOG_LEVEL_DEFAULT)
    {
        char log_buf[LOG_BUFF_SIZE];

        memcpy(log_buf, log_level_to_str(log_level), LOG_ERROR_DESC_SIZE);

        va_list args;
        va_start(args, format);
        int size = vsnprintf(log_buf + LOG_ERROR_DESC_SIZE, sizeof(log_buf) - LOG_ERROR_DESC_SIZE, format, args);
        va_end(args);

        if (size > 0)
        {
            serial_send_block((uint8_t*)log_buf, size + LOG_ERROR_DESC_SIZE);
        }
    }
}

void logger_serial_print_arr(uint8_t log_level, const char *p_str, uint8_t *p_data, uint8_t len)
{
    uint8_t log_buf[LOG_BUFF_SIZE];
    int size = snprintf((char *)log_buf, sizeof(log_buf), "%s ", p_str);
    for (uint8_t i = 0; i < len; ++i)
    {
        size += sprintf((char *)log_buf + size, "%02X ", p_data[i]);
    }
    size += sprintf((char *)log_buf + size, "\r\n");
    serial_send_block(log_buf, size);
}