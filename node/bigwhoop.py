#!/usr/bin/python

# thank you for those great libs!
from rtlsdr import RtlSdr
from rtlsdr import librtlsdr
import numpy as np
from xml.dom import minidom
import time
import pickle
import os
import json

'''
BigWhoop...
will measure everything what is happening in the radio-frequency spectrum,
globally, continously, with your help!
'''


'''
starting the sdr-device and let it read out a number of samples
'''
def start_sdr(scan_frequency, scan_n_samples, sdr):
    # configure device
    # sdr.sample_rate = scan_samplerate  # Hz
    sdr.center_freq = scan_frequency     # Hz
    # sdr.freq_correction = 60   # PPM
    # sdr.gain = 'auto'
    sdr.gain = 28
    result = sdr.read_samples(scan_n_samples)
    return result

'''
this is a a veeeeery simple spectrum analyzer hopping each
frequency to get the full spectrum that is possible with
"ezcap USB 2.0 DVB-T/DAB/FM dongle", more devices will follow
'''
def analyze_full_spectrum_basic(device_number):
    print "you use a", librtlsdr.rtlsdr_get_device_name(device_number)

    scan_samplerate = task_hw_setting_samplerate[device_number]
    scan_n_samples = task_hw_setting_nsamples[device_number]

    # create object and start rtl-sdr device.
    # if more than one device, select it by device_number
    sdr = RtlSdr(device_index=device_number)

    # configure device
    sdr.sample_rate = scan_samplerate
    # sdr.freq_correction = 60   # PPM
    # sdr.gain = 'auto'
    sdr.gain = 20

    result_timestamp = []
    result_geo_lon = []
    result_geo_lat = []
    result_geo_alt = []
    result_frequency = []
    result_mean_amplitude = []
    result_max_amplitude = []

    for scan_frequency in range(task_freq_scanstart[device_number], task_freq_scanend[device_number], scan_samplerate):
        result_timestamp.append(time.time())
        result_geo_lon.append(180.0)
        result_geo_lat.append(90.0)
        result_geo_alt.append(0.0)

        sdr_iq_stream = start_sdr(scan_frequency, scan_n_samples, sdr)
        sdr_iq_stream = np.abs(sdr_iq_stream)
        mean = np.mean(sdr_iq_stream)
        max = np.max(sdr_iq_stream)
        progressbar = float(scan_frequency-task_freq_scanstart[device_number])/(task_freq_scanend[device_number]-task_freq_scanstart[device_number])
        print progressbar, scan_frequency, mean, max

        result_frequency.append(scan_frequency)
        result_mean_amplitude.append(mean)
        result_max_amplitude.append(max)

        # creating the progress bar file used by BOINC.
        # 0.0 = 0%, 1.0 = 100% ready
        # only writing the last progress into the file
        # notice: this is only working for one scan. It does not include the scan duration yet!
        f = open('progressbar.csv', 'w+')
        f.write(str(progressbar))
        f.close()

    sdr.close()
    return [result_timestamp, result_frequency, result_mean_amplitude, result_max_amplitude, result_geo_lon, result_geo_lat, result_geo_alt]

'''
shortening just in case the input is too long. so people can include their
infos, but not too much
'''
def shortening_string(input):
    length = len(input)
    if length > 64:
        length = 64
    print input[:length]
    return input[:length]

'''
Loading in the ground station data.
Optionally, the user can put in data in there to provide additional information about his gs set up.
The only mandatory information is here to put in his geo location (long, lat, alt), but this will also be checked.
'''
def load_groundstation_config():
    doc = minidom.parse("set_your_groundstation_config.xml")

    global gs_meta, gs_location
    gs_meta = []
    gs_location = []

    # doc.getElementsByTagName returns NodeList
    gs_meta.append(shortening_string(doc.getElementsByTagName("gs_name")[0].firstChild.data))
    gs_meta.append(shortening_string(doc.getElementsByTagName("gs_info")[0].firstChild.data))
    gs_meta.append(shortening_string(doc.getElementsByTagName("gs_info_url")[0].firstChild.data))

    gs_location.append(doc.getElementsByTagName("gs_location_long")[0].firstChild.data)
    gs_location.append(doc.getElementsByTagName("gs_location_lat")[0].firstChild.data)
    gs_location.append(doc.getElementsByTagName("gs_location_alt_meter")[0].firstChild.data)

    sensors = doc.getElementsByTagName("sensor")
    for sensor in sensors:
        sid = sensor.getAttribute("id")
        sen_name = sensor.getElementsByTagName("sen_name")[0]
        sen_usbport = sensor.getElementsByTagName("sen_usbport")[0]
        #print("id:%s, Sensor Name:%s, Sensor USB Port:%s" %
        #      (sid, sen_name.firstChild.data, sen_usbport.firstChild.data))


'''
loading in the BOINC ready work unit data chunk.
this is currently xml, because it is human readable and the user will see, what is done
on the computer and scanned in the spectrum.
'''
def load_workunit():
    doc = minidom.parse("workunit.xml")

    global sid, task_durationmin, task_freq_scanstart,task_freq_scanend, task_analysis_mode, task_hw_setting_samplerate, task_hw_setting_gain, task_hw_setting_nsamples
    sid = []
    task_durationmin = []
    task_freq_scanstart = []
    task_freq_scanend = []
    task_analysis_mode = []
    task_hw_setting_samplerate = []
    task_hw_setting_gain = []
    task_hw_setting_nsamples = []

    # doc.getElementsByTagName returns NodeList
    wu_info = doc.getElementsByTagName("wu_info")[0]
    print("Workunit Info:%s" %
           (wu_info.firstChild.data))

    tasks = doc.getElementsByTagName("task")

    for task in tasks:
        sid.append(int(task.getAttribute("id")))
        task_durationmin.append(int(task.getElementsByTagName("task_durationmin")[0].firstChild.data))
        task_freq_scanstart.append(int(task.getElementsByTagName("task_freq_scanstart")[0].firstChild.data))
        task_freq_scanend.append(int(task.getElementsByTagName("task_freq_scanend")[0].firstChild.data))
        task_analysis_mode = task.getElementsByTagName("task_analysis_mode")[0]
        task_hw_setting_samplerate.append(int(task.getElementsByTagName("task_hw_setting_samplerate")[0].firstChild.data))
        task_hw_setting_gain.append(int(task.getElementsByTagName("task_hw_setting_gain")[0].firstChild.data))
        task_hw_setting_nsamples.append(int(task.getElementsByTagName("task_hw_setting_nsamples")[0].firstChild.data))


'''
In case of a hard shut off/down of the computer and software, the software will dump and resume from a
savepoint file that is performed here.
It is a simple python pickle.
'''
def boinc_dump_savepoint_file(filename, result, timer):
    pickle.dump( [timer,result], open( filename, "wb" ) )

def boinc_load_savepoint_file(filename):
    input = pickle.load( open( filename, "rb" ) )
    return input[0], input[1]

'''
writing the output of the analysis.
it is js for now and will be specified in the next team meeting
'''
def writing_output(container):
    getout = (json.dumps(container, sort_keys=True, indent=4))
    f = open("result.js", "w+")
    f.write(getout)
    f.close

'''
creating a dictionary for all the output data
'''
def create_out_structure():
    meta = {}
    meta['client'] = {}
    meta['client']['id'] = 'hash value'
    meta['client']['name'] = 'NodeZero'
    meta['client']['url'] = 'www.AerospaceResearch.net'

    meta['client']['sensor'] = {}
    meta['client']['sensor']['id'] = 0
    meta['client']['sensor']['name'] = 'generic sdr device'
    meta['client']['sensor']['ppm'] = 0

    meta['sw'] = {}
    meta['sw']['version'] = 0
    meta['sw']['os'] = 'WinLinuxOS'
    meta['sw']['bit'] = '32bit64bit'

    data = {}
    data['geolocation'] = {}
    data['geolocation']['user_input'] = {'time' : 0.0, 'lon' : 180.0, 'lat' : 90.0, 'alt' : 0.0}
    data['geolocation']['ipgeo'] = {'time' : 0.0, 'lon' : 180.0, 'lat' : 90.0, 'alt' : 0.0}
    data['geolocation']['gps'] = {'time' : 0.0, 'lon' : 180.0, 'lat' : 90.0, 'alt' : 0.0}
    data['geolocation']['adsb'] = {'time' : 0.0,'lon' : 180.0, 'lat' : 90.0, 'alt' : 0.0}
    data['mode'] = 'analyze_full_spectrum_basic'
    data['dataset'] = {}

    return {'meta' : meta, 'data' : data}

'''
here, the json putput is stored in a json table for the data-set field.
this is still specific and needs to be changed for other functions!
the style is based on this for now http://www.patrick-wied.at/static/heatmapjs/example-heatmap-googlemaps.html
'''
def creating_json_data(input):
    out = []
    for k in range(len(input)):
        for l in range(len(input[k][0])):
            out.append({'timestamp' : input[k][0][l], 'frequency' : input[k][1][l], 'mean_amplitude' : input[k][2][l], 'max_amplitude' : input[k][3][l], 'lon' : input[k][4][l], 'lat' : input[k][5][l], 'alt' : input[k][6][l]})
    return out

'''
let's start here.
'''
def main():
    print "loading groundstation config data..."
    load_groundstation_config()
    print gs_meta
    print gs_location

    print "loading the workunit data"
    load_workunit()

    filename_savepoint = "savepoint.p"
    if os.path.exists(filename_savepoint):
        print "loading savepoints"

    result = []

    print "scanning spectrum is in between", task_freq_scanstart, "and", task_freq_scanend
    if librtlsdr.rtlsdr_get_device_count() > 0:
        print "starting sdr-device..."
        print "for now, only one device is used. Soon, more..."
        device_number = 0

        # setting the scan timer
        # and loading the saved data to start from there
        if os.path.exists(filename_savepoint):
            time_counter, result = boinc_load_savepoint_file(filename_savepoint)
            print time_counter
            print result
        else:
            time_counter = task_durationmin[device_number]

        # preparation of the output structure as json for everything meta and data
        container = create_out_structure()


        # start the scanning cycle with time and frequencies.
        # currently, only time base can be resumed.
        # import matplotlib.pyplot as plt
        while time_counter > 0:
            time_start = time.time()

            result.append(analyze_full_spectrum_basic(device_number))
            time_counter = time_counter - (time.time() - time_start)
            boinc_dump_savepoint_file(filename_savepoint, result, time_counter)

            # plt.plot(result[-1][1],result[-1][2])
            # plt.plot(result[-1][1],result[-1][3])
            print "time to go",time_counter, " result", result
        # plt.ylabel('some numbers')
        # plt.show()


        #wrapping and cleaning up
        container['data']['mode'] = "analyze_full_spectrum_basic"
        container['data']['dataset'] = {}
        container['data']['dataset'] = creating_json_data(result)
        writing_output(container)
        os.remove(filename_savepoint)

    else:
        print "no sdr-device found"

    print "thank you for helping"

if __name__ == '__main__':
    main()