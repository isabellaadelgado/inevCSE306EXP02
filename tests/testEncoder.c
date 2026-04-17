#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>
#include <sys/stat.h>
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
 
/*find_pattern */
static const char* find_pattern(const char* text, size_t text_len,const char* pattern, size_t pattern_len){
    if (pattern_len == 0 || pattern_len > text_len) return NULL;
    for (size_t i = 0; i <= text_len - pattern_len; i++) {
        if (memcmp(text + i, pattern, pattern_len) == 0) return text + i;
    }
    return NULL;
}
 
#define MIN_MATCH_LEN 8
static int safe_find_longest_prefix_match(const char* carrier_data, size_t carrier_size, const char* secret_prefix, size_t max_len) {
    for (int len = (int)max_len; len >= MIN_MATCH_LEN; len--) {
        if (find_pattern(carrier_data, carrier_size, secret_prefix, len) != NULL)
            return len;
    }
    return 0;
}
 
/* flush_literal_buffer*/
static void flush_literal_buffer_to_buf(unsigned char* out, int* out_pos, unsigned char* lit_buf, int* count){
    if (*count > 0) {
        out[(*out_pos)++] = (unsigned char)(*count);
        memcpy(out + *out_pos, lit_buf, *count);
        *out_pos += *count;
        *count = 0;
    }
}
 
/*tests: find_pattern*/
TEST(test_find_pattern_found)
{
    const char* haystack = "hello world foo bar";
    const char* needle   = "foo";
    const char* result   = find_pattern(haystack, strlen(haystack),needle,strlen(needle));
    CHECK(result != NULL);
    CHECK(memcmp(result, "foo", 3) == 0);
}
 
TEST(test_find_pattern_not_found){
    const char* haystack = "hello world";
    const char* needle   = "xyz";
    CHECK(find_pattern(haystack, strlen(haystack), needle, strlen(needle)) == NULL);
}
 
TEST(test_find_pattern_empty_needle){
    const char* haystack = "hello";
    CHECK(find_pattern(haystack, strlen(haystack), "", 0) == NULL);
}
 
TEST(test_find_pattern_needle_longer_than_haystack){
    const char* haystack = "hi";
    const char* needle   = "hello world";
    CHECK(find_pattern(haystack, 2, needle, strlen(needle)) == NULL);
}
 
TEST(test_find_pattern_exact_match){
    const char* text = "exactmatch";
    CHECK(find_pattern(text, strlen(text), text, strlen(text)) == text);
}
 
TEST(test_find_pattern_at_end){
    const char* haystack = "abcdefghXXXXXXXX";
    const char* needle   = "XXXXXXXX";
    const char* result   = find_pattern(haystack, strlen(haystack),
                                        needle,   strlen(needle));
    CHECK(result != NULL);
    CHECK(result == haystack + 8);
}
TEST(test_find_pattern_binary_data){
    unsigned char hay[] = {0x00, 0x01, 0xAB, 0xCD, 0xEF, 0x02};
    unsigned char ndl[] = {0xAB, 0xCD, 0xEF};
    const char* result  = find_pattern((char*)hay, sizeof(hay),
                                       (char*)ndl, sizeof(ndl));
    CHECK(result != NULL);
    CHECK(result == (char*)hay + 2);
}
 
/* safe_find_longest_prefix_match*/
TEST(test_safe_find_no_match)
{
    const char* carrier = "abcdefgh";
    const char* secret  = "ZZZZZZZZ";
    int r = safe_find_longest_prefix_match(carrier, strlen(carrier),
                                           secret,  strlen(secret));
    CHECK(r == 0);
}
 
TEST(test_safe_find_exact_min_match){
    const char* carrier = "PREFIX__abcdefghSUFFIX";
    const char* secret  = "abcdefgh";
    int r = safe_find_longest_prefix_match(carrier, strlen(carrier),
                                           secret,  strlen(secret));
    CHECK(r == MIN_MATCH_LEN);
}
 
TEST(test_safe_find_longer_match){
    const char* carrier = "PREFIX__abcdefghijklmnSUFFIX";
    const char* secret  = "abcdefghijklmn";
    int r = safe_find_longest_prefix_match(carrier, strlen(carrier),
                                           secret,  strlen(secret));
    CHECK(r == 14);
}
 
TEST(test_safe_find_below_min_not_returned) {
    const char* carrier = "----abcd----";
    const char* secret  = "abcdXXXX";   /* first 4 match, rest don't */
    int r = safe_find_longest_prefix_match(carrier, strlen(carrier),
                                           secret,  strlen(secret));
    CHECK(r == 0);
}
 
TEST(test_flush_literal_buffer_empty) {
    unsigned char out[256] = {0};
    int pos = 0;
    unsigned char lit[127];
    int count = 0;
    flush_literal_buffer_to_buf(out, &pos, lit, &count);
/* nothing written */
    CHECK(pos == 0);
    CHECK(count == 0);
}
 
TEST(test_flush_literal_buffer_single_byte) {
    unsigned char out[256] = {0};
    int pos = 0;
    unsigned char lit[127] = {0x42};
    int count = 1;
    flush_literal_buffer_to_buf(out, &pos, lit, &count);
/* 1-byte header + 1 byte payload */
    CHECK(pos == 2);
    /* counts byte */
    CHECK(out[0] == 1);     /* count byte */
    /* the literal */
    CHECK(out[1] == 0x42);
    /*resets*/
    CHECK(count == 0);    
}
 
TEST(test_flush_literal_buffer_max_bytes){
    unsigned char out[256] = {0};
    int pos = 0;
    unsigned char lit[127];
    memset(lit, 0xAA, 127);
    int count = 127;
    flush_literal_buffer_to_buf(out, &pos, lit, &count);
    CHECK(pos == 128);     
    CHECK(out[0] == 127);
    for (int i = 1; i <= 127; i++) CHECK(out[i] == 0xAA);
    CHECK(count == 0);
}
 
TEST(test_flush_literal_buffer_resets_count){
    unsigned char out[256] = {0};
    int pos = 0;
    unsigned char lit[127] = {0x01, 0x02, 0x03};
    int count = 3;
    flush_literal_buffer_to_buf(out, &pos, lit, &count);
    CHECK(count == 0);
}
 
int main(void){
    printf(" encoder tests \n");
    printf("\n[find_pattern]\n");
    RUN(test_find_pattern_found);
    RUN(test_find_pattern_not_found);
    RUN(test_find_pattern_empty_needle);
    RUN(test_find_pattern_needle_longer_than_haystack);
    RUN(test_find_pattern_exact_match);
    RUN(test_find_pattern_at_end);
    RUN(test_find_pattern_binary_data);
 
    printf("\n[safe_find_longest_prefix_match]\n");
    RUN(test_safe_find_no_match);
    RUN(test_safe_find_exact_min_match);
    RUN(test_safe_find_longer_match);
    RUN(test_safe_find_below_min_not_returned);
 
    printf("\n[flush_literal_buffer]\n");
    RUN(test_flush_literal_buffer_empty);
    RUN(test_flush_literal_buffer_single_byte);
    RUN(test_flush_literal_buffer_max_bytes);
    RUN(test_flush_literal_buffer_resets_count);
    printf("\n%d/%d tests passed\n", tests_passed, tests_run);
    return (tests_passed == tests_run) ? EXIT_SUCCESS : EXIT_FAILURE;
}
 
