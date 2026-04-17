#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

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

#include "image.h"


TEST(test_detect_bmp) {
    image_format_t fmt = format_from_extension("photo.bmp");
    CHECK(fmt == IMG_BMP);
}

TEST(test_detect_jpg) {
    image_format_t fmt = format_from_extension("photo.jpg");
    CHECK(fmt == IMG_JPEG);
}

TEST(test_detect_jpeg) {
    image_format_t fmt = format_from_extension("photo.jpeg");
    CHECK(fmt == IMG_JPEG);
}

TEST(test_detect_png) {
    image_format_t fmt = format_from_extension("image.png");
    CHECK(fmt == IMG_PNG);
}

TEST(test_detect_gif) {
    image_format_t fmt = format_from_extension("anim.gif");
    CHECK(fmt == IMG_GIF);
}

TEST(test_detect_tiff) {
    image_format_t fmt = format_from_extension("scan.tiff");
    CHECK(fmt == IMG_TIFF);
}

TEST(test_detect_tif) {
    image_format_t fmt = format_from_extension("scan.tif");
    CHECK(fmt == IMG_TIFF);
}

TEST(test_detect_raw) {
    image_format_t fmt = format_from_extension("photo.raw");
    CHECK(fmt == IMG_RAW);
}

TEST(test_detect_unknown_extension) {
    image_format_t fmt = format_from_extension("data.xyz");
    CHECK(fmt == IMG_UNSUPPORTED);
}

TEST(test_detect_no_extension) {
    image_format_t fmt = format_from_extension("noextension");
    CHECK(fmt == IMG_UNSUPPORTED);
}

TEST(test_detect_null_filename) {
    image_format_t fmt = format_from_extension(NULL);
    CHECK(fmt == IMG_UNSUPPORTED);
}

TEST(test_detect_case_insensitive_BMP) {
    image_format_t fmt = format_from_extension("PHOTO.BMP");
    CHECK(fmt == IMG_BMP);
}

TEST(test_detect_case_insensitive_JPG) {
    image_format_t fmt = format_from_extension("photo.JPG");
    CHECK(fmt == IMG_JPEG);
}

TEST(test_detect_dot_only) {
    image_format_t fmt = format_from_extension("file.");
    CHECK(fmt == IMG_UNSUPPORTED);
}

TEST(test_detect_hidden_file_no_ext) {
    image_format_t fmt = format_from_extension(".hidden");
    CHECK(fmt == IMG_UNSUPPORTED);
}


TEST(test_format_name_bmp) {
    const char* name = format_name(IMG_BMP);
    CHECK(strcmp(name, "BMP") == 0);
}

TEST(test_format_name_jpeg) {
    const char* name = format_name(IMG_JPEG);
    CHECK(strcmp(name, "JPEG") == 0);
}

TEST(test_format_name_unsupported) {
    const char* name = format_name(IMG_UNSUPPORTED);
    CHECK(name != NULL);  /* should return something like "unsupported" */
}


TEST(test_is_image_bmp_yes) {
    CHECK(is_image_file("photo.bmp") == true);
}

TEST(test_is_image_txt_no) {
    CHECK(is_image_file("readme.txt") == false);
}

TEST(test_is_image_null_no) {
    CHECK(is_image_file(NULL) == false);
}

int main(void) {
    printf("=== image format unit tests ===\n");

    printf("\n[format_from_extension]\n");
    RUN(test_detect_bmp);
    RUN(test_detect_jpg);
    RUN(test_detect_jpeg);
    RUN(test_detect_png);
    RUN(test_detect_gif);
    RUN(test_detect_tiff);
    RUN(test_detect_tif);
    RUN(test_detect_raw);
    RUN(test_detect_unknown_extension);
    RUN(test_detect_no_extension);
    RUN(test_detect_null_filename);
    RUN(test_detect_case_insensitive_BMP);
    RUN(test_detect_case_insensitive_JPG);
    RUN(test_detect_dot_only);
    RUN(test_detect_hidden_file_no_ext);

    printf("\n[format_name]\n");
    RUN(test_format_name_bmp);
    RUN(test_format_name_jpeg);
    RUN(test_format_name_unsupported);

    printf("\n[is_image_file]\n");
    RUN(test_is_image_bmp_yes);
    RUN(test_is_image_txt_no);
    RUN(test_is_image_null_no);

    printf("\n%d/%d tests passed\n", tests_passed, tests_run);
    return (tests_passed == tests_run) ? EXIT_SUCCESS : EXIT_FAILURE;
}