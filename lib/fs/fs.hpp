#ifndef FS_HPP
#define FS_HPP

#include <LittleFS.h>

bool fs_init();

fs::File fs_open(const char *path, const char *mode, bool create = false);
void fs_close(fs::File *file);

size_t fs_size(fs::File *file);
size_t fs_write(fs::File *file, const uint8_t *data, size_t size);

int fs_read(fs::File *file, uint8_t *buf, size_t size);

#endif // FS_HPP
