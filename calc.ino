/*

 Welcome to ARC (Arduino RPN Calculator) v. 1.0 ... (c) 2016 by deetee aka zooxo
 
 ARC is a scientific calculator which can do basic (+*-/) and high level
 mathematical operations (trigonometric, statistics, regression). See below for
 detailed commands.
 
 By defining (and compiling) one input- and one output channel ARC can be operated
 by a serial keyboard (terminal software like screen or putty) or a 12 key
 keyboard (4 rows, 3 columns). Output will be shown on a terminal or a
 128x64 OLED-display.
 
 Note: Due to the 8-bit-processor ARC calculates only 5 to 6 digits exactly. This
 should be enough for most calculations (except you are a bookkeeper who
 wants to add billion-amounts with cent-accuracy).
 
 COMMANDS and KEYS:
 * Basic keys:
 ENTER, DEL, +, -, *, /, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, EE, CHS, STO, RCL
 * Stack operations:
 SWAP, LASTx, ROT+, ROT-
 * Settings:
 DEG, RAD, FIX, SCI, SCROFF (screen off time in s, 2...200), CLOCK (counts h.ms)
 * Mathematics:
 SQRT, SQR, 1/X, POWER, FACT (!), PI
 * Logarithmic, exponential:
 LN, EXP, LOG, 10x, Py,x, Cy,x
 * Trigonometric:
 SIN, COS, TAN, ASIN, ACOS, ATAN
 * Hyperbolic:
 SINH, COSH, TANH, ASINH, ACOSH, ATANH
 * Statistic:
 CLRSUM, SUM+, SUM-, MEAN/STDDEV (mean value and standard deviation)
 * Linear regression:
 A+BX (interception and slope), ->X,Y (estimation of x and y)
 * Other:
 ANNU (present value for a given interest rate and duration),
 GAUSS (density and distribution),
 ->P (convert to polar and ...), ->R (... rectangular coordinates)
 ->H.MS (convert hours to hours, minutes, seconds ...), ->H (... and back)
 ->RAD (convert degrees to radians), ->DEG (and back)
 
 */

// DEFINES & INCLUDES

// Define IO-channels
#define INSERIALKBD 0 // serial keyboard (9600 baud)
// not implemented: #define INSERIALKBDASDW 0 // serial keyboard with ASDW-cursor only
// not implemented: #define INKBD 0 // cursor-keyboard with 2 rows and 3 cols (on pins .. and ...)
#define INKBD4X3 1 // keyboard with 4 rows and 3 cols (on pins 6/7/8/9 and 2/3/4)
#define OUTSERIAL 0 // serial terminal (9600 baud)
#define OUTOLED 1 // 128x64 OLED i2c on SCL/SDA resp. A5/A4 (nano)

#define SHIFTHELP 1 // shows help screen on OLED if shift was pressed

// Serial header
#if OUTSERIAL || INSERIALKBD || INSERIALKBDASDW
#include <SPI.h>
#endif

// Initialize custom keyboard (4x3)
#if INKBD4X3
#include <Keypad.h>
const byte ROWS = 4; //four rows
const byte COLS = 3; //four columns
//define the cymbols on the buttons of the keypads
char hexaKeys[ROWS][COLS] = {
  {
    '7','8','9'
  }
  ,{
    '4','5','6'
  }
  ,{
    '1','2','3'
  }
  ,{
    '0','f',' '
  }
};
byte rowPins[ROWS] = {
  9, 8, 7, 6}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {
  4, 3, 2}; //connect to the column pinouts of the keypad
Keypad customKeypad = Keypad( makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS); 
#endif

// Initialize OLED
#if OUTOLED
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);
#endif

#define NUL 0x00
#define CTRLC 0x03
#define BS 0x08
#define TAB 0x09
#define LF 0x0a
#define CR 0x0d
#define DEL 0x7f
#define NULLCHAR '\0'

#define STRLEN 16 // maximal input string length
#define NEARZERO 1e-37 // if value lower use FIX format
#define ALMOSTSCI 1e9 // if value higher use SCI format
#define DEFAULTSCREENOFFTIME 10 // screenoff in seconds
#define DEFAULTDTOSTREFLAGS 255 //flags for dtostre()
#define DEFAULTFIX 4 // default number of FIX-digits
#define DEFAULTREGISTERVALUE 0.0 // default value for float-variables
#define DEFAULTTEXTLEN1 5 // max textlength on OLED using textsize 1
#define DEFAULTTEXTLEN2 10 // max textlength on OLED using textsize 2

#if OUTSERIAL
#define CLEARSTRING "                " // clears input string (=STRLEN)
#endif


// GLOBAL VARIABLES

float x,y,z,u,lastx,tmp,sto,rad,sx,sxx,sy,sxy;
byte fix,sn;
boolean stacklift;
char s[STRLEN]="";
#if OUTOLED || INKBD4X3
long startmillis;
byte screenofftime=DEFAULTSCREENOFFTIME; // screenoff in s till keypress
#endif

#if OUTOLED || INKBD4X3
byte menusetnr=0;
#endif

// SUBPROGRAMS

#if OUTOLED
void displayoff() { // clears display if screenofftime is extended
  if(millis()-startmillis>(long)screenofftime*1000) {
    display.clearDisplay();
    display.display();
  }
}
#endif

#if INKBD4X3
char getcustomkey(byte menunr) { // query keypress on custom keyboard
  char customKey;
  menusetnr=menunr;
  printstack();
  for(;;) { // query for key
#if OUTOLED
    displayoff();
#endif
    customKey=customKeypad.getKey();
    if(customKey) {
      startmillis=millis(); // reset startmillis
      return(customKey); // return key
    }
  }
}
#endif

char inchar() { // read character from in-channel
#if INSERIALKBD
  if(Serial.available()) return Serial.read();
  else return(NULLCHAR);
#elif INKBD4X3
  char key1=getcustomkey(0);
  if(key1=='f') {
    switch(getcustomkey(1)) {
    case ' ':
      return('+');
      break;
    case '3':
      return('-');
      break;
    case '6':
      return('*');
      break;
    case '9':
      return('/');
      break;
    case 'f': // decimal point
      return('.');
      break;
    case '0':
      return(DEL);
      break;
    case '1':
      return('e');
      break;
    case '2':
      return('#');
      break;
    case '4': // MENU
      switch(getcustomkey(4)) {
      case '4': // MENU MATH2
        switch(getcustomkey(44)) {
        case '4':
          return('L');
          break;
        case '5':
          return('g');
          break;
        case '6':
          return('I');
          break;
        case '7':
          return('l');
          break;
        case '8':
          return('k');
          break;
        case '9':
          return('J');
          break;
        default:
          return(NULLCHAR);
          break;
        }
        break;
      case '5': // MENU CONV
        switch(getcustomkey(45)) {
        case '4':
          return('W');
          break;
        case '5':
          return('y');
          break;
        case '6':
          return('H');
          break;
        case '7':
          return('w');
          break;
        case '8':
          return('Y');
          break;
        case '9':
          return('Z');
          break;
        default:
          return(NULLCHAR);
          break;
        }
        break;
      case '7': // MENU TRIG
        switch(getcustomkey(47)) {
        case '4':
          return('S');
          break;
        case '5':
          return('C');
          break;
        case '6':
          return('T');
          break;
        case '7':
          return('s');
          break;
        case '8':
          return('c');
          break;
        case '9':
          return('t');
          break;
        default:
          return(NULLCHAR);
          break;
        }
        break;
      case '8': // MENU HYP
        switch(getcustomkey(48)) {
        case '4':
          return('V');
          break;
        case '5':
          return('B');
          break;
        case '6':
          return('N');
          break;
        case '7':
          return('v');
          break;
        case '8':
          return('b');
          break;
        case '9':
          return('n');
          break;
        default:
          return(NULLCHAR);
          break;
        }
        break;
      case '9': // MENU STAT
        switch(getcustomkey(49)) {
        case '4':
          return('G');
          break;
        case '5':
          return('O');
          break;
        case '6':
          return('a');
          break;
        case '7':
          return('j');
          break;
        case '8':
          return('o');
          break;
        case '9':
          return('z');
          break;
        default:
          return(NULLCHAR);
          break;
        }
        break;
      default:
        return(NULLCHAR);
        break;
      }
      break;
    case '5': // SETTINGS
      switch(getcustomkey(5)) {
      case '4':
        return('D');
        break;
      case '5':
        return('f');
        break;
      case '6':
        return('!');
        break;
      case '7':
        return('d');
        break;
      case '8':
        return('F');
        break;
      case '9':
        return(':');
        break;
      default:
        return(NULLCHAR);
        break;
      }
      break;
    case '7': // STACK
      switch(getcustomkey(7)) {
      case '4':
        return('X');
        break;
      case '5':
        return('R');
        break;
      case '6':
        return('M');
        break;
      case '7':
        return('x');
        break;
      case '8':
        return('r');
        break;
      case '9':
        return('m');
        break;
      default:
        return(NULLCHAR);
        break;
      }
      break;
    case '8': // MATH
      switch(getcustomkey(8)) {
      case '4':
        return('P');
        break;
      case '5':
        return('U');
        break;
      case '6':
        return('p');
        break;
      case '7':
        return('Q');
        break;
      case '8':
        return('u');
        break;
      case '9':
        return('i');
        break;
      default:
        return(NULLCHAR);
        break;
      }
      break;
    default:
      return(NULLCHAR);
      break;
    }
  }
  else return(key1);
#else
  return(NULLCHAR);
#endif
}

#if OUTSERIAL
void outchar(byte c) { // write character to outchannel
  Serial.write(c);
}
#endif

#if OUTSERIAL
void newline() { // start newline at outchannel
  outchar(CR); 
  outchar(LF);
}
#endif

#if OUTOLED
void printoled() { // print all to OLED-display
  char st[STRLEN]="";
  byte i,j,flags=DEFAULTDTOSTREFLAGS;
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  if((abs(x)>=NEARZERO)&&(abs(x)<=exp(-(fix+1)*log(10)))||(abs(x)>=ALMOSTSCI)||(fix>=100)) { // print x in SCI
    if(fix>100||abs(x)<=exp(-(fix+1)*log(10))) dtostre(x,st,DEFAULTFIX,flags);
    else dtostre(x,st,fix,flags);
    for(j=0;j<strlen(st);j++) if(st[j]=='E')
      for(i=j;i<strlen(st);i++) st[i]=st[i+1];
    display.print(st);
  }
  else { // print x normally
    dtostrf(x,DEFAULTTEXTLEN2,fix,st);
    if(strlen(st)>DEFAULTTEXTLEN2) st[DEFAULTTEXTLEN2]=NULLCHAR;
    display.print(st);
  }
  display.setTextSize(4); // print input string
  display.setTextColor(WHITE);
  display.setCursor(0,16);
  if(strlen(s)>DEFAULTTEXTLEN1) { // input string too long
    display.print("<");
    for(i=strlen(s)-DEFAULTTEXTLEN1+1;i<strlen(s);i++) display.print(s[i]);
  }
  else
    display.print(s); // input string short enough

  if(menusetnr) {
    display.setCursor(0,48);
    display.setTextSize(1);
    switch(menusetnr) {
    case 1:
#if SHIFTHELP
      display.clearDisplay();
      display.setCursor(0,32);
      display.println(F("STACK    MATH      /"));
      display.println(F("MENU     SET       *"));
      display.println(F("EE       +/-       -"));
      display.println(F("DEL       .        +"));
#endif
      break;
    case 4:
      display.println(F("TRIG    HYP     STAT"));
      display.println(F("MATH2   CONV    ----"));
      break;
    case 44:
      display.println(F("LN      LOG     Py,x"));
      display.println(F("EXP     FACT    Cy,x"));
      break;
    case 45:
      display.println(F("->H.MS  ->R    ->RAD"));
      display.println(F("->H     ->P    ->DEG"));
      break;
    case 47:
      display.println(F("SIN     COS      TAN"));
      display.println(F("ASIN    ACOS    ATAN"));
      break;
    case 48:
      display.println(F("SINH    COSH    TANH"));
      display.println(F("ASINH   ACOSH  ATANH"));
      break;
    case 49:
      display.println(F("MEAN    A+BX   CLRSUM"));
      display.println(F("GAUSS   ->X,Y    ANNU"));
      break;
    case 5:
      display.println(F("DEG     SCI    CLOCK"));
      display.println(F("RAD     FIX   SCROFF"));
      break;
    case 7:
      display.println(F("SWAP    ROT+     RCL"));
      display.println(F("LASTx   ROT-     STO"));
      break;
    case 8:
      display.println(F("SQRT    SUM+    1/X"));
      display.println(F("POWER   SUM-     PI"));
      break;
    }
  }
  display.display();
}
void printoledstopwatch() {
  char st[STRLEN]="";
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  dtostrf(((x*100)-(int)(x*100))*100,DEFAULTTEXTLEN2,1,st);
  display.print(st);
  display.setTextSize(4);
  display.setTextColor(WHITE);
  display.setCursor(4,32);
  dtostrf(((int)(x*100))/100.0,DEFAULTTEXTLEN1,2,st);
  display.print(st);
  display.display();
}
#endif

void displaystring() { // write input string to outchannel
#if OUTSERIAL
  outchar(CR);
  Serial.write(CLEARSTRING);
  outchar(CR);
  Serial.print(s);
#elif OUTOLED
  printoled();
#endif
}

void printstack() { // print stack
#if OUTSERIAL
  char st[STRLEN];
  byte i,j,flags=DEFAULTDTOSTREFLAGS;
  if(fix>=100) {
    dtostre(z,st,fix-100,flags);
    for(j=0;j<strlen(st);j++) if(st[j]=='E')
      for(i=j;i<strlen(st);i++) st[i]=st[i+1];
    newline(); 
    Serial.print(st); 
    dtostre(y,st,fix-100,flags);
    for(j=0;j<strlen(st);j++) if(st[j]=='E')
      for(i=j;i<strlen(st);i++) st[i]=st[i+1];
    newline(); 
    Serial.print(st); 
    dtostre(x,st,fix-100,flags);
    for(j=0;j<strlen(st);j++) if(st[j]=='E')
      for(i=j;i<strlen(st);i++) st[i]=st[i+1];
    newline(); 
    Serial.print(st); 
  }   
  else {
    Serial.print(z,fix);
    newline(); 
    Serial.print(y,fix);
    newline();
    Serial.print(x,fix);
    newline();
  }
  newline();
  newline();
#elif OUTOLED
  printoled();
#endif
}

#if OUTSERIAL
void help() { // print help message
  newline();
  Serial.write("Qsqr|sqrt  Wh.ms|h   Eee      Rrot+|-  Ttan|arc Y->p|r    Usum+|sum- I1/x|comb Oa+bx|>xy Ppi|pwr"); 
  newline();
  Serial.write("Aannu      Ssin|a    Ddeg|rad Ffix|sci G!|gauss Hhlp|>deg Jmean|perm Klog|10^x Lln|exp");
  newline();
  Serial.write("Zcsum|>rad Xswp|lstx Ccos|arc Vsinh|ar Bcosh|ar Ntanh|ar  Mrcl|sto   #chs :clk !scroff SPCentr"); 
  newline();
  newline();
}
#endif

float hmstoh(float hms) { // hms->h
  //  return((int)hms+((int)(100*(hms-(int)hms)))/60.0+(100*(100*hms-(int)(100*hms)))/3600.0);
  unsigned long T=(hms+0.000005)*100000;
  int hh=(int)(T/100000),mm=(int)(T/1000)-hh*100,ss=T/10-hh*10000-mm*100,t=T-hh*100000-mm*1000-ss*10;
  return(hh+mm/60.0+ss/3600.0+t/36000.0);
}
float htohms(float h) { // h->hms
  h+=0.000005;
  return((int)h+((int)(60*(h-(int)h)))/100.0+(60*((60*(h-(int)h))-(int)(60*(h-(int)h))))/10000.0);
}

void push() { // push stack
  u=z; 
  z=y; 
  y=x; 
}

void pop() { // pull stack
  x=y; 
  y=z; 
  z=u; 
}


// SETUP and LOOP

void setup() {
  x=y=z=u=lastx=tmp=sto=sx=sxx=sy=sxy=DEFAULTREGISTERVALUE;
  sn=0;
  rad=M_PI/180.0;
  fix=DEFAULTFIX;
  stacklift=false;
#if OUTSERIAL || INSERIALKBD || INSERIALKBDASDW
  Serial.begin(9600);
#endif
#if OUTOLED
  startmillis=millis();
  display.begin(SSD1306_SWITCHCAPVCC, 0x3c);
  display.clearDisplay(); // Welcome message
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.println(F(" ARC 1.0"));
  display.println();
  display.println();
  display.println(F(" WELCOME"));
  display.display();
  delay(750);
  printstack();
#endif
}

void loop() {
  char key;
  byte i,ipos;
  boolean ise; // is EE (e or E) in input string?
  key=NULLCHAR; 
  key=inchar(); // read character from inchannel
  if(key!=NULLCHAR) { // process if character input occurs
#if OUTOLED
    startmillis=millis(); // reset startmillis
    printstack();
#endif
    if((key>='0')&&(key<='9')||(key=='.')||(key==',')||(key=='E')||(key=='e')) {
      if(((key=='E')||(key=='e'))&&(strlen(s)<=0)) strcat(s,"1"); 
      ;
      if(key==',') key='.'; // decimal komma equals decimal point
      if(strlen(s)<STRLEN) { // concatenate input character to string
        strcat(s," "); 
        s[strlen(s)-1]=key;
      }
      displaystring();
    }
    else if((strlen(s)>0)&&(key==DEL)) { // backspace was pressed
      s[strlen(s)-1]=NULLCHAR; 
      displaystring();
    }
    else if((strlen(s)>0)&&(key=='#')) { // change prefix of EE
      ise=false; 
      ipos=1;
      for(i=1;i<=strlen(s);i++) if((s[i]=='e')||(s[i]=='E')) {
        ise=true; 
        ipos=i;
      }
      if(ise) { // CHS of EE
        if(s[ipos+1]=='-') for(i=ipos+1;i<strlen(s);i++) s[i]=s[i+1];
        else {
          for(i=strlen(s);i>ipos;i--) s[i+1]=s[i]; 
          s[ipos+1]='-';
        }
        displaystring();
      }
      else { // CHS
        if(s[0]=='-') {
          for(i=0;i<strlen(s);i++) s[i]=s[i+1];
        }
        else {
          strcat(s," "); 
          for(i=strlen(s)-1;i>0;i--) s[i]=s[i-1]; 
          s[0]='-';
        }
        displaystring();
      }
    }
    else { // no digit entered
      if(strlen(s)>0) { // process input string with or without stacklift
        if(stacklift) {
          push();
          lastx=x;
          x=atof(s);
        }
        else {
          lastx=x;
          x=atof(s);
        }
      }
      switch(key) { // operation demanded
      case ' ': // ENTER
        push();
        stacklift=false; 
        break;
      case DEL: // CLx
        lastx=x;
        x=0;
        break;
      case '+': 
        lastx=x;
        x=x+y; 
        y=z; 
        z=u; 
        stacklift=true; 
        break; // operation
      case '-': 
        lastx=x;
        x=y-x; 
        y=z; 
        z=u; 
        stacklift=true; 
        break;
      case '*': 
        lastx=x;
        x=x*y; 
        y=z; 
        z=u; 
        stacklift=true; 
        break;
      case '/': 
        lastx=x;
        x=y/x; 
        y=z; 
        z=u; 
        stacklift=true; 
        break;
      case '#': // change prefix of register x
        lastx=x;
        x=-x; 
        break;
      case 'a': // calculate annuity
        lastx=x;
        x=(1-1/exp(x*log(1+y)))/y; 
        y=z; 
        z=u; 
        break;
      case 'b': // sinh
        lastx=x;
        x=(exp(x)+exp(-x))/2; 
        stacklift=true;
        break;
      case 'B': // asinh
        lastx=x;
        x=log(x+sqrt(x*x-1));
        break;
      case 'c': // cos
        lastx=x;
        x=cos(x*rad); 
        stacklift=true;
        break;
      case 'C': // acos
        lastx=x;
        x=(M_PI/2-atan(x/(sqrt(1-x*x))))/rad; 
        stacklift=true;
        break;
      case 'd': // -> deg
        rad=M_PI/180.0; 
        break;
      case 'D': // -> rad
        rad=1.0; 
        break;
      case 'f': // fix
        fix=(int)abs(x); 
        pop();
        stacklift=true; 
        break;
      case 'F': // SCI
        if(fix<100) fix+=100; 
        break;
      case 'g': // !
        for(tmp=i=1;i<=x;i++) tmp=tmp*i; 
        lastx=x;
        x=tmp;
        stacklift=true;
        break;
      case 'G': // GAUSS
        tmp=x;
        push();
        lastx=x;
        x=1/(1+exp(-0.07*tmp*tmp*tmp-1.6*tmp));
        push();
        x=1/sqrt(2*M_PI)*exp(-tmp*tmp/2);
        stacklift=true; 
        break;
#if OUTSERIAL
      case 'h': // help
        help(); 
        break;
#endif
      case 'H': // ->DEG
        lastx=x;
        x*=180/M_PI;
        stacklift=true;
        break;
      case 'i': // 1/x
        lastx=x;
        x=1.0/x; 
        stacklift=true;
        break;
      case 'I': // COMB
        lastx=x;
        tmp=x;
        x=1.0;
        for(i=y-tmp+1;i<=y;i++) x*=i;
        for(i=1;i<=tmp;i++) x/=i;
        y=z;
        z=u;
        stacklift=true;
        break;
      case 'j': // MEAN/STDDEV
        push();
        lastx=x;
        x=sqrt((sxx-sx*sx/sn)/(sn-1));
        push();
        x=sx/sn;
        stacklift=true; 
        break;
      case 'J': // PERM
        lastx=x;
        tmp=x;
        x=1.0;
        for(i=y-tmp+1;i<=y;i++) x*=i;
        y=z;
        z=u;
        stacklift=true;
        break;
      case 'k': // log
        lastx=x;
        x=log(x)/log(10); 
        stacklift=true;
        break;
      case 'K': // 10powX
        lastx=x;
        x=exp(x*log(10)); 
        stacklift=true;
        break;
      case 'l': // ln
        lastx=x;
        x=log(x); 
        stacklift=true;
        break;
      case 'L': // exp
        lastx=x;
        x=exp(x); 
        stacklift=true;
        break;
      case 'm': // RCL
        push();
        lastx=x;
        x=sto; 
        stacklift=true; 
        break;
      case 'M': // STO
        sto=x; 
        break;
      case 'n': // tanh
        lastx=x;
        x=(exp(x)-exp(-x))/(exp(x)+exp(-x)); 
        stacklift=true;
        break;
      case 'N': // atanh
        lastx=x;
        x=log(sqrt((1+x)/(1-x))); 
        break;
      case 'o': // A+BX
        push();
        lastx=x;
        x=tmp=(sxy/sn-sx/sn*sy/sn)/(sxx/sn-sx*sx/sn/sn);
        push();
        x=sy/sn-tmp*sx/sn;
        stacklift=true; 
        break;
      case 'O': // L.R. ->X,Y
        push();
        tmp=(sxy/sn-sx/sn*sy/sn)/(sxx/sn-sx*sx/sn/sn);
        lastx=x;
        x=sy/sn-tmp*sx/sn+tmp*y;
        push();
        x=(z-sy/sn+tmp*sx/sn)/tmp;
        stacklift=true; 
        break;
      case 'p': // PI
        if(stacklift) push();
        lastx=x;
        x=M_PI; 
        stacklift=true;
        break;
      case 'P': // YpowerX
        lastx=x;
        x=exp(x*log(y)); 
        y=z; 
        z=u; 
        stacklift=true;
        break;
      case 'q': // sqr
        lastx=x;
        x=x*x; 
        stacklift=true;
        break;
      case 'Q': // sqrt
        lastx=x;
        x=sqrt(x);
        stacklift=true;
        break;
      case 'r': // ROT down
        tmp=x; 
        pop();
        u=tmp;
        break;
      case 'R': // ROT up
        tmp=u; 
        push();
        lastx=x;
        x=tmp;
        break;
      case 's': // sin
        lastx=x;
        x=sin(x*rad); 
        stacklift=true;
        break;
      case 'S': // asin
        lastx=x;
        x=atan(x/(sqrt(1-x*x)))/rad; 
        break;
      case 't': // tan
        lastx=x;
        x=sin(x*rad)/cos(x*rad); 
        stacklift=true;
        break;
      case 'T': // atan
        lastx=x;
        x=atan(x)/rad; 
        stacklift=true;
        break;
      case 'u': // SUM+
        sn++;
        sx+=x;
        sy+=y;
        sxx+=x*x;
        sxy+=x*y;
        push();
        lastx=x;
        x=sn;
        stacklift=false; 
        break;
      case 'U': // SUM-
        sn--;
        sx-=x;
        sxx-=x*x;
        sy-=y;
        sxy-=x*y;
        push();
        lastx=x;
        x=sn;
        stacklift=false;
        break;
      case 'v': // sinh
        lastx=x;
        x=(exp(x)-exp(-x))/2; 
        stacklift=true;
        break;
      case 'V': // asinh
        lastx=x;
        x=log(x+sqrt(x*x-1)); 
        stacklift=true;
        break;
      case 'w': // ->H.MS
        lastx=x;
        x=htohms(x);
        //        x=(int)x+((int)(60*(x-(int)x)))/100.0+(60*((60*(x-(int)x))-(int)(60*(x-(int)x))))/10000;
        stacklift=true;
        break;
      case 'W': // ->H
        lastx=x;
        x=hmstoh(x);
        //        x=(int)x+((int)(100*(x-(int)x)))/60.0+(100*(100*x-(int)(100*x)))/3600;
        stacklift=true;
        break;
      case 'x': // swap
        tmp=x; 
        lastx=x;
        x=y; 
        y=tmp; 
        break;
      case 'X': // LASTx
        push();
        x=lastx;
        break;
      case 'y': // ->P
        tmp=y;
        y=atan(tmp/x)/rad;
        lastx=x;
        x=sqrt(x*x+tmp*tmp);
        stacklift=true; 
        break;
      case 'Y': // ->R
        tmp=y;
        y=x*sin(tmp*rad);
        lastx=x;
        x=x*cos(tmp*rad);
        stacklift=true; 
        break;
      case 'z': // CLRSUM
        sn=sx=sxx=sy=sxy=DEFAULTREGISTERVALUE;
        break;
      case 'Z': // ->RAD
        lastx=x;
        x*=M_PI/180; 
        stacklift=true;
        break;
#if OUTOLED
      case '!': // set screenofftime
        if((x>=2)&&(x<=200)) screenofftime=x; 
        pop();
        stacklift=true; 
        break;
#endif
      case ':': // stopwatch
        char customKey;
        long temp,stopmillis=millis();
        lastx=x;
        temp=hmstoh(abs(x))*3600000;
        for(;;) { // query for key
#if INKBD4X3
          customKey=customKeypad.getKey();
          if(customKey) {
            break;
          }
#elif INSERIALKBD
          key=inchar();
          if(key!=NULLCHAR) {
            break;
          }
#endif
          else {
            x=htohms((temp+millis()-stopmillis)/3600000.0);
            if(x>=24) {
              temp=0.0;
              stopmillis=millis();
            }
#if OUTOLED
            printoledstopwatch();
#elif OUTSERIAL
            Serial.println(x,4);
            delay(1000);
#endif
          }
        }
        startmillis=millis();
        break;
      }
      s[0]=NULLCHAR;
      printstack();
    }
  }
#if OUTOLED
  else displayoff();
#endif
}




















































































































































































