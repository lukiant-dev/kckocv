LIBS=-I/usr/include/opencv  -lopencv_core -lopencv_imgproc -lopencv_highgui -lopencv_ml -lopencv_video -lopencv_features2d -lopencv_calib3d -lopencv_objdetect -lopencv_contrib -lopencv_legacy -lopencv_flann -lm -lX11 -ltiff -ljpeg -lpng -lGLU -lGL -ldc1394  -L -lXext -pthread
CC=g++

SOURCES=3.cpp
HEADERS=
OBJECTS=$(SOURCES:.cpp=.o)

all: main

main: $(OBJECTS) 
	$(CC) -o $@ $(OBJECTS) $(LIBS)

$(OBJECTS): %.o: %.cpp $(HEADERS)
	$(CC) -c $< -o $@
	
clean:
	-rm -rf *.o
