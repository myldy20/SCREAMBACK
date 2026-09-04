#pragma once

#define PLUG_NAME "SCREAMBACK"
#define PLUG_MFR "Ilya Tolstoukhov"
#define PLUG_VERSION_HEX 0x00020000
#define PLUG_VERSION_STR "0.2.0"
#define PLUG_UNIQUE_ID 'ScBk'
#define PLUG_MFR_ID 'IyTo'
#define PLUG_URL_STR "https://github.com/myldy20/SCREAMBACK"
#define PLUG_EMAIL_STR ""
#define PLUG_COPYRIGHT_STR "Copyright 2026 Ilya Tolstoukhov"
#define PLUG_CLASS_NAME SCREAMBACK

#define BUNDLE_NAME "SCREAMBACK"
#define BUNDLE_MFR "myldy20"
#define BUNDLE_DOMAIN "com"

#define SHARED_RESOURCES_SUBPATH "SCREAMBACK"

#define PLUG_CHANNEL_IO "1-1 2-2"
#define PLUG_LATENCY 0
#define PLUG_TYPE 0
#define PLUG_DOES_MIDI_IN 1
#define PLUG_DOES_MIDI_OUT 0
#define PLUG_DOES_MPE 0
#define PLUG_DOES_STATE_CHUNKS 0
#define PLUG_HAS_UI 1
#define PLUG_WIDTH 720
#define PLUG_HEIGHT 450
#define PLUG_FPS 60
#define PLUG_SHARED_RESOURCES 0
#define PLUG_HOST_RESIZE 1
#define PLUG_MIN_WIDTH 540
#define PLUG_MIN_HEIGHT 338
#define PLUG_MAX_WIDTH 1440
#define PLUG_MAX_HEIGHT 900

#define AUV2_ENTRY SCREAMBACK_Entry
#define AUV2_ENTRY_STR "SCREAMBACK_Entry"
#define AUV2_FACTORY SCREAMBACK_Factory
#define AUV2_VIEW_CLASS SCREAMBACK_View
#define AUV2_VIEW_CLASS_STR "SCREAMBACK_View"

#define AAX_TYPE_IDS 'ScB1'
#define AAX_TYPE_IDS_AUDIOSUITE 'ScA1'
#define AAX_PLUG_MFR_STR "myldy20"
#define AAX_PLUG_NAME_STR "SCREAMBACK\nSCBK"
#define AAX_PLUG_CATEGORY_STR "Effect"
#define AAX_DOES_AUDIOSUITE 0

#define VST3_SUBCATEGORY "Fx"

#define CLAP_MANUAL_URL "https://github.com/myldy20/SCREAMBACK"
#define CLAP_SUPPORT_URL "https://github.com/myldy20/SCREAMBACK/issues"
#define CLAP_DESCRIPTION "Controlled amplifier-style feedback without an amplifier"
#define CLAP_FEATURES "audio-effect"

#define APP_NUM_CHANNELS 2
#define APP_N_VECTOR_WAIT 0
#define APP_MULT 1
#define APP_COPY_AUV3 0
#define APP_SIGNAL_VECTOR_SIZE 64

#define ROBOTO_FN "Roboto-Regular.ttf"
