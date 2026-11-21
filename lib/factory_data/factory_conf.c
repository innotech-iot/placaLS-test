#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>

#include "factory_conf.h"

#define FACTORY_CONF_PATH "/lfs/sys/factory.bin"
#define FACTORY_CONF_DIR  "/lfs/sys"

factory_file_t factory_file = { 0 };

static bool factory_file_save(void){
    FILE *file;

    if(mkdir(FACTORY_CONF_DIR, 0777) != 0 && errno != EEXIST){
        return false;
    }

    file = fopen(FACTORY_CONF_PATH, "wb");

    if(file == NULL){
        return false;
    }

    factory_file.magic = FACTORY_CONF_MAGIC;
    factory_file.version = FACTORY_CONF_VERSION;

    size_t written = fwrite(&factory_file, sizeof(factory_file), 1, file);
    fclose(file);

    return written == 1;
}


static bool factory_file_load(void){
    FILE *file;
    factory_file_t loaded;

    file = fopen(FACTORY_CONF_PATH, "rb");

    if(file == NULL){
        return false;
    }

    size_t read = fread(&loaded, sizeof(loaded), 1, file);
    fclose(file);

    if(read != 1){
        return false;
    }

    if(loaded.magic != FACTORY_CONF_MAGIC){
        return false;
    }

    if(loaded.version != FACTORY_CONF_VERSION){
        return false;
    }

    memcpy(&factory_file, &loaded, sizeof(factory_file));
    return true;
}
