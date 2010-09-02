DROP DATABASE IF EXISTS `Person`;

CREATE DATABASE `Person`;

USE `Person`;

DROP TABLE IF EXISTS `persons`;

CREATE TABLE `persons` (
	`personId` 	int(10) 	NOT NULL PRIMARY KEY AUTO_INCREMENT,
	`firstname` 	varchar(50) 	NOT NULL,
	`lastname` 	varchar(50),
	`birthdate` 	date,
	`occupation` 	varchar(50),
	`phoneNr`	varchar(20)
) ENGINE=InnoDB  DEFAULT CHARSET=latin1 ;

