#!/usr/bin/python

from flask import Flask
import config
import os
import sqlite3
import json

import json
from functools import wraps
from flask import redirect, request, current_app


app = Flask(__name__)
app.config['DB_FILE'] = config.DB_FILE

def support_jsonp(f):
    """Wraps JSONified output for JSONP"""
    @wraps(f)
    def decorated_function(*args, **kwargs):
        callback = request.args.get('callback', False)
        if callback:
            content = str(callback) + '(' + str(f().data) + ')'
            return current_app.response_class(content,\
                mimetype='application/json')
        else:
            return f(*args, **kwargs)
    return decorated_function


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
@support_jsonp
def reply_data(freq):
    connection = sqlite3.connect(config.DB_FILE)
    connection.row_factory = dict_factory
    cursor = connection.cursor()
    cursor.execute(
    """
    SELECT DISTINCT
    location_lat AS lat, location_lon AS lng,
    amp_mean AS mean, amp_max
    FROM data
    WHERE freq = %s
    GROUP BY
        client_id_hash;
    """ % freq)
    ret = cursor.fetchall()
    connection.close()
    return jsonify(ret)

if __name__ == "__main__":
	app.run(host='0.0.0.0', port=5001, debug=True)
