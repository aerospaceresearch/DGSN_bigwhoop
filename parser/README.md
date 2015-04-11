# Parser

Basic parser to process recorded data in JSON format and to store
resulting datasets in the sqlite3 database.


## Build it!

### Dependencies

* jsoncpp (>= 0.6.0)
  https://github.com/open-source-parsers/jsoncpp

* soci (>= 3.2.3)
  https://github.com/SOCI/soci


### Compilation

  Magic is all you need.

## Usage

The client generates a JSON file containing the results. It can be
provided to the parser as an command line option.

```Shell
./parser result.json
```

The resulting data should be added to the sqlite3 database called
*database.db*. It can be inspected with the sqlite3 utility. Relevant data
to display can be provided using the *db2json* utility.

For a list of available command line options call
```Shell
./parser --help
```

## Database layout

The data is stored in a sqlite3 database according to the following layout.


### Table: db_version

| v_major | v_minor |
|--------:|--------:|
|      .. |      .. |

The version of the database is stored in *db_version*. That way the parser
can verify the version of the database with its own and additionally
backward compatibility can be maintained.


### Table: data

| id | time | freq | amp_max | amp_mean | scan_mode | [location] | [client] | [software] |
|---:|-----:|-----:|--------:|---------:|----------:|-----------:|---------:|-----------:|
| .. |   .. |   .. |      .. |       .. |        .. |         .. |       .. |         .. |

The table *data* holds all the basic uninterpreted data. The time of
measurement is stored in the *time* column as the unix timestamp in the
double precision floating point format together with the frequency, the
maximum and the average amplitude. *scan_mode* in this table is always
'analyze_full_spectrum_basic'.


#### Table section: location in data

| .. | location_alt | location_lat | location_lon | .. |
|---:|-------------:|-------------:|-------------:|---:|
| .. |           .. |           .. |           .. | .. |

The location in the data table is stored as the altitude, latitude and
longitude as floating point values.


#### Table section: client in data

| .. | client_id_hash | client_name | sensor_id | sensor_name | sensor_antenna | sensor_ppm | wu | url | .. |
|---:|---------------:|------------:|----------:|------------:|---------------:|-----------:|---:|----:|---:|
| .. |             .. |          .. |        .. |          .. |             .. |         .. | .. |  .. | .. |

Currently the client data is stored for each measurement point. It
contains the unique *client_id_hash*. *client_name*, *sensor_name*,
*sensor_antenna* and *url* are user defined values for providing a basic
context for the recorded data. *url* can be an e-mail address or an actual
URL. *sensor_id* is currently provided by the user. It is the index of the
sensor on the current device. *wu* holds the work unit id provided in the
*workunit.xml* for the client.


#### Table section: software in data

| .. | sw_bit | sw_os | sw_v_major | sw_v_minor | sw_v_revision | .. |
|---:|-------:|------:|-----------:|-----------:|--------------:|---:|
| .. |     .. |    .. |         .. |         .. |            .. | .. |

The software section of the data table holds basic information about the
software that was used for the recording. The register size of the
architecture on the device is stored in *sw_bit* together with *sw_os* for
the running operating system and the version information of the software
used to collect the data in *sw_v_major*, *sw_v_minor* and *sw_v_revision*.


### Table: adsb

| id | time | location_alt | location_lat | location_lon |
|---:|-----:|-------------:|-------------:|-------------:|
| .. |   .. |           .. |           .. |           .. |

In the *adsb* table basic ADSB data is stored. Here, the according
timestamp is saved in the floating point format as the unix time. The
datasets in the table hold furthermore the position of the aircraft in
*location_alt*, *location_lat* and *location_lon* as the altitude,
latitude and longitude respectively.

_Note_: This part of the database will change in further development.


## References

Things and stuff that was used for this part of the BigWhoop project.

* boost: http://www.boost.org/
* C++: https://isocpp.org/
* GCC: https://gcc.gnu.org/
* JSON: http://json.org/
* Python: https://www.python.org/
* sqlite3: https://www.sqlite.org/

