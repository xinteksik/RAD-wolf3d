/* RAD-Wolf bring-up: GPL-3.0-or-later. Stdio backend for host/newlib. */
#include "radwolf_stdio_file.h"
#include <stdio.h>
#include <limits.h>
#include <string.h>

static ptrdiff_t stdio_read(void *handle, void *dst, size_t bytes)
{
    if (bytes > (size_t)PTRDIFF_MAX) bytes = (size_t)PTRDIFF_MAX;
    size_t got = fread(dst, 1, bytes, handle);
    return ferror((FILE *)handle) ? -1 : (ptrdiff_t)got;
}

static int stdio_seek(void *handle, uint32_t offset)
{
#if LONG_MAX < UINT32_MAX
    if ((uint64_t)offset > (uint64_t)LONG_MAX) return -1;
#endif
    return fseek(handle, (long)offset, SEEK_SET);
}

static int stdio_size(void *handle, uint32_t *bytes)
{
    long position = ftell(handle);
    if (position < 0 || fseek(handle, 0, SEEK_END) != 0) return -1;
    long end = ftell(handle);
    int restored = fseek(handle, position, SEEK_SET);
    if (restored != 0 || end < 0 || (uint64_t)end > UINT32_MAX) return -1;
    *bytes = (uint32_t)end;
    return 0;
}

static int stdio_close(void *handle)
{
    return fclose(handle);
}

int radwolf_stdio_open(radwolf_file *file, const char *path)
{
    static const radwolf_file_ops ops = {stdio_read, stdio_seek, stdio_size, stdio_close};
    if (file == NULL || path == NULL) return -1;
    *file = (radwolf_file){0};
    const char *open_path = path;
#ifdef RADWOLF_BAREMETAL
    char card_path[128];
    if (strncmp(path, "SD:", 3) != 0) {
        int length = snprintf(card_path, sizeof(card_path), "SD:RADWOLF/%s", path);
        if (length < 0 || (size_t)length >= sizeof(card_path)) return -1;
        open_path = card_path;
    }
#endif
    FILE *handle = fopen(open_path, "rb");
    if (handle == NULL) return -1;
    return radwolf_file_init(file, &ops, handle);
}
