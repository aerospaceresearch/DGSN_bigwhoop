#!/usr/bin/python

from flask import Flask
import config
import os
import sqlite3
import json

app = Flask(__name__)
app.config['DB_FILE'] = config.DB_FILE

def allowed_file(filename):
    return filename.endswidth('.db');

@app.route('/', methods=['GET', 'POST'])
def upload_file():
    if request.method == 'POST':
        file = request.files['file']
        if file and allowed_file(file.filename):
            filename = secure_filename(file.filename)
            file.save(os.path.join(app.config['UPLOAD_FOLDER'], filename))
            return redirect(url_for('uploaded_file',
                                    filename=filename))
    return 'no file specified'

def dict_factory(cursor, row):
    """
    Row factory function.
    """
    d = {}
    for idx, col in enumerate(cursor.description):
        d[col[0]] = row[idx]
    return d

@app.route('/data/<int:freq>', methods=['GET'])
def reply_data(freq):
    connection = sqlite3.connect(config.DB_FILE)
    connection.row_factory = dict_factory
    cursor = connection.cursor()
    cursor.execute(
    """
    SELECT DISTINCT
    client_id_hash, time, location_lat, location_lon,
    MAX(amp_mean) AS amp_mean, amp_max
    FROM data
    WHERE freq = %s
    GROUP BY
        client_id_hash;
    """ % freq)
    ret = json.dumps(cursor.fetchall())
    connection.close()
    return ret

if __name__ == "__main__":
	app.run(host='0.0.0.0', port=5001, debug=True)
