package com.example.voiceclient;
public class AudioEngine {
    static { System.loadLibrary("audio_engine_cpp"); }
    public native int nativeStart();
    public native int nativeStop();
    public native int nativeJoinCall(String configJson);
    public native int nativeEndCall();
    public native int nativeIsCallActive();
    public native int nativeReadInput(float[] buffer, int frames);
    public native int nativeWriteOutput(float[] buffer, int frames);
    public native int nativeGetSampleRate();
    public native int nativeGetFramesPerBurst();
    public native int nativeGetBufferSize();
    public native float nativeGetInputLatencyMs();
    public native float nativeGetOutputLatencyMs();
}
