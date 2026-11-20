#ifndef MYOB300_CARD_IDENTITY_H
#define MYOB300_CARD_IDENTITY_H

#include "lib/types/types_interface.h"

lib_bool myob300_card_identity_matches(const lib_u8 *bytes, lib_size byte_count);

#endif
