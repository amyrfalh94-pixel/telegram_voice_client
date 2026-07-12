[app]
title = Telegram Voice Client
package.name = voiceclient
package.domain = com.example
source.dir = .
source.include_exts = py,png,jpg,kv,atlas
version = 0.1
requirements = python3,kivy,telethon,pyjnius
android.permissions = INTERNET,RECORD_AUDIO,MODIFY_AUDIO_SETTINGS,FOREGROUND_SERVICE
android.api = 31
android.minapi = 21
android.ndk = 25b
android.arch = arm64-v8a
p4a.branch = master
android.add_src = android_src
android.add_jni_libs = jniLibs
orientation = portrait
fullscreen = 0
[buildozer]
log_level = 2
