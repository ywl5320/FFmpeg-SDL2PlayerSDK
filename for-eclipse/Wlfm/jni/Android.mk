LOCAL_PATH := $(call my-dir)


#ffmpeg lib
include $(CLEAR_VARS)
LOCAL_MODULE := avcodec
LOCAL_SRC_FILES := ffmpeg/libavcodec-57.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := avdevice
LOCAL_SRC_FILES := ffmpeg/libavdevice-57.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := avfilter
LOCAL_SRC_FILES := ffmpeg/libavfilter-6.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := avformat
LOCAL_SRC_FILES := ffmpeg/libavformat-57.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := avutil
LOCAL_SRC_FILES := ffmpeg/libavutil-55.so
include $(PREBUILT_SHARED_LIBRARY)


include $(CLEAR_VARS)
LOCAL_MODULE := postproc
LOCAL_SRC_FILES := ffmpeg/libpostproc-54.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := swresample
LOCAL_SRC_FILES := ffmpeg/libswresample-2.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := swscale
LOCAL_SRC_FILES := ffmpeg/libswscale-4.so
include $(PREBUILT_SHARED_LIBRARY)


include $(CLEAR_VARS)

LOCAL_MODULE := wlPlayer

SDL_PATH := ./sdl
SDL_PATH_P := ../

LOCAL_C_INCLUDES := $(LOCAL_PATH)/sdl/include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/ffmpeg/include

# Add your application source files here...
LOCAL_SRC_FILES := $(SDL_PATH)/src/main/android/SDL_android_main.c \
				player.c

LOCAL_SHARED_LIBRARIES := avcodec avdevice avfilter avformat avutil postproc swresample swscale SDL2

LOCAL_LDLIBS := -lGLESv1_CM -lGLESv2 -llog

include $(BUILD_SHARED_LIBRARY)


###########################
#
# SDL shared library
#
###########################

include $(CLEAR_VARS)

LOCAL_MODULE := SDL2

LOCAL_C_INCLUDES := $(LOCAL_PATH)/sdl/include

LOCAL_EXPORT_C_INCLUDES := $(LOCAL_C_INCLUDES)

LOCAL_SRC_FILES := \
	$(subst $(LOCAL_PATH)/,, \
	$(wildcard $(LOCAL_PATH)/sdl/src/*.c) \
	$(wildcard $(LOCAL_PATH)/sdl/src/audio/*.c) \
	$(wildcard $(LOCAL_PATH)/sdl/src/audio/android/*.c) \
	$(wildcard $(LOCAL_PATH)/sdl/src/audio/dummy/*.c) \
	$(LOCAL_PATH)/sdl/src/atomic/SDL_atomic.c \
	$(LOCAL_PATH)/sdl/src/atomic/SDL_spinlock.c.arm \
	$(wildcard $(LOCAL_PATH)/sdl/src/core/android/*.c) \
	$(wildcard $(LOCAL_PATH)/sdl/src/cpuinfo/*.c) \
	$(wildcard $(LOCAL_PATH)/sdl/src/dynapi/*.c) \
	$(wildcard $(LOCAL_PATH)/sdl/src/events/*.c) \
	$(wildcard $(LOCAL_PATH)/sdl/src/file/*.c) \
	$(wildcard $(LOCAL_PATH)/sdl/src/haptic/*.c) \
	$(wildcard $(LOCAL_PATH)/sdl/src/haptic/dummy/*.c) \
	$(wildcard $(LOCAL_PATH)/sdl/src/joystick/*.c) \
	$(wildcard $(LOCAL_PATH)/sdl/src/joystick/android/*.c) \
	$(wildcard $(LOCAL_PATH)/sdl/src/loadso/dlopen/*.c) \
	$(wildcard $(LOCAL_PATH)/sdl/src/power/*.c) \
	$(wildcard $(LOCAL_PATH)/sdl/src/power/android/*.c) \
	$(wildcard $(LOCAL_PATH)/sdl/src/filesystem/android/*.c) \
	$(wildcard $(LOCAL_PATH)/sdl/src/render/*.c) \
	$(wildcard $(LOCAL_PATH)/sdl/src/render/*/*.c) \
	$(wildcard $(LOCAL_PATH)/sdl/src/stdlib/*.c) \
	$(wildcard $(LOCAL_PATH)/sdl/src/thread/*.c) \
	$(wildcard $(LOCAL_PATH)/sdl/src/thread/pthread/*.c) \
	$(wildcard $(LOCAL_PATH)/sdl/src/timer/*.c) \
	$(wildcard $(LOCAL_PATH)/sdl/src/timer/unix/*.c) \
	$(wildcard $(LOCAL_PATH)/sdl/src/video/*.c) \
	$(wildcard $(LOCAL_PATH)/sdl/src/video/android/*.c) \
	$(wildcard $(LOCAL_PATH)/sdl/src/test/*.c))

LOCAL_CFLAGS += -DGL_GLEXT_PROTOTYPES
LOCAL_LDLIBS := -ldl -lGLESv1_CM -lGLESv2 -llog -landroid

include $(BUILD_SHARED_LIBRARY)
