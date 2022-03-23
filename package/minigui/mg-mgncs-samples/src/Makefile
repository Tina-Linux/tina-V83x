TARGET = text-highlight effect_scroll test-framework anim_window test_listbox

INCLUDES	:=  $(foreach dir,$(SRCDIRS),-I$(dir))            \
		   -I$(STAGING_DIR)/usr/include/allwinner/include \
		   -I$(STAGING_DIR)/usr/include/allwinner \
		   -I$(STAGING_DIR)/usr/include

LOCAL_CFLAGS =
LOCAL_LDFLAGS = -lminigui_ths -lts -lasound -ljpeg -lpng -ldl -lm -lpthread -lmgi \
		-lpthread -ldl -lminigui_ths -lpng  -ljpeg -lfreetype -lz -lm \
		-luapi -lrt -lc  -lasound \
		-lmgplus -lmgncs -lmgncs4touch -lmgeff -lsqlite3 -lxml2 -lchipmunk -lmgutils

#%.o: %.c
HIGHLIGHT_SRC = text-highlight.c
HIGHLIGHT_OBJ = text-highlight.o
EFFECT_SCROLL_SRC = effect_scroll.c
EFFECT_SCROLL_OBJ = effect_scroll.o
TEST_FRAMEWORK_SRC = test-framework.c test-utils.c chipmunk-utils.c chipmunk-utils.h test-framework.h
TEST_FRAMEWORK_OBJ = test-framework.o test-utils.o chipmunk-utils.o
ANIMWINDOW_SRC = anim_window.c
ANIMWINDOW_OBJ = anim_window.o
LISTBOX_VIEW_SRC = test_listbox.c activity_listbox.c test_list_res.h
LISTBOX_VIEW_OBJ = test_listbox.o activity_listbox.o
%.o: %.c
	$(CC) $(INCLUDES) $(CFLAGS) $(LDFLAGS) $(LOCAL_CFLAGS) $(LOCAL_LDFLAGS) -c -o $@ $<

text-highlight: $(HIGHLIGHT_OBJ)
	$(CC) -o $@ $(HIGHLIGHT_OBJ) $(CFLAGS) $(LDFLAGS) $(LOCAL_CFLAGS) $(LOCAL_LDFLAGS) -g

effect_scroll: $(EFFECT_SCROLL_OBJ)
	$(CC) -o $@ $(EFFECT_SCROLL_OBJ) $(CFLAGS) $(LDFLAGS) $(LOCAL_CFLAGS) $(LOCAL_LDFLAGS) -g
test-framework: $(TEST_FRAMEWORK_OBJ)
	$(CC) -o $@ $(TEST_FRAMEWORK_OBJ) $(CFLAGS) $(LDFLAGS) $(LOCAL_CFLAGS) $(LOCAL_LDFLAGS) -g
anim_window: $(ANIMWINDOW_OBJ)
	$(CC) -o $@ $(ANIMWINDOW_OBJ) $(CFLAGS) $(LDFLAGS) $(LOCAL_CFLAGS) $(LOCAL_LDFLAGS) -g
test_listbox: $(LISTBOX_VIEW_OBJ)
	$(CC) -o $@ $(LISTBOX_VIEW_OBJ) $(CFLAGS) $(LDFLAGS) $(LOCAL_CFLAGS) $(LOCAL_LDFLAGS) -g
.PHONY: all clean
all: $(TARGET)

clean:
	rm -rf $(TARGET) *.o
