CV_STUFF = `pkg-config opencv --cflags --libs` 
SQL_STUFF = `mysql_config --cflags --libs`

build: clean
	g++ $(CV_STUFF) -o detection Detection.cpp TestDetection.cpp 

clean:
	rm -f main *.xml *.bmp

	





