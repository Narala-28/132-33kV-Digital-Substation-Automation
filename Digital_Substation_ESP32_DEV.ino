/************************************************************
 * PROJECT : 132/33 kV DIGITAL SUBSTATION
 * STATION : SOUTH SUBSTATION
 * MODULE  : SOUTH DEV COMMUNICATION / EF CONTROLLER
 * VERSION : 1.1.0
 * DEVELOPED BY : Narala Heshma Sree
 * PURPOSE :
 *   - Earth Fault Detection
 *   - MQTT Communication (HiveMQ Cloud)
 *   - UART Communication with ESP32-S3
 *   - Annunciation & Mimic Control
 ************************************************************/

//=========================================================
// Libraries
//=========================================================

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>

//=========================================================
// WiFi & MQTT Objects
//=========================================================

WiFiClientSecure espClient;
PubSubClient client(espClient);

//=========================================================
// WiFi Configuration
//=========================================================

const char* ssid = "YOUR_WIFI_SSID";
const char* wifiPassword = "YOUR_WIFI_PASSWORD";

//=========================================================
// HiveMQ Cloud Configuration
//=========================================================

const char* mqtt_server = "YOUR_HIVEMQ_SERVER";

const int mqtt_port = 8883;

const char* mqtt_user = "YOUR_MQTT_USERNAME";
const char* mqtt_password = "YOUR_MQTT_PASSWORD";

//=========================================================
// Earth Fault Analog Inputs
//=========================================================

// 132kV Feeders
#define EF_FDR1_HV     32
#define EF_FDR2_HV     33

// Power Transformer-1
#define EF_PTR1_HV     34
#define EF_PTR1_LV     35

// Power Transformer-2
#define EF_PTR2_HV     36
#define EF_PTR2_LV     39

// 33kV Feeders
#define EF_33FDR1      27
#define EF_33FDR2      26      // South Board (Intentional)

//=========================================================
// Annunciation Outputs
//=========================================================

#define ANN_EF_FDR1        4
#define ANN_EF_FDR2        5

#define ANN_EF_PTR1_HV    13    // South Board (Intentional)
#define ANN_EF_PTR1_LV    14

#define ANN_EF_PTR2_HV    18
#define ANN_EF_PTR2_LV    19

#define ANN_EF_33FDR1     21
#define ANN_EF_33FDR2     22

#define ANN_COMMON        23

//=========================================================
// Earth Fault Status Flags
//=========================================================

bool FDR1_EF_SENT = false;
bool FDR2_EF_SENT = false;

bool PTR1_HVEF_SENT = false;
bool PTR1_LVEF_SENT = false;

bool PTR2_HVEF_SENT = false;
bool PTR2_LVEF_SENT = false;

bool F33_1_EF_SENT = false;
bool F33_2_EF_SENT = false;

//=========================================================
// UART Communication
//=========================================================

HardwareSerial S3Serial(2);

//=========================================================
// Function Prototypes
//=========================================================

void setup_wifi();
void reconnect();
void callback(char* topic, byte* payload, unsigned int length);
void protectionLogic();

//=========================================================
// Setup Function
//=========================================================

void setup()
{
    //-----------------------------------------------------
    // Serial Monitor
    //-----------------------------------------------------

    Serial.begin(115200);
    Serial.println();
    Serial.println("========================================");
    Serial.println(" SOUTH SUBSTATION - ESP32 DEV");
    Serial.println(" Communication Controller Started");
    Serial.println("========================================");

    //-----------------------------------------------------
    // WiFi Connection
    //-----------------------------------------------------

    setup_wifi();

    //-----------------------------------------------------
    // MQTT Configuration
    //-----------------------------------------------------

    // Prototype TLS configuration. Replace with CA validation for production deployment.
    espClient.setInsecure();

    client.setServer(mqtt_server, mqtt_port);

    client.setCallback(callback);

    //-----------------------------------------------------
    // UART Communication with ESP32-S3
    //-----------------------------------------------------

    S3Serial.begin(115200, SERIAL_8N1, 26, 27);

    //-----------------------------------------------------
    // Annunciation Outputs
    //-----------------------------------------------------

    pinMode(ANN_EF_FDR1, OUTPUT);
    pinMode(ANN_EF_FDR2, OUTPUT);

    pinMode(ANN_EF_PTR1_HV, OUTPUT);
    pinMode(ANN_EF_PTR1_LV, OUTPUT);

    pinMode(ANN_EF_PTR2_HV, OUTPUT);
    pinMode(ANN_EF_PTR2_LV, OUTPUT);

    pinMode(ANN_EF_33FDR1, OUTPUT);
    pinMode(ANN_EF_33FDR2, OUTPUT);

    pinMode(ANN_COMMON, OUTPUT);

    //-----------------------------------------------------
    // Initialize Outputs
    //-----------------------------------------------------

    digitalWrite(ANN_EF_FDR1, LOW);
    digitalWrite(ANN_EF_FDR2, LOW);

    digitalWrite(ANN_EF_PTR1_HV, LOW);
    digitalWrite(ANN_EF_PTR1_LV, LOW);

    digitalWrite(ANN_EF_PTR2_HV, LOW);
    digitalWrite(ANN_EF_PTR2_LV, LOW);

    digitalWrite(ANN_EF_33FDR1, LOW);
    digitalWrite(ANN_EF_33FDR2, LOW);

    digitalWrite(ANN_COMMON, LOW);

    Serial.println("System Initialization Completed");
}

//=========================================================
// WiFi Connection Function
//=========================================================

void setup_wifi()
{
    Serial.println();
    Serial.print("Connecting to WiFi");

    WiFi.begin(ssid, wifiPassword);

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }

    Serial.println();

    Serial.println("----------------------------------------");
    Serial.println("WiFi Connected Successfully");
    Serial.print("IP Address : ");
    Serial.println(WiFi.localIP());
    Serial.println("----------------------------------------");
}
//=============================================================
// Function : callback()
// Purpose  : Process MQTT Messages Received from
//            North 132/33kV Digital Substation
//=============================================================

void callback(char* topic, byte* payload, unsigned int length)
{
    String message = "";

    for (int i = 0; i < length; i++)
    {
        message += (char)payload[i];
    }

    String Topic = String(topic);
    Topic.trim();

    Serial.println();
    Serial.println("******** MQTT CALLBACK ********");

    Serial.print("Topic   : ");
    Serial.println(Topic);

    Serial.print("Payload : ");
    Serial.println(message);

    //---------------------------------------------------------
    // Protection Logic
    //---------------------------------------------------------

    if (Topic == "North132/EF/FDR1")
    {
        Serial.println("North FDR1 Earth Fault Received");
    }
    else if (Topic == "North132/EF/FDR2")
    {
        Serial.println("North FDR2 Earth Fault Received");
    }
    else if (Topic == "North132/EF/PTR1HV")
    {
        Serial.println("North PTR1 HV Earth Fault Received");
    }
    else if (Topic == "North132/EF/PTR1LV")
    {
        Serial.println("North PTR1 LV Earth Fault Received");
    }
    else if (Topic == "North132/EF/PTR2HV")
    {
        Serial.println("North PTR2 HV Earth Fault Received");
    }
    else if (Topic == "North132/EF/PTR2LV")
    {
        Serial.println("North PTR2 LV Earth Fault Received");
    }
    else if (Topic == "North132/EF/33FDR1")
    {
        Serial.println("North 33kV Feeder-1 Earth Fault Received");
    }
    else if (Topic == "North132/EF/33FDR2")
    {
        Serial.println("North 33kV Feeder-2 Earth Fault Received");
    }

    //---------------------------------------------------------
    // Distance Protection
    //---------------------------------------------------------

    else if (Topic == "North132/DIST/FDR1/Z1")
    {
        Serial.println("North FDR1 Zone-1 Trip");
    }
    else if (Topic == "North132/DIST/FDR1/Z2")
    {
        Serial.println("North FDR1 Zone-2 Trip");
    }
    else if (Topic == "North132/DIST/FDR1/Z3")
    {
        Serial.println("North FDR1 Zone-3 Trip");
    }
    else if (Topic == "North132/DIST/FDR2/Z1")
    {
        Serial.println("North FDR2 Zone-1 Trip");
    }
    else if (Topic == "North132/DIST/FDR2/Z2")
    {
        Serial.println("North FDR2 Zone-2 Trip");
    }
    else if (Topic == "North132/DIST/FDR2/Z3")
    {
        Serial.println("North FDR2 Zone-3 Trip");
    }

    //---------------------------------------------------------
    // Feeder Over Current Protection
    //---------------------------------------------------------
    Serial.println("========== MQTT ==========");
Serial.print("Topic = <");
Serial.print(Topic);
Serial.println(">");

Serial.print("Length = ");
Serial.println(Topic.length());

Serial.print("Payload = <");
Serial.print(message);
Serial.println(">");

     if (Topic == "North132/OC/FDR1")
    {
        Serial.println("North FDR1 Over Current Trip");
    }
    else if (Topic == "North132/OC/FDR2")
    {
        Serial.println("North FDR2 Over Current Trip");
    }

    //---------------------------------------------------------
    // Transformer Differential Protection
    //---------------------------------------------------------

    Serial.println("========== MQTT MESSAGE ==========");
    Serial.print("Topic = [");
    Serial.print(Topic);
    Serial.println("]");

   Serial.print("Length = ");
    Serial.println(Topic.length());
    if (Topic == "North132/DIFF/PTR1")
    {
        Serial.println("North PTR1 Differential Trip");
    }
    else if (Topic == "North132/DIFF/PTR2")
    {
        Serial.println("North PTR2 Differential Trip");
    }

    //---------------------------------------------------------
    // Transformer HV Over Current
    //---------------------------------------------------------

    else if (Topic == "North132/HVOC/PTR1")
    {
        Serial.println("North PTR1 HV Over Current Trip");
    }
    else if (Topic == "North132/HVOC/PTR2")
    {
        Serial.println("North PTR2 HV Over Current Trip");
    }

    //---------------------------------------------------------
    // Transformer LV Over Current
    //---------------------------------------------------------

    else if (Topic == "North132/LVOC/PTR1")
    {
        Serial.println("North PTR1 LV Over Current Trip");
    }
    else if (Topic == "North132/LVOC/PTR2")
    {
        Serial.println("North PTR2 LV Over Current Trip");
    }

    //---------------------------------------------------------
    // 33kV Feeders
    //---------------------------------------------------------

    else if (Topic == "North132/OC/33FDR1")
    {
        Serial.println("North 33kV Feeder-1 Over Current Trip");
    }
    else if (Topic == "North132/OC/33FDR2")
    {
        Serial.println("North 33kV Feeder-2 Over Current Trip");
    }

    //---------------------------------------------------------
    // Unknown Topic
    //---------------------------------------------------------

    else
    {
        Serial.println("Unknown MQTT Topic");
    }
}
//=============================================================
// Function : reconnect()
//=============================================================

void reconnect()
{
    while (!client.connected())
    {
        Serial.println("Connecting MQTT...");

        if (client.connect("SouthDEV", mqtt_user, mqtt_password))
        {
            Serial.println("MQTT Connected");

            if(client.subscribe("North132/#"))
            {
                Serial.println("Subscribed : North132/#");
            }
            else
            {
                Serial.println("Subscription FAILED");
            }
        }
        else
        {
            Serial.print("MQTT State = ");
            Serial.println(client.state());

            delay(3000);
        }
    }
}

//=============================================================
// Main Loop
//=============================================================

void loop()
{
    //---------------------------------------------------------
    // Maintain MQTT Connection
    //---------------------------------------------------------

    if (!client.connected())
    {
        reconnect();
    }

    client.loop();

    //---------------------------------------------------------
    // Heartbeat Message
    //---------------------------------------------------------

    static unsigned long lastMsg = 0;
    Serial.println(client.connected());

    if (millis() - lastMsg > 5000)
    {
        lastMsg = millis();

        client.publish("South132/Test", "HELLO_FROM_SOUTH");

        Serial.println("Published : HELLO_FROM_SOUTH");
    }

    //---------------------------------------------------------
    // UART Receive From ESP32-S3
    //---------------------------------------------------------

    if (S3Serial.available())
    {
        String msg = S3Serial.readStringUntil('\n');
        msg.trim();    
        Serial.print("RX=[");
        Serial.print(msg);
        Serial.println("]"); 

        //Serial.print("UART RX : ");
        //Serial.println(msg);

        //---------------- FDR1 Distance ----------------

        if (msg.indexOf("ANN_FDR1_Z1") >= 0)
        {
            Serial.println("132kV FDR1 DISTANCE ZONE-1");
            client.publish("South132/DIST/FDR1/Z1", "TRIP");
        }

        else if (msg.indexOf("ANN_FDR1_Z2") >= 0)
        {
            Serial.println("132kV FDR1 DISTANCE ZONE-2");
            client.publish("South132/DIST/FDR1/Z2", "TRIP");
        }

        else if (msg.indexOf("ANN_FDR1_Z3") >= 0)
        {
            Serial.println("132kV FDR1 DISTANCE ZONE-3");
            client.publish("South132/DIST/FDR1/Z3", "TRIP");

        }
        else if(msg=="CLR_FDR1_Z1")
{
    Serial.println("RESET ZONE1");
    client.publish("South132/DIST/FDR1/Z1_RESET","RESET");
}

else if(msg=="CLR_FDR1_Z2")
{
    Serial.println("RESET ZONE2");
    client.publish("South132/DIST/FDR1/Z2_RESET","RESET");
}

else if(msg=="CLR_FDR1_Z3")
{
    Serial.println("RESET ZONE3");
    client.publish("South132/DIST/FDR1/Z3_RESET","RESET");
}

        //---------------- FDR2 Distance ----------------

        else if (msg.indexOf("ANN_FDR2_Z1") >= 0)
        {
            Serial.println("132kV FDR2 DISTANCE ZONE-1");
            client.publish("South132/DIST/FDR2/Z1", "TRIP");
        }

        else if (msg.indexOf("ANN_FDR2_Z2") >= 0)
        {
            Serial.println("132kV FDR2 DISTANCE ZONE-2");
            client.publish("South132/DIST/FDR2/Z2", "TRIP");
        }

        else if (msg.indexOf("ANN_FDR2_Z3") >= 0)
        {
            Serial.println("132kV FDR2 DISTANCE ZONE-3");
            client.publish("South132/DIST/FDR2/Z3", "TRIP");    
        }
        else if(msg=="CLR_FDR2_Z1")
{
    client.publish("South132/DIST/FDR2/Z1_RESET","RESET");
}

else if(msg=="CLR_FDR2_Z2")
{
    client.publish("South132/DIST/FDR2/Z2_RESET","RESET");
}

else if(msg=="CLR_FDR2_Z3")
{
    client.publish("South132/DIST/FDR2/Z3_RESET","RESET");
}
        //---------------- Feeder Over Current ----------------

        else if(msg == "ANN_FDR1_OC")
{
        Serial.println("132kV FEEDER-1 OVERCURRENT");
        client.publish("South132/OC/FDR1","TRIP");
}

        else if(msg == "ANN_FDR2_OC")
        {
            Serial.println("132kV FEEDER-2 OVERCURRENT");
            client.publish("South132/OC/FDR2", "TRIP");
        }

        //---------------- PTR1 ----------------

        else if(msg == "ANN_PTR1_DIFF")
{
    Serial.println("PTR1 DIFFERENTIAL");

    bool ok = client.publish("South132/DIFF/PTR1","TRIP");

    Serial.print("Publish = ");
    Serial.println(ok);

    Serial.print("MQTT Connected = ");
    Serial.println(client.connected());

    Serial.print("MQTT State = ");
    Serial.println(client.state());

    client.loop();      // force MQTT processing
}

        else if(msg.indexOf("ANN_PTR1_HVOC") >= 0)
{
    Serial.println("PTR1 HV OVERCURRENT");

    bool ok = client.publish("South132/HVOC/PTR1","TRIP");

    Serial.print("Publish=");
    Serial.println(ok);
}
            else if(msg == "ANN_PTR1_LVOC")
           {
             Serial.println("PTR1 LV OVERCURRENT");
              client.publish("South132/LVOC/PTR1","TRIP");
           }
        //---------------- PTR2 ----------------

        else if (msg == "ANN_PTR2_DIFF")
        {
            Serial.println("PTR2 DIFFERENTIAL");
            client.publish("South132/DIFF/PTR2", "TRIP");
        }

        else if (msg == "ANN_PTR2_HVOC")
        {
            Serial.println("PTR2 HV OVERCURRENT");
            client.publish("South132/HVOC/PTR2", "TRIP");
        }

        else if (msg == "ANN_PTR2_LVOC")
        {
            Serial.println("PTR2 LV OVERCURRENT");
            client.publish("South132/LVOC/PTR2", "TRIP");
        }

        //---------------- 33kV Feeders ----------------

        else if (msg == "ANN_33FDR1_OC")
        {
            Serial.println("33kV FEEDER-1 OVERCURRENT");
            client.publish("South132/OC/33FDR1", "TRIP");
        }

        else if (msg == "ANN_33FDR2_OC")
        {
            Serial.println("33kV FEEDER-2 OVERCURRENT");
            client.publish("South132/OC/33FDR2", "TRIP");
        }
    
    //================ FDR1 OC RESET =================

else if(msg == "CLR_FDR1_OC")
{
    Serial.println("132kV FEEDER-1 OC RESET");
    client.publish("South132/OC/FDR1_RESET","RESET");
}

//================ FDR2 OC RESET =================

else if(msg == "CLR_FDR2_OC")
{
    Serial.println("132kV FEEDER-2 OC RESET");
    client.publish("South132/OC/FDR2_RESET","RESET");
}

//================ PTR1 DIFF RESET =================

else if(msg == "CLR_PTR1_DIFF")
{
    Serial.println("PTR1 DIFFERENTIAL RESET");
    client.publish("South132/DIFF/PTR1_RESET","RESET");
}

//================ PTR1 HV OC RESET =================

else if(msg == "CLR_PTR1_HVOC")
{
    Serial.println("PTR1 HV OVERCURRENT RESET");
    client.publish("South132/HVOC/PTR1_RESET","RESET");
}

//================ PTR1 LV OC RESET =================

else if(msg == "CLR_PTR1_LVOC")
{
    Serial.println("PTR1 LV OVERCURRENT RESET");
    client.publish("South132/LVOC/PTR1_RESET","RESET");
}

//================ PTR2 DIFF RESET =================

else if(msg == "CLR_PTR2_DIFF")
{
    Serial.println("PTR2 DIFFERENTIAL RESET");
    client.publish("South132/DIFF/PTR2_RESET","RESET");
}

//================ PTR2 HV OC RESET =================

else if(msg == "CLR_PTR2_HVOC")
{
    Serial.println("PTR2 HV OVERCURRENT RESET");
    client.publish("South132/HVOC/PTR2_RESET","RESET");
}

//================ PTR2 LV OC RESET =================

else if(msg == "CLR_PTR2_LVOC")
{
    Serial.println("PTR2 LV OVERCURRENT RESET");
    client.publish("South132/LVOC/PTR2_RESET","RESET");
}

//================ 33kV FDR1 OC RESET =================

else if(msg == "CLR_33FDR1_OC")
{
    Serial.println("33kV FEEDER-1 OC RESET");
    client.publish("South132/OC/33FDR1_RESET","RESET");
}

//================ 33kV FDR2 OC RESET =================

else if(msg == "CLR_33FDR2_OC")
{
    Serial.println("33kV FEEDER-2 OC RESET");
    client.publish("South132/OC/33FDR2_RESET","RESET");
}

 }   //---------------------------------------------------------
    // Earth Fault Logic
    //---------------------------------------------------------

    protectionLogic();
    //=============================================================
// Function : protectionLogic()
// Purpose  : Earth Fault Protection Logic
//            • Read Analog Inputs
//            • Detect Earth Fault
//            • Operate Annunciation
//            • Send UART Message to ESP32-S3
//            • Publish MQTT Message
//=============================================================
}
void protectionLogic()
{
    //---------------------------------------------------------
    // Earth Fault Pickup Settings
    //---------------------------------------------------------

    const float EF_FDR_PICKUP    = 60.0;
    const float EF_PTR_HV_PICKUP = 16.0;
    const float EF_PTR_LV_PICKUP = 64.0;
    const float EF_33_PICKUP     = 60.0;

    //---------------------------------------------------------
    // Read Earth Fault Analog Inputs
    //---------------------------------------------------------

    float EF1 = map(analogRead(EF_FDR1_HV), 0, 4095, 0, 300);
    float EF2 = map(analogRead(EF_FDR2_HV), 0, 4095, 0, 300);

    float EF3 = map(analogRead(EF_PTR1_HV), 0, 4095, 0, 80);
    float EF4 = map(analogRead(EF_PTR1_LV), 0, 4095, 0, 320);

    float EF5 = map(analogRead(EF_PTR2_HV), 0, 4095, 0, 80);
    float EF6 = map(analogRead(EF_PTR2_LV), 0, 4095, 0, 320);

    float EF7 = map(analogRead(EF_33FDR1), 0, 4095, 0, 300);
    float EF8 = map(analogRead(EF_33FDR2), 0, 4095, 0, 300);

    //---------------------------------------------------------
    // Display Earth Fault Measurements
    //---------------------------------------------------------

    Serial.println();
    Serial.println("========== EARTH FAULT STATUS ==========");

    Serial.print("132kV FDR1 EF   : "); Serial.println(EF1);
    Serial.print("132kV FDR2 EF   : "); Serial.println(EF2);

    Serial.print("PTR1 HV EF      : "); Serial.println(EF3);
    Serial.print("PTR1 LV EF      : "); Serial.println(EF4);

    Serial.print("PTR2 HV EF      : "); Serial.println(EF5);
    Serial.print("PTR2 LV EF      : "); Serial.println(EF6);

    Serial.print("33kV FDR1 EF    : "); Serial.println(EF7);
    Serial.print("33kV FDR2 EF    : "); Serial.println(EF8);

    Serial.println("========================================");

    //=========================================================
    // 132kV Feeder-1 Earth Fault Protection
    //=========================================================

    if (EF1 > EF_FDR_PICKUP)
    {
        digitalWrite(ANN_EF_FDR1, HIGH);

        if (!FDR1_EF_SENT)
        {
            Serial.println("FDR1_EF_TRIP");

            S3Serial.println("FDR1_EF_TRIP");

            client.publish("South132/EF/FDR1", "FDR1_EF_TRIP");

            Serial.println("MQTT : South132/EF/FDR1 Published");

            FDR1_EF_SENT = true;
        }
    }
    else
    {
        digitalWrite(ANN_EF_FDR1, LOW);

        if (FDR1_EF_SENT)
        {
            Serial.println("FDR1_EF_RESET");

            S3Serial.println("FDR1_EF_RESET");

            FDR1_EF_SENT = false;
        }
    }

    //=========================================================
    // 132kV Feeder-2 Earth Fault Protection
    //=========================================================

    if (EF2 > EF_FDR_PICKUP)
    {
        digitalWrite(ANN_EF_FDR2, HIGH);

        if (!FDR2_EF_SENT)
        {
            Serial.println("FDR2_EF_TRIP");

            S3Serial.println("FDR2_EF_TRIP");

            client.publish("South132/EF/FDR2", "FDR2_EF_TRIP");

            Serial.println("MQTT : South132/EF/FDR2 Published");

            FDR2_EF_SENT = true;
        }
    }
    else
    {
        digitalWrite(ANN_EF_FDR2, LOW);

        if (FDR2_EF_SENT)
        {
            Serial.println("FDR2_EF_RESET");

            S3Serial.println("FDR2_EF_RESET");

            FDR2_EF_SENT = false;
        }
    }

    //=========================================================
    // PTR1 HV Earth Fault Protection
    //=========================================================

    if (EF3 > EF_PTR_HV_PICKUP)
    {
        digitalWrite(ANN_EF_PTR1_HV, HIGH);

        if (!PTR1_HVEF_SENT)
        {
            Serial.println("PTR1_HVEF_TRIP");

            S3Serial.println("PTR1_HVEF_TRIP");

            client.publish("South132/EF/PTR1HV", "PTR1_HVEF_TRIP");

            Serial.println("MQTT : South132/EF/PTR1HV Published");

            PTR1_HVEF_SENT = true;
        }
    }
    else
    {
        digitalWrite(ANN_EF_PTR1_HV, LOW);

        if (PTR1_HVEF_SENT)
        {
            Serial.println("PTR1_HVEF_RESET");

            S3Serial.println("PTR1_HVEF_RESET");

            PTR1_HVEF_SENT = false;
        }
    }
        //=========================================================
    // PTR1 LV Earth Fault Protection
    //=========================================================

    if (EF4 > EF_PTR_LV_PICKUP)
    {
        digitalWrite(ANN_EF_PTR1_LV, HIGH);

        if (!PTR1_LVEF_SENT)
        {
            Serial.println("PTR1_LVEF_TRIP");

            S3Serial.println("PTR1_LVEF_TRIP");

            client.publish("South132/EF/PTR1LV", "PTR1_LVEF_TRIP");

            Serial.println("MQTT : South132/EF/PTR1LV Published");

            PTR1_LVEF_SENT = true;
        }
    }
    else
    {
        digitalWrite(ANN_EF_PTR1_LV, LOW);

        if (PTR1_LVEF_SENT)
        {
            Serial.println("PTR1_LVEF_RESET");

            S3Serial.println("PTR1_LVEF_RESET");

            PTR1_LVEF_SENT = false;
        }
    }

    //=========================================================
    // PTR2 HV Earth Fault Protection
    //=========================================================

    if (EF5 > EF_PTR_HV_PICKUP)
    {
        digitalWrite(ANN_EF_PTR2_HV, HIGH);

        if (!PTR2_HVEF_SENT)
        {
            Serial.println("PTR2_HVEF_TRIP");

            S3Serial.println("PTR2_HVEF_TRIP");

            client.publish("South132/EF/PTR2HV", "PTR2_HVEF_TRIP");

            Serial.println("MQTT : South132/EF/PTR2HV Published");

            PTR2_HVEF_SENT = true;
        }
    }
    else
    {
        digitalWrite(ANN_EF_PTR2_HV, LOW);

        if (PTR2_HVEF_SENT)
        {
            Serial.println("PTR2_HVEF_RESET");

            S3Serial.println("PTR2_HVEF_RESET");

            PTR2_HVEF_SENT = false;
        }
    }

    //=========================================================
    // PTR2 LV Earth Fault Protection
    //=========================================================

    if (EF6 > EF_PTR_LV_PICKUP)
    {
        digitalWrite(ANN_EF_PTR2_LV, HIGH);

        if (!PTR2_LVEF_SENT)
        {
            Serial.println("PTR2_LVEF_TRIP");

            S3Serial.println("PTR2_LVEF_TRIP");

            client.publish("South132/EF/PTR2LV", "PTR2_LVEF_TRIP");

            Serial.println("MQTT : South132/EF/PTR2LV Published");

            PTR2_LVEF_SENT = true;
        }
    }
    else
    {
        digitalWrite(ANN_EF_PTR2_LV, LOW);

        if (PTR2_LVEF_SENT)
        {
            Serial.println("PTR2_LVEF_RESET");

            S3Serial.println("PTR2_LVEF_RESET");

            PTR2_LVEF_SENT = false;
        }
    }

    //=========================================================
    // 33kV Feeder-1 Earth Fault Protection
    //=========================================================

    if (EF7 > EF_33_PICKUP)
    {
        digitalWrite(ANN_EF_33FDR1, HIGH);

        if (!F33_1_EF_SENT)
        {
            Serial.println("F33_1_EF_TRIP");

            S3Serial.println("F33_1_EF_TRIP");

            client.publish("South132/EF/33FDR1", "F33_1_EF_TRIP");

            Serial.println("MQTT : South132/EF/33FDR1 Published");

            F33_1_EF_SENT = true;
        }
    }
    else
    {
        digitalWrite(ANN_EF_33FDR1, LOW);

        if (F33_1_EF_SENT)
        {
            Serial.println("F33_1_EF_RESET");

            S3Serial.println("F33_1_EF_RESET");

            F33_1_EF_SENT = false;
        }
    }

    //=========================================================
    // 33kV Feeder-2 Earth Fault Protection
    //=========================================================

    if (EF8 > EF_33_PICKUP)
    {
        digitalWrite(ANN_EF_33FDR2, HIGH);

        if (!F33_2_EF_SENT)
        {
            Serial.println("F33_2_EF_TRIP");

            S3Serial.println("F33_2_EF_TRIP");

            client.publish("South132/EF/33FDR2", "F33_2_EF_TRIP");

            Serial.println("MQTT : South132/EF/33FDR2 Published");

            F33_2_EF_SENT = true;
        }
    }
    else
    {
        digitalWrite(ANN_EF_33FDR2, LOW);

        if (F33_2_EF_SENT)
        {
            Serial.println("F33_2_EF_RESET");

            S3Serial.println("F33_2_EF_RESET");

            F33_2_EF_SENT = false;
        }
    }

    //=========================================================
    // Common Earth Fault Alarm
    //=========================================================

    if (EF1 > EF_FDR_PICKUP ||
        EF2 > EF_FDR_PICKUP ||

        EF3 > EF_PTR_HV_PICKUP ||
        EF5 > EF_PTR_HV_PICKUP ||

        EF4 > EF_PTR_LV_PICKUP ||
        EF6 > EF_PTR_LV_PICKUP ||

        EF7 > EF_33_PICKUP ||
        EF8 > EF_33_PICKUP)
    {
        digitalWrite(ANN_COMMON, HIGH);
    }
    else
    {
        digitalWrite(ANN_COMMON, LOW);
    }

}
