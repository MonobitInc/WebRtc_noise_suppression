
target:splitter

ROOT_DIR=../

CXX = g++ 

INCS+=-I test/
INCS+=-I rtc_base/
OBJS+=rtc_base/checks.o

INCS+=-I AudioFile/
OBJS+=AudioFile/AudioFile.o

INCS+=-I splitting_filter/
TMP_SRCS=$(wildcard splitting_filter/*.cc)
OBJS+=$(TMP_SRCS:.cc=.o)
OBJS+=$(TMP_SRCS:.cc=.o)

LDLIBS+=-lgtest -lpthread

CPPFLAGS += ${INCS} -std=c++11

splitter:splitting_filter.o ${OBJS}
	echo ${TMP_SRCS}
	echo ${OBJS}
	${CXX} $^ -o $@ ${LDLIBS}



.PHONY:clean
clean:
	find . -name "*.o" -type f -delete



