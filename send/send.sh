#!/bin/bash

curl -X POST -d @'../parser/db2json/db.json' \
http://104.236.115.96:3000/bigwhoop/data/

