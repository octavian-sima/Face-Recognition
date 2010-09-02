/* 
 * File:   Database.h
 * Author: Elena Holobiuc
 */

#ifndef _DATABASE_H
#define	_DATABASE_H

#include <mysql.h>

#define HOST            "localhost"
#define USER            "root"
#define PASSWORD        "password"
#define DATABASE_NAME   "Person"

#define FIRSTNAME   0
#define LASTNAME    1
#define BIRTHDATE   2
#define OCCUPATION  3
#define PHONE_NR    4

using namespace std;

class Database {

    // Construction & Destruction
public:
    Database();

    ~Database();

public:

    /* 
     * Method that connects to mysql database
     */
    void connect();
    /*
     * Method that disconnects from mysql database
     */
    void disconnect();
    /*
     * Method that returns the id of a person from database or -1 if
     * the person does not exist
     */
    int getPersonId(char* fname, char* lname);
    /*
     * Method that return all person information from database
     */
    string getAllInfo(int personId);
    /*
     * Method that returns a certain information from a database
     * using a stored function 
     */
    string getInfo(int personId, int infoType);
    /*
     * Method that inserts a new person into the database
     * using a stored procedure
     */
    void insertNewPerson(int id, char* fname, char* lname,
                        char* bdate, char* occupation, char* phoneNr);


private:

    // current mysql connection
    MYSQL *connection_;
    MYSQL mysql_;

};


#endif	/* _DATABASE_H */

