#!/usr/bin/python


import sys
import getopt
import sqlite3


single_entry_per_node = False;
selected_frequency = 73008000;

def dict_factory(cursor, row):
    """
    Row factory function.
    """
    d = {}
    for idx, col in enumerate(cursor.description):
        d[col[0]] = row[idx]
    return d


def db2json(file_in, file_out):
    """
    Main function for reading file_in and providing file_out with
    according JSON output.
    """
    connection = sqlite3.connect(file_in)
    connection.row_factory = dict_factory
    cursor = connection.cursor()
    if single_entry_per_node:
        cursor.execute(
        """
        SELECT DISTINCT
        client_id_hash, MAX(time) AS time, location_lat, location_lon,
        amp_mean, amp_max
        FROM data
        WHERE freq = %s
        GROUP BY
            client_id_hash;
        """ % selected_frequency)
    else:
        cursor.execute("SELECT client_id_hash, time, "\
                "location_lat, location_lon FROM data")
    results = cursor.fetchall()
    print results
    print ""
    connection.close()


def print_usage():
    """
    Print usage in case of emergency.
    """
    print sys.argv[0] + ' -i <inputfile.db> -o <outputfile.json>'
    print '  -v           print version'
    print '  -h           print help'
    print '  -s           include only one last entry per node in the'\
                         'JSON file'


def print_version():
    """
    Print version information.
    """
    print 'db2json v1.0 - sqlite3 database to json converter'


def process_args(argv):
    """
    Process command line options.
    """
    file_input = ''
    file_output = 'database.json'
    try:
        opts, args = getopt.getopt(argv,"vhi:o:s",["ifile=","ofile="])
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
        elif opt in ("-o", "--ofile"):
            file_output = arg

    if not file_input:
        print '[Note] No input data'
        sys.exit()

    return file_input, file_output


def main(argv):
    """
    Main function.
    """
    file_input, file_output = process_args(argv)
    db2json(file_input, file_output)


if __name__ == "__main__":
   main(sys.argv[1:])

