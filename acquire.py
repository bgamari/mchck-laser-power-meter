#!/usr/bin/python

import serial
import time

s = serial.Serial('/dev/ttyACM0', timeout=1)

def parse(l):
        name = l[:l.find(':')]
        start = l.find('raw=')+4
        end = l.find(',')
        value = int(l[start:end])
        return name, value

while True:
        s.write('\n')
        for i in range(4):
                l = s.readline()
                if len(l) == 0: break
                try:
                        name, value = parse(l)
                        print name, value
                except:
                        print '# fail'
                        pass

        time.sleep(100e-3)
