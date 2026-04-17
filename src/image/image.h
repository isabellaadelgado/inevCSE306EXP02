#ifndef IMAGE_H
#define IMAGE_H

#include <stdbool.h>

typedef enum {
    IMG_UNSUPPORTED = 0,
    IMG_BMP,
    IMG_JPEG,
    IMG_PNG,
    IMG_GIF,
    IMG_TIFF,
    IMG_RAW
} image_format_t;

image_format_t format_from_extension(const char* filename);

const char* format_name(image_format_t fmt);

bool is_image_file(const char* filename);

#endif 