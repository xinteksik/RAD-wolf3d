/* RAD-Wolf bring-up: GPL-3.0-or-later. */
#include "radwolf_file.h"

int radwolf_file_init(radwolf_file *file, const radwolf_file_ops *ops, void *handle)
{
    if (file == NULL || ops == NULL || handle == NULL || ops->read == NULL ||
        ops->seek == NULL || ops->size == NULL || ops->close == NULL)
        return -1;
    file->ops = ops;
    file->handle = handle;
    return 0;
}

static int is_open(const radwolf_file *file)
{
    return file != NULL && file->ops != NULL && file->handle != NULL;
}

int radwolf_file_read_exact(radwolf_file *file, void *dst, size_t bytes)
{
    if (!is_open(file) || (dst == NULL && bytes != 0)) return -1;
    unsigned char *out = dst;
    while (bytes != 0) {
        ptrdiff_t got = file->ops->read(file->handle, out, bytes);
        if (got <= 0 || (size_t)got > bytes) return -1;
        out += got;
        bytes -= (size_t)got;
    }
    return 0;
}

int radwolf_file_seek(radwolf_file *file, uint32_t offset)
{
    return is_open(file) ? file->ops->seek(file->handle, offset) : -1;
}

int radwolf_file_size(radwolf_file *file, uint32_t *bytes)
{
    return is_open(file) && bytes != NULL ? file->ops->size(file->handle, bytes) : -1;
}

int radwolf_file_close(radwolf_file *file)
{
    if (!is_open(file)) return 0;
    int result = file->ops->close(file->handle);
    file->handle = NULL;
    file->ops = NULL;
    return result;
}
