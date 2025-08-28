#ifndef __WVDEFAULT_H__
#define __WVDEFAULT_H__

/**
 * add your image driver here
 */

#ifdef INTEGRATE
static DriverProc default_drivers[] = {
#ifdef DOGIF
    TryGIFDriver,
#endif
#ifdef DOPNG
    TryPNGDriver,
#endif
#ifdef DOJPEG
    TryJPEGDriver,
#endif
#ifdef DOTIFF
    TryTIFFDriver,
#endif
#ifdef DOXPM
    TryXPMDriver,
#endif
#ifdef DOMSP
    TryMSPDriver,
#endif
#ifdef DOXBM
    TryXBMDriver,
#endif
#ifdef DOTGA
    TryTGADriver,
#endif
};

static const char* default_exts[] = {
#ifdef DOJPEG
    "JPEG", "*.jpg;*.jpeg", /**/
#endif
#ifdef DOPNG
    "PNG",  "*.png", /**/
#endif
#ifdef DOTIFF
    "TIFF", "*.tiff;*.tif", /**/
#endif
#ifdef DOGIF
    "GIF",  "*.gif", /**/
#endif
#ifdef DOMSP
    "MSP",  "*.msp", /**/
#endif
#ifdef DOTGA
    "TGA",  "*.tga", /**/
#endif
#ifdef DOXBM
    "XBM",  "*.xbm", /**/
#endif
#ifdef DOXPM
    "XPM",  "*.xpm", /**/
#endif
};
#endif

#endif
