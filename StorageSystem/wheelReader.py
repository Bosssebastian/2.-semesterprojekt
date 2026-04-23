from machine import Pin, ADC
import time

test = Pin(25, Pin.OUT)

test.value(1)

adc1 = ADC(26)
adc2 = ADC(27)
adc3 = ADC(28)

threshold = 1.15

CIFFERTABEL = [
    (1,1,0),
    (1,1,0),
    (0,1,0),
    (1,0,0),
    (1,1,1),
    (1,1,1),
    (0,1,1),
    (0,0,1),
    (1,0,1),
    
    ]

def deCipher():
    a = adc1.interpretValues()
    b = adc2.interpretValues()
    c = adc3.interpretValues()
    if (a,b,c) == CIFFERTABEL[0]:
        return 1
    if (a,b,c) == CIFFERTABEL[1]:
        return 2
    if (a,b,c) == CIFFERTABEL[2]:
        return 3
    if (a,b,c) == CIFFERTABEL[3]:
        return 4
    if (a,b,c) == CIFFERTABEL[4]:
        return 5
    if (a,b,c) == CIFFERTABEL[5]:
        return 6
    if (a,b,c) == CIFFERTABEL[6]:
        return 7
    if (a,b,c) == CIFFERTABEL[7]:
        return 8
    else:
        return False

class DrejeBaenk:
    def __init__(self, pin):
        self.adc = ADC(pin)
        self.calibratedValue = 0
        
    
    def read(self):
        return self.adc.read_u16()
    
    def readCalibrated(self):
        return self.read() - self.calibratedValue
    
    def calibrate(self):
        self.calibratedValue = self.read()
     
    def interpretValues(self):
        if self.read() > (self.calibratedValue * threshold):
            return 1
        else:
            return 0
        

adc1 = DrejeBaenk(26)
adc2 = DrejeBaenk(27)
adc3 = DrejeBaenk(28)

adc1.calibrate()
adc2.calibrate()
adc3.calibrate()


while True:
    time.sleep_ms(100)
    test.value(1)
    time.sleep_ms(100)
    test.value(0)
    
    print(adc1.readCalibrated(), adc2.readCalibrated(), adc3.readCalibrated())
    #print(adc1.interpretValues(), adc2.interpretValues(), adc3.interpretValues())
    print(deCipher())
    
    
