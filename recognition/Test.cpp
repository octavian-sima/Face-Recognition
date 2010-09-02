/* 
 * File:   Test.cpp
 * Author: Octavian Sima
 *
 * Recognition module testing unit 
 */

#include <stdlib.h>

#include "Recognition.h"

//if false, use a file that contains a list of paths to face images
#define USE_DATABASE_FOR_TRAINING   false

//path to training database or to a file with list of images
#define TRAINING_SRC_PATH           "tests/lower3.txt"

//default test file path
#define TEST_FILE_PATH              "tests/upper6.txt"

//face image files extension
#define IMAGE_FILE_EXTENSION        "pgm"

//minimum recognition confidence for a person to be classify as known
#define RECOGNITION_THRESHOLD       0.5

//save average image and eigenvectors as images after training?
#define SAVE_TRAINING_DATA          true

//path to a xml file where training data is stored
#define TRAINING_DATA_FILE          "savedData.xml"

//if false, use EUCLIDIAN_DISTANCE
#define USE_MAHALANOBIS_DISTANCE   false

void showHeader() {
    printf("Test Recognition module\n");
    printf("Usage:\n");
    printf("For training application with default options run as\n");
    printf("\t ./recognition train \n");

    printf("For training application from database (such person_database) run as\n");
    printf("\t ./recognition train database databaseFolderPath \n");

    printf("For training application from a file (ex in tests/ ) run as\n");
    printf("\t ./recognition train file trainFilePath \n");

    printf("For testing application with default options run as\n");
    printf("\t ./recognition test \n");

    printf("For testing application from a file (ex in tests/ ) run as\n");
    printf("\t ./recognition test testFilePath recognitionThreshold\n");

    printf("For help run as \n");
    printf("\t ./recognition help \n");
    
    printf("\nTraining data result can be found in %s file.\n",TRAINING_DATA_FILE);
}


int main(int argc, char** argv) {
    Recognition* recognition = new Recognition();

    if (argc < 2) {
        showHeader();
        return 0;
    } else if (!strcmp(argv[1], "train")) {
        recognition->initialize(RECOGNITION_THRESHOLD,
                USE_MAHALANOBIS_DISTANCE ? MAHALANOBIS_DISTANCE : EUCLIDIAN_DISTANCE,
                (char*)IMAGE_FILE_EXTENSION, (char*)TRAINING_DATA_FILE);

        if (argc == 2) { //use default settings
            int res = recognition->train(USE_DATABASE_FOR_TRAINING ? DATABASE_SRC : FILE_SRC,
                (char*)TRAINING_SRC_PATH, SAVE_TRAINING_DATA);
            if (res != 0) {
                delete recognition;
                return (EXIT_FAILURE);
            }
            recognition->saveTrainingData((char*)TRAINING_DATA_FILE);

        } else if (argc == 4) {
            bool useDatabase = true;
            if (!strcmp(argv[2], "database")) {
                useDatabase = true;
            } else if (!strcmp(argv[2], "file")) {
                useDatabase = false;

            } else {
                fprintf(stderr, "Invalid arguments!\n");
                showHeader();
                delete recognition;
                return (EXIT_FAILURE);
            }

            int res = recognition->train(useDatabase, argv[3], SAVE_TRAINING_DATA);
            if (res != 0) {
                delete recognition;
                return (EXIT_FAILURE);
            }

            recognition->saveTrainingData((char*)TRAINING_DATA_FILE);

        } else { //wrong
             fprintf(stderr, "Invalid arguments!\n");
             showHeader();
             delete recognition;
             return (EXIT_FAILURE);
        }
           
    } else if (!strcmp(argv[1], "test")) {

        if (argc == 2) { //use default settings
            recognition->initialize(RECOGNITION_THRESHOLD,
                USE_MAHALANOBIS_DISTANCE ? MAHALANOBIS_DISTANCE : EUCLIDIAN_DISTANCE,
                (char*)IMAGE_FILE_EXTENSION, (char*)TRAINING_DATA_FILE);

            if (recognition->loadTrainingData((char*)TRAINING_DATA_FILE) != 0) {
                fprintf(stderr, "Run training first! \n");
                delete recognition;
                return (EXIT_FAILURE);
            }
            
            recognition->testRecognitionPerformance((char*)TEST_FILE_PATH);
            
        } else if (argc == 4) {
            char* filePath = argv[2];
            float threshold = atof(argv[3]);

            recognition->initialize(threshold,
                USE_MAHALANOBIS_DISTANCE ? MAHALANOBIS_DISTANCE : EUCLIDIAN_DISTANCE,
                (char*)IMAGE_FILE_EXTENSION, (char*)TRAINING_DATA_FILE);

            if (recognition->loadTrainingData((char*)TRAINING_DATA_FILE) != 0) {
                fprintf(stderr, "Run training first! \n");
                delete recognition;
                return (EXIT_FAILURE);
            }

            recognition->testRecognitionPerformance((char*)filePath);

        } else { //wrong
             fprintf(stderr, "Invalid arguments!\n");
             showHeader();
             delete recognition;
             return (EXIT_FAILURE);
        }
    }
    
    delete recognition;
    
    return (EXIT_SUCCESS);
}
