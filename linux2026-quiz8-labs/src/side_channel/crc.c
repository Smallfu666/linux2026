#include <stdint.h>
#include <stddef.h>

static uint32_t crc_table[256];
static int crc_table_ready;

static uint32_t crc_step(uint32_t crc, uint8_t byte) {
    crc ^= byte;
    for (int i = 0; i < 8; ++i) {
        uint32_t mask = (uint32_t)-(int32_t)(crc & 1u);
        crc = (crc >> 1) ^ (0xEDB88320u & mask);
    }
    return crc;
}

void crc_init_table(void) {
    if (crc_table_ready) {
        return;
    }
    for (uint32_t i = 0; i < 256; ++i) {
        uint32_t crc = i;
        for (int bit = 0; bit < 8; ++bit) {
            crc = (crc >> 1) ^ (0xEDB88320u & (uint32_t)-(int32_t)(crc & 1u));
        }
        crc_table[i] = crc;
    }
    crc_table_ready = 1;
}

__attribute__((noinline)) uint32_t crc32_table(const uint8_t *data, size_t len) {
    uint32_t crc = 0xFFFFFFFFu;
    for (size_t i = 0; i < len; ++i) {
        crc = crc_table[(crc ^ data[i]) & 0xFFu] ^ (crc >> 8);
    }
    return ~crc;
}

__attribute__((noinline)) uint32_t crc32_branchless(const uint8_t *data, size_t len) {
    uint32_t crc = 0xFFFFFFFFu;
    for (size_t i = 0; i < len; ++i) {
        crc = crc_step(crc, data[i]);
    }
    return ~crc;
}

void crc_touch_table(void) {
    volatile uint32_t sink = 0;
    for (size_t i = 0; i < 256; ++i) {
        sink ^= crc_table[i];
    }
    (void)sink;
}

uint32_t *crc_table_base(void) {
    return crc_table;
}
