/* 
 * File:   Detection.h 
 * Author: Elena Holobiuc
 *
 * Detection module - it can be used to detect one or more faces from an
 *                    input image.
 * You can initialize the module with the following parameters:
 *       --cascade=cascade_name
         --scale=scale_value
         --hits=hits_number
         --size=min_template_size_value
         --resultImageWidth=width_of_detected_face_image
         --resultImageHeight=height_of_detected_face_image
 *
 * Created on May 18, 2010, 3:34 PM
 */

#ifndef _DETECTION_H
#define	_DETECTION_H

#define DETECT_ALL_FACES 0
#define DETECT_A_FACE    1

#define EYE_DETECTION true

#include "CppInterfaces.h"
#include "IDetection.h"
#include "cvaux.h"
#include "highgui.h"


class Detection : implements IDetection {

// Construction & Destruction
public:
    Detection();
    
    ~Detection();

public:
    /*
    * Method for initializing face detection algorithm
    */
    void initialize(vector<char*> args);

    /*
     * Method that detects nrOfDetectedFaces in an image and returns a vector
     * containing the coordinates of the detected faces
     */
    vector<CvRect> detectObjects(IplImage* image, int nrOfDetectedFaces);

    /*
     * Method which returns all faces found in an image
     */
     vector<IplImage*> getDetectedObjects(IplImage* image,
                                                vector<CvRect> objects);

     /*
     * Method that displays all detected faces in a certain window
     */
    void showFaces(IplImage* image,vector<CvRect> faces, char* windowName);

    /*
     * Method that saves a vector of IplImage* in a given directory
     */
    void saveImage(vector<IplImage*> faces,string path);
    /*
     * Method that resizes a source image to a
     * given width and height  without changing
     * its aspect ratio
     */
    IplImage* resizeImage(IplImage *sourceImage, int width, int height);
     /*
     * Method that highlights all detected faces in image
     * and saves the result to path
     */
    void saveResultToFile(IplImage* image, vector<CvRect> faces, char* path);


private:

    /*
     * Method that performs eye detection and returns true if eyes
     * were found and false, otherwise
     */
    bool eyeDetection(IplImage* faceImage);
    
    /*
     * Method that determines whether two rectangles overlap
     */
    bool checkRectOverlap(CvRect r1, CvRect r2);

    /*
     * Method that returns a new image containing just a certain portion
     * from the source image
     */
    IplImage* cropImage(IplImage *sourceImage, CvRect r);

    /*
     * Method that processes images in order to use
     * them for face recognition
     */
    IplImage* processImage(IplImage *sourceImage);

     /*
     * Method that resizes a source image to a
     * given width and height by changing
     * its aspect ratio ( implemented by OpenCV)
     */
    IplImage* cvResizeImage(IplImage *sourceImage, int width, int height);
    

    // name of haar classifier used for face detection
    string cascadeName_;
    // name of haar classifier used for eye detection
    string eyeCascadeName_;
    // name of the image used for face detection
    string imageName_;
    // scale parameter used for increasing the classifier dimension
    double scaleValue_;
    // number of overlapping detections for an object to be considered a face
    int minNeighbours_;
    // flags used for detection:
    /*
     * CV_HAAR_DO_CANNY_PRUNING - skig regions with no lines
     * CV_HAAR_SCALE_IMAGE - scale the image, and not the classifier
     * CV_HAAR_FIND_BIGGEST_OBJECT - return only the biggest object
     * CV_HAAR_DO_ROUGH_SEARCH - stop the search when the first candidate
     *                           is found
     */
    int flags_;
    // nr of detected faces from the input image
    int nrOfDetectedFaces_;
    // the smallest region in which to search for a face
    int templateSize_;
    // the smallest region in which to search for an eye
    int templateEyeSize_;
    // HaarCascade classifier used for face detection
    CvHaarClassifierCascade* cascade_;
    // HaarCascade classifier used for eye detection
    CvHaarClassifierCascade* eyeCascade_;

    // width and height of the result face images
    int resultImageWidth_;
    int resultImageHeight_;

    // the index of the current result image
    int index_;
    
};


#endif	/* _DETECTION_H */

