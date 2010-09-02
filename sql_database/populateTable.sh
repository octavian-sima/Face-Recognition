#!/bin/sh
username=$1

mysql -u $username -p < "database_proc/populate_table.sql"

