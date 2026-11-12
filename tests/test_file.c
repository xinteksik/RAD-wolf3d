#include "radwolf_file.h"
#include <assert.h>
#include <string.h>

typedef struct fake_file {
    const char *data;
    size_t position;
    int fail;
    unsigned closed;
} fake_file;

static ptrdiff_t fake_read(void *handle, void *dst, size_t bytes)
{
    fake_file *file = handle;
    if (file->fail) return -1;
    size_t available = 6 - file->position;
    if (bytes > available) bytes = available;
    if (bytes > 2) bytes = 2; /* Force partial reads. */
    memcpy(dst, file->data + file->position, bytes);
    file->position += bytes;
    return (ptrdiff_t)bytes;
}

static int fake_seek(void *handle, uint32_t offset)
{
    fake_file *file = handle;
    if (offset > 6 || file->fail) return -1;
    file->position = offset;
    return 0;
}

static int fake_size(void *handle, uint32_t *bytes)
{
    if (((fake_file *)handle)->fail) return -1;
    *bytes = 6;
    return 0;
}

static int fake_close(void *handle)
{
    fake_file *file = handle;
    ++file->closed;
    return file->fail ? -1 : 0;
}

void test_files(void)
{
    const radwolf_file_ops ops = {fake_read, fake_seek, fake_size, fake_close};
    fake_file fake = {"abcdef", 0, 0, 0};
    radwolf_file file = {0};
    char data[8] = {0};
    uint32_t bytes = 0;
    assert(radwolf_file_read_exact(&file, data, 6) == -1);
    assert(radwolf_file_init(&file, &ops, &fake) == 0);
    assert(radwolf_file_size(&file, &bytes) == 0 && bytes == 6);
    assert(radwolf_file_read_exact(&file, data, 6) == 0);
    assert(memcmp(data, "abcdef", 6) == 0);
    assert(radwolf_file_read_exact(&file, data, 1) == -1); /* EOF */
    assert(radwolf_file_seek(&file, 2) == 0);
    assert(radwolf_file_read_exact(&file, data, 2) == 0 && data[0] == 'c');
    assert(radwolf_file_seek(&file, 7) == -1);
    assert(radwolf_file_read_exact(&file, NULL, 1) == -1);
    assert(radwolf_file_read_exact(&file, NULL, 0) == 0);
    fake.fail = 1;
    assert(radwolf_file_read_exact(&file, data, 1) == -1);
    assert(radwolf_file_close(&file) == -1);
    assert(radwolf_file_close(&file) == 0 && fake.closed == 1);
    assert(radwolf_file_seek(&file, 0) == -1);
}
