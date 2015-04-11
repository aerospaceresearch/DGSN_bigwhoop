#!/usr/bin/python

import urllib2
import requests
import getopt
import sys



def print_usage():
    """
    Print usage in case of emergency.
    """
    print sys.argv[0] + ' -i <inputfile.db>'
    print '  -v           print version'
    print '  -h           print help'


def print_version():
    """
    Print version information.
    """
    print sys.argv[0] + ' - send JSON data'


def process_args(argv):
    """
    Process command line options.
    """
    file_input = ''
    try:
        opts, args = getopt.getopt(argv,"vhi:s",["ifile="])
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
        elif opt == '-s':
            global single_entry_per_node
            single_entry_per_node = True
        elif opt in ("-i", "--ifile"):
            file_input = arg

    if not file_input:
        print '[Note] No input data'
        sys.exit()

    return file_input


def main(argv):
    """
    Main function.
    """
    file_input = process_args(argv)
    # TODO: Send data in file

if __name__ == "__main__":
   main(sys.argv[1:])

