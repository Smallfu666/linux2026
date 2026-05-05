#include <stdint.h>
#include <stddef.h>

static uint32_t rotl32(uint32_t x, int r) {
    return (x << r) | (x >> (32 - r));
}

uint32_t simple_sum_hash(const char *s) {
    uint32_t h = 0x9e3779b9u;
    for (size_t i = 0; s[i] != '\0'; ++i) {
        h += (uint8_t)s[i];
        h = rotl32(h, 5) ^ 0x85ebca6bu;
    }
    return h ^ (uint32_t)__builtin_strlen(s);
}

uint32_t murmurhash3_32(const char *key, uint32_t seed) {
    const uint8_t *data = (const uint8_t *)key;
    size_t len = 0;
    while (key[len] != '\0') {
        ++len;
    }

    const uint32_t c1 = 0xcc9e2d51u;
    const uint32_t c2 = 0x1b873593u;
    uint32_t h1 = seed;
    size_t nblocks = len / 4;

    for (size_t i = 0; i < nblocks; ++i) {
        uint32_t k1 = (uint32_t)data[i * 4 + 0]
                    | ((uint32_t)data[i * 4 + 1] << 8)
                    | ((uint32_t)data[i * 4 + 2] << 16)
                    | ((uint32_t)data[i * 4 + 3] << 24);
        k1 *= c1;
        k1 = rotl32(k1, 15);
        k1 *= c2;

        h1 ^= k1;
        h1 = rotl32(h1, 13);
        h1 = h1 * 5u + 0xe6546b64u;
    }

    const uint8_t *tail = data + nblocks * 4;
    uint32_t k1 = 0;
    switch (len & 3u) {
    case 3: k1 ^= (uint32_t)tail[2] << 16; /* fallthrough */
    case 2: k1 ^= (uint32_t)tail[1] << 8;  /* fallthrough */
    case 1: k1 ^= (uint32_t)tail[0];
            k1 *= c1;
            k1 = rotl32(k1, 15);
            k1 *= c2;
            h1 ^= k1;
            break;
    default:
            break;
    }

    h1 ^= (uint32_t)len;
    h1 ^= h1 >> 16;
    h1 *= 0x85ebca6bu;
    h1 ^= h1 >> 13;
    h1 *= 0xc2b2ae35u;
    h1 ^= h1 >> 16;
    return h1;
}
