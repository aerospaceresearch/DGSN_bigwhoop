#!/usr/bin/python

# thank you for those great libs!
from rtlsdr import RtlSdr
from rtlsdr import librtlsdr
import numpy as np
from xml.dom import minidom

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
    # sdr.gain = 20
    result = sdr.read_samples(scan_n_samples)
    return result

'''
this is a a veeeeery simple spectrum analyzer hopping each
frequency to get the full spectrum that is possible with
"ezcap USB 2.0 DVB-T/DAB/FM dongle", more devices will follow
'''
def analyze_full_spectrum_basic(device_number):
    print "you use a", librtlsdr.rtlsdr_get_device_name(device_number)

    scan_samplerate = 2000*1024
    scan_n_samples = scan_samplerate*1

    # create object and start rtl-sdr device.
    # if more than one device, select it by device_number
    sdr = RtlSdr(device_index=device_number)

    # configure device
    sdr.sample_rate = scan_samplerate
    # sdr.freq_correction = 60   # PPM
    # sdr.gain = 'auto'
    sdr.gain = 20


    result_frequency = []
    result_mean_amplitude = []
    result_max_amplitude = []

    for scan_frequency in range(task_freq_scanstart[device_number], task_freq_scanend[device_number], scan_samplerate):
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
        f = open('progressbar.csv', 'w+')
        f.write(str(progressbar))
        f.close()

    sdr.close()
    return [result_frequency, result_mean_amplitude, result_max_amplitude]

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
writing the output of the analysis.
it is js for now and will be specified in the next team meeting
'''
def writing_output(result):
    f = open("result.js", "w+")
    f.write("meta tbd\n")
    f.write("data tbd\n")
    f.write(str(result))
    f.close

    '''import matplotlib.pyplot as plt
    plt.plot(result[0],result[1])
    plt.plot(result[0],result[2])
    plt.ylabel('some numbers')
    plt.show()'''

'''
let's start here.
'''
def main():
    if librtlsdr.rtlsdr_get_device_count() > 0:
        print "loading groundstation config data..."
        load_groundstation_config()
        print gs_meta
        print gs_location

        print "loading the workunit data"
        load_workunit()
        print "scanning spectrum is in between", task_freq_scanstart, "and", task_freq_scanend

        print "starting sdr-device..."
        print "for now, only one device is used. Soon, more..."
        device_number = 0
        result = analyze_full_spectrum_basic(device_number)

        writing_output(result)
    else:
        print "no sdr-device found"

    print "thank you for helping"

if __name__ == '__main__':
    main()