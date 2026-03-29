/**
 * Threading tests
 */

#ifndef UTHREAD_TESTS_H
#define UTHREAD_TESTS_H

void uthread_test_sleep(void);

#ifdef ULIB_MULTITHREAD
void uthread_test_base(void);
void uthread_test_join(void);
void uthread_test_failing(void);

#define UTHREAD_TESTS uthread_test_base, uthread_test_join, uthread_test_failing, uthread_test_sleep
#else

void ulib_test_no_multithread(void);

#define UTHREAD_TESTS ulib_test_no_multithread, uthread_test_sleep
#endif

#endif // UTHREAD_TESTS_H
