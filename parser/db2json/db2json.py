#!/usr/bin/python

import sys
import getopt
import sqlite3


def dict_factory(cursor, row):
    d = {}
    for idx, col in enumerate(cursor.description):
        d[col[0]] = row[idx]
    return d


def print_db(file_in, file_out):
    connection = sqlite3.connect(file_in)
    connection.row_factory = dict_factory
    cursor = connection.cursor()
    cursor.execute("select * from data")
    results = cursor.fetchall()
    print results
    print ""
    connection.close()


def print_usage():
    print 'db2json.py -i <inputfile.db> -o <outputfile.json>'


def print_version():
    print 'db2json v1.0 - sqlite3 database to json converter'


def process_args(argv):
    file_input = 'database.db'
    file_output = 'database.json'
    try:
        opts, args = getopt.getopt(argv,"vhi:o:",["ifile=","ofile="])
    except getopt.GetoptError:
        print_usage()
        sys.exit(2)
    for opt, arg in opts:
        if opt == '-h':
            print_usage()
            sys.exit()
        elif opt == '-v':
            print_version()
            sys.exit(0)
        elif opt in ("-i", "--ifile"):
            file_input = arg
        elif opt in ("-o", "--ofile"):
            file_output = arg
    return file_input, file_output


def main(argv):
    file_input, file_output = process_args(argv)
    print_db(file_input, file_output)


if __name__ == "__main__":
   main(sys.argv[1:])

