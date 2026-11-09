#ifndef LOG_H
#define LOG_H

#include <stdint.h>

#define LOG_SERIALIZED_SIZE 34

/** S = Stage, E = Error code, T = Status */
#define LOG_MACHINE_STATUS_MOUNT(S, E, T)    \
    ( (uint16_t)S << LOG_MS_STAGE_OFF ) |    \
    ( (uint16_t)E << LOG_MS_ERROR_OFF ) |    \
    ( (uint16_t)T )

typedef enum {
    LOG_EVENT_ERROR,
    LOG_EVENT_START_OPERATION,
    LOG_EVENT_END_OPERATION,
    LOG_EVENT_RESTART,
    LOG_EVENT_CONNECTED,
    LOG_EVENT_DISCONNECTED,
    LOG_EVENT_CONFIG_CHANGED,
    LOG_EVENT_OPEN_CONF_MODE,
    LOG_EVENT_FIRMWARE_UPDATE_START,
    LOG_EVENT_FIRMWARE_UPDATE_END
} log_event_type_t;

typedef enum {
    LOG_ERROR_NO,
    LOG_ERROR_CRITICAL_SYSTEM_FAIL,
    LOG_ERROR_MCP_FAIL,
    LOG_ERROR_RTC_FAIL,
    LOG_ERROR_MQTTS_SERVER_AUTH,
    LOG_ERROR_MACHINE_NOT_RESPONSE,
    LOG_ERROR_FIRMWARE_UPDATE
} log_error_code_t;

typedef enum {
    LOG_MACHINE_BUSY,
    LOG_MACHINE_FREE,
    LOG_MACHINE_ERROR
} log_machine_status_t;

typedef enum {
    LOG_STAGE_1,
    LOG_STAGE_2,
    LOG_STAGE_3,
    LOG_STAGE_4
} log_machine_status_stage_t;

typedef struct {
    struct {
        int64_t timestamp;
        int64_t washing_start_epoch;
        int64_t dryer_start_epoch;
    } timers;

    struct {
        /*
            15             9 8     7 6       2  1      0
            +---------------+---------+---------+
            |   reserved    |  stage  |  error  |status|
            +---------------+---------+---------+
                7 bits       2 bits    5 bits   2 bits
        */
        uint16_t wshbits; /** washing bitmap */

        /*
            15             9 8     7 6       2  1      0
            +---------------+---------+---------+
            |   reserved    |  stage  |  error  |status|
            +---------------+---------+---------+
                7 bits       2 bits    5 bits   2 bits
        */
        uint16_t drybits; /** dryer bitmap */
    } status;

    struct {
        /**      12 11 10 9 8 7 6 5 4 3 2 1 0
         *  xxx  0  0  0  0 0 0 0 0 0 0 0 0 0
         * 
         *   0: washing-free
         *   1: washing-busy
         *   2: dryer-free
         *   3: dryer-busy
         *   4: stage-1
         *   5: stage-2
         *   6: stage-3
         *   7: stage-4
         *   8: dongle-washing
         *   9: dongle-dryer
         *  10: net-button
         *  11: washing-relay
         *  12: dryer-relay
         */
        uint16_t iobits;
    } io;

    struct {
        /**   P7 P6 P5 P4 P3 P2 P1 */
        /** x 0  0  0  0  0  0  0*/
        uint8_t pbits; /** pumps bitmap */
    } pumps;

    struct {
        uint8_t type;
    } event;

    struct {
        struct {
            uint8_t reason;
        } restart;

        struct {
            uint8_t code;
        } error;
    } system;
} log_buf_t;

#endif // LOG_H
