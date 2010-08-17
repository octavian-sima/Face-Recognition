/* 
 * File:   main.cpp
 * Author: elena
 */

#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include "Detection.h"


int main(int argc, char** argv) {

    // initialize detection module
    Detection* detection = new Detection();
    if (argc == 1) {
    printf(                 "Usage: \n"
                            "          [--cascade=cascade_name]\n"
                            "          [--scale=scale_value]\n"
                            "          [--hits=hits_number]\n"
                            "          [--size=min_template_size_value]\n"
                            "          [--resultImageWidth=width_of_detected_face_image]\n"
                            "          [--resultImageHeight=height_of_detected_face_image]\n"
                            "           file\n");
        exit(0);
    }
         
    vector<char*> args;
    for (int i = 1; i < argc-1; i++)
        args.push_back(argv[i]);
    detection->initialize(args);

    //create output directory if not exist
    string dirName = "result";
    DIR* dir = opendir(dirName.c_str());
    if (! dir) {
        string command = "mkdir ";
        command.append(dirName);
        system(command.c_str());
    }

    // open list of files for detection
    FILE *f = fopen(argv[argc-1],"r");
    while (!feof(f)) {
        
        char fullImageName[100];
        fscanf (f,"%s",fullImageName);

        // get image Name ( remove directory path, if exists)
        string imageName;
        string key ("/");
        imageName.assign(fullImageName);
        size_t found;
        found = imageName.rfind(key);
        if (found!=string::npos) {
            imageName = imageName.substr (found+1);
        }
       
       
        // load image
        IplImage* image = cvLoadImage(fullImageName, CV_LOAD_IMAGE_COLOR);
        if ( !image ) {
            printf("Error loading image!%s",fullImageName);
            exit(0);
        }

        // detect faces from image and save output
        vector<CvRect> faces = detection->detectObjects(image,DETECT_ALL_FACES);
        char resultName[110];
        sprintf(resultName,"%s/%s",dirName.c_str(),imageName.c_str());
        detection->saveResultToFile(image,faces,resultName);
    }

    fclose(f);

    delete detection;
  
    return (EXIT_SUCCESS);
}

