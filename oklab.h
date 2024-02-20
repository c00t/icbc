#pragma once
#define _USE_MATH_DEFINES
#include <cmath>
#include <stdint.h>
#include <vector>
template <typename S>
inline S clamp(S value, S low, S high) { return (value < low) ? low : ((value > high) ? high : value); }
const float SCALE_L = 1.0f / 65535.0f;
const float SCALE_A = (1.0f / 65535.0f) * (0.276216f - (-0.233887f));
const float OFS_A = -0.233887f;
const float SCALE_B = (1.0f / 65535.0f) * (0.198570f - (-0.311528f));
const float OFS_B = -0.311528f;

// Oklab have signed a,b
const float MIN_L = 0.000000f, MAX_L = 1.000000f;
const float MIN_A = -0.233888f, MAX_A = 0.276217f;
const float MIN_B = -0.311529f, MAX_B = 0.198570f;

struct Lab
{
    float L;
    float a;
    float b;
};
struct Lab16
{
    uint16_t L, a, b;
};
struct LCh
{
    float L;
    float C;
    float h;
};
struct RGB
{
    float r;
    float g;
    float b;
};
struct Lin
{
    float r;
    float g;
    float b;
};
struct color_rgba
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
};
std::vector<Lab16> g_srgb_to_oklab16;
static float g_srgb_to_linear[256];

Lab linear_srgb_to_oklab(Lin c)
{
    float l = 0.4122214708f * c.r + 0.5363325363f * c.g + 0.0514459929f * c.b;
    float m = 0.2119034982f * c.r + 0.6806995451f * c.g + 0.1073969566f * c.b;
    float s = 0.0883024619f * c.r + 0.2817188376f * c.g + 0.6299787005f * c.b;

    float l_ = cbrtf(l);
    float m_ = cbrtf(m);
    float s_ = cbrtf(s);

    return {
        0.2104542553f * l_ + 0.7936177850f * m_ - 0.0040720468f * s_,
        1.9779984951f * l_ - 2.4285922050f * m_ + 0.4505937099f * s_,
        0.0259040371f * l_ + 0.7827717662f * m_ - 0.8086757660f * s_,
    };
}

Lin oklab_to_linear_srgb(Lab c)
{
    float l_ = c.L + 0.3963377774f * c.a + 0.2158037573f * c.b;
    float m_ = c.L - 0.1055613458f * c.a - 0.0638541728f * c.b;
    float s_ = c.L - 0.0894841775f * c.a - 1.2914855480f * c.b;

    float l = l_ * l_ * l_;
    float m = m_ * m_ * m_;
    float s = s_ * s_ * s_;

    return {
        +4.0767416621f * l - 3.3077115913f * m + 0.2309699292f * s,
        -1.2684380046f * l + 2.6097574011f * m - 0.3413193965f * s,
        -0.0041960863f * l - 0.7034186147f * m + 1.7076147010f * s,
    };
}
/*
vec3 srgb_from_linear_srgb(vec3 x) {

    vec3 xlo = 12.92*x;
    vec3 xhi = 1.055 * pow(x, vec3(0.4166666666666667)) - 0.055;

    return mix(xlo, xhi, step(vec3(0.0031308), x));

}
*/
RGB linear_srgb_to_srgb(Lin c)
{
    // assert all components are in [0,1]
    _ASSERT(c.r >= 0.0f && c.r <= 1.0f);
    _ASSERT(c.g >= 0.0f && c.g <= 1.0f);
    _ASSERT(c.b >= 0.0f && c.b <= 1.0f);

    return {
        c.r <= 0.0031308f ? 12.92f * c.r : 1.055f * powf(c.r, 0.4166666666666667f) - 0.055f,
        c.g <= 0.0031308f ? 12.92f * c.g : 1.055f * powf(c.g, 0.4166666666666667f) - 0.055f,
        c.b <= 0.0031308f ? 12.92f * c.b : 1.055f * powf(c.b, 0.4166666666666667f) - 0.055f,
    };
}
/*
vec3 linear_srgb_from_srgb(vec3 x) {

    vec3 xlo = x / 12.92;
    vec3 xhi = pow((x + 0.055)/(1.055), vec3(2.4));

    return mix(xlo, xhi, step(vec3(0.04045), x));

}
*/
Lin srgb_to_linear_srgb(RGB c)
{
    // assert all components are in [0,1]
    _ASSERT(c.r >= 0.0f && c.r <= 1.0f);
    _ASSERT(c.g >= 0.0f && c.g <= 1.0f);
    _ASSERT(c.b >= 0.0f && c.b <= 1.0f);

    return {
        c.r <= 0.04045f ? c.r / 12.92f : powf((c.r + 0.055f) / 1.055f, 2.4f),
        c.g <= 0.04045f ? c.g / 12.92f : powf((c.g + 0.055f) / 1.055f, 2.4f),
        c.b <= 0.04045f ? c.b / 12.92f : powf((c.b + 0.055f) / 1.055f, 2.4f),
    };
}
static float f_inv(float x)
{
    if (x <= 0.04045f)
        return x / 12.92f;
    else
        return powf(((x + 0.055f) / 1.055f), 2.4f);
}

static void init_srgb_to_linear()
{
    for (uint32_t i = 0; i < 256; i++)
        g_srgb_to_linear[i] = f_inv(i / 255.0f);
}

static void init_oklab_table()
{
    g_srgb_to_oklab16.resize(256 * 256 * 256);
    printf("Computing Oklab table\n");

    for (uint32_t r = 0; r <= 255; r++)
    {
        for (uint32_t g = 0; g <= 255; g++)
        {
            for (uint32_t b = 0; b <= 255; b++)
            {
                color_rgba c = {r, g, b, 255};
                Lab l(linear_srgb_to_oklab({g_srgb_to_linear[c.r], g_srgb_to_linear[c.g], g_srgb_to_linear[c.b]}));

                _ASSERT(l.L >= MIN_L && l.L <= MAX_L);
                _ASSERT(l.a >= MIN_A && l.a <= MAX_A);
                _ASSERT(l.b >= MIN_B && l.b <= MAX_B);

                float lL = std::round(((l.L - MIN_L) / (MAX_L - MIN_L)) * 65535.0f);
                float la = std::round(((l.a - MIN_A) / (MAX_A - MIN_A)) * 65535.0f);
                float lb = std::round(((l.b - MIN_B) / (MAX_B - MIN_B)) * 65535.0f);

                lL = clamp(lL, 0.0f, 65535.0f);
                la = clamp(la, 0.0f, 65535.0f);
                lb = clamp(lb, 0.0f, 65535.0f);

                Lab16 &v = g_srgb_to_oklab16[r + g * 256 + b * 65536];
                v.L = (uint16_t)lL;
                v.a = (uint16_t)la;
                v.b = (uint16_t)lb;

                // Lab cl = srgb_to_oklab(c);
            }
        }
    }
}

static inline Lab srgb_to_oklab(const color_rgba &c)
{
    const Lab16 &l = g_srgb_to_oklab16[c.r + c.g * 256 + c.b * 65536];

    Lab res;
    res.L = l.L * SCALE_L;
    res.a = l.a * SCALE_A + OFS_A;
    res.b = l.b * SCALE_B + OFS_B;

    return res;
}

static inline Lab srgb_to_oklab_norm(const color_rgba &c)
{
    const Lab16 &l = g_srgb_to_oklab16[c.r + c.g * 256 + c.b * 65536];

    Lab res;
    res.L = l.L * SCALE_L;
    res.a = l.a * SCALE_L;
    res.b = l.b * SCALE_L;

    return res;
}
static inline RGB oklab_norm_to_srgb(const Lab &c)
{
    float l = c.L;
    float a = (c.a - OFS_A);
    float b = (c.b - OFS_B);

    Lin ll = oklab_to_linear_srgb({l, a, b});
    return linear_srgb_to_srgb(ll);
}

LCh oklab_to_lch(Lab c)
{
    float C = sqrtf(c.a * c.a + c.b * c.b);
    float h = atan2f(c.b, c.a);
    if (h < 0.0f)
        h += 2.0f * M_PI;
    return {c.L, C, h};
}

Lab lch_to_oklab(LCh c)
{
    return {c.L, c.C * cosf(c.h), c.C * sinf(c.h)};
}