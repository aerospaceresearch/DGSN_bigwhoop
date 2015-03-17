import multiprocessing
from multiprocessing import Queue, Manager
import subprocess
import time
import numpy as np
import os
import sys

def do_smth(i,input, frequency):
    #in_file_iq_stream = np.fromfile(open(out_file_iq_stream, mode="rb"), dtype=np.uint8)
    rf_iq_stream = bin_to_complex(input)
    rf_iq_stream -= np.ones_like(rf_iq_stream) * complex(127, 127)
    mean = np.mean(np.abs(rf_iq_stream))
    max = np.max(np.abs(rf_iq_stream))
    #print i, frequency, max, mean
    return [frequency, max, mean]

def worker(workerid, bands, q_sdr):
    ## check for existing raw file of the previous run

    pathname = os.path.dirname(sys.argv[0])
    rtlsdr_execution_link = pathname+'/rtl_sdr/x64/rtl_sdr.exe'

    for band in range(len(bands)):
        frequency = bands[band]
        n_samples = 3000000
        # recording the samples with rtlsdr devices
        dsp = start_recording_with_rtl_sdr(rtlsdr_execution_link, '-d '+str(workerid), '-f '+str(frequency), '-s 2048000','-n '+str(n_samples), '-g 28')

        # in case of droped frames, or other strange input, it should be avoided to pass those values to processing
        if len(dsp) <= n_samples*2 and len(dsp) > 0:
            # do a simple processing to put a serial stdout into the needed I/Q stream
            print "some output", workerid, frequency, dsp, len(dsp), np.mean(dsp), np.max(dsp), np.min(dsp)
            result = dsp[::2] + complex(0,1)*dsp[1::2]
            result -= np.ones_like(result) * complex(127, 127)
            mean = np.mean(np.abs(result))
        else:
            print "with device", workerid, "went something wrong at frequency of", frequency

    print "finished all bands. leave worker",workerid

def checking_hw():
    # checking if any of the listed hw is plugged in
    # in future, there could also be other sdr hw being called from here
    return call_rtl_sdr()

def call_rtl_sdr():
    # calling any rtl_sdr based hw that is connected to the PC
    # http://www.rtl-sdr.com

    pathname = os.path.dirname(sys.argv[0])
    rtlsdr_execution_link = pathname+'/rtl_sdr/x64/rtl_sdr.exe'

    p = subprocess.Popen([rtlsdr_execution_link, '-d 0', '-f 107700000', '-n 2', '-g 28', '-'], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    # reading in the stdout
    read_stdin = p.communicate()[1]
    print read_stdin

    hw_details_id = []
    hw_details_name = []

    hw_available = 0
    hw_number_parsing_start = read_stdin.find("Found ")
    hw_number_parsing_end = read_stdin.find("device(s):")
    if hw_number_parsing_start > -1:
        hw_available = np.int(read_stdin[hw_number_parsing_start+6:hw_number_parsing_end])

    for i in range(hw_available):
        number = str(i)+":"
        hw_details_id.append(i)
        hw_details_name.append(read_stdin[read_stdin.find(number)+len(number)+2:read_stdin.find("\n", read_stdin.find(number)+1)])
    return hw_available

def start_recording_with_rtl_sdr(rtlsdr_execution_link, device, frequency, sample, samples, gain):
    print "recording with", device,"at",frequency, "Hz"
    rtl_sdr_cmds =[rtlsdr_execution_link, device, frequency, sample, samples, gain, "-"]
    p = subprocess.Popen(rtl_sdr_cmds, stdout=subprocess.PIPE, stderr=subprocess.PIPE, shell=True)
    out = p.communicate()[0]
    print "with device", device,"there are", len(out),"inputs in stdout"
    return (np.fromstring(out, dtype=np.uint8))

if __name__ == '__main__':

    # checking if and how many rtladr devices are connected
    rtlsdr_devices = checking_hw()
    print "there are",rtlsdr_devices,"rtlsdr devices attached to your pc"


    # preparing the monitoring spectrum
    freq_max = 2000000000
    freq_min = 40000000
    samplerate = 2048000

    print "splitting the spectrum into ",int((freq_max-freq_min)/samplerate)," chunks for the spektrum monitoring"
    print ""

    bands = []
    for j in range(rtlsdr_devices):
        part = []
        for i in range(freq_min,freq_max,samplerate):
        #for i in range(freq_max/rtlsdr_devices*j,freq_max/rtlsdr_devices+freq_max/rtlsdr_devices*j,samplerate):
            part.append(i)
        bands.append(part)


    # starting the monitoring
    manager = Manager()
    q_sdr = manager.dict()
    jobs = []

    print "starting ", rtlsdr_devices," workers. one for each rtlsdr device"

    for workers in range(rtlsdr_devices):
        p = multiprocessing.Process(target=worker, args=(workers,bands[workers],q_sdr))
        jobs.append(p)
        p.start()

    for proc in jobs:
        proc.join()