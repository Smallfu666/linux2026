#include <errno.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#ifndef AMEI_INSTRUMENT
#define AMEI_INSTRUMENT 0
#endif
#ifndef ENABLE_DEBLOCK
#define ENABLE_DEBLOCK 1
#endif
#ifndef ENABLE_BILATERAL
#define ENABLE_BILATERAL 1
#endif

#define foreach(a, d) for (int a = 0; a < d; a++)
#define A(v) ((v) < 0 ? -(v) : (v))
#define C8(v) ((v) < 0 ? 0 : (v) > 255 ? 255 : (v))
static const uint32_t ct[] = { /* Streaming PNG: 16-entry nibble CRC */
    0,          0x1db71064, 0x3b6e20c8, 0x26d930ac, 0x76dc4190, 0x6b6b51f4,
    0x4db26158, 0x5005713c, 0xedb88320, 0xf00f9344, 0xd6d6a3e8, 0xcb61b38c,
    0x9b64c2b0, 0x86d3d2d4, 0xa00ae278, 0xbdbdf21c,
};
static uint32_t CC;
static unsigned ad_a, ad_b;
#define PB(u) fputc(u,stdout)
#define P4B(u) do{PB((u)>>24);PB(((u)>>16)&255);PB(((u)>>8)&255);PB((u)&255);}while(0)
#define PC(u) do{unsigned _v=(u)&255;PB(_v);CC^=_v;CC=(CC>>4)^ct[CC&15];CC=(CC>>4)^ct[CC&15];}while(0)
#define P4(u) do{PC((u)>>24);PC(((u)>>16)&255);PC(((u)>>8)&255);PC((u)&255);}while(0)
#define PL(u) do{PC((u)&255);PC(((u)>>8)&255);}while(0)
#define PA(u) do{unsigned _a=(u)&255;PC(_a);ad_a=(ad_a+_a)%65521;ad_b=(ad_b+ad_a)%65521;}while(0)
#define PO(s,l) do{P4B(l);CC=~0U;foreach(_q,4)PC((s)[_q]);}while(0)
#define PE() P4B(~CC)
static const char *H =
    "4ZmQ;9Z5\\1k^QTkmJD@=gbeB[@alfNAZ?>3NaIB]d_I7h9;=Q8aM^M=@3JEeg^9UOi1"
    "eUBkjfK[`40\\IKQg?\\FR;CaFBaCe:ioATMIFmSUk^g05X4XRY3Xh4aoMFF8W`P7MRM"
    "Jnk9Y\\MBVZ1NK2=YUHF@O`9NmC93M31_a5h@g\\mmd7AoeAAgm:?Z37>ZIGQ>F0YdMe"
    "g?d]WWF@G`T=IKlV;KGbi8\\<5hmFS11A[]_Xf?B598nhd2anSoUVkDD`0Gm@fO]`B2L"
    "K<cn6Sm\\TL_U<8<WN>[ZWGXU1\\7WZf;PQZhneTe7BV?<_7_1RKXoBI;VQRc?GakMY9"
    "X7?FBcNm5LB>C7Bj<l>aaNoB@0k463f;IfPi5E@]DAeM00J=\\C901VlF035:>aCNTbT"
    "WO9DAVW@DPZc`0[BU??[5J7R>5h7AHkc7h8:XW\\=NF2iYKJM[mZld=WSU:YdMiSjQn"
    "\\k11:Kn>1WgW7_nKZWdmj23T3jZXBm]Wb[2f1R:i4VaD@LU:A1_9j`dA4eQ>WTXD>`m"
    "85B@48_RjCHk1gcn5meS:EU3R\\m1i[i20=L30GDa>^Q7420@hljIdUNfoT6g;_3;FDS"
    "C]cOXc=D97>^3Ko3C2IX1U21a9A];SU=QJK@XR]h^;:lHikZT8C]e1PL=NCVJhnA\\25"
    "CUM_NJb59AQOC?H^;K2]Zn6RE:bBT:N75jm9nDoHllZ5U>_0L:T`ieTC1f=Gf0fVe<^d"
    "eWZB2biMeY;kN7m70VWG?ANB92^K5JJ]AQD;6k9DO=L^76cmY^1ae8ojF3:F2GdGO2T6"
    "=NEFX\\UHOcgKlLVU:`7nM?2MBDLQ<_:fb51S<C=W:KP9ZVI;f@g::Zbn=OL;IW8;L\\"
    "GIYF;QO\\Pd<ZD_7W=MSG^W91GVEl?4`UQ2P;5=kV75SdL[;FY;WPjWo8>Nk>^4oKE\\"
    "_12jbO_FmPW3oG7=cBWk?blijB48`@3Q:_ef1g^@0PhUS721ejdBIhJ1EL2i5D[k[oIH"
    "G]@aL`;]HE\\6b:Yi9aK@4Aom1E4c765V40";
static unsigned char G[780], *x, *E;
static int z0[1 << 20], z1[1 << 20], z2[1 << 20], *y[] = {z0, z1, z2};
static int i[2048], X[166], p, r = 1, f, O, P, Z[1 << 20];

#if AMEI_INSTRUMENT
static FILE *g_block_csv;
static FILE *g_block_summary;
static FILE *g_coeff_csv;
static FILE *g_coeff_summary;
static FILE *g_compression_metrics;
static long long g_block_count_by_size[65];
static long long g_block_pixels_by_size[65];
static long long g_coeff_total[3][65][2];
static long long g_coeff_nonzero[3][65][2];
static long long g_coeff_abs_sum[3][65][2];
static int g_coeff_max_abs[3][65][2];
static long long g_coeff_total_all;
static long long g_coeff_nonzero_all;
static long long g_coeff_abs_sum_all;
static long long g_coeff_sq_sum_all;
static int g_next_block_id;
static int g_image_pixels;
static int g_compressed_bytes;

static void mkpath(const char *path) {
    if (mkdir(path, 0777) != 0 && errno != EEXIST) {
        perror(path);
        exit(1);
    }
}

static void instrument_begin(int width, int height) {
    mkpath("results");
    mkpath("results/coeff_stats");
    mkpath("results/compression");
    memset(g_block_count_by_size, 0, sizeof g_block_count_by_size);
    memset(g_block_pixels_by_size, 0, sizeof g_block_pixels_by_size);
    memset(g_coeff_total, 0, sizeof g_coeff_total);
    memset(g_coeff_nonzero, 0, sizeof g_coeff_nonzero);
    memset(g_coeff_abs_sum, 0, sizeof g_coeff_abs_sum);
    memset(g_coeff_max_abs, 0, sizeof g_coeff_max_abs);
    g_coeff_total_all = 0;
    g_coeff_nonzero_all = 0;
    g_coeff_abs_sum_all = 0;
    g_coeff_sq_sum_all = 0;
    g_next_block_id = 0;
    g_image_pixels = width * height;
    g_compressed_bytes = 776;

    g_block_csv = fopen("results/coeff_stats/block_distribution.csv", "w");
    g_block_summary = fopen("results/coeff_stats/block_distribution_summary.txt", "w");
    g_coeff_csv = fopen("results/coeff_stats/coefficients.csv", "w");
    g_coeff_summary = fopen("results/coeff_stats/coeff_summary.csv", "w");
    g_compression_metrics = fopen("results/compression/compression_metrics.csv", "w");
    if (!g_block_csv || !g_block_summary || !g_coeff_csv || !g_coeff_summary || !g_compression_metrics) {
        perror("instrument_begin");
        exit(1);
    }
    fprintf(g_block_csv, "block_id,left,top,level,block_size,is_leaf\n");
    fprintf(g_coeff_csv, "block_id,channel,block_size,coeff_index,is_low_freq,level,abs_level,q,dequantized_value,context_base_if_available\n");
}

static void instrument_block(int block_id, int left, int top, int level, int block_size) {
    if (!g_block_csv) {
        return;
    }
    fprintf(g_block_csv, "%d,%d,%d,%d,%d,1\n", block_id, left, top, level, block_size);
    if (block_size >= 0 && block_size < 65) {
        g_block_count_by_size[block_size]++;
        g_block_pixels_by_size[block_size] += (long long) block_size * block_size;
    }
}

static void instrument_coeff(int block_id, int channel, int block_size, int coeff_index, int level, int value, int context_base) {
    int is_low = coeff_index < (block_size * block_size) / 8;
    int abs_level = value < 0 ? -value : value;
    fprintf(g_coeff_csv, "%d,%d,%d,%d,%d,%d,%d,NA,%d,%d\n",
            block_id, channel, block_size, coeff_index, is_low, level, abs_level, value, context_base);
    if (channel >= 0 && channel < 3 && block_size >= 0 && block_size < 65 && is_low >= 0 && is_low < 2) {
        g_coeff_total[channel][block_size][is_low]++;
        if (value != 0) {
            g_coeff_nonzero[channel][block_size][is_low]++;
        }
        g_coeff_abs_sum[channel][block_size][is_low] += abs_level;
        if (abs_level > g_coeff_max_abs[channel][block_size][is_low]) {
            g_coeff_max_abs[channel][block_size][is_low] = abs_level;
        }
    }
    g_coeff_total_all++;
    if (value != 0) {
        g_coeff_nonzero_all++;
    }
    g_coeff_abs_sum_all += abs_level;
    g_coeff_sq_sum_all += (long long) value * (long long) value;
}

static void instrument_finish(int width, int height, int *planes[3]) {
    if (!g_block_summary || !g_coeff_summary || !g_compression_metrics) {
        return;
    }
    for (int block_size = 1; block_size < 65; ++block_size) {
        if (!g_block_count_by_size[block_size]) {
            continue;
        }
        fprintf(g_block_summary, "%d,%lld,%lld,%.6f\n",
                block_size,
                g_block_count_by_size[block_size],
                g_block_pixels_by_size[block_size],
                g_image_pixels ? (100.0 * (double) g_block_pixels_by_size[block_size] / (double) g_image_pixels) : 0.0);
    }
    fprintf(g_coeff_summary, "channel,block_size,frequency_band,total_coeffs,nonzero_coeffs,nonzero_ratio,mean_abs_level,max_abs_level\n");
    for (int channel = 0; channel < 3; ++channel) {
        for (int block_size = 1; block_size < 65; ++block_size) {
            for (int band = 0; band < 2; ++band) {
                long long total = g_coeff_total[channel][block_size][band];
                if (!total) {
                    continue;
                }
                long long nz = g_coeff_nonzero[channel][block_size][band];
                long long abs_sum = g_coeff_abs_sum[channel][block_size][band];
                int max_abs = g_coeff_max_abs[channel][block_size][band];
                fprintf(g_coeff_summary, "%d,%d,%s,%lld,%lld,%.6f,%.6f,%d\n",
                        channel, block_size, band == 0 ? "low" : "high",
                        total, nz,
                        total ? (double) nz / (double) total : 0.0,
                        total ? (double) abs_sum / (double) total : 0.0,
                        max_abs);
            }
        }
    }

    double sums[3] = {0.0, 0.0, 0.0};
    double sq_sums[3] = {0.0, 0.0, 0.0};
    for (int ch = 0; ch < 3; ++ch) {
        for (int idx = 0; idx < width * height; ++idx) {
            double v = (double) planes[ch][idx];
            sums[ch] += v;
            sq_sums[ch] += v * v;
        }
    }
    fprintf(g_compression_metrics, "metric,value,unit,note\n");
    fprintf(g_compression_metrics, "compressed_bytes,%d,bytes,embedded bitstream length\n", g_compressed_bytes);
    fprintf(g_compression_metrics, "image_width,%d,px,decoded width\n", width);
    fprintf(g_compression_metrics, "image_height,%d,px,decoded height\n", height);
    fprintf(g_compression_metrics, "pixels,%d,px,total decoded pixels\n", width * height);
    fprintf(g_compression_metrics, "bits_per_pixel,%.8f,bits/pixel,compressed_bytes*8/(width*height)\n",
            (double) g_compressed_bytes * 8.0 / (double) (width * height));
    for (int ch = 0; ch < 3; ++ch) {
        double mean = sums[ch] / (double) (width * height);
        double var = sq_sums[ch] / (double) (width * height) - mean * mean;
        fprintf(g_compression_metrics, "decoded_%s_variance,%.8f,unitless,from post-filter decoded plane\n",
                ch == 0 ? "y" : (ch == 1 ? "cg" : "co"), var);
    }
    double coeff_mean = g_coeff_total_all ? (double) g_coeff_abs_sum_all / (double) g_coeff_total_all : 0.0;
    double coeff_var = g_coeff_total_all ? ((double) g_coeff_sq_sum_all / (double) g_coeff_total_all) - coeff_mean * coeff_mean : 0.0;
    fprintf(g_compression_metrics, "residual_coeff_variance_if_available,%.8f,unitless,coefficient-domain variance estimate\n", coeff_var);
    fprintf(g_compression_metrics, "nonzero_coeff_ratio,%.8f,ratio,nonzero_coeffs/total_coeffs\n",
            g_coeff_total_all ? (double) g_coeff_nonzero_all / (double) g_coeff_total_all : 0.0);

    fclose(g_block_csv);
    fclose(g_block_summary);
    fclose(g_coeff_csv);
    fclose(g_coeff_summary);
    fclose(g_compression_metrics);
    g_block_csv = g_block_summary = g_coeff_csv = g_coeff_summary = g_compression_metrics = NULL;
}
#endif

static int t(int c) {
    if (r < 256) r *= 256, p *= 256, p += (x < E) ? *x++ : 0;
    int *m = X + c * 2, F = *m + m[1] + 2, u = r * (*m + 1) / F, k = p >= u;
    k ? (p -= u, r -= u) : (r = u);
    if (++m[k], F > 63) *m /= 2, m[1] /= 2;
    return k;
}
static int n(int c) {
    int a = 0, b = 1;
    while (!t(c + a)) a++;
    while (a--) b = b << 1 | t(4);
    return b - 1;
}
static void R(int *S, int Y, int *T, int V, int U, int d, int L) {
    foreach (h, d)
        foreach (a, d) {
            int g = 0;
            foreach (j, d)
                g += T[j * V + h * U] *
                     (int) lrint(cos(acos(-1) / d * (a + .5) * j) *
                                 sqrt(2 - !j) * 1024);
            S[a * Y + h * U] = (g + (1 << (L - 1))) >> L;
        }
}
static void W(int z, int l, int g) {
    if (g > 5 || (g > 2 && t(g - 3))) {
        int c = 1 << --g;
        foreach (a, 4) W(z + a % 2 * c, l + a / 2 * c, g);
        return;
    }
    int c = 1 << g, d = c * c, q = n(73);
    #if AMEI_INSTRUMENT
    int block_id = g_next_block_id++;
    instrument_block(block_id, z, l, g, c);
    #endif
    foreach (B, 3) {
        int *o = y[B] + l * f + z, I = B > 0;
        foreach (a, d)
            i[a] = 0;
        for (int a = 0; a < d; a++) {
            if (t(61 + g * 2 + I)) break;
            a += n(5 + I * 10);
            int k = 1 - 2 * t(3);
            if (a < d) {
                int value = k * (n(25 + (I + (a < d / 8) * 2) * 10) + 1) * (B ? P : O);
                i[a] = value;
                #if AMEI_INSTRUMENT
                instrument_coeff(block_id, B, c, a, g, value, 25 + (I + (a < d / 8) * 2) * 10);
                #endif
            }
        }
        if (!q) {
            int v = 0;
            foreach (a, c) v += (l ? o[-f + a] : 0) + (z ? o[a * f - 1] : 0);
            *i += z && l ? v / 2 : v;
        }
        R(i + d, 1, i, 1, c, c, 10);
        R(o, f, i + d, c, 1, c, 10 + g);
        if (!q) continue;
        int C = q < 17, w = C ? 9 - q : q - 25;
        foreach (a, c)
            foreach (j, c) {
                int J, k;
                foreach (nb, 2) {
                    int h = a * w + w;
                    J = h & 7, h = (h >> 3) + j + nb;
                    if ((k = h < 0)) h = (h * 8 + w / 2) / w - 2;
                    h = h < c ? h : c - 1;
                    i[nb] =
                        k ^ C ? (z ? o[h * f - 1] : 0) : (l ? o[-f + h] : 0);
                }
                o[C ? j * f + a : a * f + j] +=
                    (*i * (8 - J) + i[1] * J + 4) >> 3;
            }
    }
}
static void D(int *q, int w, int h) {
    for (int b = 4; b <= 64; b <<= 1)
        foreach (d, 2) {
            int nb = d ? w : h, ns = d ? h : w, s = d ? 1 : w;
            for (int j = b; j < nb; j += b)
                foreach (k, ns) {
                    int u = d ? k * w + j : j * w + k, a = q[u - s], v = q[u];
                    if (A(a - v) > 4) {
                        q[u - s] = (3 * a + v + 2) >> 2;
                        q[u] = (a + 3 * v + 2) >> 2;
                        if (j >= 2)
                            q[u - 2 * s] = (7 * q[u - 2 * s] + a + 4) >> 3;
                        if (j + 1 < nb) q[u + s] = (7 * q[u + s] + v + 4) >> 3;
                    }
                }
        }
}
static void F(int *q, int w, int h, int th, int gl)
{
    memcpy(Z, q, (size_t) (w * h) * sizeof(int));
    foreach (a, h)
        foreach (j, w) {
            int u = a * w + j, c = Z[u], g = 0;
            g += j ? A(c - Z[u - 1]) : 0, g += j + 1 < w ? A(c - Z[u + 1]) : 0,
                g += a ? A(c - Z[u - w]) : 0,
                g += a + 1 < h ? A(c - Z[u + w]) : 0;
            if (g > gl) continue;
            int s = c * 4, wt = 4, lt = th - g / 6;
            if (lt < th / 2)
                lt = th / 2;
            for (int dy = -1; dy <= 1; dy++)
                for (int dx = -1; dx <= 1; dx++) {
                    int ny = a + dy, nx = j + dx;
                    if (ny < 0 || ny >= h || nx < 0 || nx >= w || (!dy && !dx))
                        continue;
                    int v = Z[ny * w + nx];
                    if (A(v - c) <= lt) {
                        int sw = (2 - A(dx)) * (2 - A(dy));
                        s += v * sw, wt += sw;
                    }
                }
            q[u] = (s + wt / 2) / wt;
        }
}
int main(void) {
    for (const char *s = H; *s; s += 4) {
        int j = (int) (s - H) / 4 * 3, v = (s[0] - 48) << 18 |
                                           (s[1] - 48) << 12 |
                                           (s[2] - 48) << 6 | (s[3] - 48);
        G[j] = v >> 16; G[j + 1] = v >> 8; G[j + 2] = v;
    }
    x = G, E = G + 776;
    r *= 256, p *= 256, p += (x < E) ? *x++ : 0;
    int g = n(5), N;
    f = 1 << g, N = f - n(5), O = n(5), P = n(5);
    #if AMEI_INSTRUMENT
    instrument_begin(f, N);
    #endif
    W(0, 0, g);
    if (ENABLE_DEBLOCK) {
        foreach (c, 3) D(y[c], f, N);
    }
    if (ENABLE_BILATERAL) {
        F(y[0], f, N, 8, 42); F(y[1], f, N, 14, 56), F(y[2], f, N, 14, 56);
    }
    #if AMEI_INSTRUMENT
    instrument_finish(f, N, y);
    #endif
    /* PNG output: signature, IHDR, IDAT (streaming), IEND */
    unsigned pp = (unsigned) f * 3 + 1;
    ad_a = 1, ad_b = 0;
    foreach (si, 8)
        PB("\x89PNG\r\n\x1a\n"[si]);
    PO("IHDR", 13); P4(f); P4(N); PC(8); PC(2);
    foreach (zi, 3) PC(0);
    PE(); PO("IDAT", 2 + N * (5 + pp) + 4); PC(0x78); PC(1);
    foreach (row, N) {
        PC(row == N - 1); PL(pp); PL(~pp); PA(0);
        foreach (col, f) {
            int idx = row * f + col, d = y[0][idx] - y[1][idx];
            PA(C8(d + y[2][idx]));
            PA(C8(y[0][idx] + y[1][idx]));
            PA(C8(d - y[2][idx]));
        }
    }
    P4((ad_b << 16) | ad_a); PE(); PO("IEND", 0); PE();
}
