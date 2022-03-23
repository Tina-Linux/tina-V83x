TARGET=smartlinkd_xrairkiss

ROOTDIR = .
SRC_PATH :=./radiotap .

vpath %.c ${SRC_PATH}

INCLUDE_PATH = ${ROOTDIR}/include
LIB_INC_DIR  = ${ROOTDIR}/radiotap
LIB_PATH     = ${ROOTDIR}/lib/$(CPU)
OBJ_PATH     = ${ROOTDIR}/obj
EXE_PATH     = ${ROOTDIR}/bin

LIBS = -lairkiss_aes \
	-L../../../../daemon/libclient/linux_client \
	-lsmartlinkd_client

A_CFLAGS = -I$(INCLUDE_PATH) -I$(LIB_INC_DIR) -Wall -I../../../../daemon/include -I../../../../daemon/libclient/linux_client
A_LDFLAGS = -L$(LIB_PATH)

LOCAL_SRC := $(foreach x,$(SRC_PATH),$(wildcard $(addprefix $(x)/*,.c)))
LOCAL_OBJ := $(addprefix $(OBJ_PATH)/, $(addsuffix .o,$(notdir $(basename $(LOCAL_SRC)))))

all : $(TARGET)
$(TARGET): $(LOCAL_OBJ)
	$(CXX) -g -D_DEBUG $^ -pthread -pipe $(A_LDFLAGS) $(LIBS) -o ${EXE_PATH}/$@
	chmod 777  ${EXE_PATH}/$(TARGET)
	cp ${EXE_PATH}/smartlinkd_xrairkiss $(CONFIG_PREFIX)/usr/bin/

${OBJ_PATH}/%.o: %.c
	@mkdir -p ${EXE_PATH}$
	@mkdir -p ${OBJ_PATH}$
	$(warning  $(LOCAL_OBJ))
	$(CC) -c -g -D_DEBUG -D_GNU_SOURCE -W -pthread -pipe $(A_CFLAGS) $(A_LDFLAGS) $(LIBS) -fPIC $< -o $@
clean:
	@rm -rf ${EXE_PATH} ${OBJ_PATH}
distclean: clean
