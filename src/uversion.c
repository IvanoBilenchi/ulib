/**
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2021 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 */

#include "uversion.h"
#include "ustrbuf.h"
#include "ustream.h"
#include "ustring.h"
#include <stddef.h>

int uversion_compare(UVersion lhs, UVersion rhs) {
    if (lhs.major < rhs.major) return -1;
    if (lhs.major > rhs.major) return 1;
    if (lhs.minor < rhs.minor) return -1;
    if (lhs.minor > rhs.minor) return 1;
    if (lhs.patch < rhs.patch) return -1;
    if (lhs.patch > rhs.patch) return 1;
    return 0;
}

UString uversion_to_string(UVersion const *version) {
    UOStream stream;
    UStrBuf buf = ustrbuf();

    if (uostream_to_strbuf(&stream, &buf) || uostream_write_version(&stream, version, NULL)) {
        ustrbuf_deinit(&buf);
        return ustring_null;
    }

    return ustrbuf_to_ustring(&buf);
}
