#include <Arduino.h>
#include <LittleFS.h>

bool fs_init(){
    if(!LittleFS.begin(false, "/littlefs", 10)){
        printf("[FS]Error to init littlefs\n");
        return false;
    }

    return true;
}

fs::File fs_open(const char *path, const char *mode, bool create = false){
    return LittleFS.open(String(path), mode, create);
}

size_t fs_size(fs::File *file){
    return file->size();
}

size_t fs_write(fs::File *file, const uint8_t *data, size_t size){
    return file->write(data, size);
}

int fs_read(fs::File *file, uint8_t *buf, size_t size){
    return file->read(buf, size);
}

void fs_close(fs::File *file){
    file->close();
}
