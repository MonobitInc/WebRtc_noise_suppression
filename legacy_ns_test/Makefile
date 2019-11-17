
target:noise_sup

CXX = g++ 
CC = gcc

ROOT_DIR = ..
WEBRTC_ROOT = ${ROOT_DIR}/webrtc
COMMON_ROOT = ${ROOT_DIR}/common

INCS += -I ${COMMON_ROOT}
OBJS += ${COMMON_ROOT}/AudioFile/AudioFile.o

INCS += -I ${WEBRTC_ROOT}
WEBRTC_CCS += $(wildcard ${WEBRTC_ROOT}/common_audio/resampler/*.cc)
WEBRTC_CCS += $(wildcard ${WEBRTC_ROOT}/api/units/*.cc)
WEBRTC_CCS += $(wildcard ${WEBRTC_ROOT}/rtc_base/*.cc)
WEBRTC_CCS += $(wildcard ${WEBRTC_ROOT}/rtc_base/memory/*.cc)
WEBRTC_CCS += $(wildcard ${WEBRTC_ROOT}/modules/audio_processing/*.cc)
WEBRTC_CCS += $(wildcard ${WEBRTC_ROOT}/modules/audio_processing/ns/*.cc)
WEBRTC_OBJS += $(WEBRTC_CCS:.cc=.o)
WEBRTC_CS += $(wildcard ${WEBRTC_ROOT}/modules/audio_processing/legacy_ns/*.c)
WEBRTC_CS += $(wildcard ${WEBRTC_ROOT}/common_audio/third_party/fft4g/*.c)
WEBRTC_CS += $(wildcard ${WEBRTC_ROOT}/common_audio/third_party/spl_sqrt_floor/*.c)
WEBRTC_OBJS += $(WEBRTC_CS:.c=.o)

LDLIBS += -lgtest -lpthread

CFLAGS += ${INCS} -g
CFLAGS += -DWEBRTC_NS_FLOAT -DWEBRTC_POSIX
CXXFLAGS += ${CFLAGS} -std=c++14

libwebrtc.a:${WEBRTC_OBJS}
	$(AR) -r $@ $^

noise_sup:noise_sup.o ${OBJS} libwebrtc.a
	${CXX} $^ -o $@ ${LDLIBS}


.PHONY:clean
clean:
	rm -f noise_sup
	rm -f libwebrtc.a
	find . -name "*.o" -type f -delete
	find ../webrtc/ -name "*.o" -type f -delete



