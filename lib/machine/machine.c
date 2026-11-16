#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "machine.h"
#include "IOs.h"

#define EMPTY_BUF { 0 }

#define PATH_SIZE sizeof("/lfs/machine/washing/history/metadata.bin")

#define __machine_priv_lib__ static inline
#define __loop__ for(;;)

#define __machine_lock__()                                          \
    (!machine->semph ||                                             \
     xSemaphoreTake(machine->semph, pdMS_TO_TICKS(5000)) == pdTRUE)

#define __machine_unlock__()                \
    do {                                    \
        if(machine->semph){                 \
            xSemaphoreGive(machine->semph); \
        }                                   \
    } while(0)

__machine_priv_lib__ machine_err_t format_history_(machine_select_t select);
__machine_priv_lib__ machine_err_t load_history_metadata__bin_(machine_select_t select);
__machine_priv_lib__ machine_err_t load_history_history__bin_(machine_t *machine, machine_select_t select);
__machine_priv_lib__ machine_err_t save_history_history__bin_(machine_history_t *history, machine_select_t select);
__machine_priv_lib__ void create_path_(const char *dirname, machine_select_t select);

machine_t washing = EMPTY_BUF;
machine_t dryer = EMPTY_BUF;

machine_err_t machine_setup(machine_t *machine, machine_select_t select){
    memset(machine, 0, sizeof(*machine));
    machine->select = select;
    machine_err_t err = load_history_metadata__bin_(select);

    if(err){
        machine->history.state = MACHINE_ERROR;
        machine->history.err = MACHINE_MEM_ERR;
        return err;
    }

    err = load_history_history__bin_(machine, select);

    machine->history.state = err ? MACHINE_ERROR : machine->history.state;
    machine->history.err = err ? MACHINE_MEM_ERR : MACHINE_SYS_OK;

    machine->semph = xSemaphoreCreateMutex();
    machine->boot_verify = true;
    return err;
}

machine_err_t machine_set_state(machine_t *machine, machine_st_t state){
    __machine_lock__();

    machine->history.state = state;
    machine_err_t err = save_history_history__bin_(&machine->history, machine->select);

    if(err){
        machine->history.err = MACHINE_MEM_ERR;
        __machine_unlock__();
        return err;
    }

    __machine_unlock__();
    return MACHINE_OK;
}

machine_err_t machine_set_stage_pumps(machine_t *machine, machine_stage_num_t st_num, machine_pumps_t *pumps, uint8_t counter){
    __machine_lock__();

    memset(machine->history.stage[st_num].st_pumps.p, MACHINE_PUMP_N, sizeof(machine_pumps_num_t[MAX_PUMPS_USED_PER_STAGE]));
    machine->history.stage[st_num].st_pumps.c = counter;
    memcpy(machine->history.stage[st_num].st_pumps.p, pumps, counter * sizeof(machine_pumps_num_t));

    machine_err_t err = save_history_history__bin_(&machine->history, machine->select);

    if(err){
        machine->history.err = MACHINE_MEM_ERR;
        __machine_unlock__();
        return err;
    }

    __machine_unlock__();
    return MACHINE_OK;
}

machine_history_t machine_history(machine_t *machine){
    __machine_lock__();
    machine_history_t h_copy;
    memcpy(&h_copy, &machine->history, sizeof(h_copy));
    __machine_unlock__();

    return h_copy;
}

bool machine_is_locked(machine_t *machine){
    __machine_lock__();
    bool locked = machine->history.locked;
    __machine_unlock__();

    return locked;
}

machine_err_t machine_set_locked(machine_t *machine, machine_bool_t locked){
    __machine_lock__();
    machine->history.locked = locked;
    machine_err_t err = save_history_history__bin_(&machine->history, machine->select);

    if(err){
        machine->history.err = MACHINE_MEM_ERR;
        __machine_unlock__();
        return err;
    }

    __machine_unlock__();
    return MACHINE_OK;
}

__machine_priv_lib__ machine_err_t load_history_metadata__bin_(machine_select_t select){
    char path[PATH_SIZE] = EMPTY_BUF;

    sprintf(path, "/lfs/machine/%s/history/metadata.bin", select == MACHINE_WASHING ? "washing" : "dryer");
    FILE *history_metadata_bin = fopen(path, "rb");

    if(!history_metadata_bin){
        return format_history_(select);
    }

    machine_history_metadata_t metadata = EMPTY_BUF;
    size_t nread = fread(&metadata, sizeof(metadata), 1, history_metadata_bin);

    fclose(history_metadata_bin);

    if(
        nread != 1                                       ||
        metadata.magic_num != MACHINE_METADATA_MAGIC_NUM ||
        metadata.version != MACHINE_METADATA_LAST_VERSION
    ){
        return format_history_(select);
    }

    return MACHINE_OK;
}

__machine_priv_lib__ machine_err_t load_history_history__bin_(machine_t *machine, machine_select_t select){
    char path[PATH_SIZE] = EMPTY_BUF;
    sprintf(path, "/lfs/machine/%s/history/history.bin", select == MACHINE_WASHING ? "washing" : "dryer");

    FILE *history_bin = fopen(path, "rb");

    if(!history_bin){
        return MACHINE_ERR_OPEN_HISTORY;
    }

    memset(&machine->history, 0, sizeof(machine->history));
    size_t nread = fread(&machine->history, sizeof(machine_history_t), 1, history_bin);
    fclose(history_bin);

    return nread != 1 ? MACHINE_ERR_LOAD_HISTORY : MACHINE_OK;
}

__machine_priv_lib__ machine_err_t save_history_history__bin_(machine_history_t *history, machine_select_t select){
    char path[PATH_SIZE] = EMPTY_BUF;
    sprintf(path, "/lfs/machine/%s/history/history.bin", select == MACHINE_WASHING ? "washing" : "dryer");

    FILE *history_bin = fopen(path, "wb");

    if(!history_bin){
        return MACHINE_ERR_OPEN_HISTORY;
    }

    size_t nwritten = fwrite(history, sizeof(*history), 1, history_bin);
    fclose(history_bin);

    return nwritten != 1 ? MACHINE_ERR_SAVE_HISTORY : MACHINE_OK;
}

__machine_priv_lib__ machine_err_t format_history_(machine_select_t select){
    char path[PATH_SIZE] = EMPTY_BUF;
    sprintf(path, "/lfs/machine/%s/history/history.bin", select == MACHINE_WASHING ? "washing" : "dryer");

    FILE *history_bin = fopen(path, "wb");

    if(!history_bin){
        create_path_("history", select);
        history_bin = fopen(path, "wb");

        if(!history_bin){
            return MACHINE_ERR_OPEN_HISTORY;
        }
    }

    machine_history_t history = EMPTY_BUF;
    history.st_num = MACHINE_STAGE_I;
    
    size_t nwritten = fwrite(&history, sizeof(history), 1, history_bin);
    fclose(history_bin);

    if(nwritten != 1){
        return MACHINE_ERR_SAVE_HISTORY;
    }

    sprintf(path, "/lfs/machine/%s/history/metadata.bin", select == MACHINE_WASHING ? "washing" : "dryer");
    FILE *history_metadata_bin = fopen(path, "wb");

    if(!history_metadata_bin){
        return MACHINE_ERR_OPEN_HISTORY;
    }

    machine_history_metadata_t metadata = {
        .magic_num = MACHINE_METADATA_MAGIC_NUM,
        .version   = MACHINE_METADATA_LAST_VERSION
    };

    nwritten = fwrite(&metadata, sizeof(metadata), 1, history_metadata_bin);
    fclose(history_metadata_bin);

    return nwritten != 1 ? MACHINE_ERR_SAVE_HISTORY : MACHINE_OK;
}

__machine_priv_lib__ void create_path_(const char *dirname, machine_select_t select){
    mkdir("/lfs/machine", 0777);

    const char *machine = select == MACHINE_WASHING ? "washing" : "dryer";
    char dir[PATH_SIZE] = EMPTY_BUF;

    sprintf(dir, "/lfs/machine/%s", machine);
    mkdir(dir, 0777);

    sprintf(dir, "/lfs/machine/%s/%s", machine, dirname);
    mkdir(dir, 0777);
}

#undef PATH_SIZE
#undef __loop__
#undef __machine_priv_lib__
