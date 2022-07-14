#define Version "Semaphore 4.1BKD Slow Move 2022/06/21 modified by Barry Draper"
//Adds show movement of servos and option for block signal logic
//#define Version "Semaphore 2.4 2021/08/02 14:30"
//   -- documentation re LoopDly
//   -- documentation re BounceArc set to 0
// "Semaphore 2.2 2021/05/02 12:00"
//   -- Add Bounce
// "Semaphore 2.1 2021/04/18 22:00"
//
// Controls Dual Semaphore Controller board from MRCS
// Color    Resist          analog        button       digital      index
// Red2      R1                A0           S4            D4          3
// Yel2      R2                A1           S5            D3          4
// Grn2      R3                A2           S6            D2          5
// Red1      R4                A3           S1            D7          0
// Yel1      R5                A6           S2            D6          1
// Grn1      R6                A7           S3            D5          2
#include <Servo.h>

// How many signals/servos to process?
#define NumSigs 2
// FullArc - the maximum degrees of arc to travel
#define FullArc 130

//Chg 4.1 Add slow speed and Block Signal mode
#define Blksig true  //activates ABS logic for inputs - Green is default
#define SpdToGrn 50   //Speed moving toward Green; higher is slower
#define SpdToRed 10  //Speed moving toward Red; higher is slower


// Chg 2.2 vvvvvvvvvvvvv
// BounceArc - factor for arc of bounce 
//    - (green-red)/BounceArc
//    - 0 means no bounce, 18 max
#define BounceArc 9
//    BounceDly - delay between moves
#define BounceDly 70
// Chg 2.2 ^^^^^^^^^^^^
// MinPotChg - the minimum ohm change we will act on
#define MinPotChg 5
// HoldMS - time before looking for a new signal 
#define HoldMS 10
// latch = true - hold last active button
#define Latch false
// mirror Leave false - no longer needed
#define Mirror false

// main loop delay - sample rate
#define LoopDly 10

// debugging
#define Trace 0
// if debugging, make the LoopDly large

// note ordering:    1R 1Y 1G 2R 2Y 2G
int const PotPins[6] = {A3,A6,A7,A0,A1,A2};
int const ButPins[6] = { 7, 6, 5, 4, 3, 2};
#define RED 0
#define YEL 1
#define GRN 2
int const LEDs [2] = {9,10};// note: high = off
bool flash = 0;

// servos and pins
Servo SemiP[2];
int const ServPins[2] = {12,11};
int       ServState [2]; // red/yellow/green
int       Posn [2];      // pot position
long int  PosnTM [2];    // last position time change

// Chg 4.1 Soeed control variables
int Stop1, Stop2, bladeset1, bladeset2, bladepos1, bladepos2;
long int Time1 = millis();
long int Time2 = millis();
long int heartbeat = millis();
int R1, R2, Y1, Y2, G1, G2, b1, b2;
int CWspeed1, CWspeed2, CCWspeed1, CCWspeed2;

// #################################################
void LEDlert(int Patrn, int Repet) {
  // specific for the Dual Semaphore Controller
  // Patrn (0,1) for heart beat
  //   0 = slow blink - active
  //   1 = rapid blink - working
  //   2 = fast blink - problem
  int TurnOn = LOW;
  int TurnOff = HIGH;

  int ii,jj, Iner;
  Iner = (3-Patrn)*100;
  
  for (jj = 0; jj < Repet; ++jj){  
  for (ii = 0; ii < Patrn+1; ++ii){  
    digitalWrite(LEDs[0], TurnOn);
    delay(Iner);
    digitalWrite(LEDs[1], TurnOn);
    digitalWrite(LEDs[0], TurnOff);
    delay(Iner);
    digitalWrite(LEDs[1], TurnOff);
    if (jj != Repet-1){delay(Iner);}
  }// for ii
    if (jj != Repet-1){delay(Iner*5);}
  }// for jj
}// LEDlert

// Chg 2.2 vvvvvvvvvvvvv
// #################################################
void BounceRed(int Iset, int Redval) {
int Ioff, nxtpos, Grnval, bdly;
  // return to Red causes bounce
  #if (BounceArc > 0) 
  Ioff = Iset * 3;
  Grnval = analogRead(PotPins[Ioff+GRN]);
  if (Ioff == 0 && Mirror){Grnval = 1023 - Grnval;}
  Grnval = map(Grnval, 0, 1023, 0, FullArc);
  nxtpos = (Grnval - Redval)/BounceArc;
    if (Trace > 0){
      Serial.print("Bounce G: ");
      Serial.print(Grnval);
      Serial.print(" R:");
      Serial.print(Redval);
      Serial.print(" nxt: ");
      Serial.println(nxtpos);
    }// Trace
  do {
    if (Trace > 0){
      Serial.print("Bounce to ");
      Serial.println(Redval + nxtpos);
    }// Trace
  delay (BounceDly);
    SemiP[Iset].write(Redval + nxtpos);
  delay (BounceDly);
    SemiP[Iset].write(Redval);
    nxtpos /= BounceArc;
  } while (nxtpos != 0);
  #endif
}// BounceRed
// Chg 2.2 ^^^^^^^^^^^^


  // #################################################
void setup() {
  long int TuneTM;
  int PotPos[6];
  // Tuning - time watching for a pot change
  #define Tuning 15000
  
  int ii, jj, Cnt, MPin, PotVal;
  if (Trace > 0){
      Serial.begin(9600);
      Serial.println(Version);
  }// Trace
  // setup servos and variables
  for (ii = 0; ii < 6; ++ii){
    pinMode(PotPins[ii], INPUT);
    pinMode(ButPins[ii], INPUT_PULLUP);
  }// for ii
  for (ii = 0; ii < NumSigs; ++ii){
    SemiP[ii].attach(ServPins[ii]);
  // read red pot & position
    Posn[ii] = analogRead(PotPins[ii*3]);
    if (ii == 0 && Mirror){Posn[ii] = 1023 - Posn[ii];}
    SemiP[ii].write(map(Posn[ii], 0, 1023, 0, FullArc));
    PosnTM[ii] = millis();  
    ServState[ii] = ii * 3; 
    if (Trace > 0){
      Serial.print("Servo ");
      Serial.print(ii);
      Serial.print(" pot ");
      Serial.println(Posn[ii]);
    }
  }// for ii
  for (ii = 0; ii < 2; ++ii){
    pinMode(LEDs[ii], OUTPUT);
    digitalWrite(LEDs[ii], HIGH);
  }// for ii
  if (Trace > 0){// light both for startup
    LEDlert(0,1); // 
  }// Trace
  // initialize pot position
  for (jj = 0; jj < 6; ++jj){
    PotPos[jj] = analogRead(PotPins[jj]);
  }// jj
  TuneTM = millis();
  do {
    for (jj = 0; jj < 6; ++jj){
      PotVal = analogRead(PotPins[jj]);
      if (abs(PotVal - PotPos[jj]) > MinPotChg){
        TuneTM = millis();
        PotPos[jj] = PotVal;
        ii = jj / 3;
        if (ii == 0 && Mirror){PotVal = 1023 - PotVal;}
        SemiP[ii].write(map(PotVal, 0, 1023, 0, FullArc));
        }// if pot moved
    }// jj
    LEDlert(1,1); // notify tuning time
  } while (TuneTM + Tuning > millis());

  //Chg 4.0 Reads pot positions for Blksig mode
  R1 = map(analogRead(PotPins[0]), 0, 1023, 0, FullArc);
  Y1 = map(analogRead(PotPins[1]), 0, 1023, 0, FullArc);
  G1 = map(analogRead(PotPins[2]), 0, 1023, 0, FullArc);
  R2 = map(analogRead(PotPins[3]), 0, 1023, 0, FullArc);
  Y2 = map(analogRead(PotPins[4]), 0, 1023, 0, FullArc);
  G2 = map(analogRead(PotPins[5]), 0, 1023, 0, FullArc);
   if (Blksig == true){
  bladepos1 = R2;
  bladepos2 = R1;}

  if (R2 > G2){
    CWspeed1 = SpdToGrn;
    CCWspeed1 = SpdToRed;}
    else {
      CWspeed1 = SpdToRed;
      CCWspeed1 = SpdToGrn;}

  if (R1 > G1){
    CWspeed2 = SpdToGrn;
    CCWspeed2 = SpdToRed;}
    else {
      CWspeed2 = SpdToRed;
      CCWspeed2 = SpdToGrn;}
 // Sets speed values int proper servo direction
 

 
  } // end setup
  // #################################################

  // #################################################
void loop() {
  int iset, ioff, ii, jj, ibutn, potval, mapval;

  //Add 4.0 reset for Millis overflow
  if (millis() <= 1000) {Time1 = Time2 = heartbeat = millis();}

  for (iset = 0; iset < NumSigs; ++iset){
    // check timer and skip if not yet expired
    if (PosnTM[iset] + HoldMS > millis()|| Blksig == true){continue;}
    
    ibutn = ioff = iset * 3; // default to red
    jj = 0; // # of buttons pushed
    // loop through buttons
    for (ii = ioff; ii < ioff +3; ++ii){
      potval = digitalRead(ButPins[ii]);
        if (Trace > 1){
          Serial.print("Button ");
          Serial.print(ii);
          Serial.print(" pin ");
          Serial.print(ButPins[ii]);
          Serial.print(" value ");
          Serial.println(potval);
        }// Trace
      if (potval == LOW){
        ++jj;
        ibutn = ii;
        potval = analogRead(PotPins[ibutn]);
        if (iset == 0 && Mirror){potval = 1023 - potval;}
        mapval = map(potval, 0, 1023, 0, FullArc);
        if (Trace > 0){
          Serial.print("Semaphore ");
          Serial.print(iset);
          Serial.print(" Button ");
          Serial.print(ii);
          Serial.println(" active");
         }// Trace
      }
    }// for ii
    // done scanning buttons
    if (jj > 1){LEDlert(2,10);}// if jj
    if (Trace > 0){
          Serial.print("Set ");
          Serial.print(iset);
          Serial.print(" actives ");
          Serial.print(jj);
          Serial.print(" ibutn ");
          Serial.print(ibutn);
          Serial.print(" state ");
          Serial.println(ServState[iset]);
      } // Trace

    if (jj == 0 && Latch) {ibutn = ServState[iset];}
    potval = analogRead(PotPins[ibutn]);
    if (iset == 0 && Mirror){potval = 1023 - potval;}
    mapval = map(potval, 0, 1023, 0, FullArc);
// button pushed or latch false
  if (ibutn != ServState[iset]
         && 
       (abs(potval - Posn[iset]) > MinPotChg)
       )
     {
      if (Trace > 0){
          Serial.print("Set ");
          Serial.print(iset);
          Serial.print(" state ");
          Serial.print(ServState[iset]);
          Serial.print(" button ");
          Serial.print(ibutn);
          Serial.print(" pot ");
          Serial.print(potval);
          Serial.print(" moved to ");
          Serial.print(mapval);
          Serial.println(" angle ");        
       } // Trace
     // new button state
      ServState[iset] = ibutn;
      Posn[iset]      = potval;
      PosnTM[iset]    = millis();
      //SemiP[iset].write(mapval); //Chg 4.1
      if (iset == 0){bladeset2 = mapval;}
      if (iset == 1){bladeset1 = mapval;}
    
// Chg 2.2 vvvvvvvvvvvvv
   // if (ibutn == ioff) // RED - call BounceRed
    //{BounceRed(iset, mapval);}
// Chg 2.2 ^^^^^^^^^^^^^
     }// conditions
  }// for iset
  
  //Chg 4.0 Button read and ABS logic.  bladeset is desired position for servo to move to
if (Blksig == true && digitalRead(ButPins[3]) == 0){bladeset1 = R2;}
if (Blksig == true && digitalRead(ButPins[3]) == 1 && digitalRead(ButPins[4]) == 0){bladeset1 = Y2;}
if (Blksig == true && digitalRead(ButPins[3]) == 1 && digitalRead(ButPins[4]) == 1){bladeset1 = G2;}

if (Blksig == true && digitalRead(ButPins[0]) == 0){bladeset2 = R1;}
if (Blksig == true && digitalRead(ButPins[0]) == 1 && digitalRead(ButPins[1]) == 0){bladeset2 = Y1;}
if (Blksig == true && digitalRead(ButPins[0]) == 1 && digitalRead(ButPins[1]) == 1){bladeset2 = G1;}

  //Chg 4.0 Compares bladeset and bladepos and moves servo in one degree steps if needed
  
  if (bladeset1 != R2) {b1 = 0;}
  if (bladeset1 < bladepos1 && Time1 < millis()){
   Time1 = millis()+ CWspeed1; 
    bladepos1 = bladepos1 - 1;
    SemiP[1].write(bladepos1);}
    
  if (bladeset1 > bladepos1 && Time1 < millis()){
    Time1 = millis() + CCWspeed1;
    bladepos1 = bladepos1 + 1;
    SemiP[1].write(bladepos1);}
    
    if (bladepos1 == R2 && b1 == 0) {BounceRed(1, R2); b1 = 1;}
    
   if (bladeset2 != R1) {b2 = 0;}
   if (bladeset2 < bladepos2 && Time2 < millis()){
    Time2 = millis()+ CWspeed2; 
    bladepos2 = bladepos2 - 1;
    SemiP[0].write(bladepos2);}
     
  if (bladeset2 > bladepos2 && Time2 < millis()){
    Time2 = millis() + CCWspeed2;
    bladepos2 = bladepos2 + 1;
    SemiP[0].write(bladepos2);
    } 
    
    if (bladepos2 == R1 && b2 == 0) {BounceRed(0, R1); b2 = 1;}

//Chg 4.0  Runtime blue LED heartbeat to avoid slow down using delay

if (millis()- heartbeat >= 900){
  heartbeat = millis();
  if (flash == 0){
    flash = 1;
    digitalWrite(LEDs[0], 0);
    digitalWrite(LEDs[1], 1);} 
    else {flash = 0;
    digitalWrite(LEDs[1], 0);
    digitalWrite(LEDs[0], 1);}} 
  
}

  //if (LoopDly > 0){
    //LEDlert(0,1); // heartbeat  
    //delay(LoopDly);}
 // loop
