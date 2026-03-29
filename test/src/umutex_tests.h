/**
 * Mutex tests
 */

#ifndef UMUTEX_TESTS_H
#define UMUTEX_TESTS_H

void umutex_test_basic(void);
void umutex_test_try_immediate(void);

void urmutex_test_basic(void);
void urmutex_test_recursive(void);
void urmutex_test_try_recursive(void);

void urwmutex_test_basic(void);
void urwmutex_test_try(void);

#ifdef ULIB_MULTITHREAD

void umutex_test_try_timeout_success(void);
void umutex_test_sync(void);

void urmutex_test_try_timeout_success(void);
void urmutex_test_sync(void);

void urwmutex_test_write_blocks_write(void);
void urwmutex_test_write_blocks_read(void);
void urwmutex_test_read_blocks_write(void);
void urwmutex_test_sync(void);

#define UMUTEX_TESTS                                                                               \
    umutex_test_basic, umutex_test_try_immediate, umutex_test_try_timeout_success,                 \
        umutex_test_sync, urmutex_test_basic, urmutex_test_recursive, urmutex_test_try_recursive,  \
        urmutex_test_try_timeout_success, urmutex_test_sync, urwmutex_test_basic,                  \
        urwmutex_test_try, urwmutex_test_write_blocks_write, urwmutex_test_write_blocks_read,      \
        urwmutex_test_read_blocks_write, urwmutex_test_sync

#else

#define UMUTEX_TESTS                                                                               \
    umutex_test_basic, umutex_test_try_immediate, urwmutex_test_basic, urwmutex_test_try,          \
        urmutex_test_basic, urmutex_test_try_recursive, urmutex_test_recursive

#endif // ULIB_MULTITHREAD

#endif // UMUTEX_TESTS_H
