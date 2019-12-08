#ifndef ERROR_H_
#define ERROR_H_

#define ERROR_BASE_NUM  0

typedef enum
{
    ERROR_SUCCESS = ERROR_BASE_NUM,
    ERROR_NO_MEM,
    ERROR_INVALID_PARAM,
    ERROR_DATA_LENGTH,
    ERROR_NOT_FOUND,
    ERROR_NULL_PTR,
    ERROR_INTERNAL,
    ERROR_UNKNOW_CMD,
    ERROR_BUSY,
    ERROR_INVALID_STATE
} error_t;

#endif /* ERROR_H_ */
