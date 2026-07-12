#include "audio_engine.h"
#include "ring_buffer.h"
#include <oboe/Oboe.h>
#include <android/log.h>
#include <atomic>
#include <cstring>
#define TAG "AudioEngine"
using namespace oboe;

// كشف الأخطاء بدون تسريب للذاكرة
class SafeErrorCallback : public AudioStreamErrorCallback {
public:
    void onErrorAfterClose(AudioStream *oboeStream, Result error) override {
        __android_log_print(ANDROID_LOG_ERROR, TAG, "Stream error: %s", convertToText(error));
    }
};

struct AudioEngine {
    AudioConfig config;
    AudioStream *input_stream = nullptr;
    AudioStream *output_stream = nullptr;
    RingBuffer *input_rb = nullptr;
    RingBuffer *output_rb = nullptr;
    std::atomic<bool> running{false};
    class InputCallback *in_cb = nullptr;
    class OutputCallback *out_cb = nullptr;
    SafeErrorCallback *error_callback = nullptr; // مملوك لتجنب التسرب
};

class InputCallback : public AudioStreamDataCallback {
public:
    RingBuffer *rb;
    std::atomic<bool> *running;
    DataCallbackResult onAudioReady(AudioStream*, void* audioData, int32_t numFrames) override {
        if(!running->load(std::memory_order_relaxed)) return DataCallbackResult::Stop;
        ring_buffer_write(rb, static_cast<float*>(audioData), numFrames);
        return DataCallbackResult::Continue;
    }
};

class OutputCallback : public AudioStreamDataCallback {
public:
    RingBuffer *rb;
    std::atomic<bool> *running;
    DataCallbackResult onAudioReady(AudioStream*, void* audioData, int32_t numFrames) override {
        if(!running->load(std::memory_order_relaxed)) {
            std::memset(audioData,0,numFrames*sizeof(float));
            return DataCallbackResult::Stop;
        }
        float* d=static_cast<float*>(audioData);
        int r=ring_buffer_read(rb,d,numFrames);
        if(r<numFrames) std::memset(d+r,0,(numFrames-r)*sizeof(float));
        return DataCallbackResult::Continue;
    }
};

extern "C" AudioEngine* audio_engine_create(const AudioConfig* c){
    AudioEngine* e = new AudioEngine();
    e->config = *c;
    size_t cap = c->frames_per_burst * c->channel_count * c->buffer_size_multiplier;
    e->input_rb = ring_buffer_create(cap);
    e->output_rb = ring_buffer_create(cap);
    if(!e->input_rb || !e->output_rb){
        ring_buffer_destroy(e->input_rb);
        ring_buffer_destroy(e->output_rb);
        delete e;
        return nullptr;
    }
    e->error_callback = new SafeErrorCallback();
    return e;
}
void audio_engine_destroy(AudioEngine* e){
    if(e){
        audio_engine_stop(e);
        ring_buffer_destroy(e->input_rb);
        ring_buffer_destroy(e->output_rb);
        delete e->error_callback;
        delete e;
    }
}
int audio_engine_start(AudioEngine* e){
    if(!e) return -1;
    e->in_cb = new InputCallback();
    e->out_cb = new OutputCallback();
    e->in_cb->rb = e->input_rb;
    e->in_cb->running = &e->running;
    e->out_cb->rb = e->output_rb;
    e->out_cb->running = &e->running;

    AudioStreamBuilder inB;
    inB.setDirection(Direction::Input)
       ->setPerformanceMode(PerformanceMode::LowLatency)
       ->setSharingMode(e->config.use_exclusive ? SharingMode::Exclusive : SharingMode::Shared)
       ->setFormat(AudioFormat::Float)
       ->setSampleRate(e->config.sample_rate)
       ->setChannelCount(e->config.channel_count)
       ->setUsage(Usage::VoiceCommunication)
       ->setInputPreset(InputPreset::VoiceCommunication)
       ->setDataCallback(e->in_cb)
       ->setErrorCallback(e->error_callback);

    Result r = inB.openStream(&e->input_stream);
    if(r != Result::OK){
        if(e->config.use_exclusive){
            inB.setSharingMode(SharingMode::Shared);
            r = inB.openStream(&e->input_stream);
        }
        if(r != Result::OK){
            delete e->in_cb; e->in_cb = nullptr;
            delete e->out_cb; e->out_cb = nullptr;
            return -1;
        }
    }
    e->input_stream->setBufferSizeInFrames(e->input_stream->getFramesPerBurst());

    AudioStreamBuilder outB;
    outB.setDirection(Direction::Output)
        ->setPerformanceMode(PerformanceMode::LowLatency)
        ->setSharingMode(e->config.use_exclusive ? SharingMode::Exclusive : SharingMode::Shared)
        ->setFormat(AudioFormat::Float)
        ->setSampleRate(e->config.sample_rate)
        ->setChannelCount(e->config.channel_count)
        ->setUsage(Usage::VoiceCommunication)
        ->setContentType(ContentType::Speech)
        ->setDataCallback(e->out_cb)
        ->setErrorCallback(e->error_callback);

    r = outB.openStream(&e->output_stream);
    if(r != Result::OK){
        if(e->config.use_exclusive){
            outB.setSharingMode(SharingMode::Shared);
            r = outB.openStream(&e->output_stream);
        }
        if(r != Result::OK){
            e->input_stream->close();
            delete e->in_cb; e->in_cb = nullptr;
            delete e->out_cb; e->out_cb = nullptr;
            return -1;
        }
    }
    e->output_stream->setBufferSizeInFrames(e->output_stream->getFramesPerBurst());

    e->running = true;
    e->output_stream->start();
    e->input_stream->start();
    return 0;
}
int audio_engine_stop(AudioEngine* e){
    if(!e) return -1;
    e->running = false;
    if(e->input_stream){ e->input_stream->stop(); e->input_stream->close(); e->input_stream = nullptr; }
    if(e->output_stream){ e->output_stream->stop(); e->output_stream->close(); e->output_stream = nullptr; }
    delete e->in_cb; e->in_cb = nullptr;
    delete e->out_cb; e->out_cb = nullptr;
    return 0;
}
int audio_engine_read_input(AudioEngine* e, float* buf, int f){ return e ? ring_buffer_read(e->input_rb, buf, f) : 0; }
int audio_engine_write_output(AudioEngine* e, const float* buf, int f){ return e ? ring_buffer_write(e->output_rb, buf, f) : 0; }
int audio_engine_get_sample_rate(const AudioEngine* e){ return e ? e->config.sample_rate : 0; }
int audio_engine_get_frames_per_burst(const AudioEngine* e){ return e ? e->config.frames_per_burst : 0; }
int audio_engine_get_buffer_size(const AudioEngine* e){ return e ? e->config.frames_per_burst * e->config.buffer_size_multiplier : 0; }
float audio_engine_get_input_latency_ms(const AudioEngine* e){
    if(e && e->input_stream) return (float)e->input_stream->getFramesPerBurst() * 1000.0f / e->config.sample_rate;
    return 0.0f;
}
float audio_engine_get_output_latency_ms(const AudioEngine* e){
    if(e && e->output_stream) return (float)e->output_stream->getFramesPerBurst() * 1000.0f / e->config.sample_rate;
    return 0.0f;
}
