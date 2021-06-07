// trigger@applet.ifttt.com
#include <Adafruit_DHT.h>
#include <BH1750Lib.h>

// Constants
#define DHTPIN 2
#define DHTTYPE DHT11

#define DARK 10
#define OVERCAST 300

#define TWOHOURS 7200

// Creating instances from the relevant libraries.
BH1750Lib lightSensor;
DHT dht(DHTPIN, DHTTYPE);

// Pre-defined variables.
int motion = D3;
int ledGreen = D4;
int ledRed = D5;
int buzzer = D6;

int light;
int temperature;
int humidity;

int count = 0;

bool alarmArmed = false;

void setup()
{
    // Setting pin modes
    pinMode(motion, INPUT);
    pinMode(ledGreen, OUTPUT);
    pinMode(ledRed, OUTPUT);
    pinMode(buzzer, OUTPUT);
    
    // Sets the lightsensor
    lightSensor.begin(BH1750LIB_MODE_CONTINUOUSHIGHRES);
    // Sets the humidity temperature sensor
    dht.begin();
    // Subscribed to user's disarm and arm commands via IFTTT.
    Particle.subscribe("ArmDisarm", handleArm, MY_DEVICES);
    // LED is green at first because the system is disarmed.
    ledDisarm();
    // Buzzer is off at start.
    digitalWrite(buzzer, LOW);
}

void loop()
{
    // Loops for two hours before publishing weather data.
    while (count < TWOHOURS)
    {
        if (alarmArmed == true)
        {
            checkIntruder();
        }
        else
        {
            digitalWrite(buzzer, LOW);
        }

        count++;
        delay(1000);
    }

    count = 0;
    publishData();
}

// Function to check for intruder.
// If motion detected data is published to user via IFTTT.
void checkIntruder()
{
    if (digitalRead(motion) == HIGH)
    {
        Particle.publish("Motion", "Movement Detected");
        analogWrite(buzzer, 127, 5);
    }
}

// Function to return light data.
String readLight()
{
    light = lightSensor.lightLevel();
    String lightAmbient = "Surrounding Light: | ";

    if (light <= DARK)
    {
        lightAmbient = "Surrounding Light: Dark | ";
    }
    else if (light > DARK && light <= OVERCAST)
    {
        lightAmbient = "Surrounding Light: Overcast | ";
    }
    else if (light > OVERCAST)
    {
        lightAmbient = "Surrounding Light: Sunny | ";
    }
    return lightAmbient;
}

// Function to return temperature data.
String readTempHumid()
{
    String temperature = String(dht.getTempCelcius());
    String humidity = String(dht.getHumidity());
    
    String weather = "Temperature: "+temperature+" | Humidity: "+humidity;
    
    return weather;
}

// Function to publish weather data from the two functions above.
void publishData()
{
    String weather = readLight()+"\n"+readTempHumid();
    Particle.publish("Weather", weather);
}

// Function to turn green LED off and turn red LED on when arming.
void ledArm()
{
    digitalWrite(ledGreen, LOW);
    digitalWrite(ledRed, HIGH);
}

// Function to turn green LED on and turn red LED off when disarming.
void ledDisarm()
{
    digitalWrite(ledRed, LOW);
    digitalWrite(ledGreen, HIGH);
}

// Function to arm or disarm based on its current status when an 
// event the Particle is subscribed to publishes an occurrence under user's input.
void handleArm(const char *event, const char *data)
{
    alarmArmed = !alarmArmed;
    
    if (!alarmArmed)
    {
        ledDisarm();
    }
    else
    {
        ledArm();
    }
}