/* 
 * File:   Detection.cpp
 * Author: elena
 */

#include <stdlib.h>
#include <iostream>
#include <cstdio>
#include "Detection.h"


/*
 * Module for detecting all faces from a given image
 */

Detection::Detection() {

    // set default values
    this->cascadeName_ = "cascade/haarcascade_frontalface_alt2.xml";
    this->eyeCascadeName_ = "cascade/haarcascade_eye.xml";
    this->scaleValue_ = 1.1f;
    this->minNeighbours_ = 2;
    this->flags_ = 0;
    this->templateSize_ = 25;
    this->templateEyeSize_ = 10;
    this->resultImageWidth_ = 92;
    this->resultImageHeight_ = 112;
    this->nrOfDetectedFaces_ = 0;
    this->index_ = 0;

    this->cascade_ = (CvHaarClassifierCascade*)cvLoad(this->cascadeName_.c_str(),
                                                        0, 0, 0 );


    this->eyeCascade_ = (CvHaarClassifierCascade*)cvLoad(this->eyeCascadeName_.c_str(),
                                                        0, 0, 0 );

    if ( !this->cascade_ ) {
        fprintf(stderr, "[Detection] Error loading cascade![%s]\n",
                                        this->cascadeName_.c_str());
    }

    if ( !this->eyeCascade_ ) {
        fprintf(stderr, "[Detection] Error loading cascade![%s]\n",
                                        this->eyeCascadeName_.c_str());
    }

    // set default flags for face detection
    this->flags_ |= CV_HAAR_DO_CANNY_PRUNING;
 
}

Detection::~Detection() {

    // release cascade
    cvReleaseHaarClassifierCascade(&this->cascade_);
    cvReleaseHaarClassifierCascade(&this->eyeCascade_);
    
}


void Detection::initialize(vector<char*> args) {
  
    // valid argument options
    const string cascadeOption       = "--cascade=";
    const string scaleOption         = "--scale=";
    const string minHitsOption       = "--hits=";
    const string nrOfDetectedFaces   = "--detectedFaces=";
    const string templateSizeOption  = "--size=";
    const string resultImageWidth    = "--resultImageWidth=";
    const string resultImageHeight   = "--resultImageHeight=";

    // read input parameters
    for (int i = 0; i < args.size(); i++) {

      
        // cascade parameter
        if (cascadeOption.compare(0, cascadeOption.length(), args[i],
                                  cascadeOption.length()) == 0) {
            this->cascadeName_.assign(args[i] + cascadeOption.length());

            this->cascade_ = (CvHaarClassifierCascade*)cvLoad(this->cascadeName_.c_str(),
                                                        0, 0, 0 );


            if ( !this->cascade_ ) {
                fprintf(stderr, "[Detection] Error loading cascade![%s]\n",
                                        this->cascadeName_.c_str());
            }
        }

        // scale parameter
        else if (scaleOption.compare(0, scaleOption.length(), args[i],
                 scaleOption.length()) == 0) {
            if( !sscanf( args[i] + scaleOption.length(), "%lf", 
                    &this->scaleValue_ )  || this->scaleValue_ < 1 )
                this->scaleValue_ = 1.1f;
        }

        // min hits parameter
        else if (minHitsOption.compare(0, minHitsOption.length(), args[i],
                minHitsOption.length()) == 0) {
            if( !sscanf( args[i] + minHitsOption.length(), "%d", 
                    &this->minNeighbours_ )  || this->minNeighbours_ < 1 )
                this->minNeighbours_ = 2;
        }

        
        // nr of detected faces parameter
        else if (nrOfDetectedFaces.compare(0, nrOfDetectedFaces.length(), args[i],
                nrOfDetectedFaces.length()) == 0) {
            if( !sscanf( args[i] + nrOfDetectedFaces.length(), "%d",
                    &this->nrOfDetectedFaces_ )  || this->nrOfDetectedFaces_ < 0 )
                this->nrOfDetectedFaces_ = 0;
        }
        // template size parameter
        else if (templateSizeOption.compare(0,
                templateSizeOption.length(), args[i],
                templateSizeOption.length()) == 0) {
            if( !sscanf( args[i] + templateSizeOption.length(), "%d",
                    &this->templateSize_) || this->templateSize_ < 10 )
                this->templateSize_ = 25;
        }

        // result image width parameter
        else if (resultImageWidth.compare(0,
                resultImageWidth.length(), args[i],
                resultImageWidth.length()) == 0) {
            if( !sscanf( args[i] + resultImageWidth.length(), "%d",
                    &this->resultImageWidth_) || this->resultImageWidth_ < 1 )
                this->resultImageWidth_ = 92;
        }

        // result image height parameter
        else if (resultImageHeight.compare(0,
                resultImageHeight.length(), args[i],
                resultImageHeight.length()) == 0) {
            if( !sscanf( args[i] + resultImageHeight.length(), "%d",
                    &this->resultImageHeight_) || this->resultImageHeight_ < 1 )
                this->resultImageHeight_ = 112;
        }

        else {
            fprintf(stderr, "[Detection] Error in method initialize!(Unknown parameter!)\n"
                            "Valid parameters:\n "
                            "          --cascade=cascade_name\n "
                            "          --scale=scale_value\n "
                            "          --hits=hits_number\n "
                            "          --size=min_template_size_value \n"
                            "          --resultImageWidth=width_of_detected_face_image\n"
                            "          --resultImageHeight=height_of_detected_face_image\n");
         
        }
        
    }

}


bool Detection::checkRectOverlap(CvRect r1, CvRect r2) {

    bool overlap = false;
    float percent = 0.8;

    if (r1.height <=0 || r1.width <=0 ||
        r2.height <=0 || r2.width <=0) {
        fprintf(stderr, "[Detection] Error in method checkRectOverlap!\n"
                        "Usage: checkRectOverlap(CvRect r1, CvRect r2).\n");
    }

    if ( r1.x > r2.x + r2.width ||
         r1.x + r1.width < r2.x ||
         r1.y > r2.y + r2.height ||
         r1.y + r1.height < r2.y ) {

        return overlap;
    }

    // determine rectangle with minimum area
    double area1 = r1.height * r1.width;
    double area2 = r2.height * r2.width;
    double minArea = area1 < area2 ? area1 : area2;

    // determine the overlap area between r1 and r2
    double overlapArea = (max(r1.x, r2.x) - min(r1.x + r1.width, r2.x + r2.width)) *
                         (max(r1.y, r2.y) - min(r1.y + r1.height, r2.y + r2.height));

    // if overlap are is greater than 80% => return true
    if (overlapArea > percent*minArea) {
        overlap = true;
    }

    /*
    if (overlap)
        if (overlapArea > 0.0 )
            printf("aici = %lf %lf\n", overlapArea, minArea, r1.x, r1.y);
    */
    
    return overlap;

}

IplImage* Detection::cvResizeImage(IplImage* sourceImage, int width, int height) {

    if (!sourceImage || width <= 0 || height <= 0) {
        fprintf(stderr, "[Detection] Error in method cvResizeImage!\n"
                        "Usage: cvResizeImage(IplImage* sourceImage, "
                        "int width, int height).\n");
    }
    IplImage* resultImage;
    // create result image
    resultImage = cvCreateImage(cvSize(width, height),
                               sourceImage->depth,
                               sourceImage->nChannels);

    if (width > sourceImage->width && height > sourceImage->height) {
        // CV_INTER_LINEAR: good with increasing image dimensions
       cvResize(sourceImage,       // source image
               resultImage,         // result image
               CV_INTER_LINEAR      // interpolation = Bilinear
               );
    }

    else {
        // CV_INTER_AREA: good with decreasing image dimensions
       cvResize(sourceImage,     // source image
               resultImage,       // result image
               CV_INTER_AREA      // interpolation = Pixel area re-sampling
               );
    }

    return resultImage;

}


IplImage* Detection::resizeImage(IplImage* sourceImage, int width, int height) {

    if (!sourceImage || width <= 0 || height <= 0) {
        fprintf(stderr, "[Detection] Error in method resizeImage!\n"
                        "Usage: resizeImage(IplImage* sourceImage, "
                        "int width, int height).\n");
    }
    
    IplImage* resultImage;
    int sourceWidth;
    int sourceHeight;
    CvRect r;

    // get source image width and height
    sourceWidth = sourceImage->width;
    sourceHeight = sourceImage->height;

    // source image aspect ratio
    float sourceRatio = ((float)sourceWidth / sourceHeight);
    // result image aspect ratio
    float resultRatio = ((float)width / height);
    // determine cropped area from picture
    if (sourceRatio > resultRatio) {
	int dw = (sourceHeight * width) / height;
	r = cvRect((sourceWidth - dw)/2, 0, dw, sourceHeight);
    }
    else {
	int dh = (sourceWidth * height) / width;
	r = cvRect(0, (sourceHeight - dh)/2, sourceWidth, dh);
    }

    IplImage *croppedImage = cropImage(sourceImage, r);

    // resize cropped image using openCv resize method
    resultImage = this->cvResizeImage(croppedImage, width, height);

    // release cropped image
    cvReleaseImage( &croppedImage );

    // return resized image
    return resultImage;
    
}

IplImage* Detection::cropImage(IplImage *sourceImage, CvRect r) {


    if (!sourceImage || r.height < 0 || r.width < 0 ) {
        fprintf(stderr, "[Detection] Error in method cropImage!\n"
                        "Usage: cropImage(IplImage *sourceImage, CvRect r).\n");
    }
    IplImage *resultImage;
    CvSize size;

    // select the needed region from the image
    cvSetImageROI((IplImage*)sourceImage, r);
    // create result image
    size = cvSize(r.width, r.height);
    resultImage = cvCreateImage(size, IPL_DEPTH_8U, sourceImage->nChannels);
    // copy cropped rectangle into the result image
    cvCopy(sourceImage, resultImage);

    // clear selected region from initial image
    cvResetImageROI(sourceImage);

    // return an image containing just the required surface
    return resultImage;
}

IplImage* Detection::processImage(IplImage* sourceImage) {

    if (!sourceImage) {
        fprintf(stderr, "[Detection] Error in method processImage!\n"
                        "Usage: processImage(IplImage* sourceImage).\n");
    }

    // grayscale image
    IplImage *greyImage;
    // processed image
    IplImage *resultImage;

    // convert initial image to grayscale
    greyImage = cvCreateImage( cvGetSize(sourceImage), IPL_DEPTH_8U, 1);
    cvCvtColor( sourceImage, greyImage, CV_BGR2GRAY );

    // resize grayscale image

    //CvSize size = cvSize(this->resultImageWidth_, this->resultImageHeight_);
    //resultImage = cvCreateImage(size, IPL_DEPTH_8U, 1);
    //cvResize(greyImage, resultImage, CV_INTER_LINEAR);

    // method 1 - resize image without cropping it
    /*
    resultImage = this->cvResizeImage(greyImage, this->resultImageWidth_,
                                      this->resultImageHeight_ );
     */

    // method 2 - resize image by cropping it first
    resultImage = this->resizeImage(greyImage, this->resultImageWidth_,
                                    this->resultImageHeight_);
    

    // equalize brightness and contrast
    cvEqualizeHist(resultImage, resultImage);

    // release grayscale image
    cvReleaseImage(&greyImage);

    // return processed image
    return resultImage;

}


bool Detection::eyeDetection(IplImage* faceImage) {

    if ( !faceImage ) {
        fprintf(stderr, "[Detection] Error in method eyeDetection!\n"
                        "Usage: eyeDetection(IplImage* faceImage)\n");
        return false;
    }


    // list of detected objects
    CvSeq* detectedObjects;
    bool eyesDetected = false;
    // expandable memory buffer
    CvMemStorage* storage;

    // create storage
    storage = cvCreateMemStorage(0);
    cvClearMemStorage( storage );

    
    faceImage = resizeImage(faceImage, 100, 150);
    CvRect aux = cvRect(0,0,faceImage->width,3*faceImage->height/4);
    faceImage = this->cropImage(faceImage, aux);
    detectedObjects = cvHaarDetectObjects(faceImage,
                    this->eyeCascade_,      // eye cascade
                    storage,                // memory storage
                    this->scaleValue_,      // haar scale value
                    this->minNeighbours_,   // min detected region to be
                                            // considered an eye
                    0,
                    cvSize(this->templateEyeSize_, this->templateEyeSize_));


    //printf("nr of detected eyes: %d\n",detectedObjects->total);
    if (detectedObjects->total > 0) {
        eyesDetected = true;
    }
    // release storage
    if (storage) {
        cvReleaseMemStorage(&storage);
    }

    return eyesDetected;
}

vector<CvRect> Detection::detectObjects(IplImage* image,int nrOfDetectedFaces) {


    if (!image || nrOfDetectedFaces < -1) {
        fprintf(stderr, "[Detection] Error in method detectObjects!\n"
                        "Usage: detectObjects(IplImage* image,"
                        "int nrOfDetectedFaces).\n");
    }

    // set flags according to the number of faces needed to be detected
    if (nrOfDetectedFaces >= 0) {

        this->flags_ = 0 | CV_HAAR_DO_CANNY_PRUNING;
        this->nrOfDetectedFaces_ = nrOfDetectedFaces;
    }


    if (nrOfDetectedFaces == DETECT_A_FACE) {

        this->flags_ |=   CV_HAAR_DO_ROUGH_SEARCH
                         | CV_HAAR_FIND_BIGGEST_OBJECT;
    }

    // image used for face detection in grayscale
    IplImage* detectImage;
    // image used for face detection scaled (optional)
    IplImage* small_img;
    // expandable memory buffer
    CvMemStorage* storage;
    // list of detected objects ( possible faces )
    CvSeq* detectedObjects;
    // smallest region in which to search for a face
    CvSize minTemplateSize = cvSize(this->templateSize_, this->templateSize_);
    // vector of detected objects
    vector<CvRect> objects;
    // vector of detected faces
    vector<CvRect> faces;

    // convert image to grayscale
    detectImage = cvCreateImage( cvSize(image->width,image->height), 8, 1 );
    cvCvtColor( image, detectImage, CV_BGR2GRAY );
    // equalize brightness and contrast
    cvEqualizeHist( detectImage, detectImage );

    // create storage
    storage = cvCreateMemStorage(0);
    cvClearMemStorage( storage );

   // printf("parameters: scale=%f, hits=%d, flags=%d, templateSize=%d",
   //         this->scaleValue_, this->minNeighbours_, this->flags_,
   //         this->templateSize_);

    double t = 0;
    t = (double)cvGetTickCount();
    //detect faces from image
    detectedObjects = cvHaarDetectObjects(
            detectImage,            // input image
            this->cascade_,         // haar classifier
            storage,                // memory for detected faces
            this->scaleValue_,      // scale factor for cascade classifier
            this->minNeighbours_,   // nr of overlapping detection for an
                                    // object to be considered face
            this->flags_,           // detection settings
            minTemplateSize         // smallest region in which to search
                                    // for a face
            );

    t = (double)cvGetTickCount() - t;
    //printf( "detection time = %g ms\n", t/((double)cvGetTickFrequency()*1000.) );
    if (!detectedObjects) {

        fprintf(stderr, "[Detection] Error! No objects found in face detection!\n");
         // relesese detect image
        cvReleaseImage( &detectImage );
        // release storage
        if (storage) {
            cvReleaseMemStorage(&storage);
        }
        return (vector<CvRect>)(NULL);
    }

    int nrOfDetectedObjects = this->nrOfDetectedFaces_;
    if (nrOfDetectedObjects == 0
        || this->nrOfDetectedFaces_ > detectedObjects->total) {
        nrOfDetectedObjects = detectedObjects->total;
    }

    faces.reserve(nrOfDetectedObjects);
    objects.reserve(nrOfDetectedObjects);
    // get detected faces
    for(int i = 0; i < nrOfDetectedObjects ; i++ ) {

        CvRect* r = (CvRect*)cvGetSeqElem( detectedObjects, i );
        objects.push_back(*r);

        bool diffObject = true;
        // eliminate double detection for a face
        for (int j = 0; j < i; j++ ) {

            CvRect prev = objects.at(j);
            if (checkRectOverlap(prev, *r)) {
                diffObject = false;
                break;
            }
        }

        if (diffObject) {

            int dx = (*r).width/10;
            int dy = (*r).height/10;
            //int x = ((*r).x - dx) >= 0 ? ((*r).x - dx) : 0;
            int x = (*r).x;
            int y = ((*r).y - dy) >= 0 ? ((*r).y - dy) : 0;
           
            int w = (*r).width;
            int h = ((*r).height + 2*dy) <= image->height ? ((*r).height + 2*dy) :
                                                        image->height;
          

            // check if detected objects is actually a face (contains eyes)
            
            IplImage* faceImage = this->cropImage(detectImage, *r);
            if (EYE_DETECTION) {
                if (this->eyeDetection(faceImage)) {
                    faces.push_back(*r);
                }
            }
            else {
                faces.push_back(*r);
            }
            cvReleaseImage(&faceImage);  
        }

    }

    //release faces vector
    objects.clear();
    vector<CvRect>().swap(objects);

    // relesese detect image
    cvReleaseImage( &detectImage );
    // release storage
    if (storage) {
        cvReleaseMemStorage(&storage);
    }

    return faces;

}
  

 vector<IplImage*> Detection::getDetectedObjects(IplImage* image,
                                                vector<CvRect> objects) {
    

    vector<IplImage*> resultImages;
   
    // get detected faces
    for(vector<CvRect>::const_iterator r = objects.begin(); r != objects.end(); r++ ) {
         
        IplImage* faceImage = this->cropImage(image, *r);
        IplImage* processedImage = this->processImage(faceImage);
        resultImages.push_back(processedImage);

        cvReleaseImage(&faceImage);            
      
    }

    //return resultImages;
    return resultImages;
}

void Detection::showFaces(IplImage* image, vector<CvRect> faces, char* windowName) {

    for(vector<CvRect>::const_iterator r = faces.begin(); r != faces.end(); r++ ) {

        // draw an rectangle on a face ( test phase )

        CvPoint pt1 = { r->x, r->y };
        CvPoint pt2 = { r->x + r->width, r->y + r->height };
        cvRectangle(image, pt1, pt2, CV_RGB(0,255,0), 2, 4, 0);

    }

    cvShowImage(windowName, image);

}

 void Detection::saveImage(vector<IplImage*> faces, string path) {

     char filename[100];

     for(vector<IplImage*>::const_iterator face = faces.begin();
                                            face != faces.end(); face++ ) {


         sprintf(filename, "%s/%d.pgm", path.c_str(), this->index_++);
         cvSaveImage(filename, *face);
     }

 }

 void Detection::saveResultToFile(IplImage* image, vector<CvRect> faces, char* path) {

     for(vector<CvRect>::const_iterator r = faces.begin(); r != faces.end(); r++ ) {

        // draw an rectangle on a face ( test phase )

        CvPoint pt1 = { r->x, r->y };
        CvPoint pt2 = { r->x + r->width, r->y + r->height };
        cvRectangle(image, pt1, pt2, CV_RGB(0,255,0), 2, 4, 0);

    }

    cvSaveImage(path, image);
     
 }

