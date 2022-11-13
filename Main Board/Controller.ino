// Automatic Retractable clothsline - testing prototype
// Using Arduino pro mini 5V 16MHz

#include "DHTStable.h"

/*/ Pin Assignment;

  +---------------------+-------+-----------------------------+
  |      Assignment     | Pin   |           Remarks           |
  +---------------------+-------+-----------------------------+ */

const int Power = A0;
const int LDR = A1;    // 0= dark
const int Rain = 2;    // Input pullup (Rain=low)
const int Rotary = 3;  // Input pullup
const int swRight = 4; // Input pullup
#define DHT11_PIN 5
const int swLeft = 6; // Input pullup
const int ledLeft = 7;
const int ledRight = 8;
const int motorLeft = 9;
const int motorRight = 10;

const float foldInLimit = -3;
const float foldOutLimit = 3;

// Variables
bool ledLR = 0;
volatile bool startDir; // stratup Direction (0 = left)
bool currentDir;        // Current Direction (0 = left)
volatile bool motorDir; // Current motor Direction (0 = left)
unsigned long prevTime = 0;
int line_Length = 0;
volatile int step_count;
float tempSensor;
float humCalculated;
float humSensor;
float decisionLevel = 0;
float ldrLevel;
DHTStable DHT;

void setup()
{
    // Serial.begin(9600);

    // pinModes
    pinMode(Power, INPUT);
    pinMode(LDR, INPUT);
    pinMode(Rain, INPUT_PULLUP);
    pinMode(Rotary, INPUT_PULLUP);
    pinMode(swRight, INPUT_PULLUP);
    pinMode(swLeft, INPUT_PULLUP);
    pinMode(ledLeft, OUTPUT);
    pinMode(ledRight, OUTPUT);
    pinMode(motorLeft, OUTPUT);
    pinMode(motorRight, OUTPUT);

    delay(100);
    attachInterrupt(digitalPinToInterrupt(Rotary), step_counter, CHANGE);
    Power_check();
    startup();
    limitSet();
}

void loop()
{
    // put your main code here, to run repeatedly:
    if (!digitalRead(Rain))
    {
        foldIn();
        decisionLevel = -4;
    }
    if (!digitalRead(swRight))
    {
        foldOut();
    }
    if (!digitalRead(swLeft))
    {
        foldIn();
    }
    if (millis() - prevTime >= 10000)
    {
        prevTime = millis();
        if (DHT.read11(DHT11_PIN) != DHTLIB_OK)
        {
            error();
        }
        humSensor = DHT.getHumidity();
        tempSensor = DHT.getTemperature();
        humCalculated = (-0.0043 * sq(tempSensor)) + (0.639 * tempSensor) + 53.822;
        ldrLevel = float(analogRead(LDR)) / 1024;
        decisionLevel = decisionLevel + (0.25 * float(humCalculated - humSensor)) + (0.25 * (ldrLevel) * int(currentDir)) + (0.25 * (1 - ldrLevel) * int(!currentDir));
        if (decisionLevel < foldInLimit)
        {
            foldIn();
            decisionLevel = foldInLimit;
        }
        if (decisionLevel > foldOutLimit)
        {
            foldOut();
            decisionLevel = foldOutLimit;
        }
        // Serial.println(decisionLevel);
    }
}

void foldIn()
{
    if (!currentDir)
    {
        return;
    }
    tone(motorLeft, 2000, 300);
    delay(500);
    tone(motorLeft, 2000, 300);
    delay(500);
    step_count = 0;
    while (abs(step_count) < line_Length)
    {
        if (millis() - prevTime >= 500)
        {
            prevTime = millis();
            digitalWrite(ledLeft, ledLR);
            ledLR = !ledLR;
        }
        digitalWrite(motorRight, HIGH);
    }
    digitalWrite(motorRight, LOW);
    digitalWrite(ledLeft, LOW);
    currentDir = 0;
}

void foldOut()
{
    if (currentDir)
    {
        return;
    }
    Power_check();
    tone(motorLeft, 2000, 300);
    delay(500);
    tone(motorLeft, 2000, 300);
    delay(500);
    step_count = 0;
    while (abs(step_count) < line_Length)
    {
        if (millis() - prevTime >= 500)
        {
            prevTime = millis();
            digitalWrite(ledRight, ledLR);
            ledLR = !ledLR;
        }
        digitalWrite(motorLeft, HIGH);
    }
    digitalWrite(motorLeft, LOW);
    digitalWrite(ledRight, LOW);
    currentDir = 1;
}

void limitSet()
{
    step_count = 0;
    while (1)
    {
        Serial.println(step_count);
        if (millis() - prevTime >= 500)
        {
            prevTime = millis();
            digitalWrite(ledLeft, ledLR);
            digitalWrite(ledRight, ledLR);
            ledLR = !ledLR;
        }
        if (!digitalRead(swLeft) && !digitalRead(swRight) && (step_count > 0))
        {
            digitalWrite(motorLeft, LOW);
            digitalWrite(motorRight, LOW);
            line_Length = step_count;
            currentDir = !startDir;
            delay(500);
            digitalWrite(ledLeft, HIGH);
            digitalWrite(ledRight, HIGH);
            tone(motorLeft, 3000, 500);
            delay(500);
            digitalWrite(ledLeft, LOW);
            digitalWrite(ledRight, LOW);
            break;
        }
        else if ((!digitalRead(swLeft) && digitalRead(swRight)))
        {
            motorDir = 0;
            digitalWrite(motorLeft, motorDir);
            digitalWrite(motorRight, !motorDir);
        }
        else if ((digitalRead(swLeft) && !digitalRead(swRight)))
        {
            motorDir = 1;
            digitalWrite(motorLeft, motorDir);
            digitalWrite(motorRight, !motorDir);
        }
        else
        {
            digitalWrite(motorLeft, LOW);
            digitalWrite(motorRight, LOW);
        }
    }
}

void step_counter()
{
    if (startDir != motorDir)
    {
        step_count++;
    }
    else
    {
        step_count--;
    }
}

void startup()
{
    // Stratup Tone
    tone(motorLeft, 2000, 300);
    delay(300);
    tone(motorLeft, 2500, 300);
    delay(300);
    tone(motorLeft, 3000, 500);

    // Selecting startup position
    while (1)
    {
        if (millis() - prevTime >= 300)
        {
            prevTime = millis();
            digitalWrite(ledLeft, ledLR);
            digitalWrite(ledRight, !ledLR);
            ledLR = !ledLR;
        }
        if (!digitalRead(swLeft) || !digitalRead(swRight))
        {
            startDir = digitalRead(swLeft);
            digitalWrite(ledLeft, !startDir);
            digitalWrite(ledRight, startDir);
            tone(motorLeft, 2000, 500);
            delay(500);
            digitalWrite(ledLeft, LOW);
            digitalWrite(ledRight, LOW);
            break;
        }
    }
}

void Power_check()
{
    // Battry check
    if (analogRead(A0) < 750)
    { // if below 3.7V
        tone(motorLeft, 2000, 200);
        delay(400);
        tone(motorLeft, 2000, 200);
        delay(400);
        tone(motorLeft, 2000, 200);
        delay(400);
        error();
    }
}

void error()
{
    while (1)
    {
        if (millis() - prevTime >= 5000)
        {
            prevTime = millis();
            tone(motorLeft, 2000, 100);
        }
    }
}
