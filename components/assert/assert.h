#ifndef _ASSERT_H__
#define _ASSERT_H__

#include <stdint.h>

#if defined(CONFIG_ASSERT_ENABLE)

void assert_callback(uint16_t line, const char *func_name);

#define ASSERT(expr)                    \
if (!(expr))                            \
{                                       \
    assert_callback(__LINE__, __func__);\
}
#else /* CONFIG_ASSERT_ENABLE */
#define ASSERT(expr)
#endif /* CONFIG_ASSERT_ENABLE */
#endif /* _ASSERT_H__ */
