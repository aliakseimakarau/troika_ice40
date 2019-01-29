#ifndef HASH_H
#define HASH_H

#include <stdint.h>
#define NUMBER_OF_TRITS_IN_A_TRYTE 3
#define HASH_LENGTH 243
#define STATE_LENGTH 3 * HASH_LENGTH
#define TRYTE_LENGTH 2673
#define TRANSACTION_LENGTH TRYTE_LENGTH * 3
typedef int64_t bc_trit_t;
typedef char trit_t;

#endif
