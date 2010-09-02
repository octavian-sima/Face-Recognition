USE Person;

DROP PROCEDURE IF EXISTS InsertNewPerson;
DELIMITER $$
CREATE PROCEDURE InsertNewPerson(
		IN id_ 		int(10),
		IN fname_ 	VARCHAR(50),
		IN lname_ 	VARCHAR(50),
		IN bdate_ 	date,
		IN occupation_ 	VARCHAR(50),
		IN phoneNr_	VARCHAR(20))
	BEGIN
		INSERT into persons values (	id_,
					    	fname_,
					    	lname_,
					    	bdate_,
						occupation_,
						phoneNr_
					);
	END$$
DELIMITER ;

DROP PROCEDURE IF EXISTS InsertExistingPersons;
DELIMITER $$
CREATE PROCEDURE InsertExistingPersons(
		IN nrOfPersons	int(10))
	BEGIN
		DECLARE i 	INT DEFAULT 1;
		DECLARE fname 	VARCHAR(10);
		DECLARE lname	VARCHAR(20);
		WHILE (i <= nrOfPersons) DO
			SET fname = CONCAT("s",i);
			SET lname = CONCAT("lastname",i);
			CALL InsertNewPerson(i,fname,lname,NULL,NULL,NULL);
			SET i = i + 1;
		END WHILE;
	END$$
DELIMITER ;	
	

DROP PROCEDURE IF EXISTS GetPersonInformation;
DELIMITER $$
CREATE PROCEDURE GetPersonInformation(
		IN 	id_ 	int(10),
		OUT 	fname 	VARCHAR(50),
		OUT	lname	VARCHAR(50),
		OUT	bdate	date,
		OUT	job	VARCHAR(50),
		OUT	nr	VARCHAR(20))
     BEGIN
 		SELECT 	firstname, lastname,
			birthdate, occupation,
			phoneNr INTO fname,
			lname, bdate,
			job, nr 
 		FROM persons
 		WHERE id_ = personId;
     END$$
DELIMITER ;

DROP PROCEDURE IF EXISTS GetPersonFirstName;
DELIMITER $$
CREATE PROCEDURE GetPersonFirstName(
		IN 	id_ 	int(10),
		OUT 	name 	VARCHAR(50))
     BEGIN
 		SELECT firstname
 		INTO name
 		FROM persons
 		WHERE id_ = personId;
     END$$
DELIMITER ;

DROP PROCEDURE IF EXISTS GetPersonID;
DELIMITER $$
CREATE PROCEDURE GetPersonID(
		IN 	fname_ 	VARCHAR(50),
		IN	lname_	VARCHAR(50),
		OUT 	id 	int(10))
     BEGIN
 		SELECT personId
 		INTO id
 		FROM persons
 		WHERE fname_ = firstname AND
		      lname_ = lastname;
     END$$
DELIMITER ;

DROP PROCEDURE IF EXISTS GetPersonLastName;
DELIMITER $$
CREATE PROCEDURE GetPersonLastName(
		IN 	id_ 	int(10),
		OUT 	name 	VARCHAR(50))
     BEGIN
 		SELECT lastname
 		INTO name
 		FROM persons
 		WHERE id_ = personId;
     END$$
DELIMITER ;

DROP PROCEDURE IF EXISTS GetPersonBirthdate;
DELIMITER $$
CREATE PROCEDURE GetPersonBirthdate(
		IN 	id_ 	int(10),
		OUT 	bdate 	date)
     BEGIN
 		SELECT birthdate
 		INTO bdate
 		FROM persons
 		WHERE id_ = personId;
     END$$
DELIMITER ;

DROP PROCEDURE IF EXISTS GetPersonOccupation;
DELIMITER $$
CREATE PROCEDURE GetPersonOccupation(
		IN 	id_ 	int(10),
		OUT 	job 	VARCHAR(50))
     BEGIN
 		SELECT occupation
 		INTO job
 		FROM persons
 		WHERE id_ = personId;
     END$$
DELIMITER ;	

DROP PROCEDURE IF EXISTS GetPersonPhoneNr;
DELIMITER $$
CREATE PROCEDURE GetPersonPhoneNr(
		IN 	id_ 	int(10),
		OUT 	nr 	VARCHAR(20))
     BEGIN
 		SELECT phoneNr
 		INTO nr
 		FROM persons
 		WHERE id_ = personId;
     END$$
DELIMITER ;		


