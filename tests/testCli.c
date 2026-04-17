#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>

static int tests_run    = 0;
static int tests_passed = 0;

#define TEST(name) static void name(void)
#define RUN(name) do { \
    printf("  running %-45s", #name); \
    tests_run++; \
    name(); \
    tests_passed++; \
    printf("PASS\n"); \
} while(0)

#define CHECK(expr) do { \
    if (!(expr)) { \
        printf("FAIL\n    assertion failed: %s  (%s:%d)\n", \
               #expr, __FILE__, __LINE__); \
        exit(EXIT_FAILURE); \
    } \
} while(0)

#include "cli.h"


TEST(test_parse_encoding_ascii) {
    encoding_t enc = parse_encoding_flag("-ASCII");
    CHECK(enc == ENC_ASCII);
}

TEST(test_parse_encoding_utf8) {
    encoding_t enc = parse_encoding_flag("-UTF-8");
    CHECK(enc == ENC_UTF8);
}

TEST(test_parse_encoding_utf16) {
    encoding_t enc = parse_encoding_flag("-UTF-16");
    CHECK(enc == ENC_UTF16);
}

TEST(test_parse_encoding_unknown_returns_default) {
    encoding_t enc = parse_encoding_flag("-EBCDIC");
    CHECK(enc == ENC_UNKNOWN);
}

TEST(test_parse_encoding_null_returns_default) {
    encoding_t enc = parse_encoding_flag(NULL);
    CHECK(enc == ENC_UNKNOWN);
}


TEST(test_parse_mode_text) {
    steg_mode_t mode = parse_mode_flag("-text");
    CHECK(mode == MODE_TEXT);
}

TEST(test_parse_mode_image) {
    steg_mode_t mode = parse_mode_flag("-image");
    CHECK(mode == MODE_IMAGE);
}

TEST(test_parse_mode_unknown) {
    steg_mode_t mode = parse_mode_flag("-video");
    CHECK(mode == MODE_UNKNOWN);
}

TEST(test_parse_mode_null) {
    steg_mode_t mode = parse_mode_flag(NULL);
    CHECK(mode == MODE_UNKNOWN);
}


TEST(test_cli_config_basic_encoder) {
    char *argv[] = {"encoder", "secret.txt", "carrier1.bmp", "carrier2.png"};
    int argc = 4;
    cli_config_t cfg;
    int rc = parse_cli_args(argc, argv, &cfg);
    CHECK(rc == 0);
    CHECK(strcmp(cfg.secret_file, "secret.txt") == 0);
    CHECK(cfg.num_carriers == 2);
    CHECK(cfg.strict_mode == false);
    CHECK(cfg.encoding == ENC_UTF8);  
    CHECK(cfg.mode == MODE_TEXT);     
}

TEST(test_cli_config_strict_mode) {
    char *argv[] = {"encoder", "--strict", "secret.txt", "carrier.bmp"};
    int argc = 4;
    cli_config_t cfg;
    int rc = parse_cli_args(argc, argv, &cfg);
    CHECK(rc == 0);
    CHECK(cfg.strict_mode == true);
    CHECK(strcmp(cfg.secret_file, "secret.txt") == 0);
}

TEST(test_cli_config_with_encoding_flag) {
    char *argv[] = {"encoder", "-UTF-16", "secret.txt", "carrier.bmp"};
    int argc = 4;
    cli_config_t cfg;
    int rc = parse_cli_args(argc, argv, &cfg);
    CHECK(rc == 0);
    CHECK(cfg.encoding == ENC_UTF16);
}

TEST(test_cli_config_with_mode_flag) {
    char *argv[] = {"encoder", "-image", "photo.png", "carrier.bmp"};
    int argc = 4;
    cli_config_t cfg;
    int rc = parse_cli_args(argc, argv, &cfg);
    CHECK(rc == 0);
    CHECK(cfg.mode == MODE_IMAGE);
}

TEST(test_cli_config_too_few_args) {
    char *argv[] = {"encoder"};
    int argc = 1;
    cli_config_t cfg;
    int rc = parse_cli_args(argc, argv, &cfg);
    CHECK(rc != 0);
}

TEST(test_cli_config_combined_flags) {
    char *argv[] = {"encoder", "--strict", "-UTF-16", "-image",
                    "secret.png", "c1.bmp", "c2.jpg"};
    int argc = 7;
    cli_config_t cfg;
    int rc = parse_cli_args(argc, argv, &cfg);
    CHECK(rc == 0);
    CHECK(cfg.strict_mode == true);
    CHECK(cfg.encoding == ENC_UTF16);
    CHECK(cfg.mode == MODE_IMAGE);
    CHECK(cfg.num_carriers == 2);
}

int main(void) {
    printf("=== CLI unit tests ===\n");

    printf("\n[parse_encoding_flag]\n");
    RUN(test_parse_encoding_ascii);
    RUN(test_parse_encoding_utf8);
    RUN(test_parse_encoding_utf16);
    RUN(test_parse_encoding_unknown_returns_default);
    RUN(test_parse_encoding_null_returns_default);

    printf("\n[parse_mode_flag]\n");
    RUN(test_parse_mode_text);
    RUN(test_parse_mode_image);
    RUN(test_parse_mode_unknown);
    RUN(test_parse_mode_null);

    printf("\n[cli_config parsing]\n");
    RUN(test_cli_config_basic_encoder);
    RUN(test_cli_config_strict_mode);
    RUN(test_cli_config_with_encoding_flag);
    RUN(test_cli_config_with_mode_flag);
    RUN(test_cli_config_too_few_args);
    RUN(test_cli_config_combined_flags);

    printf("\n%d/%d tests passed\n", tests_passed, tests_run);
    return (tests_passed == tests_run) ? EXIT_SUCCESS : EXIT_FAILURE;
}