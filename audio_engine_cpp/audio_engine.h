#ifndef AUDIO_ENGINE_H
#define AUDIO_ENGINE_H
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
typedef struct AudioEngine AudioEngine;
typedef struct { int32_t sample_rate; int32_t channel_count; int32_t frames_per_burst; int32_t buffer_size_multiplier; int use_exclusive; } AudioConfig;
AudioEngine* audio_engine_create(const AudioConfig* config);
void audio_engine_destroy(AudioEngine* engine);
int audio_engine_start(AudioEngine* engine);
int audio_engine_stop(AudioEngine* engine);
int audio_engine_read_input(AudioEngine* engine, float* buffer, int frames);
int audio_engine_write_output(AudioEngine* engine, const float* buffer, int frames);
int audio_engine_get_sample_rate(const AudioEngine* engine);
int audio_engine_get_frames_per_burst(const AudioEngine* engine);
int audio_engine_get_buffer_size(const AudioEngine* engine);
float audio_engine_get_input_latency_ms(const AudioEngine* engine);
float audio_engine_get_output_latency_ms(const AudioEngine* engine);
#ifdef __cplusplus
}
#endif
#endif
