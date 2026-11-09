#include <string.h>
#include "log.h"

#define LOG_BUF_TIMESTAMP_OFF           0
#define LOG_BUF_WASHING_START_EPOCH_OFF 1
#define LOG_BUF_DRYER_START_EPOCH_OFF   2
#define LOG_BUF_TIMERS_N                3

#define LOG_BUF_WSHBITS_OFF 0
#define LOG_BUF_DRYBITS_OFF 1
#define LOG_BUF_STATUS_N    2

#define LOG_BUF_IO_OFF 0
#define LOG_BUF_IO_N   1

#define LOG_BUF_PUMPS_OFF 0
#define LOG_BUF_PUMPS_N   1

#define LOG_BUF_EVENT_OFF 0
#define LOG_BUF_EVENT_N   1

#define LOG_BUF_RESTART_OFF 0
#define LOG_BUF_RESTART_N   1

#define LOG_BUF_ERROR_OFF 0
#define LOG_BUF_ERROR_N   1

#define LOG_MS_STAGE_OFF 0x07
#define LOG_MS_ERROR_OFF 0x02

void log_serialize(const log_buf_t *log, uint8_t *outbuf){
    size_t offset = 0;

    memcpy(outbuf + offset, &log->timers.timestamp, sizeof(log->timers.timestamp));
    offset += sizeof(log->timers.timestamp);

    memcpy(outbuf + offset, &log->timers.washing_start_epoch, sizeof(log->timers.washing_start_epoch));
    offset += sizeof(log->timers.washing_start_epoch);

    memcpy(outbuf + offset, &log->timers.dryer_start_epoch, sizeof(log->timers.dryer_start_epoch));
    offset += sizeof(log->timers.dryer_start_epoch);

    memcpy(outbuf + offset, &log->status.wshbits, sizeof(log->status.wshbits));
    offset += sizeof(log->status.wshbits);

    memcpy(outbuf + offset, &log->status.drybits, sizeof(log->status.drybits));
    offset += sizeof(log->status.drybits);

    memcpy(outbuf + offset, &log->io.iobits, sizeof(log->io.iobits));
    offset += sizeof(log->io.iobits);

    memcpy(outbuf + offset, &log->pumps.pbits, sizeof(log->pumps.pbits));
    offset += sizeof(log->pumps.pbits);

    memcpy(outbuf + offset, &log->event.type, sizeof(log->event.type));
    offset += sizeof(log->event.type);

    memcpy(outbuf + offset, &log->system.restart.reason, sizeof(log->system.restart.reason));
    offset += sizeof(log->system.restart.reason);

    memcpy(outbuf + offset, &log->system.error.code, sizeof(log->system.error.code));
}
