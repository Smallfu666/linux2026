#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct {
    uint64_t state;
} rng_t;

static uint32_t rng_u32(rng_t *rng) {
    uint64_t x = rng->state;
    x ^= x >> 12;
    x ^= x << 25;
    x ^= x >> 27;
    rng->state = x;
    return (uint32_t) ((x * 2685821657736338717ULL) >> 32);
}

static void make_byte_table(uint32_t table[256]) {
    for (unsigned i = 0; i < 256; ++i) {
        uint32_t crc = i;
        for (int bit = 0; bit < 8; ++bit) {
            if (crc & 1U) {
                crc = (crc >> 1) ^ 0xEDB88320U;
            } else {
                crc >>= 1;
            }
        }
        table[i] = crc;
    }
}

static void make_nibble_table(uint32_t table[16]) {
    for (unsigned i = 0; i < 16; ++i) {
        uint32_t crc = i;
        for (int bit = 0; bit < 4; ++bit) {
            if (crc & 1U) {
                crc = (crc >> 1) ^ 0xEDB88320U;
            } else {
                crc >>= 1;
            }
        }
        table[i] = crc;
    }
}

static uint32_t update_byte(uint32_t crc, uint8_t byte, const uint32_t table[256]) {
    return table[(crc ^ byte) & 0xFFU] ^ (crc >> 8);
}

static uint32_t update_nibble(uint32_t crc, uint8_t nibble, const uint32_t table[16]) {
    return table[(crc ^ nibble) & 0x0FU] ^ (crc >> 4);
}

static uint32_t update_byte_via_nibble(uint32_t crc, uint8_t byte, const uint32_t table[16]) {
    crc = update_nibble(crc, (uint8_t) (byte & 0x0F), table);
    crc = update_nibble(crc, (uint8_t) (byte >> 4), table);
    return crc;
}

int main(void) {
    uint32_t byte_table[256];
    uint32_t nibble_table[16];
    make_byte_table(byte_table);
    make_nibble_table(nibble_table);

    rng_t rng = {0x20260404u};
    const uint32_t states_fixed[] = {0U, 0xFFFFFFFFU, 0x12345678U};
    int all_match = 1;
    char first_mismatch[256] = "none";
    int tested_states = 0;
    int tested_bytes = 0;

    for (size_t s = 0; s < sizeof(states_fixed) / sizeof(states_fixed[0]); ++s) {
        uint32_t state = states_fixed[s];
        ++tested_states;
        for (int byte = 0; byte < 256; ++byte) {
            ++tested_bytes;
            uint32_t nb = update_byte_via_nibble(state, (uint8_t) byte, nibble_table);
            uint32_t bb = update_byte(state, (uint8_t) byte, byte_table);
            if (nb != bb) {
                all_match = 0;
                snprintf(first_mismatch, sizeof first_mismatch,
                         "state=0x%08x byte=%d nibble_result=0x%08x byte_result=0x%08x",
                         state, byte, nb, bb);
                goto done;
            }
        }
    }

    for (int i = 0; i < 10000; ++i) {
        uint32_t state = rng_u32(&rng);
        ++tested_states;
        for (int byte = 0; byte < 256; ++byte) {
            ++tested_bytes;
            uint32_t nb = update_byte_via_nibble(state, (uint8_t) byte, nibble_table);
            uint32_t bb = update_byte(state, (uint8_t) byte, byte_table);
            if (nb != bb) {
                all_match = 0;
                snprintf(first_mismatch, sizeof first_mismatch,
                         "state=0x%08x byte=%d nibble_result=0x%08x byte_result=0x%08x",
                         state, byte, nb, bb);
                goto done;
            }
        }
    }

done:
    printf("all_match=%s\n", all_match ? "true" : "false");
    printf("tested_states=%d\n", tested_states);
    printf("tested_bytes=%d\n", tested_bytes);
    printf("nibble_table_bytes=%zu\n", sizeof nibble_table);
    printf("byte_table_bytes=%zu\n", sizeof byte_table);
    printf("first_mismatch_if_any=%s\n", first_mismatch);
    return all_match ? 0 : 1;
}
