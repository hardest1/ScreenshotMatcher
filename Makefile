CC = g++
CFLAGS = -g -Wall -I/usr/local/include/opencv4 # change this to your opencv install
SRCS = main.cpp
PROG = ScreenshotMatcher

OPENCV = -lopencv_calib3d -lopencv_features2d -lopencv_imgproc -lopencv_core -lopencv_xfeatures2d -lopencv_imgcodecs
LIBS = $(OPENCV)

$(PROG):$(SRCS)
	$(CC) $(CFLAGS) -o $(PROG) $(SRCS) $(LIBS)
