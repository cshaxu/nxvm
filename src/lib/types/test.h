#ifndef LIB_TYPES_TEST_H
#define LIB_TYPES_TEST_H

/* Test assertions remain external C vocabulary, but Types owns their single
 * declaration boundary just as it owns streams and platform declarations. */
#include <assert.h>
#include <stdlib.h>

#define lib_test_assert assert
#define lib_test_at_exit atexit

#endif
