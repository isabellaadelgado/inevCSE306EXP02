#include <string.h>
#include <strings.h>  
#include <ctype.h>
#include "image.h"

static const char* get_extension(const char* filename) {
    if (!filename) return NULL;
    const char* dot = strrchr(filename, '.');
    if (!dot || dot == filename) return NULL;
    if (*(dot + 1) == '\0') return NULL;  /* trailing dot, no extension */
    return dot + 1;
}

image_format_t format_from_extension(const char* filename) {
    const char* ext = get_extension(filename);
    if (!ext) return IMG_UNSUPPORTED;

    if (strcasecmp(ext, "bmp")  == 0) return IMG_BMP;
    if (strcasecmp(ext, "jpg")  == 0) return IMG_JPEG;
    if (strcasecmp(ext, "jpeg") == 0) return IMG_JPEG;
    if (strcasecmp(ext, "png")  == 0) return IMG_PNG;
    if (strcasecmp(ext, "gif")  == 0) return IMG_GIF;
    if (strcasecmp(ext, "tiff") == 0) return IMG_TIFF;
    if (strcasecmp(ext, "tif")  == 0) return IMG_TIFF;
    if (strcasecmp(ext, "raw")  == 0) return IMG_RAW;

    return IMG_UNSUPPORTED;
}

const char* format_name(image_format_t fmt) {
    switch (fmt) {
        case IMG_BMP:         return "BMP";
        case IMG_JPEG:        return "JPEG";
        case IMG_PNG:         return "PNG";
        case IMG_GIF:         return "GIF";
        case IMG_TIFF:        return "TIFF";
        case IMG_RAW:         return "RAW";
        case IMG_UNSUPPORTED: return "unsupported";
    }
    return "unsupported";
}

bool is_image_file(const char* filename) {
    return format_from_extension(filename) != IMG_UNSUPPORTED;
}