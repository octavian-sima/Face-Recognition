/*
 * File:   Recognition.cpp
 * Author: Octavian Sima
 *
 * Recognition module implementation 
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <string>
#include <set>

#include "Recognition.h"


Recognition::Recognition() {
    this->trainFacesNo_ = 0;
    this->eigenVectorsNo_ = 0;
    this->distanceType_ = EUCLIDIAN_DISTANCE;
    this->recogThreshold_ = 0.6f;
    strcpy(this->imageExtension, "pgm");
    this->defaultTrainingDataFile_ = strdup(DEFAULT_TRAINING_DATA_FILE);

    this->faceImages_ = 0;
    this->eigenVectors_ = 0;
    this->averagedImage_ = 0;

    this->personRealIDs_ = 0;
    this->eigenValues_ = 0;
    this->projectedTrainFaces_ = 0;
};

Recognition:: ~Recognition() {
    //free space

    //release images if not already done
    if (this->faceImages_ != 0) {
        for (int i = 0; i < this->trainFacesNo_; i++)
            cvReleaseImage(&(this->faceImages_[i]));
        cvFree(this->faceImages_);
    }
    
    if (this->eigenVectors_ != 0) {
        for (int i = 0; i < this->eigenVectorsNo_; i++)
            cvReleaseImage(&(this->eigenVectors_[i]));
        cvFree(this->eigenVectors_);
    }

    if (this->averagedImage_ != 0) {
        cvReleaseImage(&(this->averagedImage_));
    }

    //release cvMat
    if (this->personRealIDs_ != 0) {
        cvReleaseMat(&(this->personRealIDs_));
    }

    if (this->eigenValues_ != 0) {
        cvReleaseMat(&(this->eigenValues_));
    }

    if (this->projectedTrainFaces_ != 0) {
        cvReleaseMat(&(this->projectedTrainFaces_));
    }

    if (this->defaultTrainingDataFile_ != 0) {
        free(this->defaultTrainingDataFile_);
    }
};

   
int Recognition::initialize(float recognitionThreshold, int distanceType,
            char* faceImageExtension, char* defaultTrainingDataFile) {

    printf("Initializing ...\n");

    this->recogThreshold_ = recognitionThreshold;
    this->distanceType_ = distanceType;
    if (strlen(faceImageExtension) != 3) {
        fprintf(stderr, "Invalid image extension! Should be like pgm, jpg, png etc.\n");
        return 1;
    }
    strcpy(this->imageExtension, faceImageExtension);
    if (this->defaultTrainingDataFile_ != 0)  {
        free(this->defaultTrainingDataFile_);
    }
    this->defaultTrainingDataFile_ = strdup(defaultTrainingDataFile);
            
    return 0;
};


int Recognition::train(int trainingSetSourceType, char* pathToTrainingSet, bool saveEigens) {
    int trainingSetLoadResult;

    printf("Training on known face images...\n");
    
    //load training face images
    if (trainingSetSourceType == FILE_SRC)
        trainingSetLoadResult = loadTrainingFaceImages(pathToTrainingSet);
    else if (trainingSetSourceType == DATABASE_SRC)
        trainingSetLoadResult = loadTrainingFacesDatabase(pathToTrainingSet);

    if (trainingSetLoadResult != 0) //error in loading training set
        return trainingSetLoadResult;

    //perform PCA for dimensionality reduction
    if (performPrincipalComponentAnalysis())
        return 1;   //error in PCA function

    //project training images onto PCA subspace
    if (projectTrainingImages())
        return 1;   //error in projection

    if (saveEigens) {  //save eigenfaces and average image to files
        saveEigenFaces();
    }

    //free training face images (we'll use only projections from now)
    if (this->faceImages_ != 0) {
        for (int i = 0; i < this->trainFacesNo_; i++)
            cvReleaseImage(&(this->faceImages_[i]));
        cvFree(this->faceImages_);
    }
    
    return 0;
}

int Recognition::loadTrainingFaceImages(char* fileName) {
    FILE* facesFile;
    char data[256];
    int trainFacesNo = 0;    //number of training faces

    printf("Reading training set from file '%s'...\n", fileName);

    //open file
    facesFile = fopen(fileName, "r");
    if (!facesFile) {
        fprintf(stderr, "Error in opening training set file: %s", fileName);
        return 1;
    }

    //get lines no.
    while (fgets(data, 256, facesFile))
        trainFacesNo++;
        
    rewind(facesFile);
    
    this->trainFacesNo_ = trainFacesNo;

    if (trainFacesNo < 3) {
        fprintf(stderr, "Too few training faces (min 3)!\n");
        return 1;
    }

    //allocate data for storing images
    faceImages_ = (IplImage**)cvAlloc(trainFacesNo * sizeof(IplImage*));
    //create cvMat with person real IDs - CV_32SC1 - 32bit signed integer
    //cvMat is easier to be stored and read to/from file
    personRealIDs_ = cvCreateMat(1, trainFacesNo, CV_32SC1);

    //read file line by line
    for (int faceIndex = 0; faceIndex < trainFacesNo; faceIndex++) {
        int id;
        fscanf(facesFile, "%i %s", personRealIDs_->data.i+faceIndex, data);
        faceImages_[faceIndex] = cvLoadImage(data, CV_LOAD_IMAGE_GRAYSCALE);
        if (!faceImages_[faceIndex]) {
            fprintf(stderr, "Error loading face image: %s", data);
            return 1;
        }
    }

    fclose(facesFile);
    
    printf("All %i face images have been succesfully read from file.\n", trainFacesNo);
    
    return 0;
}

int Recognition::loadTrainingFacesDatabase(char* databasePath) {
    DIR* dir;
    DIR* subjectDir;
    struct dirent* subdir;
    string databasePathStr (databasePath);
    int trainFacesNo = 0;    //number of training faces
    
    printf("Reading training set from database '%s'...\n", databasePath);
    //open database dir
    dir = opendir (databasePath);
    if (!dir) {
        fprintf(stderr, "Cannot open database path '%s'", databasePath);
        return 1;
    }
    
    //get total number of face images
    string cmd("ls -R -l " + databasePathStr + " | grep ." + this->imageExtension + " | wc -l");
    FILE* countPipe = popen(cmd.c_str(),"r");
    fscanf(countPipe,"%i",&trainFacesNo);
    pclose(countPipe);

    this->trainFacesNo_ = trainFacesNo;

    //allocate data for storing images
    faceImages_ = (IplImage**)cvAlloc(trainFacesNo * sizeof(IplImage*));
    if (!faceImages_) {
        fprintf(stderr, "Cannot allocate faceImages_ !\n");
        return 1;
    }
    //create cvMat with person real IDs - CV_32SC1 - 32bit signed integer
    //cvMat is easier to be stored and read to/from file
    personRealIDs_ = cvCreateMat(1, trainFacesNo, CV_32SC1);

    //read face images from subfolders
    int faceIndex = 0;
    while (subdir = readdir(dir)) {
	if (subdir->d_type == DT_DIR) {
           
            int personID = atoi(subdir->d_name+1);
            if (personID) {//not face subdir
                string subdirPath (databasePathStr + "/" + subdir->d_name);
                
                printf("Entering dir %s ...\n", subdirPath.c_str());
                
                //open subject image dir
                subjectDir = opendir(subdirPath.c_str());
                while (subdir = readdir(subjectDir)) {

                    if (subdir->d_type == DT_REG) {
                        personRealIDs_->data.i[faceIndex] = personID;
                        string imagePath (subdirPath + "/" + subdir->d_name);
                        faceImages_[faceIndex] = cvLoadImage(imagePath.c_str(),
                            CV_LOAD_IMAGE_GRAYSCALE);
                        if (!faceImages_[faceIndex]) {
                            fprintf(stderr, "Error loading face image: %s", imagePath.c_str());
                            return 1;
                        }
                        //printf("Image %s succesfully loaded\n", imagePath.c_str());
                        faceIndex++;
                    }
                }
                closedir(subjectDir);
            }
        }
     }
    
    closedir(dir);
    printf("All %i face images have been succesfully read from database.\n", trainFacesNo);
    
    /*
    	//creates a large image with 'columns' images on a row made of training images.
        int colsNo = min(trainFacesNo, 10);
        int rowsNo = 1 + (trainFacesNo / 10);
        int w = faceImages_[0]->width;
        int h = faceImages_[0]->height;
        CvSize size = cvSize(colsNo * w, rowsNo * h);

        // 8-bit Greyscale UCHAR image
        IplImage *bigImg = cvCreateImage(size, IPL_DEPTH_8U, 1);

        for (int i = 0; i < trainFacesNo; i++) {
            //get the eigenface image.
            IplImage *byteImg = convertFloatImageToUcharImage(faceImages_[i]);
            //paste it into the correct position.
            int x = w * (i % colsNo);
            int y = h * (i / colsNo);
            CvRect ROI = cvRect(x, y, w, h);
            cvSetImageROI(bigImg, ROI);
            cvCopyImage(byteImg, bigImg);
            cvResetImageROI(bigImg);
            cvReleaseImage(&byteImg);
        }
        cvSaveImage("trainFaces.pgm", bigImg);
        cvReleaseImage(&bigImg);
    */
    
    return 0;
}

int Recognition::saveTrainingData(char* fileName) {
    CvFileStorage* xmlFileStorage; //file-storage interface

    if (!fileName) { //use default file
        printf("Saving training data to file %s ...\n", this->defaultTrainingDataFile_);
        xmlFileStorage = cvOpenFileStorage(this->defaultTrainingDataFile_, 0, CV_STORAGE_WRITE);
    } else {
        printf("Saving training data to file %s ...\n", fileName);
        xmlFileStorage = cvOpenFileStorage(fileName, 0, CV_STORAGE_WRITE);
    }

    //write data to file
    //this is the advantage of using CvMat (easy to read and write to file)
    cvWriteInt(xmlFileStorage, "trainFacesNo_", trainFacesNo_ );
    cvWrite(xmlFileStorage, "personRealIDs_", personRealIDs_, cvAttrList(0,0));
    cvWriteInt(xmlFileStorage, "eigenVectorsNo_", eigenVectorsNo_ );
    cvWrite(xmlFileStorage, "eigenValues_", eigenValues_, cvAttrList(0,0));
    for (int i = 0; i < eigenVectorsNo_; i++) {
        stringstream variableName;
        variableName << "eigenVector_" << i;
        cvWrite(xmlFileStorage, variableName.str().c_str(), eigenVectors_[i], cvAttrList(0,0));
    }
    cvWrite(xmlFileStorage, "averagedImage_", averagedImage_, cvAttrList(0,0));
    cvWrite(xmlFileStorage, "projectedTrainFaces_", projectedTrainFaces_, cvAttrList(0,0));
    
    //release the file-storage interface
    cvReleaseFileStorage(&xmlFileStorage);

    printf("All training data saved to file.\n");
    return 0;
}

int Recognition::loadTrainingData(char* fileName) {
    CvFileStorage* xmlFileStorage; //file-storage interface

    if (!fileName) { //use default file
        printf("Loading training data from file %s ...\n", this->defaultTrainingDataFile_);
        xmlFileStorage = cvOpenFileStorage(this->defaultTrainingDataFile_, 0, CV_STORAGE_READ);
    } else {
        printf("Loading training data from file %s ...\n", fileName);
        xmlFileStorage = cvOpenFileStorage(fileName, 0, CV_STORAGE_READ);
    }

    if (!xmlFileStorage) {
        fprintf(stderr, "Cannot open file !\n");
        return 1;
    }

    //read data from file
    //this is the advantage of using CvMat (easy to read and write to file)
    this->trainFacesNo_ = cvReadIntByName(xmlFileStorage, 0, "trainFacesNo_", 0);
    this->personRealIDs_ = (CvMat*)cvReadByName(xmlFileStorage, 0, "personRealIDs_", 0);
    this->eigenVectorsNo_ = cvReadIntByName(xmlFileStorage, 0, "eigenVectorsNo_", 0);
    this->eigenValues_ = (CvMat*)cvReadByName(xmlFileStorage, 0, "eigenValues_", 0);

    //allocate eigen vectors
    this->eigenVectors_ = (IplImage**)cvAlloc(this->trainFacesNo_ * sizeof(IplImage*));
    if (!eigenVectors_) {
        fprintf(stderr, "Cannot allocate space for eigenVectors_ !\n");
        return 1;
    }
    //read eigen vectors
    for (int i = 0; i < this->eigenVectorsNo_; i++) {
        stringstream variableName;
        variableName << "eigenVector_" << i;
        this->eigenVectors_[i] = (IplImage*)cvReadByName(xmlFileStorage, 0,
            variableName.str().c_str(), 0);
    }

    this->averagedImage_ = (IplImage*)cvReadByName(xmlFileStorage, 0, "averagedImage_", 0);
    this->projectedTrainFaces_ = (CvMat*)cvReadByName(xmlFileStorage, 0, "projectedTrainFaces_", 0);
	
    //release the file-storage interface
    cvReleaseFileStorage(&xmlFileStorage);

    printf("All training data loaded from file.\n");
    return 0;
}

vector<RecognitionResult> Recognition::recognizeFaces(vector<IplImage*> faces, int resultsNo) {
    //allocate vector for result
    vector<RecognitionResult> result;
    result.reserve(faces.size());
    
    //declare and allocate space for projected test image
    float* projectedTestImage = (float*)cvAlloc( this->eigenVectorsNo_ * sizeof(float) );
    if (!projectedTestImage) {
        fprintf(stderr, "Cannot allocate space for projectedTestImage\n");
        return result;
    }

    for (vector<IplImage*>::iterator it = faces.begin(); it < faces.end(); ++it) {
        //perform test image projection onto PCA subspace
        cvEigenDecomposite(
                *it,                //IplImage object
                eigenVectorsNo_,    //eigen objects no.
                eigenVectors_,      //eigen vectors array or read callback
                0,                  //ioFlags (for callback)
                0,                  //userData for callback data structure
                averagedImage_,     //averaged image
                projectedTestImage);//eigenvalues

        RecognitionResult recogResult = findClosestFaces(projectedTestImage, resultsNo);
        //add result
        result.push_back(recogResult);
    }

    //free space
    if (projectedTestImage != 0)
        cvFree(&projectedTestImage);
    
    return result;
}

void Recognition::testRecognitionPerformance(char* facesFilePath) {
    if (this->eigenVectors_ == 0) {
        fprintf(stderr, "Error! Run training or loadTrainingData from file before testing recognition!\n");
        //TODO: load training data from file
        return;
    }

    IplImage* testImage = 0;
    int expectedID = 0;     //recognition expected result
    int testsNo = 0;        //test face images number
    float* projectedTestImage = 0;
    
    int correctRecogNo = 0; //correct recognitions number
    double timeStart;
    double recogTotalTime;

    //read test set from file
    FILE* facesFile;
    char data[256];

    //open file
    facesFile = fopen(facesFilePath, "r");
    if (!facesFile) {
        fprintf(stderr, "Error in opening test set file: %s", facesFilePath);
        return;
    }

    //allocate space for storing test image
    testImage = (IplImage*)cvAlloc(sizeof(IplImage));
    if (!testImage) {
        fprintf(stderr, "Cannot allocate space for testImage\n");
        return;
    }

    //allocate space for projected image
    projectedTestImage = (float*)cvAlloc( this->eigenVectorsNo_ * sizeof(float) );
    if (!projectedTestImage) {
        fprintf(stderr, "Cannot allocate space for projectedTestImage\n");
        return;
    }
    
    //read file line by line
    while (fscanf(facesFile, "%i %s", &expectedID, data) != EOF) {

        testImage = cvLoadImage(data, CV_LOAD_IMAGE_GRAYSCALE);
        if (!testImage) {
            fprintf(stderr, "Error loading face image: %s", data);
            return;
        }

        timeStart = (double)cvGetTickCount();	// Record the timing.
        bool correctRecog = false;

        //perform test image projection onto PCA subspace
        cvEigenDecomposite(
                testImage,          //IplImage object
                eigenVectorsNo_,    //eigen objects no.
                eigenVectors_,      //eigen vectors array or read callback
                0,                  //ioFlags (for callback)
                0,                  //userData for callback data structure
                averagedImage_,     //averaged image
                projectedTestImage);//eigenvalues

        RecognitionResult recogResult = findClosestFaces(projectedTestImage, 1);
        
        recogTotalTime = (double)cvGetTickCount() - timeStart;
        int closestPersonID = recogResult.personIDArr.front();
        if (closestPersonID == expectedID) {
            correctRecog = true;
            correctRecogNo++;
        }
        testsNo++;

        printf("Test%i: closestPersonID = %d, expected = %d .(%s). confidence = %.3f \
                (recognitionTime = %f) \n", testsNo, closestPersonID, expectedID,
                (correctRecog) ? "correct" : "wrong", recogResult.confidenceArr.front(),
                recogTotalTime/((double)cvGetTickFrequency() * 1000.0));

        //release result space
        recogResult.confidenceArr.clear();
        vector<float>().swap(recogResult.confidenceArr);
        recogResult.personIDArr.clear();
        vector<int>().swap(recogResult.personIDArr);
    }
    printf("Total Accuracy: %d%% out of %d tests.\n", correctRecogNo * 100/testsNo, testsNo);
    
    //free space
    if (testImage != 0)
        cvReleaseImage(&testImage);
    if (projectedTestImage != 0)
        cvFree(&projectedTestImage);
}

int Recognition::performPrincipalComponentAnalysis() {
    printf("Performing Principal Component Analysis ...\n");
    
    //eigens vectors max number is training faces number - 1
    this->eigenVectorsNo_ = trainFacesNo_ - 1;
   
    //allocate eigen vectors
    eigenVectors_ = (IplImage**)cvAlloc(this->eigenVectorsNo_ * sizeof(IplImage*));
    if (!eigenVectors_) {
        fprintf(stderr, "Cannot allocate space for eigenVectors_ !\n");
        return 1;
    }

    //init eigen vectors
    CvSize faceImageSize;   //image size used for cvCreateImage
    faceImageSize.width = faceImages_[0]->width;
    faceImageSize.height = faceImages_[0]->height;
    
    for (int i = 0; i < this->eigenVectorsNo_; i++) {
        //each element stores an eigenface an represent a floating-point image,
        //with data depth = IPL_DEPTH_32F
        eigenVectors_[i] = cvCreateImage( faceImageSize, IPL_DEPTH_32F, 1);
        if (!eigenVectors_[i]) {
            fprintf(stderr, "Cannot allocate space for eigenVectors_ !\n");
            return 1;
        }
    }

    //allocate eigenvalues array (cvMat) - one row, eigenVectorsNo_ cols
    //eigenValues are floating point numbers, with one channel (CV_32FC1)
    eigenValues_ = cvCreateMat(1, this->eigenVectorsNo_, CV_32FC1);
    if (!eigenValues_) {
        fprintf(stderr, "Cannot create eigenValues_ matrix !\n");
        return 1;
    }

    //allocate the floating-point averaged image of training faces
    averagedImage_ = cvCreateImage(faceImageSize, IPL_DEPTH_32F, 1);
    if (!averagedImage_) {
        fprintf(stderr, "Cannot allocate space for averageImage_ !\n");
        return 1;
    }

    //set PCA termination criteria for cvCalcEigenObjects
    //computation stops when iteration number reaches this->eigenVectorsNo_
    CvTermCriteria termCriteria = cvTermCriteria(CV_TERMCRIT_ITER, this->eigenVectorsNo_, 1);

    //perform PCA - compute average image, center the database (substract average image
    //from each training face), compute eigenVectors and eigenValues
    //TODO: For large faces database, use READ/WRITE callbacks
    cvCalcEigenObjects( 
            this->trainFacesNo_,    //objects number
            (void*)faceImages_,     //input (can be replaced with READ Callback function
            (void*)eigenVectors_,   //output (can be replaced with WRITE Callback function
            CV_EIGOBJ_NO_CALLBACK,  //replace this if you want READ/WRITE CALLBACK functions
            0,                      //ioBufSize (0-unknown)
            0,                      //userData - pointer to structure for callback functions
            &termCriteria,          //termination criteria
            averagedImage_,         //averaged object
            eigenValues_->data.fl); //pointer to eigenValues array


    //normalize eigenvalues matrix - already done
    /*cvNormalize(
            eigenValues_,   //source
            eigenValues_,   //destination
            1,              //max output value
            0,              //min output value
            CV_L1,          //normalization type
            0);             //operation mask
     */

    printf("PCA done.\n");
    return 0;
}

int Recognition::projectTrainingImages() {
    printf("Projecting training face images onto PCA subspace ...\n");

    //allocate space for projected images
    projectedTrainFaces_ = cvCreateMat(trainFacesNo_, eigenVectorsNo_, CV_32FC1 );
    if (!projectedTrainFaces_) {
        fprintf(stderr, "Cannot allocate space for projectedTrainFaces_ \n");
        return 1;
    }
    
    int offset = projectedTrainFaces_->step / sizeof(float);
    for (int i = 0; i < trainFacesNo_; i++) {
        //perform projection
        cvEigenDecomposite(
                faceImages_[i],     //IplImage object
                eigenVectorsNo_,    //eigen objects no.
                eigenVectors_,      //eigen vectors array or read callback
                0,                  //ioFlags (for callback)
                0,                  //userData for callback data structure
                averagedImage_,     //averaged image
                projectedTrainFaces_->data.fl + i * offset);  //eigenvalues
    }

    printf("Projection done.\n");

    return 0;
}

//struct used for finding best resultsNo faces in findClosesFaces function
struct Candidate {
    int faceImageIndex;
    double distance;    //distance from test face image and this face
};

bool comparator (const Candidate &i, const Candidate &j) {
    return (i.distance < j.distance);
}

RecognitionResult Recognition::findClosestFaces(float* projectedTestFace, int resultsNo) {
    //printf("Finding closest face images ...\n");

    //vector with face candidates
    vector<Candidate> candidatesArray;
    candidatesArray.reserve(this->trainFacesNo_);

    for (int i = 0; i < this->trainFacesNo_; i++) {
        double crtDistance = 0;
        
        if (this->distanceType_ == EUCLIDIAN_DISTANCE) {
            //compute euclidian distance
            for (int j = 0; j < this->eigenVectorsNo_; j++) {
                float diff = projectedTestFace[j] -
                    projectedTrainFaces_->data.fl[i * this->eigenVectorsNo_ + j];
                crtDistance += diff * diff;
            }

        } else if (this->distanceType_ == MAHALANOBIS_DISTANCE) {
            //compute Mahalanobis distance
            for (int j = 0; j < this->eigenVectorsNo_; j++) {
                float diff = projectedTestFace[j] -
                    projectedTrainFaces_->data.fl[i * this->eigenVectorsNo_ + j];
                crtDistance += diff * diff / eigenValues_->data.fl[j];
            }
        }

        Candidate crtCandidate;// = new Candidate (i, crtDistance);
        crtCandidate.faceImageIndex = i;
        crtCandidate.distance = crtDistance;

        //add to vector
        candidatesArray.push_back(crtCandidate);
    }

    //sort candidates vector by distance in ascending order
    sort(candidatesArray.begin(), candidatesArray.end(), comparator);

    //keep best resultsNo candidates
    RecognitionResult result;
    //set used for selecting best resultsNo matching persons
    set<int> bestPersonsSet;

    for (vector<Candidate>::iterator it = candidatesArray.begin();
            it < candidatesArray.end(); ++it) {
        
        //compute recognition confidence bsased on the Euclidean distance,
        //similar images should give a confidence between 0.5 to 1.0,
        //and very different images should give a confidence between 0.0 to 0.5.
        float confidence = 1.0f - sqrt(it->distance /
                    (float)(trainFacesNo_ * eigenVectorsNo_)) / 255.0f;

        //if confidence is under the threshold -> classify as unknown person
        if (confidence < recogThreshold_) {
            //unknown person
            result.confidenceArr.push_back(confidence);
            result.personIDArr.push_back(0);
            break;
        }

        int personID = this->personRealIDs_->data.i[it->faceImageIndex];
        
        //add personId to set
        if (bestPersonsSet.find(personID) == bestPersonsSet.end()) {
            //if it isn't already there , add it
            bestPersonsSet.insert(personID);

            //add to result
            result.confidenceArr.push_back(confidence);
            result.personIDArr.push_back(personID);

            //printf("personID %i confidence %f index %i\n",personID, confidence, it->faceImageIndex);
            
            if (bestPersonsSet.size() == resultsNo)
                break;
        }
    }

    //free candidates vector
    candidatesArray.clear();
    vector<Candidate>().swap(candidatesArray);

    //free bestPersonsSet
    bestPersonsSet.clear();
    set<int>().swap(bestPersonsSet);
    
    return result;
}

/*int Recognition::findClosestFaces(float* projectedTestFace) {
    //printf("Finding closest face images ...\n");

    double minDistance = DBL_MAX;
    int closestTrainFaceIndex = 0;

    for (int i = 0; i < this->trainFacesNo_; i++) {
        double crtDistance = 0;

        if (this->distanceType_ == EUCLIDIAN_DISTANCE) {
            //compute euclidian distance
            for (int j = 0; j < this->eigenVectorsNo_; j++) {
                float diff = projectedTestFace[j] -
                    projectedTrainFaces_->data.fl[i * this->eigenVectorsNo_ + j];
                crtDistance += diff * diff;
            }

        } else if (this->distanceType_ == MAHALANOBIS_DISTANCE) {
            //compute Mahalanobis distance
            for (int j = 0; j < this->eigenVectorsNo_; j++) {
                float diff = projectedTestFace[j] -
                    projectedTrainFaces_->data.fl[i * this->eigenVectorsNo_ + j];
                crtDistance += diff * diff / eigenValues_->data.fl[j];
            }
        }

        if (crtDistance < minDistance) {
            //update min distance
            minDistance = crtDistance;
            closestTrainFaceIndex = i;
        }
    }

    //compute recognition confidence bsased on the Euclidean distance,
    //similar images should give a confidence between 0.5 to 1.0,
    //and very different images should give a confidence between 0.0 to 0.5.
    *recognitionConfidence = 1.0f -
            sqrt(minDistance / (float)(trainFacesNo_ * eigenVectorsNo_)) / 255.0f;
    //*recognitionConfidence = minDistance;

    //printf("Closest training face image is %i with confidence %f and distance %f.\n",
    //    closestTrainFaceIndex, *recognitionConfidence, minDistance);

    //return closest train face image index
    return closestTrainFaceIndex;
}*/

IplImage* Recognition::convertFloatImageToUcharImage(const IplImage *srcImg) {
    IplImage *dstImg = 0;

    if ((srcImg) && (srcImg->width > 0 && srcImg->height > 0)) {
        //spread the 32bit floating point pixels to fit within 8bit pixel range.
        double minVal, maxVal;
        cvMinMaxLoc(srcImg, &minVal, &maxVal);

        //deal with NaN and extreme values
        if (cvIsNaN(minVal) || minVal < -1e30)
                minVal = -1e30;
        if (cvIsNaN(maxVal) || maxVal > 1e30)
                maxVal = 1e30;
        if (maxVal-minVal == 0.0f)
                maxVal = minVal + 0.001; //remove potential divide by zero errors.

        // Convert the format
        dstImg = cvCreateImage(cvSize(srcImg->width, srcImg->height), 8, 1);
        cvConvertScale(srcImg, dstImg, 255.0 / (maxVal - minVal), - minVal * 255.0 / (maxVal-minVal));
    }
    return dstImg;
}


void Recognition::saveEigenFaces() {
    int columns = 8;    //number of eigenfaces on a row
    
    printf("Saving the %d eigenvector images as %s.\n", this->eigenVectorsNo_,
            EIGENFACES_IMAGE_FILE);

    if (this->eigenVectorsNo_ > 0) {
        //creates a large image with 'columns' images on a row made of eigenface images.
        //eigenface image is converted to a normal 8-bit UCHAR image instead of a 32-bit float image.
        int colsNo = min(this->eigenVectorsNo_, columns);
        int rowsNo = 1 + (this->eigenVectorsNo_ / columns);
        int w = this->eigenVectors_[0]->width;
        int h = this->eigenVectors_[0]->height;
        CvSize size = cvSize(colsNo * w, rowsNo * h);

        // 8-bit Greyscale UCHAR image
        IplImage *bigImg = cvCreateImage(size, IPL_DEPTH_8U, 1);

        for (int i = 0; i < this->eigenVectorsNo_; i++) {
            //get the eigenface image.
            IplImage *byteImg = convertFloatImageToUcharImage(this->eigenVectors_[i]);
            //paste it into the correct position.
            int x = w * (i % colsNo);
            int y = h * (i / colsNo);
            CvRect ROI = cvRect(x, y, w, h);
            cvSetImageROI(bigImg, ROI);
            cvCopyImage(byteImg, bigImg);
            cvResetImageROI(bigImg);
            cvReleaseImage(&byteImg);
        }
        cvSaveImage(EIGENFACES_IMAGE_FILE, bigImg);
        cvReleaseImage(&bigImg);
    }

    //store the average image to a file
    printf("Saving the image of the average face as %s.\n",AVERAGE_IMAGE_FILE);
    cvSaveImage(AVERAGE_IMAGE_FILE, this->averagedImage_);
}
