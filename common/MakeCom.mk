
WEBRTC_ROOT = ${COMMON_ROOT}/webrtc

INCS += -I ${COMMON_ROOT}

INCS += -I ${WEBRTC_ROOT}
WEBRTC_CCS += $(wildcard ${WEBRTC_ROOT}/common_audio/*.cc)
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
WEBRTC_CS += $(wildcard ${WEBRTC_ROOT}/common_audio/signal_processing/*.c)
WEBRTC_OBJS += $(WEBRTC_CS:.c=.o)

libwebrtc.a:${WEBRTC_OBJS}
	$(AR) -r $@ $^


.PHONY:com_clean
com_clean:
	rm -f libwebrtc.a
	find ${COMMON_ROOT} -name "*.o" -type f -delete



