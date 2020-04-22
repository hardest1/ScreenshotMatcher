CC = g++
CFLAGS = -g -pthread -Wall -I/usr/local/include/opencv4 # change this to your opencv install
SRCS = main.cpp matchscreenshot/matchscreenshot.cpp daemonize/daemonize.cpp qrcodegen/QrCode.cpp logger/logger.cpp server/server.cpp
PROG = ScreenshotMatcher

OPENCV = -lopencv_calib3d -lopencv_features2d -lopencv_imgproc -lopencv_core -lopencv_xfeatures2d -lopencv_imgcodecs
LIBS = `pkg-config --cflags --libs gtk+-3.0 appindicator3-0.1` $(OPENCV) -lX11 -lstdc++

$(PROG):$(SRCS)
	$(CC) $(CFLAGS) -o $(PROG) $(SRCS) $(LIBS)
