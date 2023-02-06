// MRCS Approach Indicator V1.0 SP Annunciator B K Draper 02-2023

// Define constants and variables

const int debnc = 500;
const int trpdly = 500;
const int releasdly = 3000;
const int Wbeltim = 1500;
const int Ebeltim = 1500;

unsigned long WDdbnc = 0;
unsigned long WHdbnc = 0;
unsigned long EDdbnc = 0;
unsigned long EHdbnc = 0;
unsigned long Wtrpdly = 0;
unsigned long Wreleasdly = 0;
unsigned long Wbelltim = 0;
unsigned long Etrpdly = 0;
unsigned long Ereleasdly = 0;
unsigned long Ebelltim = 0;

bool WDocc = false;
bool WHocc = false;
bool EDocc = false;
bool EHocc = false;
bool Wtrip = false;
bool Wclr = false;
bool Etrip = false;
bool Eclr = false;
bool Wappr = false;
bool Wleav = false;
bool Eappr = false;
bool Eleav = false;
bool Wlamp = false;
bool Wring = false;
bool Elamp = false;
bool Ering = false;
bool Woneshot = false;
bool Eoneshot = false;

void setup() {
  // put your setup code here, to run once:
// Define I/O pins

pinMode (A0, INPUT_PULLUP);
pinMode (A1, INPUT_PULLUP);
pinMode (A2, INPUT_PULLUP);
pinMode (A3, INPUT_PULLUP);
pinMode (A4, INPUT_PULLUP);
pinMode (A5, INPUT_PULLUP);



pinMode (2, OUTPUT);
digitalWrite (2, LOW);
pinMode (3, OUTPUT);
digitalWrite (4, LOW);
pinMode (4, OUTPUT);
digitalWrite (4, LOW);
pinMode (5, OUTPUT);
digitalWrite (5, LOW);
pinMode (10, OUTPUT);
digitalWrite (10, LOW);
pinMode (11, OUTPUT);
digitalWrite (11, LOW);
pinMode (12, OUTPUT);
digitalWrite (12, LOW);
pinMode (13, OUTPUT);
digitalWrite (13, LOW);

}

void loop() {
  // put your main code here, to run repeatedly:

  // Debounce inputs

if (digitalRead(A0) != WDocc) {WDdbnc = millis(); }
  if ((digitalRead(A0) == WDocc) && ((millis() - WDdbnc) > debnc)) {WDocc = !digitalRead(A0); }


if (digitalRead(A1) != WHocc) {WHdbnc = millis(); }
  if ((digitalRead(A1) == WHocc) && ((millis() - WHdbnc) > debnc)) {WHocc = !digitalRead(A1); }


if (digitalRead(A2) != EDocc) {EDdbnc = millis(); }
  if ((digitalRead(A2) == EDocc) && ((millis() - EDdbnc) > debnc)) {EDocc = !digitalRead(A2); }


if (digitalRead(A3) != EHocc) {EHdbnc = millis(); }
  if ((digitalRead(A3) == EHocc) && ((millis() - EHdbnc) > debnc)) {EHocc = !digitalRead(A3); }


// Delay West Distant Trip & Release (clear)

if (WDocc == Wtrip) {Wtrpdly = millis(); }
  if ((WDocc != Wtrip) && ((millis() - Wtrpdly) > trpdly)) {Wtrip = WDocc; }

if (WDocc == true) {Wreleasdly = millis(); Wclr = true;}
  if ((WDocc == false) && ((millis() - Wreleasdly) > releasdly)) {Wclr = false; Woneshot = false;}

// Direction Latch

if ((WHocc == true) && (WDocc == false) && (Wappr == false)) {Wleav = true; }
if ((WHocc == false) && (WDocc == true) && (Wleav == false)) {Wappr = true; }
if ((Wclr == false) && (WHocc == false)) {Wleav = false; Wappr = false; }

 // Set latch indicator LEDs

 if (Wleav == true) {digitalWrite (5, HIGH); }
 if (Wappr == true) {digitalWrite (4, HIGH); }
 if ((Wleav == false) && (Wappr == false)) {digitalWrite (4, LOW); digitalWrite (5, LOW); }

// Activate West Bell & Lamp

if ((Wtrip == true) && (Wappr == true) && (Woneshot == false)) {Wlamp = true; Wring = true; Woneshot = true;}

if (Wring == false) {Wbelltim = millis();}
  else if ((millis() - Wbeltim) > Wbelltim) {Wring = false;}

if (Wlamp == true) {digitalWrite (12, HIGH); }
  else {digitalWrite (12, LOW); }

if (Wring == true) {digitalWrite (10, HIGH); }
   else {digitalWrite (10, LOW); }

if (digitalRead(A4) == LOW ) {Wlamp = false; }



// Delay Eest Distant Trip & Release (clear)

if (EDocc == Etrip) {Etrpdly = millis(); }
  if ((EDocc != Etrip) && ((millis() - Etrpdly) > trpdly)) {Etrip = EDocc; }

if (EDocc == true) {Ereleasdly = millis(); Eclr = true;}
  if ((EDocc == false) && ((millis() - Ereleasdly) > releasdly)) {Eclr = false; Eoneshot = false;}

// Direction Latch

if ((EHocc == true) && (EDocc == false) && (Eappr == false)) {Eleav = true; }
if ((EHocc == false) && (EDocc == true) && (Eleav == false)) {Eappr = true; }
if ((Eclr == false) && (EHocc == false)) {Eleav = false; Eappr = false; }

 // Set latch indicator LEDs

 if (Eleav == true) {digitalWrite (3, HIGH); }
 if (Eappr == true) {digitalWrite (2, HIGH); }
 if ((Eleav == false) && (Eappr == false)) {digitalWrite (3, LOW); digitalWrite (2, LOW); }

// Activate East Bell & Lamp

if ((Etrip == true) && (Eappr == true) && (Eoneshot == false)) {Elamp = true; Ering = true; Eoneshot = true;}

if (Ering == false) {Ebelltim = millis();}
  else if ((millis() - Ebeltim) > Ebelltim) {Ering = false;}

if (Elamp == true) {digitalWrite (13, HIGH); }
  else {digitalWrite (13, LOW); }

if (Ering == true) {digitalWrite (11, HIGH); }
   else {digitalWrite (11, LOW); }

if (digitalRead(A5) == LOW ) {Elamp = false; }


  
// End of Loop
}
