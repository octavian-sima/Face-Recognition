/*
 * File:   Camera.cpp
 * Author: Elena Holobiuc
 */

#include <stdlib.h>
#include <dirent.h>
#include <stdio.h>
#include "../detection/Detection.h"
#include "../sql_database/Database.h"
#include "../recognition/Recognition.h"

//if false, use a file that contains a list of paths to face images
#define USE_DATABASE_FOR_TRAINING   true

//path to training database or to a file with list of images
#define TRAINING_SRC_PATH           "../person_database"

//default test file path
#define TEST_FILE_PATH              "tests/upper6.txt"

//face image files extension
#define IMAGE_FILE_EXTENSION        "pgm"

//minimum recognition confidence for a person to be classify as known
#define RECOGNITION_THRESHOLD       0.9

//save average image and eigenvectors as images after training?
#define SAVE_TRAINING_DATA          true

//path to a xml file where training data is stored
#define TRAINING_DATA_FILE          "savedData.xml"

//if false, use EUCLIDIAN_DISTANCE
#define USE_MAHALANOBIS_DISTANCE    false


/*
 * Method that captures a new image from webcam and
 * return it
 */
IplImage* getNextCameraFrame(CvCapture* capture);
/*
 * Method that creates a new folder in person_database
 * and saves images used in recognition in that folder
 */
int trainFromWebcam(string fname, string lname);
/*
 * Method that recognizes a person and displays the person's info
 */
int recognizeFromWebcam();
/*
 * Method that displays a certain message on the screen
 */
void showText(IplImage* image, vector<CvRect> faces, char* windowName,
               vector<char*> personNames);
/*
 * Method that displays a dialog in order to obtain
 * information about the trained person
 */
void openDialogBox(int id, string fname, string lname,Database* database);
/*
 * Method that reads next line from console
 */
char* getNextLine();
/*
 * Method that gets the last id from the person_database
 */
int getLastPersonId();

int main(int argc, char** argv) {

    const string train = "train";
    const string test = "test";
    if (argc == 4) {

        if (train.compare(argv[1]) == 0) {

            string fname,lname;
            fname.assign(argv[2]);
            lname.assign(argv[3]);
	    printf("Press 't' to capture a new image!\n");
            trainFromWebcam(fname,lname);
        }
    }

    else if (argc == 2) {

        if (test.compare(argv[1]) == 0) {

            recognizeFromWebcam();
        }
    }

    else {
        printf("Usage: ./camera train firstname lastname\n"
                "      ./camera test");
    }


    return (EXIT_SUCCESS);
}


IplImage* getNextCameraFrame(CvCapture* capture) {
	IplImage *cameraImage;
	cameraImage = cvQueryFrame( capture );
	if (!cameraImage) {
		fprintf(stderr, "Error! Could not access the camera.\n");
                exit(0);
	}
	return cameraImage;
}

void openDialogBox(int id, string fname, string lname, Database* database) {

    char *bdate,
         *occupation,
         *phoneNr;
   
    printf("Please insert your information in order to be inserted into"
            " the database!\n"
            "(Enter for blank).\n");
   
   
    printf("Birthdate [YYYY-MM-DD]: ");
    bdate = getNextLine();
   
    printf("Occupation: ");
    occupation = getNextLine();
  
    printf("Phone number: ");
    phoneNr = getNextLine();
  
    // insert new person into database
    database->insertNewPerson(id, (char*)fname.c_str(), (char*)lname.c_str(),
                                bdate, occupation, phoneNr);
 

}

int trainFromWebcam(string fname, string lname) {

    CvCapture* capture = 0;
    char keyPressed = 0;
    IplImage* cameraImage;

    // init database mobule
    Database* database = new Database();
    // connect to database
    database->connect();

    // check if the person already exists in the database
    int idFromDatabase = database->getPersonId((char*)fname.c_str(),
                                                  (char*)lname.c_str());
    string dirName = "";

    // if the person is already inserted in the database
    if ( idFromDatabase != -1 ) {
        char personIdToString[10];
        sprintf(personIdToString,"%d",idFromDatabase);
        dirName.append(TRAINING_SRC_PATH).append("/s").append(personIdToString);
        fprintf(stderr, "Person already exists!\n");
        printf("Override person photos from folder?[y/N]");
        char* consoleString = (char*)malloc(10*sizeof(char));
        scanf("%s",consoleString);
        if (consoleString[0] == 'N' || consoleString[0] == 'n')
            return EXIT_FAILURE;
    }

    // if the person does not exist in the database
    else {

        // get last id from person database
        int personId = getLastPersonId();
        char personIdToString[10];
        sprintf(personIdToString,"%d",++personId);

        // create new folder with the name s+lastId++
        dirName.append(TRAINING_SRC_PATH).append("/s").append(personIdToString);    
  

        DIR* dir = opendir(dirName.c_str());
        if (! dir) {
            string command = "mkdir ";
            command.append(dirName);
            system(command.c_str());
            // insert information for the current person
            openDialogBox(personId,fname,lname,database);
        }
        else {
            fprintf(stderr, "Error creating directory!");
            return EXIT_FAILURE;
        }

    }

    Detection* detection = new Detection();
    vector<char*> args;
    args.push_back((char*)"--hits=3");
    args.push_back((char*)"--size=50");
    // initialize detection module
    detection->initialize(args);

    // open webcam
    capture = cvCaptureFromCAM( 0 );
    sleep(1);
    if ( !capture ) {
	fprintf(stderr, "Error! Could not access the camera.\n");
        exit(0);
    }
    cvNamedWindow("Camera", CV_WINDOW_AUTOSIZE);

    // get next camera frame
    cameraImage = getNextCameraFrame(capture);
    cvShowImage("Camera", cameraImage);
    bool saveNextFrame = false;
    vector<IplImage*> faces;
    vector<CvRect> objects;

    while(1) {

        cameraImage = getNextCameraFrame(capture);
        cvShowImage("Camera", cameraImage);
        // detect faces in image
        objects = detection->detectObjects(cameraImage, DETECT_A_FACE);

        if (saveNextFrame) {
            faces = detection->getDetectedObjects(cameraImage, objects);
            // save detected faces in a new image
            detection->saveImage(faces,dirName);
            saveNextFrame = false;
            faces.clear();
            (vector<IplImage*>()).swap(faces);
        }
        
        // show detected faces on screen
        if (objects.size() > 0)
        detection->showFaces(cameraImage,objects,(char*)"Camera");
        
        keyPressed = cvWaitKey(20);
	if (keyPressed == 't') {
            saveNextFrame = true;
	}
        if (keyPressed == 'q') {
            break;
        }
    }

    cvReleaseCapture( &capture );
    cvDestroyWindow("Camera");
    // delete detection module
    delete detection;

    // delete database module
    delete database;

    return EXIT_SUCCESS;

}

int recognizeFromWebcam() {

    // used modules
    Database* database = new Database();
    Detection* detection = new Detection();
    Recognition* recognition = new Recognition();

    // webcam captured images
    CvCapture* capture = 0;
    IplImage* cameraImage;
    char keyPressed = 0;

    // connect to database
    database->connect();

    // initialize detection module
    vector<char*> args;
    args.push_back((char*)"--hits=3");
    args.push_back((char*)"--size=40");
    detection->initialize(args);

    // initialize recognition module
    recognition->initialize(RECOGNITION_THRESHOLD,
		USE_MAHALANOBIS_DISTANCE ? MAHALANOBIS_DISTANCE : EUCLIDIAN_DISTANCE,
		(char*)IMAGE_FILE_EXTENSION, (char*)TRAINING_DATA_FILE);

    // train using current database images
    int res = recognition->train(USE_DATABASE_FOR_TRAINING ? DATABASE_SRC : FILE_SRC,
        (char*)TRAINING_SRC_PATH, SAVE_TRAINING_DATA);
    if (res != 0) {
        delete recognition;
        delete detection;
        delete database;
        return (EXIT_FAILURE);
    }

    // save training result
    recognition->saveTrainingData((char*)TRAINING_DATA_FILE);
    
    // open webcam
    capture = cvCaptureFromCAM( 0 );
    sleep(1);
    if ( !capture ) {
	fprintf(stderr, "Error! Could not access the camera.\n");
        exit(0);
    }
    cvNamedWindow("Camera", CV_WINDOW_AUTOSIZE);

    cameraImage = getNextCameraFrame(capture);
    cvShowImage("Camera", cameraImage);
    vector<IplImage*> faces;
    vector<CvRect> objects;

     while(1) {
        // get next camera frame
        cameraImage = getNextCameraFrame(capture);
        cvShowImage("Camera", cameraImage);

        // detect objects from image
        objects = detection->detectObjects(cameraImage, DETECT_ALL_FACES);

        if (objects.size() > 0) {

            // detect faces in captured image
            faces = detection->getDetectedObjects(cameraImage, objects);
            detection->showFaces(cameraImage,objects,(char*)"Camera");

            //recognize
            vector<RecognitionResult> results = recognition->recognizeFaces(faces, 1);
            vector<char*> personNames;

            for (int i = 0; i<results.size(); i++) {

                //get person id
                int personId = results[i].personIDArr[0];
                char* result = (char*)malloc(100*sizeof(char));
                if (personId == 0)
                    sprintf(result,"unknown");
                else {
                    // get person name
                    string personName = database->getInfo(personId,FIRSTNAME);
                    sprintf(result, "%s",personName.c_str());
                }
                
                personNames.push_back(result);
            }

            // show person name
            showText(cameraImage, objects, (char*)"Camera", personNames);

            faces.clear();
            (vector<IplImage*>()).swap(faces);

        }

        keyPressed = cvWaitKey(20);
	// check for exit
        if (keyPressed == 'q') {
            break;
        }
     }

    // release capture and window
    cvReleaseCapture( &capture );
    cvDestroyWindow("Camera");

    // release modules
    delete detection;
    delete recognition;
    delete database;

    return EXIT_SUCCESS;

}

void showText(IplImage* image, vector<CvRect> faces, char* windowName,
              vector<char*> personNames) {

    int i=0;
    for(vector<CvRect>::const_iterator r = faces.begin(); r != faces.end(); r++ ) {

        // choose text position and show text on screen
        CvPoint pt = cvPoint ( r->x, r->y );
        CvFont font;
        cvInitFont(&font, CV_FONT_HERSHEY_COMPLEX, 1.0, 1.0 , 0,1, CV_AA);
        cvPutText(image, personNames.at(i), pt, &font, CV_RGB(0,255,0));
        i++;
    }

    cvShowImage(windowName, image);

}

char* getNextLine() {

    // allocate next line
    char *line = (char*)malloc(100*sizeof(char)), *linep = line;
    size_t lenmax = 100, len = lenmax;
    int c;

    if(line == NULL)
        return NULL;

    for(;;) {
        c = fgetc(stdin);
       
        if( c == EOF || c == '\n')
            break;

        // realloc line
        if(--len == 0) {
            char* linen = (char*)realloc(linep, lenmax *= 2*sizeof(char));
            len = lenmax;

            if(linen == NULL) {
                free(linep);
                return NULL;
            }
            line = linen + (line - linep);
            linep = linen;
        }

        // append read character into string
        *line++ = c;
    }

    // add string terminator
    *line = '\0';

    // return console input
    return linep;
}

int getLastPersonId() {

    DIR* dir;
    struct dirent* subdir;
    int maxPersonId = -1;

    //open database dir
    dir = opendir (TRAINING_SRC_PATH);
    if (!dir) {
        fprintf(stderr, "Cannot open database path '%s'", TRAINING_SRC_PATH);
        return 1;
    }

    // get maximum dir id from database
    while (subdir = readdir(dir)) {
        if (subdir->d_type == DT_DIR) {
            int personID = atoi(subdir->d_name+1);
            maxPersonId = (personID > maxPersonId) ? personID : maxPersonId;
        }
    }

    return maxPersonId;

}






