# Parser

Basic parser for processing the incoming data in JSON format and storing
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

```Shell
./parser result.json
```


## Database layout

The data is stored in a sqlite3 database according to the following layout.


### Table: db_version

| v_major | v_minor |
|--------:|--------:|
|      .. |      .. |


### Table: data

| id | time | freq | amp_max | amp_mean | scan_mode | <location> | <client> | <software> |
|---:|-----:|-----:|--------:|---------:|----------:|-----------:|---------:|-----------:|
| .. |   .. |   .. |      .. |       .. |        .. |         .. |       .. |         .. |


### Table part: location in data

| .. | location_alt | location_lat | location_lon | .. |
|---:|-------------:|-------------:|-------------:|---:|
| .. |           .. |           .. |           .. | .. |


### Table part: client in data

| .. | client_id_hash | client_name | sensor_id | sensor_name | sensor_antenna | sensor_ppm | wu | url | .. |
|---:|---------------:|------------:|----------:|------------:|---------------:|-----------:|---:|----:|---:|
| .. |             .. |          .. |        .. |          .. |             .. |         .. | .. |  .. | .. |


### Table part: software in data

| .. | sw_bit | sw_os | sw_v_major | sw_v_minor | sw_v_revision | .. |
|---:|-------:|------:|-----------:|-----------:|--------------:|---:|
| .. |     .. |    .. |         .. |         .. |            .. | .. |


### Table: adsb

| id | time | alt | lat | lon |
|---:|-----:|----:|----:|----:|
| .. |   .. |  .. |  .. |  .. |


## References

* C++: https://isocpp.org/
* JSON: http://json.org/
* Python: https://www.python.org/
* sqlite3: https://www.sqlite.org/

