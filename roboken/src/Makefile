all:	sample_app_tinyyolov3_img

# List all the files in src directory
SRC=sample_app_yolo_img.cpp \
	ascii.cpp \
	ascii.h \
	box.cpp \
	box.h \
	image.cpp \
	image.h \
	define.h

sample_app_tinyyolov3_img: ${SRC}
	${CXX} -std=c++14 --shared -fPIC $(filter %.cpp,${SRC})  \
	-lpthread -O2 -ldl ${LDFLAGS} \
	-DTINYYOLOV3 \
	-o tinyyolov3_lib.so


