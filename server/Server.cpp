/*
 * File:   Server.cpp
 * Author: Octavian Sima 
 *
 * Server module implementation for mobile phone application
 */

#include <string>

#include "Server.h"

Server::Server(int listenPort, bool reverseOrder) {
    this->listenPort_ = listenPort;
    this->reverseOrder_ = reverseOrder;
    this->isRunning_ = false;

    if (DEBUG_ON) {
        printf("[Server] Initialize detection module.\n");
    }
    // initialize detection module
    this->detection = new Detection();
    detection->initialize((vector<char*>)NULL);

    if (DEBUG_ON) {
        printf("[Server] Initialize recognition module.\n");
    }
    // initialize recognition module
    this->recognition = new Recognition();
    recognition->initialize(RECOGNITION_THRESHOLD,
            USE_MAHALANOBIS_DISTANCE ? MAHALANOBIS_DISTANCE : EUCLIDIAN_DISTANCE,
            (char*)IMAGE_FILE_EXTENSION, (char*)TRAINING_DATA_FILE);

    if (DEBUG_ON) {
        printf("[Server] Training recognition module.\n");
    }
    // train using current database images
    int res = recognition->train(USE_DATABASE_FOR_TRAINING ? DATABASE_SRC : FILE_SRC,
            (char*)TRAINING_SRC_PATH, SAVE_TRAINING_DATA);
    if (res != 0) {
            delete recognition;
            delete detection;
            //delete database;
            exit(0);
    }

    // save training result
    recognition->saveTrainingData((char*)TRAINING_DATA_FILE);

    if (DEBUG_ON) {
        printf("[Server] Initialize Sql database module.\n");
    }
    // initialize databse module
    this->database = new Database();
    database->connect();

    if (DEBUG_ON) {
        printf("[Server] Initialize listen socket\n");
    }

    //create listen socket
    this->listenSocketFD_ = socket(AF_INET, SOCK_STREAM, 0);
    if (this->listenSocketFD_ < 0) {
        fprintf(stderr, "Could not open listen socket\n");
        return;
    }

    struct sockaddr_in	serverAddress;  //server address information
    //set serverAddress to zero
    memset((char *) &serverAddress, 0, sizeof(serverAddress));
    //configure address
    serverAddress.sin_family = AF_INET;
    //use machine IP
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(listenPort);
    
    if (bind(listenSocketFD_, (struct sockaddr *) &serverAddress, sizeof(struct sockaddr)) < 0) {
        fprintf(stderr, "Could not bind \n");
        return;
    }

    //freeaddrinfo(&serverAddress);

    if (listen(this->listenSocketFD_, MAX_PENDING_CLIENTS) < 0) {
        fprintf(stderr, "Error calling listen \n");
        return;
    }

    //initialize thread stuff
    pthread_mutex_init(&this->mutex_, NULL);
    pthread_attr_init(&this->threadAttr_);
    pthread_attr_setdetachstate(&this->threadAttr_, PTHREAD_CREATE_DETACHED);

}

Server::~Server() {
    this->isRunning_ = false;
    close(this->listenSocketFD_);
    
    pthread_attr_destroy(&this->threadAttr_);
    pthread_mutex_destroy(&this->mutex_);
    // release modules
    delete detection;
    delete recognition;
    delete database;
}

void Server::run() {
    struct sockaddr_in	clientAddress;  //client address information
    socklen_t sockInSize = (socklen_t)sizeof(struct sockaddr_in);
    int clientSocketFD;
    
    this->isRunning_ = true;

    if (DEBUG_ON) {
        printf("Server started.\n");
    }

    while (this->isRunning_) {
        if ((clientSocketFD = accept(this->listenSocketFD_,
                (struct sockaddr*)&clientAddress, &sockInSize)) == -1)  {
            fprintf(stderr, "Error accepting client.\n");
            
        } else {

            if (DEBUG_ON) {
                printf("[sockFd %i] New connection from %s\n", clientSocketFD,
                        inet_ntoa(clientAddress.sin_addr));
            }

            //create new thread to deal with client
            pthread_t workingThread;
            //if (pthread_create(&workingThread, &threadAttr_, serveClient, (void*)clientSocketFD) != 0) {
            if (pthread_create(&workingThread, &threadAttr_, &Server::threadWork,
                new threadArgs(this,(void*)clientSocketFD)) != 0) {
                
                fprintf(stderr, "[sockFd %i] Error creating thread \n",clientSocketFD);
                return;
            }
        }
    }

    if (DEBUG_ON) {
        printf("Server stopped.\n");
    }
}

void* Server::threadWork(void* args) {
    threadArgs* tArgs = static_cast<threadArgs*>(args);
    Server* This = tArgs->This;
    void* actualArgs = tArgs->actualArgs;
    void* result = This->serveClient(actualArgs);
    delete tArgs;
    return result;
}

void* Server::serveClient(void* clientSocketFD) {
    int clientSockFd = (int)clientSocketFD;
    int candidatesNo;
    int threadID = pthread_self();

    //receive number of candidates for detected face in image
    char tempBuff[1];
    int totalBytes = 0;
    int crtRecvBytes = 0;

    if (DEBUG_ON) {
        printf("[sockFd %i] Receiving the number of candidates for "
                "detected face/faces in image...\n", clientSockFd);
    }

    while (totalBytes < 1 && crtRecvBytes != -1) {
        crtRecvBytes = recv(clientSockFd, tempBuff, 1, 0);
        totalBytes += crtRecvBytes;
    }
    //error on reading from socket
    if (crtRecvBytes == -1) {
        fprintf(stderr, "[sockFd %i] Error on reading from socket", clientSockFd);
        close(clientSockFd);
        pthread_exit(NULL);
    }

    candidatesNo = tempBuff[0];

    if (DEBUG_ON) {
        printf("[sockFd %i] Candidates number received (%i)\n",clientSockFd, candidatesNo);
    }

    char imageFilePath[50];
    sprintf(imageFilePath, "input%i.jpg", threadID);
    if (receiveImage(clientSockFd, imageFilePath) != 0) {
        fprintf(stderr, "[sockFd %i] Error receiving image %s.\n", clientSockFd, imageFilePath);
        close(clientSockFd);
        pthread_exit(NULL);
    }

    vector<IplImage*> faces;
    vector<CvRect> objects;

    IplImage* image = cvLoadImage(imageFilePath, CV_LOAD_IMAGE_COLOR);
    if (!image) {
        fprintf(stderr, "[sockFd %i] Error loading image %s.\n", clientSockFd, imageFilePath);
        close(clientSockFd);
        pthread_exit(NULL);
    }
    
    //image processing (detection, recognition, extract info from database)
    
    pthread_mutex_lock(&mutex_);
    // detect objects from image
    objects = detection->detectObjects(image, DETECT_ALL_FACES);
    pthread_mutex_unlock(&mutex_);
    
    //send detected faces no
    int detectedFacesNo = objects.size();
    printf("Sending detected faces number\n");
    if (sendInt(clientSockFd, &detectedFacesNo) != 0) {
        fprintf(stderr, "[sockFd %i] Error sending detected faces number.\n", clientSockFd);
        close(clientSockFd);
        pthread_exit(NULL);
    }
    
    if (objects.size() > 0) {
        pthread_mutex_lock(&mutex_);
        // detect faces in captured image
        faces = detection->getDetectedObjects(image, objects);
        //free objects vector
        objects.clear();
        (vector<CvRect>()).swap(objects);
        
        //recognize
        vector<RecognitionResult> results = recognition->recognizeFaces(faces, candidatesNo);

        pthread_mutex_unlock(&mutex_);
        
        for (int i = 0; i < results.size(); i++) {
            // write detected image
            char filename[50];
            sprintf(filename, "detect%i.jpg", threadID);

            pthread_mutex_lock(&mutex_);
            IplImage* detectedResized = detection->resizeImage(faces.at(i), WIDTH, HEIGHT);
            cvSaveImage(filename, detectedResized);
            cvReleaseImage(&detectedResized);
            pthread_mutex_unlock(&mutex_);

            //send detected face
            if (DEBUG_ON)
                printf("[sockFd %i] Sending detected image...\n", clientSockFd);

            if (sendImage(clientSockFd, filename) != 0) {
                fprintf(stderr, "[sockFd %i] Error sending detected image %s.\n", clientSockFd, filename);
                close(clientSockFd);
                pthread_exit(NULL);
            }

            //remove detected image
            remove(filename);
            
            //send candidates no for current detected face

            int candidatesNoForCrt = results[i].personIDArr.size();
            if (results[i].personIDArr[0] == 0)
                candidatesNoForCrt = 0;

            printf("Sending candidates number for face %i\n", i+1);
            if (sendInt(clientSockFd, &candidatesNoForCrt) != 0) {
                fprintf(stderr, "[sockFd %i] Error sending candidates number for a face.\n", clientSockFd);
                close(clientSockFd);
                pthread_exit(NULL);
            }

            if (candidatesNoForCrt == 0)
                continue;

            for (int j = 0; j < results[i].personIDArr.size(); j++) {
                //get person id
                int personId = results[i].personIDArr[j];
                char* result = (char*)malloc(200*sizeof(char));

                pthread_mutex_lock(&mutex_);
                //get person name
                string personInfo = database->getAllInfo(personId);
                pthread_mutex_unlock(&mutex_);

                //sprintf(result, "%s#%f",personInfo.c_str(), results[i].confidenceArr[j]);
		
		//convert confidence value in percent
		float percent = 100 * (results[i].confidenceArr[j] - RECOGNITION_THRESHOLD) / (1.0 - RECOGNITION_THRESHOLD);
                sprintf(result, "%s#%.2f%s",personInfo.c_str(), percent, "%" );
                
		// get person photo
                string path = TRAINING_SRC_PATH;
                char folder[10];
                sprintf(folder,"/s%d/",personId);
                path.append(folder).append(IMAGE_NAME);

                IplImage* personImage = cvLoadImage(path.c_str(), CV_LOAD_IMAGE_COLOR);
                if ( !personImage ) {
                    fprintf(stderr, "Error loading image!\n");
                    close(clientSockFd);
                    pthread_exit(NULL);
                }
                sprintf(filename, "person%i-%d-%d.jpg", threadID, i,j);

                pthread_mutex_lock(&mutex_);
                detectedResized = detection->resizeImage(personImage, WIDTH, HEIGHT);
                cvSaveImage(filename, detectedResized);
                pthread_mutex_unlock(&mutex_);
                cvReleaseImage(&personImage);

                //send person candidate photo
                if (DEBUG_ON)
                    printf("[sockFd %i] Sending candidate image...\n", clientSockFd);

                if (sendImage(clientSockFd, filename) != 0) {
                    fprintf(stderr, "[sockFd %i] Error sending image %s.\n", clientSockFd, filename);
                    close(clientSockFd);
                    pthread_exit(NULL);
                }
                if (DEBUG_ON)
                    printf("[sockFd %i] Sending candidate info length...\n", clientSockFd);

                //remove candidate phote
                remove(filename);

                int infoLen = strlen(result) +1;
                if (sendInt(clientSockFd, &infoLen) != 0) {
                    fprintf(stderr, "[sockFd %i] Error sending candidate info length.\n", clientSockFd);
                    close(clientSockFd);
                    pthread_exit(NULL);
                }

                if (DEBUG_ON)
                    printf("[sockFd %i] Sending candidate info...\n", clientSockFd);

                if (sendBytes(clientSockFd, result, infoLen) != 0) {
                    fprintf(stderr, "[sockFd %i] Error sending candidate info\n", clientSockFd);
                    close(clientSockFd);
                    pthread_exit(NULL);
                }

                free(result);
            }
        }
        //release faces vector space
        for (vector<IplImage*>::iterator it = faces.begin(); it < faces.end(); ++it) {
            cvReleaseImage(&(*it));
        }
        faces.clear();
        (vector<IplImage*>()).swap(faces);
    } 

    //remove input file
    remove(imageFilePath);
}

int Server::sendImage(int clientSocketFD, char* imageFilePath) {
    int imageSize;
    FILE* imageFile = fopen(imageFilePath,"r");
    if (!imageFile) {
        fprintf(stderr, "Could not open file %s!\n", imageFilePath);
        return 1;
    }

    //get image file size
    fseek(imageFile, 0, SEEK_END);
    imageSize = (int)ftell(imageFile);
    rewind(imageFile);

    if (DEBUG_ON) {
        printf("[sockFd %i] Sending image %s...\n", clientSocketFD, imageFilePath);
    }

    //send image total size
    if (sendInt(clientSocketFD, &imageSize) != 0) {
        return 1;
    }

    char* byteBuff = (char*)malloc(PACKAGE_MAX_SIZE);
    //send image as byte array
    while (imageSize > PACKAGE_MAX_SIZE) {
        fread(byteBuff, 1, PACKAGE_MAX_SIZE, imageFile);
        if (sendBytes(clientSocketFD, byteBuff, PACKAGE_MAX_SIZE) != 0) {
            free(byteBuff);
            return 1;
        }
        imageSize -= PACKAGE_MAX_SIZE;
    }
    fread(byteBuff, 1, imageSize, imageFile);
    if (sendBytes(clientSocketFD, byteBuff, imageSize) != 0) {
        free(byteBuff);
        return 1;
    }

    if (DEBUG_ON) {
        printf("[sockFd %i] Image %s sent.\n", clientSocketFD, imageFilePath);
    }
    free(byteBuff);
    fclose(imageFile);
    
    return 0;
}

int Server::receiveImage(int clientSocketFD, char* imageFilePath) {
    int imageSize;
    FILE* imageFile = fopen(imageFilePath,"w");
    if (!imageFile) {
        fprintf(stderr, "Could not open file %s!\n", imageFilePath);
        return 1;
    }

    if (DEBUG_ON) {
        printf("[sockFd %i] Receiving image %s...\n", clientSocketFD, imageFilePath);
    }

    //receive image total size
    if (receiveInt(clientSocketFD, &imageSize) != 0) {
        return 1;
    }

    char* byteBuff = (char*)malloc(PACKAGE_MAX_SIZE);
    //receive image as byte array
    while (imageSize > PACKAGE_MAX_SIZE) {
        if (receiveBytes(clientSocketFD, byteBuff, PACKAGE_MAX_SIZE) != PACKAGE_MAX_SIZE) {
            free(byteBuff);
            return 1;
        }
        fwrite(byteBuff, 1, PACKAGE_MAX_SIZE, imageFile);
        imageSize -= PACKAGE_MAX_SIZE;
    }
    if (receiveBytes(clientSocketFD, byteBuff, imageSize) != imageSize) {
        free(byteBuff);
        return 1;
    }
    fwrite(byteBuff, 1, imageSize, imageFile);

    if (DEBUG_ON) {
        printf("[sockFd %i] Image %s received.\n", clientSocketFD, imageFilePath);
    }
    
    free(byteBuff);
    fclose(imageFile);
    return 0;
}

int Server::sendACK(int clientSocketFD) {
    char tempBuff[1];
    tempBuff[0] = 35; // '#' character
    
    if (DEBUG_ON) {
	printf("[sockFd %i] Sending ACK...\n", clientSocketFD);
    }

    if (send(clientSocketFD, tempBuff, 1, 0) == -1) {
        fprintf(stderr,"[sockFd %i] Error sending ACK.\n", clientSocketFD);
        return 1;
    }

    if (DEBUG_ON) {
        printf("[sockFd %i] ACK sent.\n", clientSocketFD);
    }

    return 0;
}

int Server::receiveACK(int clientSocketFD) {
    char tempBuff[1];
    int totalBytes = 0;
    int crtRecvBytes = 0;

    if (DEBUG_ON) {
	printf("[sockFd %i] Receiving ACK...\n", clientSocketFD);
    }

    while (totalBytes < 1 && crtRecvBytes != -1) {
        crtRecvBytes = recv(clientSocketFD, tempBuff, 1, 0);
        totalBytes += crtRecvBytes;
    }

    if (crtRecvBytes == -1) {
        fprintf(stderr,"[sockFd %i] Error receiving ACK.\n", clientSocketFD);
        return 1;
    }

    if (DEBUG_ON) {
        printf("[sockFd %i] ACK received.\n", clientSocketFD);
    }

    return 0;
}

int Server::sendInt(int clientSocketFD, int* packageSize) {

    char* buff = (char*) packageSize;

    if (DEBUG_ON) {
        printf("[sockFd %i] Sending int value (%i)...\n", clientSocketFD, *packageSize);
    }

    if (this->reverseOrder_) {
        //reverse bytes
        char* revBuff = (char*)malloc(sizeof(int));
        for (int i = 0; i < sizeof(int); i++)
            revBuff[i] = buff[sizeof(int) - i - 1];

        if (send(clientSocketFD, revBuff, sizeof(int), 0) == -1) {
            fprintf(stderr, "[sockFd %i] Error sending int value. \n",clientSocketFD);
            return 1;
        }

        free(revBuff);

    } else {
        if (send(clientSocketFD, buff, sizeof(int), 0) == -1) {
            fprintf(stderr, "[sockFd %i] Error sending int value. \n",clientSocketFD);
            return 1;
        }
    }

    if (DEBUG_ON) {
        printf("[sockFd %i] Int (%i) sent.\n", clientSocketFD, *packageSize);
    }

    if (USE_ACK) {
        if (receiveACK(clientSocketFD) != 0)
            return 1;
        if (sendACK(clientSocketFD) != 0)
            return 1;
    }
    
    return 0;
}

int Server::receiveInt(int clientSocketFD, int* packageSize) {
    char* buff = (char*) packageSize;
    int totalBytes;

    if (DEBUG_ON) {
        printf("[sockFd %i] Receiving int value...\n", clientSocketFD);
    }

    if (this->reverseOrder_) {
        char* revBuff = (char*)malloc(sizeof(int));

        if ((totalBytes=recv(clientSocketFD, revBuff, sizeof(int), MSG_WAITALL)) == -1
                || totalBytes != sizeof(int)) {
            fprintf(stderr, "[sockFd %i] Error receiving int value. \n",clientSocketFD);
            return 1;
        }
        //reverse bytes
        for (int i = 0; i < sizeof(int); i++)
            buff[i] = revBuff[sizeof(int) - i - 1];
        free(revBuff);

    } else if ((totalBytes = recv(clientSocketFD, buff, sizeof(int), MSG_WAITALL)) == -1
            || totalBytes != sizeof(int)) {
        fprintf(stderr, "[sockFd %i] Error receiving int value.\n", clientSocketFD);
        return 1;
    }

    if (DEBUG_ON) {
        printf("[sockFd %i] Int (%i) received.\n", clientSocketFD, *packageSize);
    }

    if (USE_ACK) {
        if (sendACK(clientSocketFD) != 0)
            return 1;
        if (receiveACK(clientSocketFD) != 0)
            return 1;
    }
    
    return 0;
}

int Server::sendBytes(int clientSocketFD, char* byteBuf, int bufLen) {
    
    if (DEBUG_ON) {
        printf("[sockFd %i] Sending %i byte package\n", clientSocketFD, bufLen);
    }

    if (send(clientSocketFD, byteBuf, bufLen, 0) == -1) {
        fprintf(stderr, "[sockFd %i] Error sending bytes \n",clientSocketFD);
        return 1;
    }

    if (DEBUG_ON) {
        printf("[sockFd %i] %i byte package sent.\n", clientSocketFD, bufLen);
    }


    if (USE_ACK) {
        if (receiveACK(clientSocketFD) != 0)
            return 1;
        if (sendACK(clientSocketFD) != 0)
            return 1;
    }
    
    return 0;
}

int Server::receiveBytes(int clientSocketFD, char* byteBuf, int bufLen) {
    int totalBytes = 0;
    int crtRecvBytes = 0;

    if (DEBUG_ON) {
	printf("[sockFd %i] Receiving max %i byte package...\n", clientSocketFD, bufLen);
    }

   /* while (totalBytes < bufLen) {
        if ((crtRecvBytes = recv(clientSocketFD, byteBuf, bufLen, 0)) == -1) {
            fprintf(stderr, "[sockFd %i] Error sending bytes \n", clientSocketFD);
            return -1;
        }
        totalBytes += crtRecvBytes;
    }*/
    if ((totalBytes = recv(clientSocketFD, byteBuf, bufLen, MSG_WAITALL)) == -1
            || totalBytes != bufLen) {
        fprintf(stderr, "[sockFd %i] Error sending bytes \n", clientSocketFD);
        return -1;
    }

    if (DEBUG_ON) {
        printf("[sockFd %i] Received %i byte package.\n", clientSocketFD, totalBytes);
    }

    if (USE_ACK) {
        if (sendACK(clientSocketFD) != 0)
            return -1;
        if (receiveACK(clientSocketFD) != 0)
            return -1;
    }
    return totalBytes;
}



