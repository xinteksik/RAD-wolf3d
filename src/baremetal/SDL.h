/* RAD-Wolf bring-up: SDL2 ABI declarations for the bare-metal adapter.
 * SDL services are supplied by RAD/Circle; this header is not SDL itself. */
#ifndef RADWOLF_BAREMETAL_TYPES_H
#define RADWOLF_BAREMETAL_TYPES_H
#include <stdint.h>
typedef uint8_t Uint8;
typedef int16_t Sint16;
#include "../radwolf_wolf_clock.h"
#define SDL_Delay radwolf_wolf_delay
#define SDL_GetTicks radwolf_wolf_milliseconds
#define SDL_FillRect radwolf_wolf_fill_rect
#define SDL_Init radwolf_wolf_sdl_init
#define SDL_Quit radwolf_wolf_sdl_quit
#define SDL_NumJoysticks radwolf_wolf_joystick_count
#define SDL_GetError radwolf_wolf_sdl_error
#define SDL_GetRelativeMouseState radwolf_wolf_mouse_state
#define SDL_ShowSimpleMessageBox radwolf_wolf_message_box
#define SDL_SaveBMP radwolf_wolf_save_bmp
#define SDL_TEXTINPUTEVENT_TEXT_SIZE 32
#define SDL_BlitSurface radwolf_wolf_blit_surface
#define SDL_MapRGBA radwolf_wolf_map_rgba
#define SDL_UpdateTexture radwolf_wolf_update_texture
#define SDL_RenderCopy radwolf_wolf_render_copy
#define SDL_RenderPresent radwolf_wolf_render_present
#define SDL_EventState radwolf_wolf_event_state
#define SDL_GetModState radwolf_wolf_mod_state
#define SDL_GetMouseState radwolf_wolf_mouse_state
#define SDL_JoystickUpdate radwolf_wolf_joystick_update
#define SDL_JoystickGetAxis radwolf_wolf_joystick_axis
#define SDL_JoystickGetHat radwolf_wolf_joystick_hat
#define SDL_JoystickGetButton radwolf_wolf_joystick_button
#define SDL_JoystickNumButtons radwolf_wolf_joystick_buttons
#define SDL_JoystickNumHats radwolf_wolf_joystick_hats
#define SDL_JoystickOpen radwolf_wolf_joystick_open
#define SDL_JoystickClose radwolf_wolf_joystick_close
#define SDL_PollEvent radwolf_wolf_poll_event
#define SDL_WaitEvent radwolf_wolf_wait_event
#define SDL_ShowCursor radwolf_wolf_show_cursor
#define SDL_SetWindowGrab radwolf_wolf_set_window_grab
#define SDL_SetRelativeMouseMode radwolf_wolf_set_relative_mouse
#define SDL_WarpMouseInWindow radwolf_wolf_warp_mouse
#define SDL_FreeSurface radwolf_wolf_free_surface
#define SDL_DestroyRenderer radwolf_wolf_destroy_renderer
#define SDL_DestroyWindow radwolf_wolf_destroy_window
#define SDL_DestroyTexture radwolf_wolf_destroy_texture
#define SDL_CreateWindow radwolf_wolf_create_window
#define SDL_PixelFormatEnumToMasks radwolf_wolf_pixel_format_masks
#define SDL_CreateRGBSurface radwolf_wolf_create_surface
#define SDL_CreateRenderer radwolf_wolf_create_renderer
#define SDL_SetRenderDrawBlendMode radwolf_wolf_set_blend_mode
#define SDL_SetHint radwolf_wolf_set_hint
#define SDL_SetPaletteColors radwolf_wolf_set_palette_colors
#define SDL_CreateTexture radwolf_wolf_create_texture
#define SDL_LockSurface radwolf_wolf_lock_surface
#define SDL_UnlockSurface radwolf_wolf_unlock_surface
typedef struct SDL_Surface SDL_Surface;
typedef struct SDL_Window SDL_Window;
typedef struct SDL_Renderer SDL_Renderer;
typedef struct SDL_Texture SDL_Texture;
typedef struct SDL_Rect { int x, y, w, h; } SDL_Rect;
typedef struct SDL_Color { uint8_t r, g, b, a; } SDL_Color;
typedef struct SDL_Palette { int ncolors; SDL_Color *colors; } SDL_Palette;
typedef struct SDL_PixelFormat {
    uint32_t format;
    SDL_Palette *palette;
    uint8_t BitsPerPixel, BytesPerPixel;
    uint32_t Rmask, Gmask, Bmask, Amask;
} SDL_PixelFormat;
struct SDL_Surface {
    uint32_t flags;
    SDL_PixelFormat *format;
    int w, h, pitch;
    void *pixels;
};
typedef struct SDL_Joystick SDL_Joystick;
typedef struct SDL_Keysym { int scancode; int sym; uint16_t mod; uint32_t unused; } SDL_Keysym;
typedef struct SDL_KeyboardEvent { uint32_t type; uint8_t state, repeat; SDL_Keysym keysym; } SDL_KeyboardEvent;
typedef struct SDL_TextInputEvent { uint32_t type; char text[SDL_TEXTINPUTEVENT_TEXT_SIZE]; } SDL_TextInputEvent;
typedef union SDL_Event {
    uint32_t type;
    SDL_KeyboardEvent key;
    SDL_TextInputEvent text;
} SDL_Event;
int radwolf_wolf_fill_rect(SDL_Surface *surface, const SDL_Rect *rect,
                           uint32_t color);
int radwolf_wolf_sdl_init(uint32_t flags);
void radwolf_wolf_sdl_quit(void);
int radwolf_wolf_joystick_count(void);
const char *radwolf_wolf_sdl_error(void);
int radwolf_wolf_mouse_state(int *x, int *y);
int radwolf_wolf_message_box(uint32_t flags, const char *title,
                             const char *message, SDL_Window *window);
int radwolf_wolf_save_bmp(SDL_Surface *surface, const char *path);
int SDL_SetPaletteColors(SDL_Palette *palette, const SDL_Color *colors,
                         int first, int count);
int SDL_LockSurface(SDL_Surface *surface);
void SDL_UnlockSurface(SDL_Surface *surface);
#define SDL_MUSTLOCK(surface) 0
#define SDL_HAT_RIGHT 0x02
#define SDL_HAT_LEFT 0x08
#define SDL_HAT_DOWN 0x04
#define SDL_HAT_UP 0x01
#define SDL_MOUSEMOTION 0x400
#define SDL_IGNORE 0
#define SDL_QUIT 0x100
#define SDL_KEYDOWN 0x300
#define SDL_KEYUP 0x301
#define SDL_TEXTINPUT 0x303
#define SDL_BUTTON_LMASK SDL_BUTTON(SDL_BUTTON_LEFT)
#define SDL_BUTTON_MMASK SDL_BUTTON(SDL_BUTTON_MIDDLE)
#define SDL_BUTTON_RMASK SDL_BUTTON(SDL_BUTTON_RIGHT)
#define KMOD_NUM 0x1000
int radwolf_wolf_blit_surface(SDL_Surface *src, const SDL_Rect *src_rect,
                              SDL_Surface *dst, SDL_Rect *dst_rect);
uint32_t radwolf_wolf_map_rgba(const SDL_PixelFormat *format, uint8_t r,
                               uint8_t g, uint8_t b, uint8_t a);
int radwolf_wolf_update_texture(SDL_Texture *texture, const SDL_Rect *rect,
                                const void *pixels, int pitch);
int radwolf_wolf_render_copy(SDL_Renderer *renderer, SDL_Texture *texture,
                             const SDL_Rect *src, const SDL_Rect *dst);
void radwolf_wolf_render_present(SDL_Renderer *renderer);
int radwolf_wolf_event_state(uint32_t type, int state);
int radwolf_wolf_mod_state(void);
void radwolf_wolf_joystick_update(void);
int radwolf_wolf_joystick_axis(SDL_Joystick *joystick, int axis);
uint8_t radwolf_wolf_joystick_hat(SDL_Joystick *joystick, int hat);
int radwolf_wolf_joystick_button(SDL_Joystick *joystick, int button);
int radwolf_wolf_joystick_buttons(SDL_Joystick *joystick);
int radwolf_wolf_joystick_hats(SDL_Joystick *joystick);
SDL_Joystick *radwolf_wolf_joystick_open(int index);
void radwolf_wolf_joystick_close(SDL_Joystick *joystick);
void radwolf_wolf_warp_mouse(SDL_Window *window, int x, int y);
int radwolf_wolf_show_cursor(int toggle);
void radwolf_wolf_set_window_grab(SDL_Window *window, int grabbed);
int radwolf_wolf_set_relative_mouse(int enabled);
int radwolf_wolf_poll_event(SDL_Event *event);
int radwolf_wolf_wait_event(SDL_Event *event);
void radwolf_wolf_free_surface(SDL_Surface *surface);
void radwolf_wolf_destroy_renderer(SDL_Renderer *renderer);
void radwolf_wolf_destroy_window(SDL_Window *window);
void radwolf_wolf_destroy_texture(SDL_Texture *texture);
SDL_Window *radwolf_wolf_create_window(const char *title, int x, int y,
                                       int w, int h, uint32_t flags);
int radwolf_wolf_pixel_format_masks(uint32_t format, int *bits, uint32_t *r,
                                    uint32_t *g, uint32_t *b, uint32_t *a);
SDL_Surface *radwolf_wolf_create_surface(uint32_t flags, int w, int h,
                                         int depth, uint32_t r, uint32_t g,
                                         uint32_t b, uint32_t a);
SDL_Renderer *radwolf_wolf_create_renderer(SDL_Window *window, int index,
                                          uint32_t flags);
int radwolf_wolf_set_blend_mode(SDL_Renderer *renderer, int mode);
int radwolf_wolf_set_hint(const char *name, const char *value);
int radwolf_wolf_set_palette_colors(SDL_Palette *palette,
                                   const SDL_Color *colors, int first,
                                   int count);
SDL_Texture *radwolf_wolf_create_texture(SDL_Renderer *renderer,
                                        uint32_t format, int access,
                                        int w, int h);
int radwolf_wolf_lock_surface(SDL_Surface *surface);
void radwolf_wolf_unlock_surface(SDL_Surface *surface);
#define SDL_WINDOWPOS_UNDEFINED 0x1FFF0000
#define SDL_WINDOW_FULLSCREEN 0x00000001u
#define SDL_WINDOW_OPENGL 0x00000002u
#define SDL_PIXELFORMAT_ARGB8888 0x16362004u
#define SDL_RENDERER_ACCELERATED 0x00000002u
#define SDL_RENDERER_PRESENTVSYNC 0x00000004u
#define SDL_BLENDMODE_BLEND 1
#define SDL_HINT_RENDER_SCALE_QUALITY "SDL_RENDER_SCALE_QUALITY"
#define SDL_TEXTUREACCESS_STREAMING 1
#define SDL_DISABLE 0
#define SDL_MESSAGEBOX_ERROR 0x10u
#define SDL_MESSAGEBOX_INFORMATION 0x40u
#define SDL_BUTTON_LEFT 1
#define SDL_BUTTON_MIDDLE 2
#define SDL_BUTTON_RIGHT 3
#define SDL_BUTTON(button) (1 << ((button) - 1))
#define SDL_ALPHA_OPAQUE 255
#define SDL_NUM_SCANCODES 512
#define SDL_INIT_VIDEO 0x00000020u
#define SDL_INIT_AUDIO 0x00000010u
#define SDL_INIT_JOYSTICK 0x00000200u
/* SDL2 stable scancode values used by Wolf4SDL's keyboard tables. */
#define SDL_SCANCODE_UNKNOWN 0
#define SDL_SCANCODE_A 4
#define SDL_SCANCODE_B 5
#define SDL_SCANCODE_C 6
#define SDL_SCANCODE_D 7
#define SDL_SCANCODE_E 8
#define SDL_SCANCODE_F 9
#define SDL_SCANCODE_G 10
#define SDL_SCANCODE_H 11
#define SDL_SCANCODE_I 12
#define SDL_SCANCODE_J 13
#define SDL_SCANCODE_K 14
#define SDL_SCANCODE_L 15
#define SDL_SCANCODE_M 16
#define SDL_SCANCODE_N 17
#define SDL_SCANCODE_O 18
#define SDL_SCANCODE_P 19
#define SDL_SCANCODE_Q 20
#define SDL_SCANCODE_R 21
#define SDL_SCANCODE_S 22
#define SDL_SCANCODE_T 23
#define SDL_SCANCODE_U 24
#define SDL_SCANCODE_V 25
#define SDL_SCANCODE_W 26
#define SDL_SCANCODE_X 27
#define SDL_SCANCODE_Y 28
#define SDL_SCANCODE_Z 29
#define SDL_SCANCODE_1 30
#define SDL_SCANCODE_2 31
#define SDL_SCANCODE_3 32
#define SDL_SCANCODE_4 33
#define SDL_SCANCODE_5 34
#define SDL_SCANCODE_6 35
#define SDL_SCANCODE_7 36
#define SDL_SCANCODE_8 37
#define SDL_SCANCODE_9 38
#define SDL_SCANCODE_0 39
#define SDL_SCANCODE_RETURN 40
#define SDL_SCANCODE_ESCAPE 41
#define SDL_SCANCODE_BACKSPACE 42
#define SDL_SCANCODE_TAB 43
#define SDL_SCANCODE_SPACE 44
#define SDL_SCANCODE_MINUS 45
#define SDL_SCANCODE_EQUALS 46
#define SDL_SCANCODE_CAPSLOCK 57
#define SDL_SCANCODE_F1 58
#define SDL_SCANCODE_F2 59
#define SDL_SCANCODE_F3 60
#define SDL_SCANCODE_F4 61
#define SDL_SCANCODE_F5 62
#define SDL_SCANCODE_F6 63
#define SDL_SCANCODE_F7 64
#define SDL_SCANCODE_F8 65
#define SDL_SCANCODE_F9 66
#define SDL_SCANCODE_F10 67
#define SDL_SCANCODE_F11 68
#define SDL_SCANCODE_F12 69
#define SDL_SCANCODE_SCROLLLOCK 71
#define SDL_SCANCODE_PAUSE 72
#define SDL_SCANCODE_INSERT 73
#define SDL_SCANCODE_HOME 74
#define SDL_SCANCODE_PAGEUP 75
#define SDL_SCANCODE_DELETE 76
#define SDL_SCANCODE_END 77
#define SDL_SCANCODE_PAGEDOWN 78
#define SDL_SCANCODE_RIGHT 79
#define SDL_SCANCODE_LEFT 80
#define SDL_SCANCODE_DOWN 81
#define SDL_SCANCODE_UP 82
#define SDL_SCANCODE_KP_ENTER 88
#define SDL_SCANCODE_KP_2 90
#define SDL_SCANCODE_KP_4 92
#define SDL_SCANCODE_KP_5 93
#define SDL_SCANCODE_KP_6 94
#define SDL_SCANCODE_KP_8 96
#define SDL_SCANCODE_LCTRL 224
#define SDL_SCANCODE_LSHIFT 225
#define SDL_SCANCODE_LALT 226
#define SDL_SCANCODE_RCTRL 228
#define SDL_SCANCODE_RSHIFT 229
#define SDL_SCANCODE_RALT 230
#endif
