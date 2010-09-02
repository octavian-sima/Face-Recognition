#!/bin/sh
username=$1

mysql -u $username -p < "database_proc/create_table.sql"
mysql -u $username -p < "database_proc/create_proc.sql"





