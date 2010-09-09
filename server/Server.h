/* 
 * File:   Server.h
 * Author: Octavian Sima 
 *
 * Server module for face recognition mobile phone application
 * This server can handle multiple clients starting a new thread when a client
 * connects. The Thread deals with image file transfer and detection +
 * recognition stuff.
 */

#ifndef _SERVER_H
#define	_SERVER_H

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>

#include "../recognition/Recognition.h"
#include "../detection/Detection.h"
#include "../sql_database/Database.h"

//if true, display debug messages
#define DEBUG_ON    true

//if true, send/receive ACK after each message
#define USE_ACK     true

//for server listening socket
#define MAX_PENDING_CLIENTS 10

//maximum size for a data package
#define PACKAGE_MAX_SIZE    10000

//recognition settings
#define WIDTH 80
#define HEIGHT 70

#define IMAGE_NAME		    "1.pgm"

//if false, use a file that contains a list of paths to face images
#define USE_DATABASE_FOR_TRAINING   true

//path to training database or to a file with list of images
#define TRAINING_SRC_PATH           "../person_database"

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


class Server {

// Construction & Destruction
public:
    Server(int listenPort, bool reverseOrder);

   ~Server();

   /*
    * Runs the server - listen for clients and start a new thread to deal with it
    */
   void run();

private:

    /*
     * Thread job - serve a client (receive image, detect faces, recognize faces,
     * send back the results
     */
    void* serveClient(void* clientSocketFD);

    /*
     * Sends image read from imageFilePath to clientSocketFD
     * Returns 0 - success, 1 - error
     */
    int sendImage(int clientSocketFD, char* imageFilePath);

    /*
     * Receives image as byteBuffer from clientSocketFD and writes it to
     * imageFilePath.
     * Returns 0 - success, 1 - error
     */
    int receiveImage(int clientSocketFD, char* imageFilePath);
    
    /*
     * Sends ACK after a message using the given socket
     * Returns 0 if ACK sent OK and 1 if not
     */
    int sendACK(int clientSocketFD);

    /*
     * Receives ACK from the given socket
     * Returns 0 if ACK received OK or 1 if not
     */
    int receiveACK(int clientSocketFD);

    /*
     * Sends an integer using the given client socket
     * If reverseOrder_ is true, reverse byte order for integer value
     * Returns 0 if integer was successfully sent and 1 otherwise
     */
    int sendInt(int clientSocketFD, int* packageSize);

    /*
     * Receives an integer from the given client socket
     * If reverseOrder_ is true, reverse byte order for integer value
     * Returns 0 if integer was successfully received and 1 otherwise
     */
    int receiveInt(int clientSocketFD, int* packageSize);

    /*
     * Sends a byte buffer to a given client socket
     * Returns 0 if bytes were sent OK and 1 if not
     */
    int sendBytes(int clientSocketFD, char* byteBuf, int bufLen);

    /*
     * Receives a byte buffer from a given client socket
     * Returns the number of bytes received
     */
    int receiveBytes(int clientSocketFD, char* byteBuf, int bufLen);

    /*
     * Stuff used for creating pthread from C++
     */
    static void* threadWork(void* args);

    struct threadArgs {
        Server* This;
        void* actualArgs;
        threadArgs(Server* s, void* p) : This(s), actualArgs(p) {
        }
    };

private:
    Detection* detection;       //detection module
    Recognition* recognition;   //recognition module
    Database* database;         //database module
    int listenPort_;            //the port on which server listens for clients
    int listenSocketFD_;        //server's listen socket file description
    bool isRunning_;            //is server still running ?
    bool reverseOrder_;         //reverse byte order for integers
    pthread_mutex_t mutex_;     //used to protect crical areas
    pthread_attr_t threadAttr_; //thread attribute
    
};

#endif	/* _SERVER_H */
