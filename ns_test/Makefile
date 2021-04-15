
target:noise_sup

CXX = g++ 
CC = gcc

ROOT_DIR = ..
COMMON_ROOT = ${ROOT_DIR}/common
include ../common/MakeCom.mk

LDLIBS += -lpthread ../libgtest_mac.a

CFLAGS += ${INCS} -Wall -Werror -g
CFLAGS += -Wno-error=sign-compare
CFLAGS += -DWEBRTC_NS_FLOAT -DWEBRTC_POSIX -I../../AEC3/base/abseil

CXXFLAGS += ${CFLAGS} -std=c++14

noise_sup:noise_sup.o ${OBJS} libwebrtc.a
	${CXX} $^ -o $@ ${LDLIBS}


.PHONY:clean
clean:com_clean
	rm -f noise_sup
	rm -f libwebrtc.a
	find . -name "*.o" -type f -delete



