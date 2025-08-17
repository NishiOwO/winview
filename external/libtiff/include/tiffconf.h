/*
  Configuration defines for installed libtiff.
  This file maintained for backward compatibility. Do not use definitions
  from this file in your programs.
*/

/* clang-format off */
/* clang-format disabled because CMake scripts are very sensitive to the
 * formatting of this file. configure_file variables of type "@VAR@" are
 * modified by clang-format and won't be substituted.
 */

#ifndef _TIFFCONF_
#define _TIFFCONF_


#include <stddef.h>
#include <stdint.h>
#include <inttypes.h>


/* Signed 16-bit type */
#define TIFF_INT16_T int16_t

/* Signed 32-bit type */
#define TIFF_INT32_T int32_t

/* Signed 64-bit type */
#define TIFF_INT64_T int64_t

/* Signed 8-bit type */
#define TIFF_INT8_T int8_t

/* Unsigned 16-bit type */
#define TIFF_UINT16_T uint16_t

/* Unsigned 32-bit type */
#define TIFF_UINT32_T uint32_t

/* Unsigned 64-bit type */
#define TIFF_UINT64_T uint64_t

/* Unsigned 8-bit type */
#define TIFF_UINT8_T uint8_t

/* Signed size type */
#ifdef __WATCOMC__
#define TIFF_SSIZE_T signed
#else
#define TIFF_SSIZE_T ssize_t
#endif

/* Compatibility stuff. */

/* Define as 0 or 1 according to the floating point format supported by the
   machine */
#define HAVE_IEEEFP 1

/* The concept of HOST_FILLORDER is broken. Since libtiff 4.5.1
 * this macro will always be hardcoded to FILLORDER_LSB2MSB on all
 * architectures, to reflect past long behavior of doing so on x86 architecture.
 * Note however that the default FillOrder used by libtiff is FILLORDER_MSB2LSB,
 * as mandated per the TIFF specification.
 * The influence of HOST_FILLORDER is only when passing the 'H' mode in
 * TIFFOpen().
 * You should NOT rely on this macro to decide the CPU endianness!
 * This macro will be removed in libtiff 4.6
 */
#define HOST_FILLORDER FILLORDER_LSB2MSB

/* Native cpu byte order: 1 if big-endian (Motorola) or 0 if little-endian
   (Intel) */
#undef HOST_BIGENDIAN

/* Support CCITT Group 3 & 4 algorithms */
#undef CCITT_SUPPORT

/* Support JPEG compression (requires IJG JPEG library) */
#define JPEG_SUPPORT

/* Support JBIG compression (requires JBIG-KIT library) */
#undef JBIG_SUPPORT

/* Support LERC compression */
#define LERC_SUPPORT

/* Support LogLuv high dynamic range encoding */
#define LOGLUV_SUPPORT

/* Support LZW algorithm */
#undef LZW_SUPPORT

/* Support NeXT 2-bit RLE algorithm */
#define NEXT_SUPPORT

/* Support Old JPEG compresson (read contrib/ojpeg/README first! Compilation
   fails with unpatched IJG JPEG library) */
#undef OJPEG_SUPPORT

/* Support Macintosh PackBits algorithm */
#define PACKBITS_SUPPORT

/* Support Pixar log-format algorithm (requires Zlib) */
#define PIXARLOG_SUPPORT

/* Support ThunderScan 4-bit RLE algorithm */
#define THUNDER_SUPPORT

/* Support Deflate compression */
#undef ZIP_SUPPORT

/* Support libdeflate enhanced compression */
#undef LIBDEFLATE_SUPPORT

/* Support strip chopping (whether or not to convert single-strip uncompressed
   images to multiple strips of ~8Kb to reduce memory usage) */
#undef STRIPCHOP_DEFAULT

/* Enable SubIFD tag (330) support */
#undef SUBIFD_SUPPORT

/* Treat extra sample as alpha (default enabled). The RGBA interface will
   treat a fourth sample with no EXTRASAMPLE_ value as being ASSOCALPHA. Many
   packages produce RGBA files but don't mark the alpha properly. */
#undef DEFAULT_EXTRASAMPLE_AS_ALPHA

/* Pick up YCbCr subsampling info from the JPEG data stream to support files
   lacking the tag (default enabled). */
#undef CHECK_JPEG_YCBCR_SUBSAMPLING

/* Support MS MDI magic number files as TIFF */
#undef MDI_SUPPORT

/*
 * Feature support definitions.
 * XXX: These macros are obsoleted. Don't use them in your apps!
 * Macros stays here for backward compatibility and should be always defined.
 */
#define COLORIMETRY_SUPPORT
#define YCBCR_SUPPORT
#define CMYK_SUPPORT
#define ICC_SUPPORT
#define PHOTOSHOP_SUPPORT
#define IPTC_SUPPORT

#endif /* _TIFFCONF_ */

/* clang-format on */
