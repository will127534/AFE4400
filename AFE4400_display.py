
import numpy as np
from matplotlib.lines import Line2D
import matplotlib.pyplot as plt
import serial
from serial.tools import list_ports
import matplotlib.animation as animation
import time
print list(list_ports.comports())
#x = raw_input(">>> Input Com port name: ")
Serial = serial.Serial("COM13",115200)
long_ave = [0] * 100
count = 0
heart_ave = [0] * 3
count_h = 0
ave = 0
class Scope:
    def __init__(self, ax, maxt=3, dt=0.02):
        self.ax = ax
        self.dt = dt
        self.maxt = maxt
        self.tdata = [0]
        self.ydata = [0]
        self.line = Line2D(self.tdata, self.ydata)
        self.ax.add_line(self.line)
        self.ax.set_ylim(ave-0.02,ave+0.02)
        self.ax.set_xlim(0, self.maxt)

    def update(self, y):
        lastt = self.tdata[-1]
        if lastt > self.tdata[0] + self.maxt: # reset the arrays
            self.tdata = [self.tdata[-1]]
            self.ydata = [self.ydata[-1]]
            self.ax.set_xlim(self.tdata[0], self.tdata[0] + self.maxt)
            self.ax.figure.canvas.draw()

        t = self.tdata[-1] + self.dt
        self.tdata.append(t)
        self.ydata.append(y)
        self.line.set_data(self.tdata, self.ydata)
        return self.line,
    

def avelong(value):
    global long_ave 
    global count 
    if (count is 100):
        count = 0
        #print "one cycle"
    long_ave[count] = value
    count +=1
    out = np.mean(long_ave)
    #print outb
    return out
def aveheart(value):
    global heart_ave 
    global count_h 
    if (count_h is 3):
        count_h = 0
        #print "one cycle"
    heart_ave[count_h] = value
    count_h +=1
    out = np.mean(heart_ave)
    #print outb
    return out
beat = 0
last = 0

last_data = 0
test2 = 0
last_last_data = 0
cycle1 = 0
cycle2 = 0
cycle3 = 0

def emitter(p=0.03):
    global test2
    global last_data
    global last_last_data
    global cycle1 
    global cycle2 
    global cycle3
    global ave
    'return a random value with probability p, else 0'
    while True:
        try:
            data = Serial.readline()
            test = data.split(',')
            for i in range(0,len(test)):
                    test[i] = float(test[i].strip())
                    test[i] = test[i]
            #print len(test)
            if len(test)==6:
                
                #print test[0]
                last_last_data = last_data
                last_data = test2
                test2 = test[0]
                ave = avelong(test[0])
                scope.ax.set_ylim(ave-1000,ave+1000)
                slope1 = test2 - last_data
                slope2 = last_data - last_last_data
                if (slope1<=-90 or slope2<=-90 ):
                   
                   if ((time.time()-cycle1)>0.5):
                       
                       cycle2 = cycle1
                       cycle1 = time.time()
                       
                       scope.line.set_color('red')
                       try:
                         heart = aveheart(-1/(cycle2-cycle1)*60)
                         print heart,slope1,slope2
                       except:
                         print "heart_beat calc error"
                    
                else:
                    scope.line.set_color('blue')
                #print test2
                yield test2
            else:
               # print test
                yield 0.
        except:
            yield 0.

fig, ax = plt.subplots()
scope = Scope(ax)

# pass a generator in "emitter" to produce data for the update func
ani = animation.FuncAnimation(fig, scope.update, emitter, interval = 0,
    blit=True)


plt.show()
