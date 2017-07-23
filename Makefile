CROSSCOMPILE := arm-linux-

CFLAGS := -Wall -O2 -c
CFLAGS += -I$(PWD)/include

LDFLAGS := -lm -lfreetype -ljpeg -lts -lpthread

CC := $(CROSSCOMPILE)gcc
LD := $(CROSSCOMPILE)ld

OBJS := mainz.o \
			display/disp_managerz.o        \
			display/fbz.o                  \
			encoding/ascii_gbkz.o              \
			encoding/utf-16bez.o           \
			encoding/encoding_managerz.o   \
			encoding/utf-8z.o              \
			encoding/utf-16lez.o           \
			draw/drawtxt.o                   \
			draw/drawpicture.o				\
			fonts/asciiz.o                 \
			fonts/gbkz.o                   \
			fonts/freetypez.o              \
			fonts/font_managerz.o		   \
			input/stdin.o				   \
			input/screen.o				   \
			input/input_manager.o		   \
			file/file.o					\
			pictureformat/bmp.o			\
			pictureformat/jpeg.o		\
			pictureformat/picformat_manager.o	\
			pages/explorePage.o

all: $(OBJS)
	$(CC) $(LDFLAGS) -o FileBrowser $^

clean:
	rm -f FileBrowser
	rm -f $(OBJS)

%o:%.c
	$(CC) $(CFLAGS) -o $@ $<


