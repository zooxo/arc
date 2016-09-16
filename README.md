# ARC (Arduino RPN Calculator) 1.0

##What is ARC?
ARC is a scientific calculator which can do basic (+-*/) and high level mathematical operations (trigonometric, statistics, regression). See below for detailed commands.

## On which hardware does ARC run?
ARC runs on the popular arduino platform (AVR/ATmega) with 32 kbyte flash memory and 1 kbyte RAM. Due to the 8-bit-processor ARC calculates only 5 to 6 digits exactly. This should be enough for most calculations (except you are a bookkeeper who wants to add billion-amounts with cent-accuracy).

## How do I run ARC?
Buy an arduino, install the arduino software (including appropriate libraries) on your PC and compile/upload "arc.ino" to your arduino. By defining (and compiling) one input and one output channel ARC can be operated by a serial keyboard (terminal software like screen or putty) or a 12 key keyboard (4 rows, 3 columns). Output will be shown on a terminal or a 128x64 OLED-display.

## Which Commands does ARC support?
* Basic keys: ENTER, DEL, +, -, *, /, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, EE, CHS, STO, RCL
* Stack operations: SWAP, LASTx, ROT+, ROT-
* Settings: DEG, RAD, FIX, SCI, SCROFF (screen off time in s, 2...200), CLOCK (counts h.ms)
* Mathematics: SQRT, SQR, 1/X, POWER, FACT (!), PI
* Logarithmic, exponential: LN, EXP, LOG, 10x, Py,x, Cy,x
* Trigonometric: SIN, COS, TAN, ASIN, ACOS, ATAN
* Hyperbolic: SINH, COSH, TANH, ASINH, ACOSH, ATANH
* Statistic: CLRSUM, SUM+, SUM-, MEAN/STDDEV (mean value and standard deviation)
* Linear regression: A+BX (interception and slope), ->X,Y (estimation of x and y)
* Other: ANNU (present value for a given interest rate and duration), GAUSS (density and distribution), ->P (convert to polar and ...), ->R (... rectangular coordinates), ->H.MS (convert hours to hours, minutes, seconds ...), ->H (... and back), ->RAD (convert degrees to radians), ->DEG (and back)
 
## Some Pictures of a Miniaturized Mobile ARC

![ARC pictures](https://cloud.githubusercontent.com/assets/16148023/18578469/618225e8-7bf0-11e6-8ab1-5494e8903779.jpg "pictures")

## The Circuit Diagram
![ARC circuit diagram](https://cloud.githubusercontent.com/assets/16148023/18578474/65d0e99a-7bf0-11e6-9758-1d2680048e55.png "circuit")

## Key Assignment for the 12-Key-Keyboard
![ARC assignment of keys](https://cloud.githubusercontent.com/assets/16148023/18578478/6a3a0458-7bf0-11e6-8bd6-32abda655e6e.png "keys")

