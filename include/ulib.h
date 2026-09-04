/**
 * Umbrella header for uLib.
 *
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2021 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 *
 * @file
 */

#ifndef ULIB_H
#define ULIB_H

// IWYU pragma: begin_exports

#include "ualloc.h"
#include "uatomic.h"
#include "uattrs.h"
#include "ubarrier.h"
#include "ubit.h"
#include "ucolor.h"
#include "ucond.h"
#include "udeadline.h"
#include "udebug.h"
#include "uevent.h"
#include "ufutex.h"
#include "uhash.h"
#include "uhash_builtin.h"
#include "uhash_func.h"
#include "uiter.h"
#include "ulatch.h"
#include "uleak.h"
#include "ulib_init.h"
#include "ulib_ret.h"
#include "ulib_ret_t.h"
#include "ulock.h"
#include "ulog.h"
#include "umeta.h"
#include "umetrics.h"
#include "unumber.h"
#include "uonce.h"
#include "uplatform.h"
#include "urand.h"
#include "usem.h"
#include "ustrbuf.h"
#include "ustream.h"
#include "ustream_varint.h"
#include "ustring.h"
#include "ustring_raw.h"
#include "utest.h"
#include "uthread.h"
#include "utime.h"
#include "utime_t.h"
#include "uutils.h"
#include "uvec.h"
#include "uvec_builtin.h"
#include "uversion.h"
#include "uwarning.h"

// IWYU pragma: end_exports

#endif // ULIB_H
