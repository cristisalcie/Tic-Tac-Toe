#include "Arduino.h"
#include "PCF8574.h"


// Arduino button pins
#define upButtonPin     14
#define leftButtonPin   15
#define downButtonPin   16
#define rightButtonPin  17
#define enterButtonPin  2


// Other constants
#define red  0
#define blue 1
#define selectionAnimationInterval 1000  // milliseconds


// When buttons pressed they become LOW.
bool upButtonState       { HIGH };
bool leftButtonState     { HIGH };
bool downButtonState     { HIGH };
bool rightButtonState    { HIGH };
bool enterButtonState    { HIGH };

bool upLastButtonState       { HIGH };
bool leftLastButtonState     { HIGH };
bool downLastButtonState     { HIGH };
bool rightLastButtonState    { HIGH };
bool enterLastButtonState    { HIGH };


// Led pins
const int ledPin[2][9] {
    {  // red leds
        P1, P3, P5,
        P7,  3,  5,
        7,   9, 11
    },
    {  // blue leds
        P0, P2, P4,
        P6,  4,  6,
        8 , 10, 12
    }
};


// Other global variables;
unsigned long currentMillis{ 0 };
unsigned long previousSelectionAnimationMillis{ 0 };
int8_t currentLedPos{ 0 };
uint8_t currentPlayerTurn{ 0 };  // Player 0 (red) or Player 1 (blue)

bool ledState[2][9] {
    {  // red leds
        LOW, LOW, LOW,
        LOW, LOW, LOW,
        LOW, LOW, LOW
    },
    {  // blue leds
        LOW, LOW, LOW,
        LOW, LOW, LOW,
        LOW, LOW, LOW
    }
};


// Set the I2C HEX address
PCF8574 pcf8574(0x20);


void startAnimation() {
    for (int i = 0; i < 2; ++i) {
        for (int j = 0; j < 9; ++j) {
            if (j < 4)
                pcf8574.digitalWrite(ledPin[i][j], HIGH);
            else
                digitalWrite(ledPin[i][j], HIGH);
        }
    }
    delay(2000);
    for (int i = 0; i < 2; ++i) {
        for (int j = 0; j < 9; ++j) {
            if (j < 4)
                pcf8574.digitalWrite(ledPin[i][j], LOW);
            else
                digitalWrite(ledPin[i][j], LOW);
        }
    }
    delay(1000);
}


void setup() {
    for (int i = 0; i < 2; ++i) {
        for (int j = 0; j < 9; ++j) {
            if (j < 4) {
                pcf8574.pinMode(ledPin[i][j], OUTPUT);
                pcf8574.digitalWrite(ledPin[i][j], ledState[i][j]);
            } else {
                pinMode(ledPin[i][j], OUTPUT);
                digitalWrite(ledPin[i][j], ledState[i][j]);
            }
        }
    }
    pcf8574.begin();  // todo: should this be before any digitalWrite?

    pinMode(upButtonPin, INPUT_PULLUP);
    pinMode(leftButtonPin, INPUT_PULLUP);
    pinMode(downButtonPin, INPUT_PULLUP);
    pinMode(rightButtonPin, INPUT_PULLUP);
    pinMode(enterButtonPin, INPUT_PULLUP);

    startAnimation();
    // Following line is used to be able to use Serial.println
    // In other words it makes the conexion between computer and board.
//    Serial.begin(9600);
}


void inputSelectionAnimation() {
    if (currentMillis - previousSelectionAnimationMillis
        >= selectionAnimationInterval) {
        previousSelectionAnimationMillis = currentMillis;
        ledState[currentPlayerTurn][currentLedPos] =
            !ledState[currentPlayerTurn][currentLedPos]; 
    }
}


bool redLedActive(int8_t ledPos) {
    return ledState[red][ledPos] == HIGH;
}
bool blueLedActive(int8_t ledPos) {
    return ledState[blue][ledPos] == HIGH;
}
bool ledActive(int8_t ledPos) {
    return redLedActive(ledPos) || blueLedActive(ledPos);
}


void inputController() {
    upButtonState   = digitalRead(upButtonPin);
    leftButtonState = digitalRead(leftButtonPin);
    downButtonState = digitalRead(downButtonPin);
    rightButtonState = digitalRead(rightButtonPin);
    enterButtonState = digitalRead(enterButtonPin);

    if (upButtonState != upLastButtonState && upButtonState == LOW) {
        ledState[currentPlayerTurn][currentLedPos] = LOW;
        while (true) {
            currentLedPos -= 3;  // Skip entire row.
            if (currentLedPos < 0)
                currentLedPos += 9;
            if (ledState[currentPlayerTurn][currentLedPos] == LOW)
                break;
        }
    }

    if (leftButtonState != leftLastButtonState && leftButtonState == LOW) {
        ledState[currentPlayerTurn][currentLedPos] = LOW;
        while (true) {
            currentLedPos -= 1;  // Skip column.
            if (currentLedPos == -1 || currentLedPos == 2 || currentLedPos == 5)
                currentLedPos += 3;
            if (ledState[currentPlayerTurn][currentLedPos] == LOW)
                break;
        }
    }
    
    if (downButtonState != downLastButtonState && downButtonState == LOW) {
        ledState[currentPlayerTurn][currentLedPos] = LOW;
        while (true) {
            currentLedPos += 3;  // Skip entire row.
            if (currentLedPos >= 9)
                currentLedPos -= 9;
            if (ledState[currentPlayerTurn][currentLedPos] == LOW)
                break;
        }
    }
    
    if (rightButtonState != rightLastButtonState && rightButtonState == LOW) {
        ledState[currentPlayerTurn][currentLedPos] = LOW;
        while (true) {
            currentLedPos += 1;  // Skip column.
            if (currentLedPos == 3 || currentLedPos == 6 || currentLedPos == 9)
                currentLedPos -= 3;
            if (ledState[currentPlayerTurn][currentLedPos] == LOW)
                break;
        }
    }
    
    if (enterButtonState != enterLastButtonState && enterButtonState == LOW) {
        if (ledState[!currentPlayerTurn][currentLedPos] == LOW) {
            ledState[currentPlayerTurn][currentLedPos] = HIGH;
            currentPlayerTurn = !currentPlayerTurn;  // Valid selection, change turn.
        }
        else {
            ledState[currentPlayerTurn][currentLedPos] = LOW;
        }

        for (int i = 0; i < 9; ++i) { // todo: what if there is no available spot
            if (!redLedActive(i) && !blueLedActive(i)) {
                currentLedPos = i;
                break;
            }
        }
    }

    inputSelectionAnimation();

    upLastButtonState   = upButtonState;
    leftLastButtonState = leftButtonState;
    downLastButtonState = downButtonState;
    rightLastButtonState = rightButtonState;
    enterLastButtonState = enterButtonState;
}


void loop() {
    currentMillis = millis();
    inputController();

    for (int i = 0; i < 2; ++i) {
        for (int j = 0; j < 9; ++j) {
            if (j < 4)
                pcf8574.digitalWrite(ledPin[i][j], ledState[i][j]);
            else
                digitalWrite(ledPin[i][j], ledState[i][j]);
        }
    }
}
