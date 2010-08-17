/* 
 * File:   IDetection.h
 * Author: Elena Holobiuc 
 * 
 * Declares detection interface using CppInterfaces.h
 * These methods are called by recognition module
 */


#ifndef _IDETECTION_H
#define	_IDETECTION_H

#include "CppInterfaces.h"
#include "cv.h"

using namespace std;


DeclareInterface(IDetection)

    //sets detection type and parameters
    virtual void initialize(vector<char*> args) = 0;

    //returns a vector of IplImage* with detected objects
    virtual vector<IplImage*> getDetectedObjects(IplImage* image,
                                                vector<CvRect> objects) = 0;

    // returns a vector of CvRect with detected objects
    virtual vector<CvRect> detectObjects(IplImage* image,
                                    int nrOfDetectedFaces) = 0;

   // displays detected faces in windown 'windowName'
    virtual void showFaces(IplImage* image,vector<CvRect> faces,
                                    char* windowName) = 0;

   // saves an IplImage* in a given path
    virtual void saveImage(vector<IplImage*> faces,string path) = 0;

    // returns 'sourceImage' resized to 'width'*'height'
    virtual IplImage* resizeImage(IplImage *sourceImage, int width, int height) = 0;
    
EndInterface


#endif	/* _IDETECTION_H */

