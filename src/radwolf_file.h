/* RAD-Wolf bring-up: GPL-3.0-or-later. Read-only file boundary. */
#ifndef RADWOLF_FILE_H
#define RADWOLF_FILE_H
#include <stddef.h>
#include <stdint.h>

typedef struct radwolf_file_ops {
    /* read: bytes returned, zero at EOF, negative on I/O failure. */
    ptrdiff_t (*read)(void *handle, void *dst, size_t bytes);
    int (*seek)(void *handle, uint32_t offset);
    int (*size)(void *handle, uint32_t *bytes);
    int (*close)(void *handle);
} radwolf_file_ops;

typedef struct radwolf_file {
    const radwolf_file_ops *ops;
    void *handle;
} radwolf_file;

/* The ops object must outlive file. Handle ownership transfers on success. */
int radwolf_file_init(radwolf_file *file, const radwolf_file_ops *ops, void *handle);
/* Exact read loops over short reads. On failure dst may contain a prefix and
 * the file position may have advanced; callers must discard the result. */
int radwolf_file_read_exact(radwolf_file *file, void *dst, size_t bytes);
int radwolf_file_seek(radwolf_file *file, uint32_t offset);
int radwolf_file_size(radwolf_file *file, uint32_t *bytes);
/* Invalidates handle even if backend close fails. Repeated close is harmless. */
int radwolf_file_close(radwolf_file *file);
#endif
