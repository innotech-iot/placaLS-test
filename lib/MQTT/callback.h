#ifndef __libcallback_callback_h__
#define __libcallback_callback_h__

#ifdef __cplusplus
extern "C" {
#endif

void mqtt_callback(const char *topic, char *data);

#ifdef __cplusplus
}
#endif

#endif // __libcallback_callback_h__
