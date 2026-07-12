#include <jni.h>
#include <android/log.h>
#include "audio_engine.h"
#include "tgvoip_adapter.h"
static AudioEngine* g_engine = nullptr;
static TgVoipAdapter* g_adapter = nullptr;
extern "C" {
JNIEXPORT jint JNICALL Java_com_example_voiceclient_AudioEngine_nativeStart(JNIEnv*,jobject){
    if(g_engine) return -1;
    AudioConfig c={48000,1,128,2,1};
    g_engine=audio_engine_create(&c);
    if(!g_engine) return -1;
    int r=audio_engine_start(g_engine);
    if(r==0) g_adapter=tgvoip_adapter_create(g_engine);
    return r;
}
JNIEXPORT jint JNICALL Java_com_example_voiceclient_AudioEngine_nativeStop(JNIEnv*,jobject){
    if(g_adapter){ tgvoip_adapter_end_call(g_adapter); tgvoip_adapter_destroy(g_adapter); g_adapter=nullptr; }
    if(g_engine){ audio_engine_stop(g_engine); audio_engine_destroy(g_engine); g_engine=nullptr; }
    return 0;
}
JNIEXPORT jint JNICALL Java_com_example_voiceclient_AudioEngine_nativeJoinCall(JNIEnv* e,jobject,jstring config){
    if(!g_adapter) return -1;
    const char* cfg=e->GetStringUTFChars(config,nullptr);
    if(!cfg) return -1;
    int r=tgvoip_adapter_start_call(g_adapter,cfg);
    e->ReleaseStringUTFChars(config,cfg);
    return r;
}
JNIEXPORT jint JNICALL Java_com_example_voiceclient_AudioEngine_nativeEndCall(JNIEnv*,jobject){ return g_adapter?tgvoip_adapter_end_call(g_adapter):-1; }
JNIEXPORT jint JNICALL Java_com_example_voiceclient_AudioEngine_nativeIsCallActive(JNIEnv*,jobject){ return g_adapter?tgvoip_adapter_is_call_active(g_adapter):0; }
JNIEXPORT jint JNICALL Java_com_example_voiceclient_AudioEngine_nativeReadInput(JNIEnv* e,jobject,jfloatArray b,jint f){
    if(!g_engine) return 0;
    jsize l=e->GetArrayLength(b); if(l<f)f=l;
    jfloat* d=e->GetFloatArrayElements(b,nullptr);
    int r=audio_engine_read_input(g_engine,d,f);
    e->ReleaseFloatArrayElements(b,d,0);
    return r;
}
JNIEXPORT jint JNICALL Java_com_example_voiceclient_AudioEngine_nativeWriteOutput(JNIEnv* e,jobject,jfloatArray b,jint f){
    if(!g_engine) return 0;
    jsize l=e->GetArrayLength(b); if(l<f)f=l;
    jfloat* d=e->GetFloatArrayElements(b,nullptr);
    int r=audio_engine_write_output(g_engine,d,f);
    e->ReleaseFloatArrayElements(b,d,0);
    return r;
}
JNIEXPORT jint JNICALL Java_com_example_voiceclient_AudioEngine_nativeGetSampleRate(JNIEnv*,jobject){ return audio_engine_get_sample_rate(g_engine); }
JNIEXPORT jint JNICALL Java_com_example_voiceclient_AudioEngine_nativeGetFramesPerBurst(JNIEnv*,jobject){ return audio_engine_get_frames_per_burst(g_engine); }
JNIEXPORT jint JNICALL Java_com_example_voiceclient_AudioEngine_nativeGetBufferSize(JNIEnv*,jobject){ return audio_engine_get_buffer_size(g_engine); }
JNIEXPORT jfloat JNICALL Java_com_example_voiceclient_AudioEngine_nativeGetInputLatencyMs(JNIEnv*,jobject){ return audio_engine_get_input_latency_ms(g_engine); }
JNIEXPORT jfloat JNICALL Java_com_example_voiceclient_AudioEngine_nativeGetOutputLatencyMs(JNIEnv*,jobject){ return audio_engine_get_output_latency_ms(g_engine); }
}
