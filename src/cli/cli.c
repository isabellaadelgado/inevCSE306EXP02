/* src/cli/cli.c — CLI argument parsing for INEV encoder */
#include <string.h>
#include <strings.h>
#include <stdio.h>
#include <stdlib.h>
#include "cli.h"

encoding_t parse_encoding_flag(const char* flag) {
    if (!flag) return ENC_UNKNOWN;
    if (strcasecmp(flag, "-ASCII")  == 0 || strcasecmp(flag, "-US-ASCII") == 0) return ENC_ASCII;
    if (strcasecmp(flag, "-UTF-8")  == 0) return ENC_UTF8;
    if (strcasecmp(flag, "-UTF-16") == 0) return ENC_UTF16;
    return ENC_UNKNOWN;
}

steg_mode_t parse_mode_flag(const char* flag) {
    if (!flag) return MODE_UNKNOWN;
    if (strcasecmp(flag, "-text")  == 0) return MODE_TEXT;
    if (strcasecmp(flag, "-image") == 0) return MODE_IMAGE;
    return MODE_UNKNOWN;
}

int parse_cli_args(int argc, char* argv[], cli_config_t* out) {
    if (!out) return -1;

    out->secret_file   = NULL;
    out->carrier_files = NULL;
    out->num_carriers  = 0;
    out->strict_mode   = false;
    out->encoding      = ENC_UTF8;
    out->mode          = MODE_TEXT;

    const char** positional = malloc(argc * sizeof(const char*));
    if (!positional) return -1;
    int pos_count = 0;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--strict") == 0) {
            out->strict_mode = true;
        } else if (argv[i][0] == '-') {
            encoding_t enc = parse_encoding_flag(argv[i]);
            if (enc != ENC_UNKNOWN) {
                out->encoding = enc;
                continue;
            }
            steg_mode_t mode = parse_mode_flag(argv[i]);
            if (mode != MODE_UNKNOWN) {
                out->mode = mode;
                continue;
            }
            positional[pos_count++] = argv[i];
        } else {
            positional[pos_count++] = argv[i];
        }
    }

    if (pos_count < 2) {
        fprintf(stderr, "Usage: %s [--strict] [-text|-image] [-ASCII|-UTF-8|-UTF-16] "
                        "<secret_file> <carrier1> ...\n", argv[0]);
        free(positional);
        return -1;
    }

    out->secret_file   = positional[0];
    out->carrier_files = positional + 1;
    out->num_carriers  = pos_count - 1;

    return 0;
}