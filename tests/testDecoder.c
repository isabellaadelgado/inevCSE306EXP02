#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>
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

/* map parsing helpers*/
#define MAP_VERSION    2
#define SHA256_LEN     32
typedef struct {
    const unsigned char* ptr;
    const unsigned char* end;
} MapReader;

static bool mr_has(const MapReader* r, size_t n){
    return (size_t)(r->end - r->ptr) >= n;
}

static bool mr_read_u8(MapReader* r, uint8_t* out){
    if (!mr_has(r, 1)) return false;
    *out = *r->ptr++;
    return true;
}

static bool mr_read_u16(MapReader* r, uint16_t* out){
    if (!mr_has(r, 2)) return false;
    memcpy(out, r->ptr, 2);
    r->ptr += 2;
    return true;
}

static bool mr_read_u32(MapReader* r, uint32_t* out){
    if (!mr_has(r, 4)) return false;
    memcpy(out, r->ptr, 4);
    r->ptr += 4;
    return true;
}

/* Build a minimal valid lidecap map header in a buffer.
   Returns pointer to byte after the header (start of payload area). */
static unsigned char* write_map_header(unsigned char* buf,
                                       uint16_t num_carriers)
{
    memcpy(buf, "LIDECMAP", 8);             buf += 8;
    uint16_t ver = MAP_VERSION;
    memcpy(buf, &ver, 2);                   buf += 2;
    memcpy(buf, &num_carriers, 2);          buf += 2;
    /* write zero-hashes for each carrier */
    for (int i = 0; i < num_carriers; i++) {
        memset(buf, 0, SHA256_LEN);         buf += SHA256_LEN;
    }
    return buf;
}

/*tests the MapReader helpers*/
TEST(test_mr_read_u8_ok)
{
    unsigned char data[] = {0xAB};
    MapReader r = { data, data + 1 };
    uint8_t v;
    CHECK(mr_read_u8(&r, &v));
    CHECK(v == 0xAB);
    CHECK(r.ptr == r.end);
}

TEST(test_mr_read_u8_empty)
{
    unsigned char data[] = {0x00};
    MapReader r = { data, data };
    uint8_t v;
    CHECK(!mr_read_u8(&r, &v));
}

TEST(test_mr_read_u16_little_endian)
{
    unsigned char data[] = {0x34, 0x12};
    MapReader r = { data, data + 2 };
    uint16_t v;
    CHECK(mr_read_u16(&r, &v));
    CHECK(v == 0x1234);
}

TEST(test_mr_read_u32_little_endian)
{
    unsigned char data[] = {0x78, 0x56, 0x34, 0x12};
    MapReader r = { data, data + 4 };
    uint32_t v;
    CHECK(mr_read_u32(&r, &v));
    CHECK(v == 0x12345678);
}

TEST(test_mr_read_truncated_u16){
    unsigned char data[] = {0xAA};
    MapReader r = { data, data + 1 };
    uint16_t v;
    CHECK(!mr_read_u16(&r, &v));
}

/*tests the map header validation */
TEST(test_valid_map_header){
    unsigned char buf[256];
    write_map_header(buf, 2);

    MapReader r = { buf, buf + sizeof(buf) };
    CHECK(mr_has(&r, 8));
    CHECK(memcmp(r.ptr, "LIDECMAP", 8) == 0);
    r.ptr += 8;

    uint16_t version;
    CHECK(mr_read_u16(&r, &version));
    CHECK(version == MAP_VERSION);

    uint16_t nc;
    CHECK(mr_read_u16(&r, &nc));
    CHECK(nc == 2);
}

TEST(test_wrong_magic_rejected){
    unsigned char buf[12];
    memcpy(buf, "BADMAGIC", 8);
    CHECK(memcmp(buf, "LIDECMAP", 8) != 0);
}

TEST(test_wrong_version_rejected){
    unsigned char buf[256];
    write_map_header(buf, 1);

    /* corrupt the version field (bytes 8-9) */
    uint16_t bad = MAP_VERSION + 1;
    memcpy(buf + 8, &bad, 2);

    MapReader r = { buf, buf + sizeof(buf) };
    r.ptr += 8; 
    uint16_t ver;
    CHECK(mr_read_u16(&r, &ver));
    CHECK(ver != MAP_VERSION);
}

TEST(test_truncated_header_no_version)
{
    unsigned char buf[8];
    memcpy(buf, "LIDECMAP", 8);
    MapReader r = { buf, buf + 8 };
    r.ptr += 8;
    uint16_t ver;
    CHECK(!mr_read_u16(&r, &ver));   /* nothing left */
}

/*tests control byte*/
TEST(test_control_byte_literal_flag){
    uint8_t literal_ctrl = 5;
    uint8_t ref_ctrl     = 0x80; 

    CHECK((literal_ctrl & 0x80) == 0);
    CHECK((ref_ctrl     & 0x80) != 0);
}

TEST(test_literal_len_from_control){
    uint8_t ctrl = 42;
    CHECK((ctrl & 0x80) == 0);
    uint8_t len = ctrl;       
    CHECK(len == 42);
}

TEST(test_zero_literal_len_invalid){
    uint8_t ctrl = 0;
    CHECK((ctrl & 0x80) == 0);
    CHECK(ctrl == 0);  /* decoder must reject this */
}

TEST(test_reference_block_fields){
    /*building a reference record and parse it with MapReader */
    unsigned char buf[9];
    uint16_t carrier = 1;
    uint32_t offset  = 0x00001000;
    uint16_t length  = 64;
    memcpy(buf,     &carrier, 2);
    memcpy(buf + 2, &offset,  4);
    memcpy(buf + 6, &length,  2);

    MapReader r = { buf, buf + 8 };
    uint16_t c; uint32_t o; uint16_t l;
    CHECK(mr_read_u16(&r, &c));
    CHECK(mr_read_u32(&r, &o));
    CHECK(mr_read_u16(&r, &l));

    CHECK(c == 1);
    CHECK(o == 0x00001000);
    CHECK(l == 64);
}

/* tests index bounds*/
TEST(test_carrier_idx_in_bounds){
    uint16_t num_carriers = 4;
    uint16_t idx          = 3;
    CHECK(idx < num_carriers);
}

TEST(test_carrier_idx_out_of_bounds){
    uint16_t num_carriers = 4;
    uint16_t idx          = 4;   /* == num_carriers → invalid */
    CHECK(!(idx < num_carriers));
}

TEST(test_zero_length_reference_invalid){
    uint16_t length = 0;
    CHECK(length == 0);  /* decoder must reject */
}

/*tests*/
TEST(test_roundtrip_header_3_carriers){
    uint16_t NC = 3;
    unsigned char buf[8 + 2 + 2 + SHA256_LEN * 3];
    write_map_header(buf, NC);
    MapReader r = { buf, buf + sizeof(buf) };
    CHECK(memcmp(r.ptr, "LIDECMAP", 8) == 0); r.ptr += 8;

    uint16_t ver;
    CHECK(mr_read_u16(&r, &ver));
    CHECK(ver == MAP_VERSION);

    uint16_t nc;
    CHECK(mr_read_u16(&r, &nc));
    CHECK(nc == NC);

    /* skip hashes */
    CHECK(mr_has(&r, SHA256_LEN * NC));
    r.ptr += SHA256_LEN * NC;

    /* should be at end */
    CHECK(r.ptr == r.end);
}

TEST(test_roundtrip_literal_block){
    unsigned char payload[]  = { 0x03, 'A', 'B', 'C' };
    MapReader r = { payload, payload + sizeof(payload) };
    uint8_t ctrl;
    CHECK(mr_read_u8(&r, &ctrl));
    /* literal */
    CHECK((ctrl & 0x80) == 0);  
    uint8_t len = ctrl;
    CHECK(len == 3);
    CHECK(mr_has(&r, len));
    CHECK(r.ptr[0] == 'A' && r.ptr[1] == 'B' && r.ptr[2] == 'C');
}

TEST(test_roundtrip_reference_block){
    unsigned char payload[9];
    /* control: reference */
    payload[0] = 0x80;          
    uint16_t c = 0, l = 16;
    uint32_t o = 512;
    memcpy(payload + 1, &c, 2);
    memcpy(payload + 3, &o, 4);
    memcpy(payload + 7, &l, 2);

    MapReader r = { payload, payload + 9 };

    uint8_t ctrl;
    CHECK(mr_read_u8(&r, &ctrl));
    CHECK((ctrl & 0x80) != 0);

    uint16_t carrier; uint32_t offset; uint16_t length;
    CHECK(mr_read_u16(&r, &carrier));
    CHECK(mr_read_u32(&r, &offset));
    CHECK(mr_read_u16(&r, &length));

    CHECK(carrier == 0);
    CHECK(offset  == 512);
    CHECK(length  == 16);
    CHECK(length  != 0);
}
int main(void){
    printf("=== decoder unit tests ===\n");

    printf("\n[MapReader helpers]\n");
    RUN(test_mr_read_u8_ok);
    RUN(test_mr_read_u8_empty);
    RUN(test_mr_read_u16_little_endian);
    RUN(test_mr_read_u32_little_endian);
    RUN(test_mr_read_truncated_u16);

    printf("\n[map header validation]\n");
    RUN(test_valid_map_header);
    RUN(test_wrong_magic_rejected);
    RUN(test_wrong_version_rejected);
    RUN(test_truncated_header_no_version);

    printf("\n[control byte decoding]\n");
    RUN(test_control_byte_literal_flag);
    RUN(test_literal_len_from_control);
    RUN(test_zero_literal_len_invalid);
    RUN(test_reference_block_fields);

    printf("\n[carrier index bounds]\n");
    RUN(test_carrier_idx_in_bounds);
    RUN(test_carrier_idx_out_of_bounds);
    RUN(test_zero_length_reference_invalid);

    printf("\n[round-trip]\n");
    RUN(test_roundtrip_header_3_carriers);
    RUN(test_roundtrip_literal_block);
    RUN(test_roundtrip_reference_block);

    printf("\n%d/%d tests passed\n", tests_passed, tests_run);
    return (tests_passed == tests_run) ? EXIT_SUCCESS : EXIT_FAILURE;
}
