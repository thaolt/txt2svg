#include <stdint.h>

// Include stdlib.h only for native builds (not WebAssembly)
#ifndef __wasm__
    #include <stdlib.h>
    #define STBTT_malloc(x,u)  (malloc(x))
    #define STBTT_free(x,u)    (free(x))
    #define WASM_EXPORT
#else
// Basic type definitions to avoid standard library dependencies
#ifndef NULL
#define NULL ((void*)0)
#endif

#ifndef size_t
#define size_t unsigned int
#endif

// Custom math implementations to avoid libmath dependency

// Forward declarations
static float stbtt_abs(float x);
static float stbtt_sqrt(float x);
static float stbtt_log(float x);
static float stbtt_exp(float x);

static float stbtt_abs(float x) {
    return x < 0 ? -x : x;
}

static int stbtt_ifloor(float x) {
    int i = (int)x;
    return i - (x < i && x != i);
}

static int stbtt_iceil(float x) {
    int i = (int)x;
    return i + (x > i && x != i);
}

// Simple square root using Newton's method
static float stbtt_sqrt(float x) {
    if (x <= 0) return 0;
    float guess = x;
    float prev = 0;
    while (stbtt_abs(guess - prev) > 0.0001f) {
        prev = guess;
        guess = 0.5f * (guess + x / guess);
    }
    return guess;
}

// Natural logarithm approximation
static float stbtt_log(float x) {
    if (x <= 0) return 0;
    if (x == 1.0f) return 0;
    
    // Use log approximation: log(x) ≈ 2 * (x-1)/(x+1) for x near 1
    float y = (x - 1.0f) / (x + 1.0f);
    return 2.0f * y * (1.0f + y*y/3.0f + y*y*y*y/5.0f);
}

// Exponential function approximation
static float stbtt_exp(float x) {
    if (x == 0) return 1.0f;
    if (x < 0) return 1.0f / stbtt_exp(-x);
    
    // Use Taylor series: e^x ≈ 1 + x + x²/2! + x³/3! + ...
    float result = 1.0f;
    float term = 1.0f;
    for (int i = 1; i <= 10; i++) {
        term *= x / i;
        result += term;
    }
    return result;
}

// Simple power function for positive integer exponents
static float stbtt_pow(float x, float y) {
    if (y == 0) return 1.0f;
    if (y == 1) return x;
    
    // Handle negative exponents
    if (y < 0) {
        return 1.0f / stbtt_pow(x, -y);
    }
    
    // For integer exponents, use multiplication
    int int_y = (int)y;
    if (y == int_y && int_y > 0) {
        float result = 1.0f;
        for (int i = 0; i < int_y; i++) {
            result *= x;
        }
        return result;
    }
    
    // For fractional exponents, use exp(y * log(x)) approximation
    // This is a simplified version - for stb_truetype we mainly need cube roots
    return stbtt_exp(y * stbtt_log(x));
}

// Simple fmod implementation
static float stbtt_fmod(float x, float y) {
    if (y == 0) return 0;
    int div = (int)(x / y);
    return x - div * y;
}

// Simple cosine using Taylor series (limited range for stb_truetype)
static float stbtt_cos(float x) {
    // Normalize to [-π, π]
    float pi = 3.14159265359f;
    while (x > pi) x -= 2 * pi;
    while (x < -pi) x += 2 * pi;
    
    // Taylor series approximation
    float x2 = x * x;
    float x4 = x2 * x2;
    float x6 = x4 * x2;
    return 1.0f - x2/2.0f + x4/24.0f - x6/720.0f;
}

// Simple arccosine using approximation
static float stbtt_acos(float x) {
    // Clamp input to [-1, 1]
    if (x > 1.0f) x = 1.0f;
    if (x < -1.0f) x = -1.0f;
    
    // Simple approximation: acos(x) ≈ π/2 - x - x³/6 for small x
    // For better accuracy, use polynomial approximation
    float abs_x = stbtt_abs(x);
    if (abs_x <= 0.5f) {
        float x2 = x * x;
        float x3 = x2 * x;
        return 1.57079632679f - x - x3/6.0f;
    } else {
        // For larger values, use different approximation
        return stbtt_sqrt(1.0f - x) * (1.57079632679f + 0.5f * x);
    }
}

// Define the STBTT macros to use our custom functions
#define STBTT_ifloor(x)   stbtt_ifloor(x)
#define STBTT_iceil(x)    stbtt_iceil(x)
#define STBTT_sqrt(x)     stbtt_sqrt(x)
#define STBTT_pow(x,y)    stbtt_pow(x,y)
#define STBTT_fmod(x,y)   stbtt_fmod(x,y)
#define STBTT_cos(x)      stbtt_cos(x)
#define STBTT_acos(x)     stbtt_acos(x)
#define STBTT_fabs(x)     stbtt_abs(x)

// Define custom memory allocation functions based on build target
#define STBTT_malloc(x,u)  ((void)(u), (void*)wasm_alloc(x))
#define STBTT_free(x,u)    ((void)(u), (void)(x))

// Define custom assert to avoid assert.h
#define STBTT_assert(x)    ((void)0)

// Define custom memory functions to avoid string.h
static void* stbtt_memcpy(void* dest, const void* src, unsigned int n) {
    char* d = (char*)dest;
    const char* s = (const char*)src;
    while (n--) *d++ = *s++;
    return dest;
}

static void* stbtt_memset(void* ptr, int value, unsigned int n) {
    char* p = (char*)ptr;
    while (n--) *p++ = (char)value;
    return ptr;
}

#define STBTT_memcpy stbtt_memcpy
#define STBTT_memset stbtt_memset

// Define custom strlen to avoid string.h
static unsigned int stbtt_strlen(const char* str) {
    unsigned int len = 0;
    while (str[len]) len++;
    return len;
}
#define STBTT_strlen stbtt_strlen

#define WASM_EXPORT __attribute__((visibility("default")))

// ---------------- memory ----------------
extern uint8_t __heap_base;
static uint32_t heap_ptr = 0;

WASM_EXPORT
void wasm_reset_heap() {
    heap_ptr = (uint32_t)&__heap_base;
}

WASM_EXPORT
uint32_t wasm_alloc(uint32_t sz) {
    uint32_t p = heap_ptr;
    heap_ptr += sz;
    return p;
}

#endif // __wasm__


#define STBTT_STATIC
#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

// Include utf8.h only for native builds (not WebAssembly)
#ifndef __wasm__
    #include "utf8.h"
#else
// Minimal UTF-8 codepoint decoder for WASM
typedef int32_t utf8_int32_t;
typedef char utf8_int8_t;

static utf8_int8_t* utf8codepoint(const utf8_int8_t* str, utf8_int32_t* out_codepoint) {
    utf8_int32_t cp = 0;
    int bytes = 0;

    if (!str || !out_codepoint) return (utf8_int8_t*)str;

    if ((str[0] & 0x80) == 0) {
        // 1-byte sequence (0xxxxxxx)
        cp = str[0];
        bytes = 1;
    } else if ((str[0] & 0xE0) == 0xC0) {
        // 2-byte sequence (110xxxxx 10xxxxxx)
        cp = ((str[0] & 0x1F) << 6) | (str[1] & 0x3F);
        bytes = 2;
    } else if ((str[0] & 0xF0) == 0xE0) {
        // 3-byte sequence (1110xxxx 10xxxxxx 10xxxxxx)
        cp = ((str[0] & 0x0F) << 12) | ((str[1] & 0x3F) << 6) | (str[2] & 0x3F);
        bytes = 3;
    } else if ((str[0] & 0xF8) == 0xF0) {
        // 4-byte sequence (11110xxx 10xxxxxx 10xxxxxx 10xxxxxx)
        cp = ((str[0] & 0x07) << 18) | ((str[1] & 0x3F) << 12) | ((str[2] & 0x3F) << 6) | (str[3] & 0x3F);
        bytes = 4;
    } else {
        // Invalid UTF-8, treat as single byte
        cp = str[0];
        bytes = 1;
    }

    *out_codepoint = cp;
    return (utf8_int8_t*)(str + bytes);
}
#endif

// ---------------- utils ----------------
static uint32_t write_str(char *o, uint32_t n, const char *s) {
    while (*s) o[n++] = *s++;
    return n;
}

static uint32_t write_int(char *o, uint32_t n, int v) {
    char b[16];
    int i = 0;
    if (v < 0) { o[n++]='-'; v=-v; }
    if (!v) b[i++]='0';
    while (v) { b[i++]='0'+(v%10); v/=10; }
    while (i--) o[n++]=b[i];
    return n;
}

static uint32_t write_float(char *o, uint32_t n, float f) {
    int i = (int)f;
    n = write_int(o, n, i);
    o[n++]='.';
    int frac = (int)((f - i) * 1000);
    if (frac < 0) frac = -frac;
    if (frac < 100) o[n++]='0';
    if (frac < 10)  o[n++]='0';
    return write_int(o, n, frac);
}

// ---------------- main ----------------
WASM_EXPORT
uint32_t wasm_generate_svg(
    void* text_ptr,
    void* font_ptr,
    uint32_t font_size,
    void* color_ptr,
    void* out_ptr,
    uint32_t out_max
) {
    char *text = (char*)text_ptr;
    unsigned char *font_data = (unsigned char*)font_ptr;
    char *color = (char*)color_ptr;
    char *out = (char*)out_ptr;

    char *drawBuf = out + 256;
    uint32_t drawBufSize = out_max - 256;
    uint32_t drawOff = 0;

    stbtt_fontinfo font;
    if (!stbtt_InitFont(&font, font_data, 0))
        return 0;

    float scale = stbtt_ScaleForPixelHeight(&font, 64);

    int ascent, descent, gap;
    stbtt_GetFontVMetrics(&font, &ascent, &descent, &gap);

    int baseline = (int)(ascent * scale);

    // Track bounding box
    float min_x = 1e30f, max_x = -1e30f;
    float min_y = 1e30f, max_y = -1e30f;

    int pen_x = 10;

    const char *p = text;
    int prev_glyph = 0;

    while (*p) {
        utf8_int32_t codepoint;
        p = (const char*)utf8codepoint((const utf8_int8_t*)p, &codepoint);
        int glyph = stbtt_FindGlyphIndex(&font, codepoint);

        stbtt_vertex *v;
        int n = stbtt_GetGlyphShape(&font, glyph, &v);

        // Start a new path for this character
        if (drawOff + 20 < drawBufSize) {
            drawOff = write_str(drawBuf, drawOff, "<path fill='");
            drawOff = write_str(drawBuf, drawOff, color);
            drawOff = write_str(drawBuf, drawOff, "' d='");
        }

        for (int j = 0; j < n; j++) {
            stbtt_vertex *p = &v[j];

            float x = pen_x + p->x * scale;
            float y = baseline - p->y * scale;

            // Update bounding box
            if (x < min_x) min_x = x;
            if (x > max_x) max_x = x;
            if (y < min_y) min_y = y;
            if (y > max_y) max_y = y;

            if (p->type == STBTT_vmove) {
                if (drawOff + 50 < drawBufSize) {
                    drawBuf[drawOff++]='M';
                    drawOff = write_float(drawBuf, drawOff, x);
                    drawBuf[drawOff++]=' ';
                    drawOff = write_float(drawBuf, drawOff, y);
                }
            }
            else if (p->type == STBTT_vline) {
                if (drawOff + 50 < drawBufSize) {
                    drawBuf[drawOff++]='L';
                    drawOff = write_float(drawBuf, drawOff, x);
                    drawBuf[drawOff++]=' ';
                    drawOff = write_float(drawBuf, drawOff, y);
                }
            }
            else if (p->type == STBTT_vcurve) {
                if (drawOff + 100 < drawBufSize) {
                    drawBuf[drawOff++]='Q';
                    drawOff = write_float(drawBuf, drawOff,
                        pen_x + p->cx * scale);
                    drawBuf[drawOff++]=' ';
                    drawOff = write_float(drawBuf, drawOff,
                        baseline - p->cy * scale);
                    drawBuf[drawOff++]=' ';
                    drawOff = write_float(drawBuf, drawOff, x);
                    drawBuf[drawOff++]=' ';
                    drawOff = write_float(drawBuf, drawOff, y);
                }
            }
        }

        // Close the current character's path
        if (drawOff + 10 < drawBufSize) {
            drawOff = write_str(drawBuf, drawOff, "'/>");
        }

        stbtt_FreeShape(&font, v);

        int ax;
        stbtt_GetGlyphHMetrics(&font, glyph, &ax, 0);
        pen_x += (int)(ax * scale);

        // Apply kerning between current and next glyph
        if (*p) {
            utf8_int32_t next_codepoint;
            const char *next_ptr = (const char*)utf8codepoint((const utf8_int8_t*)p, &next_codepoint);
            int next_glyph = stbtt_FindGlyphIndex(&font, next_codepoint);
            pen_x += (int)(scale * stbtt_GetGlyphKernAdvance(&font, glyph, next_glyph));
        }
    }

    // Calculate dimensions from bounding box
    int width = (int)(max_x - min_x) + 20; // Add padding
    int height = (int)(max_y - min_y) + 20;
    int viewBox_x = (int)min_x - 10;
    int viewBox_y = (int)min_y - 10;

    uint32_t off = 0;

    // Write complete SVG header with actual dimensions
    off = write_str(out, off, "<svg xmlns='http://www.w3.org/2000/svg'");
    off = write_str(out, off, " width='");
    off = write_int(out, off, width);
    off = write_str(out, off, "' height='");
    off = write_int(out, off, height);
    off = write_str(out, off, "' viewBox='");
    off = write_int(out, off, viewBox_x);
    off = write_str(out, off, " ");
    off = write_int(out, off, viewBox_y);
    off = write_str(out, off, " ");
    off = write_int(out, off, width);
    off = write_str(out, off, " ");
    off = write_int(out, off, height);
    off = write_str(out, off, "'><g>");

    // Copy path data from drawBuf (now contains individual <path> elements)
    for (uint32_t i = 0; i < drawOff && off + 1 < out_max; i++) {
        out[off++] = drawBuf[i];
    }

    off = write_str(out, off, "</g></svg>");
    out[off]=0;
    return off;
}
