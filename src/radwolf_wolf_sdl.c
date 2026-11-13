/* RAD-Wolf bring-up: minimal Wolf4SDL surface/presentation adapter for RAD. */
#include "baremetal/SDL.h"
#include "baremetal/SDL_mixer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "radwolf_surface.h"
#include "radwolf_flat.h"
#include "radwolf_map.h"

struct SDL_Window { int unused; };
struct SDL_Renderer { int unused; };
struct SDL_Texture {
    int w, h, pitch, bytes_per_pixel;
    uint8_t *pixels;
};

extern uint32_t *DG_ScreenBuffer;
extern int (*functionAddress[])(void);

static SDL_Window window_instance;
static SDL_Renderer renderer_instance;
static SDL_Texture *active_texture;
static const char no_error[] = "RAD-Wolf bare-metal SDL adapter";

int radwolf_wolf_sdl_init(uint32_t flags)
{
    (void)flags;
    return 0;
}

void radwolf_wolf_sdl_quit(void) { }
int radwolf_wolf_joystick_count(void) { return 0; }
const char *radwolf_wolf_sdl_error(void) { return no_error; }

int radwolf_wolf_fill_rect(SDL_Surface *surface, const SDL_Rect *rect,
                           uint32_t color)
{
    if (surface == NULL || surface->pixels == NULL || surface->format == NULL)
        return -1;
    int x0 = rect != NULL ? rect->x : 0;
    int y0 = rect != NULL ? rect->y : 0;
    int x1 = rect != NULL ? x0 + rect->w : surface->w;
    int y1 = rect != NULL ? y0 + rect->h : surface->h;
    if (x0 < 0) x0 = 0;
    if (y0 < 0) y0 = 0;
    if (x1 > surface->w) x1 = surface->w;
    if (y1 > surface->h) y1 = surface->h;
    for (int y = y0; y < y1; ++y) {
        if (surface->format->BytesPerPixel == 1) {
            memset((uint8_t *)surface->pixels + y * surface->pitch + x0,
                   (uint8_t)color, (size_t)(x1 - x0));
        } else if (surface->format->BytesPerPixel == 4) {
            uint32_t *row = (uint32_t *)((uint8_t *)surface->pixels +
                                         y * surface->pitch);
            for (int x = x0; x < x1; ++x) row[x] = color;
        } else {
            return -1;
        }
    }
    return 0;
}

SDL_Window *radwolf_wolf_create_window(const char *title, int x, int y,
                                       int w, int h, uint32_t flags)
{
    (void)title; (void)x; (void)y; (void)w; (void)h; (void)flags;
    return &window_instance;
}

int radwolf_wolf_pixel_format_masks(uint32_t format, int *bits, uint32_t *r,
                                    uint32_t *g, uint32_t *b, uint32_t *a)
{
    (void)format;
    if (!bits || !r || !g || !b || !a) return 0;
    *bits = 32; *r = 0x00ff0000; *g = 0x0000ff00;
    *b = 0x000000ff; *a = 0xff000000;
    return 1;
}

SDL_Surface *radwolf_wolf_create_surface(uint32_t flags, int w, int h,
                                         int depth, uint32_t r, uint32_t g,
                                         uint32_t b, uint32_t a)
{
    (void)flags;
    if (w <= 0 || h <= 0 || (depth != 8 && depth != 32)) return NULL;
    SDL_Surface *surface = calloc(1, sizeof(*surface));
    SDL_PixelFormat *format = calloc(1, sizeof(*format));
    if (!surface || !format) { free(surface); free(format); return NULL; }
    size_t bpp = (size_t)depth / 8;
    if ((size_t)w > SIZE_MAX / bpp || (size_t)h > SIZE_MAX / ((size_t)w * bpp)) {
        free(format); free(surface); return NULL;
    }
    surface->pixels = calloc((size_t)w * (size_t)h, bpp);
    if (!surface->pixels) { free(format); free(surface); return NULL; }
    surface->w = w; surface->h = h; surface->pitch = (int)((size_t)w * bpp);
    surface->format = format;
    format->BitsPerPixel = (uint8_t)depth;
    format->BytesPerPixel = (uint8_t)bpp;
    format->Rmask = r; format->Gmask = g; format->Bmask = b; format->Amask = a;
    if (depth == 8) {
        format->palette = calloc(1, sizeof(*format->palette));
        if (format->palette) {
            format->palette->colors = calloc(256, sizeof(SDL_Color));
            if (format->palette->colors) format->palette->ncolors = 256;
        }
        if (!format->palette || !format->palette->colors) {
            radwolf_wolf_free_surface(surface);
            return NULL;
        }
    }
    return surface;
}

SDL_Renderer *radwolf_wolf_create_renderer(SDL_Window *window, int index,
                                          uint32_t flags)
{ (void)window; (void)index; (void)flags; return &renderer_instance; }
int radwolf_wolf_set_blend_mode(SDL_Renderer *renderer, int mode)
{ (void)renderer; (void)mode; return 0; }
int radwolf_wolf_set_hint(const char *name, const char *value)
{ (void)name; (void)value; return 1; }

int radwolf_wolf_set_palette_colors(SDL_Palette *palette,
                                   const SDL_Color *colors, int first,
                                   int count)
{
    if (!palette || !colors || first < 0 || count < 0 ||
        first + count > palette->ncolors) return -1;
    memcpy(palette->colors + first, colors, (size_t)count * sizeof(*colors));
    return 0;
}

SDL_Texture *radwolf_wolf_create_texture(SDL_Renderer *renderer,
                                        uint32_t format, int access,
                                        int w, int h)
{
    (void)renderer; (void)format; (void)access;
    if (w != 320 || h != 200) return NULL;
    SDL_Texture *texture = calloc(1, sizeof(*texture));
    if (!texture) return NULL;
    texture->w = w; texture->h = h; texture->bytes_per_pixel = 4;
    texture->pitch = w * 4;
    texture->pixels = calloc((size_t)h, (size_t)texture->pitch);
    if (!texture->pixels) { free(texture); return NULL; }
    active_texture = texture;
    return texture;
}

void radwolf_wolf_free_surface(SDL_Surface *surface)
{
    if (!surface) return;
    if (surface->format && surface->format->palette) {
        free(surface->format->palette->colors);
        free(surface->format->palette);
    }
    free(surface->format);
    free(surface->pixels);
    free(surface);
}
void radwolf_wolf_destroy_renderer(SDL_Renderer *renderer) { (void)renderer; }
void radwolf_wolf_destroy_window(SDL_Window *window) { (void)window; }
void radwolf_wolf_destroy_texture(SDL_Texture *texture)
{
    if (texture) {
        if (texture == active_texture) active_texture = NULL;
        free(texture->pixels);
        free(texture);
    }
}

int radwolf_wolf_blit_surface(SDL_Surface *src, const SDL_Rect *src_rect,
                              SDL_Surface *dst, SDL_Rect *dst_rect)
{
    if (!src || !dst || !src->pixels || !dst->pixels || !src->format ||
        !dst->format || src->format->BytesPerPixel != 1 ||
        dst->format->BytesPerPixel != 4 || !src->format->palette ||
        !src->format->palette->colors) return -1;
    int sx = src_rect ? src_rect->x : 0, sy = src_rect ? src_rect->y : 0;
    int dx = dst_rect ? dst_rect->x : 0, dy = dst_rect ? dst_rect->y : 0;
    int w = src_rect ? src_rect->w : src->w;
    int h = src_rect ? src_rect->h : src->h;
    if (w > src->w - sx) w = src->w - sx;
    if (h > src->h - sy) h = src->h - sy;
    if (w > dst->w - dx) w = dst->w - dx;
    if (h > dst->h - dy) h = dst->h - dy;
    if (sx < 0 || sy < 0 || dx < 0 || dy < 0 || w < 0 || h < 0) return -1;
    uint32_t rgb_lut[256], solid_lut[256];
    for (int i = 0; i < src->format->palette->ncolors && i < 256; ++i) {
        SDL_Color c = src->format->palette->colors[i];
        uint32_t rgb = ((uint32_t)c.r << 16) | ((uint32_t)c.g << 8) | c.b;
        rgb_lut[i] = rgb;
        solid_lut[i] = radwolf_flat_tag(rgb);
    }
    for (int y = 0; y < h; ++y) {
        const uint8_t *in = (const uint8_t *)src->pixels + (sy + y) * src->pitch + sx;
        uint32_t *out = (uint32_t *)((uint8_t *)dst->pixels + (dy + y) * dst->pitch) + dx;
        for (int x = 0; x < w; ++x) {
            out[x] = rgb_lut[in[x]];
            if (radwolf_map_colors) {
                out[x] = in[x] == RADWOLF_MAP_PLAYER_COLOR ? UINT32_C(0x8aff5555) :
                         in[x] == RADWOLF_MAP_DIRECTION_COLOR ? UINT32_C(0x81ffffff) :
                         solid_lut[in[x]];
            } else if (radwolf_surface_background(in + x))
                out[x] = solid_lut[in[x]];
        }
    }
    return 0;
}

uint32_t radwolf_wolf_map_rgba(const SDL_PixelFormat *format, uint8_t r,
                               uint8_t g, uint8_t b, uint8_t a)
{ (void)format; (void)a; return ((uint32_t)r << 16) | ((uint32_t)g << 8) | b; }
int radwolf_wolf_update_texture(SDL_Texture *texture, const SDL_Rect *rect,
                                const void *pixels, int pitch)
{
    if (!texture || !pixels || pitch < texture->pitch) return -1;
    int x = rect ? rect->x : 0, y = rect ? rect->y : 0;
    int w = rect ? rect->w : texture->w, h = rect ? rect->h : texture->h;
    if (x < 0 || y < 0 || w < 0 || h < 0 ||
        x + w > texture->w || y + h > texture->h) return -1;
    for (int row = 0; row < h; ++row)
        memcpy(texture->pixels + (y + row) * texture->pitch + x * 4,
               (const uint8_t *)pixels + (y + row) * pitch + x * 4,
               (size_t)w * 4);
    return 0;
}
int radwolf_wolf_render_copy(SDL_Renderer *renderer, SDL_Texture *texture,
                             const SDL_Rect *src, const SDL_Rect *dst)
{ (void)renderer; (void)texture; (void)src; (void)dst; return 0; }
void radwolf_wolf_render_present(SDL_Renderer *renderer)
{
    radwolf_audio_pump();
    (void)renderer;
    if (!DG_ScreenBuffer || !active_texture || !functionAddress || !functionAddress[1])
        return;
    const uint32_t *pixels = (const uint32_t *)active_texture->pixels;
    for (size_t i = 0; i < 320u * 200u; ++i) {
        uint32_t c = pixels[i];
        DG_ScreenBuffer[i] = c;
    }
    (*functionAddress[1])();
    radwolf_audio_pump();
}

int radwolf_wolf_lock_surface(SDL_Surface *surface)
{ return surface && surface->pixels ? 0 : -1; }
void radwolf_wolf_unlock_surface(SDL_Surface *surface) { (void)surface; }
int radwolf_wolf_mouse_state(int *x, int *y)
{ if (x) *x = 0; if (y) *y = 0; return 0; }
void radwolf_wolf_warp_mouse(SDL_Window *window, int x, int y)
{ (void)window; (void)x; (void)y; }
int radwolf_wolf_show_cursor(int toggle) { (void)toggle; return 0; }
void radwolf_wolf_set_window_grab(SDL_Window *window, int grabbed)
{ (void)window; (void)grabbed; }
int radwolf_wolf_set_relative_mouse(int enabled) { (void)enabled; return 0; }
int radwolf_wolf_poll_event(SDL_Event *event) { (void)event; return 0; }
int radwolf_wolf_wait_event(SDL_Event *event) { (void)event; return 0; }
int radwolf_wolf_mod_state(void) { return 0; }
void radwolf_wolf_joystick_update(void) { }
int radwolf_wolf_joystick_axis(SDL_Joystick *joystick, int axis)
{ (void)joystick; (void)axis; return 0; }
uint8_t radwolf_wolf_joystick_hat(SDL_Joystick *joystick, int hat)
{ (void)joystick; (void)hat; return 0; }
int radwolf_wolf_joystick_button(SDL_Joystick *joystick, int button)
{ (void)joystick; (void)button; return 0; }
int radwolf_wolf_joystick_buttons(SDL_Joystick *joystick)
{ (void)joystick; return 0; }
int radwolf_wolf_joystick_hats(SDL_Joystick *joystick)
{ (void)joystick; return 0; }
SDL_Joystick *radwolf_wolf_joystick_open(int index)
{ (void)index; return NULL; }
void radwolf_wolf_joystick_close(SDL_Joystick *joystick) { (void)joystick; }
int radwolf_wolf_event_state(uint32_t type, int state)
{ (void)type; return state; }
int radwolf_wolf_message_box(uint32_t flags, const char *title,
                             const char *message, SDL_Window *window)
{ (void)flags; (void)window; fprintf(stderr, "%s: %s\n", title, message); return 0; }
int radwolf_wolf_save_bmp(SDL_Surface *surface, const char *path)
{ (void)surface; (void)path; return -1; }
