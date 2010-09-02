CV_STUFF = `pkg-config opencv --cflags --libs`
SQL_STUFF = `mysql_config --cflags --libs`
RECOGNITION_FILE = ../recognition/Recognition.cpp
DETECTION_FILE = ../detection/Detection.cpp
DATABASE_FILE = ../sql_database/Database.cpp


build: clean 
	g++ $(CV_STUFF) $(SQL_STUFF) -o camera $(RECOGNITION_FILE) $(DETECTION_FILE) $(DATABASE_FILE) Camera.cpp

clean:
	rm -f camera *.xml *.pgm

train:
	./camera train

test:
	./camera test



