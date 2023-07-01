/////////////////////////////////////////////////////////////////////////////////////////
//
//        
// Date         : 10.06.2023
// HW ID        : 320034000247393036363135
// HW OS        : 2.3.1
// SW Version   : 3.11.0
// File         : hfs-Ln3.ino
// Author       : Ackermann Yves
// Version      : A
//
//
/////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////
//
//         Define Loopstate:
//         Defines enumerator for all States in this Code 
//
/////////////////////////////////////////////////////////////////////////////////////////
enum eGlobalState {
  idleState,
  activatedState,
  passwordLoopState,
  alarmState
};
/////////////////////////////////////////////////////////////////////////////////////////
//
//          Global Variables
//         
//
/////////////////////////////////////////////////////////////////////////////////////////

// Definition of the Pins
#define BUZZER D3
#define LEDGREEN D5
#define LEDYELLOW D6
#define LEDRED D6
#define BUTTONGREEN A1
#define BUTTONBLUE A2
#define BUTTONYELLOW A3
#define DOORSENSOR A0

// Definition of the Password
#define PASSWORD 0b101001
// Definition of Time
#define TIMELOOP 30 // in second
system_tick_t timerEnd = millis();

// Set State of enum
eGlobalState globalState = idleState;

// global variable
int passwordState = 0;
bool buttonGreenLastState = false;
bool buttonYellowLastState = false;
bool buttonBlueLastState = false;
bool buttonGreenState = false;
bool buttonYellowState = false;
bool buttonBlueState = false;
/////////////////////////////////////////////////////////////////////////////////////////
//
//          Setup()
//
/////////////////////////////////////////////////////////////////////////////////////////

void setup() {
  pinMode(BUZZER, OUTPUT);
  pinMode(LEDGREEN, OUTPUT);
  pinMode(LEDYELLOW, OUTPUT);
  pinMode(LEDRED, OUTPUT);
  pinMode(BUTTONGREEN, INPUT_PULLDOWN);
  pinMode(BUTTONYELLOW, INPUT_PULLDOWN);
  pinMode(BUTTONBLUE, INPUT_PULLDOWN);
  pinMode(DOORSENSOR, INPUT_PULLUP);
  globalState = idleState;
}
/////////////////////////////////////////////////////////////////////////////////////////
//
//          Function decleration
//
/////////////////////////////////////////////////////////////////////////////////////////

void PlayTone(int frequency);
bool SensorRead();
int PasswordCheck(int buttonLeftTrigger, int buttonRightTrigger);
void GetInput();

/////////////////////////////////////////////////////////////////////////////////////////
//
//          Loop()
//          Basic Sequence like int main()
//
/////////////////////////////////////////////////////////////////////////////////////////

void loop (){
    
    // Checks if the Button were pressed in the previous cycle
    updateButtonState(BUTTONGREEN, buttonGreenLastState, buttonGreenState);
    updateButtonState(BUTTONYELLOW, buttonYellowLastState, buttonYellowState);
    updateButtonState(BUTTONBLUE, buttonBlueLastState, buttonBlueState);

    switch (globalState){
        
        case idleState:
            // begin of State : set every output to LOW
            // phase I        : if the activation button is pressed a beeptone will be played
            // transition     : acticatedState
            digitalWrite(LEDGREEN, LOW);
            digitalWrite(LEDYELLOW, LOW);
            digitalWrite(LEDRED, LOW);
            noTone(BUZZER);
            
            if (buttonGreenState){
                PlayTone(500);
                globalState = activatedState;
            }
        break;
        
        case activatedState:
            // begin of State : -
            // phase I        : checks sensor in case of paramenter change
            //                  if Value is higher than defined Range a beeptone will be activated
            // phase II       : set a timevalue defined amount higher than actual timevalue
            // transition     : passwordLoopState
            if (SensorRead()){
                digitalWrite(LEDYELLOW, HIGH);
                tone(BUZZER, 50);
                timerEnd = millis()+ (TIMELOOP*1000);
                globalState = passwordLoopState;
            }
        break;
        
        case passwordLoopState:
            // begin of State : hands over button for passwordCheck function
            // phase I        : compares entered passcode with defined passcode
            // phase II       : correct passcode -> beeptone will be turned off and a green led will light up
            // transistion I  : idle State
            // phase III      : timevalue above configurated Value -> beeptone will be turned off and a red led will light up
            // transition II  : alarm State
            passwordState = PasswordCheck(buttonYellowState, buttonBlueState, buttonGreenState);
            if (passwordState == 1){
                noTone(BUZZER);
                digitalWrite(LEDYELLOW, LOW);
                digitalWrite(LEDGREEN, HIGH);
                PlayTone(100);
                delay(100);
                PlayTone(200);
                
                globalState = idleState;
            }
            if (millis() > timerEnd){
                digitalWrite(LEDRED, HIGH);
                noTone(BUZZER);         
                globalState = alarmState;
            }
        break;
    
        case alarmState:
            // begin of State : playes short beeptone in 2 different tunes
            // phase I        : when right buttoncombination is pressed advance to next State
            // transition     : idle State
            PlayTone(100);
            delay(100);
            PlayTone(200);
             if ((digitalRead(BUTTONYELLOW) == HIGH) &&      // Option to reset the Microcontroller if alarmState is active
                (digitalRead(BUTTONBLUE) == HIGH)) {
                globalState = idleState;
             }
        break;
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
//
//          Function
//
//////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////
//
//          SensorRead()
//          if die analog value is above threshold a 2 beeptone will be played
//          and a boolean will be returned
//
/////////////////////////////////////////////////////////////////////////////////////////

bool SensorRead(){
    bool SensorActivated = false;
    if (analogRead(DOORSENSOR) > 500) {
        delay(20);
        SensorActivated = true;
        PlayTone(100); //
        delay(100);      //
        PlayTone(200);  //
    }
    return SensorActivated;
}

/////////////////////////////////////////////////////////////////////////////////////////
//
//          playTone(int speaker, int frequency)
//          generates a short beeptone in different frequencies
//
/////////////////////////////////////////////////////////////////////////////////////////

void PlayTone(int frequency) {
  tone(BUZZER, frequency);
  delay(100);
  noTone(BUZZER);
}

/////////////////////////////////////////////////////////////////////////////////////////
//
//          int PasswordCheck(bool buttonLeftTrigger, bool buttonRightTrigger)
//          Checks if the Button were pressed in correct order (Password correct)  
//
/////////////////////////////////////////////////////////////////////////////////////////

int PasswordCheck(bool buttonLeftTrigger, bool buttonRightTrigger,) {
    static int counter = 0;
    static int enteredPassword = 0b000000;
    
        if (buttonRightTrigger || buttonLeftTrigger){
            enteredPassword <<= 2; // Shift password to make space for new entry
            if (buttonLeftTrigger) {
                enteredPassword |= 0b10; // Add combination for left button
            } else {
                enteredPassword |= 0b01; // Add combination for right button
            }
            digitalWrite(LEDYELLOW, LOW); // is used to signalise that the system acknowleged a button input
            delay(20);
            digitalWrite(LEDYELLOW, HIGH);
            counter++;
            if (counter == 3 ) {
                if (enteredPassword == PASSWORD){
                    counter = 0;
                    enteredPassword = 0b000000;
                    return 1; // Password is correct
                } else {
                    counter = 0;
                    enteredPassword = 0b000000;
                    digitalWrite(LEDYELLOW, LOW); // is used to signalise that the password is reseted
                    delay(300);
                    digitalWrite(LEDYELLOW, HIGH);
                    return -1; // Wrong password
                }   
            }
        }
     return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
//
//          void updateButtonState(int buttonPin, bool &lastState, bool &buttonState)
//          compares the Buttonstate from the previous cycle with the current buttonsignal
//          to prevent multiple Inputs
//
/////////////////////////////////////////////////////////////////////////////////////////

void updateButtonState(int buttonPin, bool &lastState, bool &buttonState)
{
    bool currentButtonState = !digitalRead(buttonPin);
    
    if(currentButtonState != lastState){
        buttonState = currentButtonState;
    }
    else{
        buttonState = false;
    }
    lastState = currentButtonState;
}