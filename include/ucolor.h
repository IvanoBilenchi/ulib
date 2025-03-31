/**
 * ANSI color codes for terminal output.
 *
 * @author Ivano Bilenchi
 *
 * @copyright Copyright (c) 2025 Ivano Bilenchi <https://ivanobilenchi.com>
 * @copyright SPDX-License-Identifier: ISC
 *
 * @file
 */

#ifndef UCOLOR_H
#define UCOLOR_H

/**
 * @defgroup color_ansi ANSI color codes
 * @{
 */

/// Black text.
#define UCOLOR_BLK "\033[0;30m"

/// Red text.
#define UCOLOR_RED "\033[0;31m"

/// Green text.
#define UCOLOR_GRN "\033[0;32m"

/// Yellow text.
#define UCOLOR_YEL "\033[0;33m"

/// Blue text.
#define UCOLOR_BLU "\033[0;34m"

/// Magenta text.
#define UCOLOR_MAG "\033[0;35m"

/// Cyan text.
#define UCOLOR_CYN "\033[0;36m"

/// White text.
#define UCOLOR_WHT "\033[0;37m"

/// Bold black text.
#define UCOLOR_BBLK "\033[1;30m"

/// Bold red text.
#define UCOLOR_BRED "\033[1;31m"

/// Bold green text.
#define UCOLOR_BGRN "\033[1;32m"

/// Bold yellow text.
#define UCOLOR_BYEL "\033[1;33m"

/// Bold blue text.
#define UCOLOR_BBLU "\033[1;34m"

/// Bold magenta text.
#define UCOLOR_BMAG "\033[1;35m"

/// Bold cyan text.
#define UCOLOR_BCYN "\033[1;36m"

/// Bold white text.
#define UCOLOR_BWHT "\033[1;37m"

/// Underlined black text.
#define UCOLOR_UBLK "\033[4;30m"

/// Underlined red text.
#define UCOLOR_URED "\033[4;31m"

/// Underlined green text.
#define UCOLOR_UGRN "\033[4;32m"

/// Underlined yellow text.
#define UCOLOR_UYEL "\033[4;33m"

/// Underlined blue text.
#define UCOLOR_UBLU "\033[4;34m"

/// Underlined magenta text.
#define UCOLOR_UMAG "\033[4;35m"

/// Underlined cyan text.
#define UCOLOR_UCYN "\033[4;36m"

/// Underlined white text.
#define UCOLOR_UWHT "\033[4;37m"

/// Black background.
#define UCOLOR_BLKB "\033[40m"

/// Red background.
#define UCOLOR_REDB "\033[41m"

/// Green background.
#define UCOLOR_GRNB "\033[42m"

/// Yellow background.
#define UCOLOR_YELB "\033[43m"

/// Blue background.
#define UCOLOR_BLUB "\033[44m"

/// Magenta background.
#define UCOLOR_MAGB "\033[45m"

/// Cyan background.
#define UCOLOR_CYNB "\033[46m"

/// White background.
#define UCOLOR_WHTB "\033[47m"

/// High intensity black background.
#define UCOLOR_BLKHB "\033[0;100m"

/// High intensity red background.
#define UCOLOR_REDHB "\033[0;101m"

/// High intensity green background.
#define UCOLOR_GRNHB "\033[0;102m"

/// High intensity yellow background.
#define UCOLOR_YELHB "\033[0;103m"

/// High intensity blue background.
#define UCOLOR_BLUHB "\033[0;104m"

/// High intensity magenta background.
#define UCOLOR_MAGHB "\033[0;105m"

/// High intensity cyan background.
#define UCOLOR_CYNHB "\033[0;106m"

/// High intensity white background.
#define UCOLOR_WHTHB "\033[0;107m"

/// High intensity black text.
#define UCOLOR_HBLK "\033[0;90m"

/// High intensity red text.
#define UCOLOR_HRED "\033[0;91m"

/// High intensity green text.
#define UCOLOR_HGRN "\033[0;92m"

/// High intensity yellow text.
#define UCOLOR_HYEL "\033[0;93m"

/// High intensity blue text.
#define UCOLOR_HBLU "\033[0;94m"

/// High intensity magenta text.
#define UCOLOR_HMAG "\033[0;95m"

/// High intensity cyan text.
#define UCOLOR_HCYN "\033[0;96m"

/// High intensity white text.
#define UCOLOR_HWHT "\033[0;97m"

/// Bold high intensity black text.
#define UCOLOR_BHBLK "\033[1;90m"

/// Bold high intensity red text.
#define UCOLOR_BHRED "\033[1;91m"

/// Bold high intensity green text.
#define UCOLOR_BHGRN "\033[1;92m"

/// Bold high intensity yellow text.
#define UCOLOR_BHYEL "\033[1;93m"

/// Bold high intensity blue text.
#define UCOLOR_BHBLU "\033[1;94m"

/// Bold high intensity magenta text.
#define UCOLOR_BHMAG "\033[1;95m"

/// Bold high intensity cyan text.
#define UCOLOR_BHCYN "\033[1;96m"

/// Bold high intensity white text.
#define UCOLOR_BHWHT "\033[1;97m"

/// Reset all attributes.
#define UCOLOR_RST "\033[0m"

/// @}

/**
 * @defgroup color_semantic Semantic colors
 * @{
 */

/// Color for dim text.
#ifndef UCOLOR_DIM
#define UCOLOR_DIM UCOLOR_HBLK
#endif

/// Color for trace messages.
#ifndef UCOLOR_TRACE
#define UCOLOR_TRACE UCOLOR_BLU
#endif

/// Color for debug messages.
#ifndef UCOLOR_DEBUG
#define UCOLOR_DEBUG UCOLOR_CYN
#endif

/// Color for informational messages.
#ifndef UCOLOR_INFO
#define UCOLOR_INFO UCOLOR_GRN
#endif

/// Color for success messages.
#ifndef UCOLOR_SUCCESS
#define UCOLOR_SUCCESS UCOLOR_GRN
#endif

/// Color for warning messages.
#ifndef UCOLOR_WARN
#define UCOLOR_WARN UCOLOR_YEL
#endif

/// Color for error messages.
#ifndef UCOLOR_ERROR
#define UCOLOR_ERROR UCOLOR_RED
#endif

/// Color for fatal error messages.
#ifndef UCOLOR_FATAL
#define UCOLOR_FATAL UCOLOR_MAG
#endif

/// @}

#endif // UCOLOR_H
