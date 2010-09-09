THREAD_STUFF = -lpthread
CV_STUFF = `pkg-config opencv --cflags --libs`
SQL_STUFF = `mysql_config --cflags --libs`
RECOGNITION_FILE = ../recognition/Recognition.cpp
DETECTION_FILE = ../detection/Detection.cpp
DATABASE_FILE = ../sql_database/Database.cpp

build: clean 
	g++ $(THREAD_STUFF) $(CV_STUFF) $(SQL_STUFF) -o run_server $(RECOGNITION_FILE) $(DETECTION_FILE) $(DATABASE_FILE) Server.cpp Run_Server.cpp

clean:
	rm -f run_server *.xml *.pgm *.jpg

