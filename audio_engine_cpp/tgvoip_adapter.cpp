#include "tgvoip_adapter.h"
#include "audio_engine.h"
#include <android/log.h>
#define TAG "TgVoip"
struct TgVoipAdapter { AudioEngine* engine; bool call_active; };
extern "C" TgVoipAdapter* tgvoip_adapter_create(void* engine) {
    TgVoipAdapter* a = new TgVoipAdapter();
    a->engine = (AudioEngine*)engine;
    a->call_active = false;
    return a;
}
void tgvoip_adapter_destroy(TgVoipAdapter* a) { delete a; }
int tgvoip_adapter_start_call(TgVoipAdapter* a, const char* config_json) {
    __android_log_print(ANDROID_LOG_INFO, TAG, "Starting call with config: %s", config_json);
    a->call_active = true;
    return 0;
}
int tgvoip_adapter_end_call(TgVoipAdapter* a) { a->call_active = false; return 0; }
int tgvoip_adapter_is_call_active(TgVoipAdapter* a) { return a->call_active ? 1 : 0; }
