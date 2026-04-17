#ifndef CLI_H
#define CLI_H

#include <stdbool.h>

/* Text encoding types (Feature Request 3) */
typedef enum {
    ENC_UNKNOWN = 0,
    ENC_ASCII,
    ENC_UTF8,
    ENC_UTF16
} encoding_t;

/* Steganography mode (Feature Request 1) */
typedef enum {
    MODE_UNKNOWN = 0,
    MODE_TEXT,
    MODE_IMAGE
} steg_mode_t;

typedef struct {
    const char*  secret_file;
    const char** carrier_files;
    int          num_carriers;
    bool         strict_mode;
    encoding_t   encoding;
    steg_mode_t  mode;
} cli_config_t;

encoding_t  parse_encoding_flag(const char* flag);

steg_mode_t parse_mode_flag(const char* flag);

int parse_cli_args(int argc, char* argv[], cli_config_t* out);

#endif