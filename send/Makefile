CC = g++
CFLAGS = -Wall -std=c++11 -I/usr/local/opencv4.8.0/include -I/usr/local/opencv4.8.0/include/opencv4 -I/usr/local/opencv4.8.0/include/opencv4/opencv2
LDFLAGS = -lopencv_core -lopencv_highgui -lopencv_imgcodecs -lopencv_videoio
# JSON 库的头文件目录
JSON_INCLUDE = -I/path/to/json/include

all: send

send: send.cpp
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

clean:
	rm -f server1
