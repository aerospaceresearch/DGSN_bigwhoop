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

| id | time | freq | amp_max | amp_mean | scan_mode | location | id_client | id_sw |
|---:|-----:|-----:|--------:|---------:|----------:|---------:|----------:|------:|
| .. |   .. |   .. |      .. |       .. |        .. |       .. |        .. |    .. |


### Table: adsb

| id | time | alt | lat | lon |
|---:|-----:|----:|----:|----:|
| .. |   .. |  .. |  .. |  .. |


### Table: client

| id | id_hash | name | sensor_id | sensor_name | sensor_antenna | wu | sensor_ppm | url |
|---:|--------:|-----:|----------:|------------:|---------------:|---:|-----------:|----:|
| .. |      .. |   .. |        .. |          .. |             .. | .. |         .. |  .. |


### Table: software

| id | bit | os | v_major | v_minor | v_revision |
|---:|----:|---:|--------:|--------:|-----------:|
| .. |  .. | .. |      .. |      .. |         .. |


## References

* C++: https://isocpp.org/
* JSON: http://json.org/
* Python: https://www.python.org/
* sqlite3: https://www.sqlite.org/

