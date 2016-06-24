CC     := g++
CFLAGS := -O2 -lpthread -Dcimg_display=0 -Wno-unused-result -llept -ltesseract -std=c++11
TARGET := serialscanner
OBJS   := main.o images.o files.o ocr.o my_exif.o exif.o 

all: $(TARGET)

$(TARGET) : $(OBJS)
	$(CC) -o $(TARGET) $(OBJS) $(CFLAGS)

%.o: %.cpp
	$(CC) -c -o $@ $<  $(CFLAGS)
	
clean :
	rm -f $(TARGET)
	rm -f *.o
	

