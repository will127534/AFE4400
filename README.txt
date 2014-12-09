I make many change to the original code to make AFE4400 work, 
be careful with this code, because it is Interrupt driven,
so if you output too fast, Arduino IDE or the program may stop.
I'am sure that the interrupt isn't too fast.
all of the data is read in 1/4 cycle of interrupt.


