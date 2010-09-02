/*
 * File:   Database.cpp
 * Author: Elena Holobiuc
 */

#include <stdio.h>
#include <string.h>
#include <iostream>
#include <mysql/mysql.h>
#include <stdlib.h>

#include "Database.h"

Database::Database() {
    
}

Database::~Database() {

    // close current connection ( if any )
    if (this->connection_) {
        mysql_close(this->connection_);
    }
}

void Database::connect() {

    // connect to mysql Database
    mysql_init(&this->mysql_);
    
    this->connection_ = mysql_real_connect(&this->mysql_,
                                            HOST,
                                            USER,
                                            PASSWORD,
                                            DATABASE_NAME,
                                            0,
                                            NULL,
                                            0
                                          );
    
    // test successful connection
    if ( this->connection_ == NULL ) {
        fprintf(stderr, "[Database] Error while connecting to mysql database!\n");
        exit(0);
    }

}

void Database::disconnect() {

    // close current connection
    mysql_close(this->connection_);
}

int Database::getPersonId(char* fname, char* lname) {

    // mysql query result
    MYSQL_RES *result;
    MYSQL_ROW row;
    char* queryString = (char*)malloc(100*sizeof(char));
    int queryErrno;
    int id = -1;

    // prepare query
    sprintf(queryString,"CALL GetPersonID('%s','%s',@idToString)",fname,lname);

     // run query
    queryErrno = mysql_query(this->connection_, queryString);
    // check for query error
    if (queryErrno !=0) {
        fprintf(stderr, "[Database] Error calling mysql function: %s\n",
                            mysql_error(this->connection_));
        return id;
    }

    // run query
    queryErrno = mysql_query(this->connection_, "SELECT @idToString");
    // check for query error
    if (queryErrno !=0) {
        fprintf(stderr, "[Database] Error calling mysql function: %s\n",
                            mysql_error(this->connection_));
        return id;
    }

    // clear query string
    free(queryString);

    // store mysql query result
    result = mysql_store_result(this->connection_);

    // get returned row
    if ( (row = mysql_fetch_row(result)) == NULL ) {
        fprintf(stderr, "[Database] No result returned by mysql query");
        mysql_free_result(result);
        return id;
    }

    // free mysql result
    mysql_free_result(result);

    if (row[0] == NULL) {
        return id;
    }

    id = atoi(row[0]);

    return id;
    
}

string Database::getAllInfo(int personId) {

    // mysql query result
    MYSQL_RES *result;
    MYSQL_ROW row;
    char* queryString = (char*)malloc(100*sizeof(char));
    int queryErrno;
    string info = "";
    // prepare query
    sprintf(queryString,"CALL GetPersonInformation(%d,@fname,@lname,"
                           "@bdate, @occupation, @phoneNr)",personId);

    // run query
    queryErrno = mysql_query(this->connection_, queryString);
    // check for query error
    if (queryErrno !=0) {
        fprintf(stderr, "[Database] Error calling mysql function: %s\n",
                            mysql_error(this->connection_));
        return info;
    }

    // run query
    queryErrno = mysql_query(this->connection_, "SELECT @fname, @lname,"
            "               @phoneNr, @bdate, @occupation");
    // check for query error
    if (queryErrno !=0) {
        fprintf(stderr, "[Database] Error calling mysql function: %s\n",
                            mysql_error(this->connection_));
        return info;
    }

    // clear query string
    free(queryString);

    // store mysql query result
    result = mysql_store_result(this->connection_);

    // get returned row
    if ( (row = mysql_fetch_row(result)) == NULL ) {
        fprintf(stderr, "[Database] No result returned by mysql query");
        mysql_free_result(result);
        return info;
    }

    mysql_free_result(result);

    // create result string: firstname lastname#phoneNr#birthdate#occupation
    info.append(row[0]).append(" ").append(row[1]);
    for (int i = 2 ; i < 5 ; i++) {

        info.append("#");
        if (row[i] == NULL) {
            info.append("Unknown");
        }
        else {
            info.append(row[i]);
        }
    }

    return info;

}

string Database::getInfo(int personId, int infoType) {

    // mysql query result
    MYSQL_RES *result;
    MYSQL_ROW row;
    char* queryString = (char*)malloc(100*sizeof(char));
    int queryErrno;
    string info = "";
    
    // prepare query
    switch(infoType) {

        case FIRSTNAME :
            sprintf(queryString,"CALL GetPersonFirstName(%d,@info)",personId);
            break;
        case LASTNAME :
            sprintf(queryString,"CALL GetPersonLastName(%d,@info)",personId);
            break;
        case BIRTHDATE :
            sprintf(queryString,"CALL GetPersonBirthdate(%d,@info)",personId);
            break;
        case OCCUPATION :
            sprintf(queryString,"CALL GetPersonOccupation(%d,@info)",personId);
            break;
        case PHONE_NR :
            sprintf(queryString,"CALL GetPersonPhoneNr(%d,@info)",personId);
            break;
    }
    
     
    // run query
    queryErrno = mysql_query(this->connection_, queryString);
    // check for query error
    if (queryErrno !=0) {
        fprintf(stderr, "[Database] Error calling mysql function: %s\n",
                            mysql_error(this->connection_));
        return info;
    }

    // run query
    queryErrno = mysql_query(this->connection_, "SELECT @info");
    // check for query error
    if (queryErrno !=0) {
        fprintf(stderr, "[Database] Error calling mysql function: %s\n",
                            mysql_error(this->connection_));
        return info;
    }

    // clear query string
    free(queryString);

    // store mysql query result
    result = mysql_store_result(this->connection_);
    
    // get returned row
    if ( (row = mysql_fetch_row(result)) == NULL ) {
        fprintf(stderr, "[Database] No result returned by mysql query");
        mysql_free_result(result);
        return info;
    }
    
    mysql_free_result(result);

    // check if info is NULL
    if (row[0] == NULL) {
        return info;
    }
    
    // return requested information
    return row[0];

}

void Database::insertNewPerson(int id, char* fname, char* lname,
                                char* bdate, char* occupation, char* phoneNr) {

    
    // mysql query result
    string queryString = "";
    int queryErrno;
    
    // prepare query
    queryString.append("CALL InsertNewPerson(");
    // append person Id
    char idToString[5];
    sprintf(idToString,"%d",id);
    queryString.append(idToString).append(",");
    // append firstName
    queryString.append("'").append(fname).append("',");
    // append lastName
    if (strlen(lname) == 0)
        queryString.append("NULL");
    else
        queryString.append("'").append(lname).append("'");
    queryString.append(",");
    // append birthdate
    if (strlen(bdate) == 0)
        queryString.append("NULL");
    else
        queryString.append("'").append(bdate).append("'");
    queryString.append(",");
    // append occupation
    if (strlen(occupation) == 0)
        queryString.append("NULL");
    else
        queryString.append("'").append(occupation).append("'");
    queryString.append(",");
    // append phone number
    if (strlen(phoneNr) == 0)
        queryString.append("NULL");
    else
        queryString.append("'").append(phoneNr).append("'");
    queryString.append(")");

   
     
    // run query
    queryErrno = mysql_query(this->connection_, queryString.c_str());
 
    // check for query error
    if (queryErrno !=0) {
        fprintf(stderr, "[Database] Error calling mysql function: %s\n",
                            mysql_error(this->connection_));
        return;
    }

}
