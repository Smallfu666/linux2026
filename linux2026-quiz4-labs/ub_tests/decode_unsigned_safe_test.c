#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

typedef struct {
    const char *bits;
    size_t len;
    size_t pos;
} stream_t;

static int next_bit_original(stream_t *s) {
    if (s->pos >= s->len) {
        return 1;
    }
    return s->bits[s->pos++] == '1' ? 1 : 0;
}

static int next_bit_safe(stream_t *s, int *eof) {
    if (s->pos >= s->len) {
        *eof = 1;
        return 0;
    }
    return s->bits[s->pos++] == '1' ? 1 : 0;
}

static int decode_unsigned_original(stream_t *s) {
    int a = 0;
    int b = 1;
    while (!next_bit_original(s)) {
        a++;
    }
    while (a--) {
        b = (b << 1) | next_bit_original(s);
    }
    return b - 1;
}

static int decode_unsigned_safe(stream_t *s) {
    int a = 0;
    unsigned long long b = 1;
    int eof = 0;
    while (!next_bit_safe(s, &eof)) {
        if (eof) {
            return -1;
        }
        if (++a > 63) {
            return -2;
        }
    }
    while (a--) {
        int bit = next_bit_safe(s, &eof);
        if (eof) {
            return -1;
        }
        b = (b << 1) | (unsigned long long) bit;
        if (b > (unsigned long long) INT_MAX + 1ULL) {
            return -3;
        }
    }
    return (int) b - 1;
}

static void make_code_for_value(char *buf, size_t cap, int v) {
    int value = v + 1;
    int bits[64];
    int nbits = 0;
    while (value > 0) {
        bits[nbits++] = value & 1;
        value >>= 1;
    }
    int prefix = nbits - 1;
    size_t pos = 0;
    for (int i = 0; i < prefix; ++i) {
        if (pos + 1 < cap) {
            buf[pos++] = '0';
        }
    }
    if (pos + 1 < cap) {
        buf[pos++] = '1';
    }
    for (int i = nbits - 2; i >= 0; --i) {
        if (pos + 1 < cap) {
            buf[pos++] = bits[i] ? '1' : '0';
        }
    }
    buf[pos] = '\0';
}

static void run_case(const char *name, const char *bits) {
    fflush(stdout);
    pid_t pid = fork();
    if (pid == 0) {
        setvbuf(stdout, NULL, _IONBF, 0);
        stream_t s1 = {bits, strlen(bits), 0};
        stream_t s2 = {bits, strlen(bits), 0};
        int safe = decode_unsigned_safe(&s2);
        printf("case=%s safe_result=%d\n", name, safe);
        int original = decode_unsigned_original(&s1);
        printf("case=%s original_result=%d\n", name, original);
        _exit(0);
    }
    int status = 0;
    waitpid(pid, &status, 0);
    printf("case=%s child_exit=%d child_signal=%d bits=%s\n",
           name,
           WIFEXITED(status) ? WEXITSTATUS(status) : -1,
           WIFSIGNALED(status) ? WTERMSIG(status) : 0,
           bits);
}

int main(void) {
    char normal0[8];
    char normal1[8];
    char normal15[32];
    char long31[80];
    char long64[160];
    char eofbits[8];

    make_code_for_value(normal0, sizeof normal0, 0);
    make_code_for_value(normal1, sizeof normal1, 1);
    make_code_for_value(normal15, sizeof normal15, 15);

    memset(long31, '0', 31);
    long31[31] = '1';
    memset(long64, '0', 64);
    long64[64] = '1';
    strcpy(eofbits, "000");

    run_case("normal_v_0", normal0);
    run_case("normal_v_1", normal1);
    run_case("normal_v_15", normal15);
    run_case("long_prefix_31", long31);
    run_case("long_prefix_64", long64);
    run_case("eof_before_terminator", eofbits);
    return 0;
}
