/* Amei face decoder: gcc -O2 amei.c -lm && ./a.out > amei.png */
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
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
    foreach (B, 3) {
        int *o = y[B] + l * f + z, I = B > 0;
        foreach (a, d)
            i[a] = 0;
        for (int a = 0; a < d; a++) {
            if (t(61 + g * 2 + I)) break;
            a += n(5 + I * 10);
            int k = 1 - 2 * t(3);
            if (a < d) i[a] =
                    k * (n(25 + (I + (a < d / 8) * 2) * 10) + 1) * (B ? P : O);
        }
        if (!q) {
            int v = 0;
            foreach (a, c) v += (l ? o[-f + a] : 0) + (z ? o[a * f - 1] : 0);
            *i += z && l ? v / 2 : v;
        }
        R(i + d, 1, i, 1, c, c, 10), R(o, f, i + d, c, 1, c, 10 + g);
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
    W(0, 0, g);
    foreach (c, 3) D(y[c], f, N);
    F(y[0], f, N, 8, 42); F(y[1], f, N, 14, 56), F(y[2], f, N, 14, 56);
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