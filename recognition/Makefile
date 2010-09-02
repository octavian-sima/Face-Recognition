CV_STUFF=`pkg-config opencv --cflags --libs`

build: clean 
	g++  $(CV_STUFF) -o recognition Recognition.cpp Test.cpp

clean:
	rm -f recognition *.xml *.pgm

train:
	./recognition train

test:
	./recognition test



