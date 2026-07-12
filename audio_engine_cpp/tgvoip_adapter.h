#ifndef TGVOIP_ADAPTER_H
#define TGVOIP_ADAPTER_H
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
typedef struct TgVoipAdapter TgVoipAdapter;
TgVoipAdapter* tgvoip_adapter_create(void* audio_engine);
void tgvoip_adapter_destroy(TgVoipAdapter* a);
int tgvoip_adapter_start_call(TgVoipAdapter* a, const char* call_config_json);
int tgvoip_adapter_end_call(TgVoipAdapter* a);
int tgvoip_adapter_is_call_active(TgVoipAdapter* a);
#ifdef __cplusplus
}
#endif
#endif
