/******************************************************************************
 * Project      : 132/33 kV Digital Substation Automation System
 * Module       : South Substation Controller (ESP32-S3)
 * File         : South_132_33kV_S3.ino
 * Hardware     : ESP32-S3 Development Board
 * Framework    : Arduino
 * IoT Platform : Blynk IoT
 * Version      : 1.1.0
 * Developed By : Narala Heshma Sree
 *
 * Description:
 * ----------------------------------------------------------------------------
 * This firmware implements the South Substation Controller of a prototype
 * 132/33 kV Digital Substation. The controller acquires electrical
 * measurements, performs protection logic, controls relay outputs,
 * monitors breaker status, and communicates real-time data to the
 * Blynk IoT platform.
 ******************************************************************************/

//=============================================================================
// Blynk Configuration
//=============================================================================

#define BLYNK_TEMPLATE_ID   "YOUR_TEMPLATE_ID"
#define BLYNK_TEMPLATE_NAME "YOUR_TEMPLATE_NAME"
#define BLYNK_AUTH_TOKEN    "YOUR_AUTH_TOKEN"

#define BLYNK_PRINT Serial

//=============================================================================
// Library Includes
//=============================================================================

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>

//=============================================================================
// Wi-Fi Credentials
//=============================================================================

char ssid[] = "YOUR_WIFI_NAME";
char pass[] = "YOUR_WIFI_PASSWORD";

//=============================================================================
// Global Objects
//=============================================================================

BlynkTimer timer;
HardwareSerial DevSerial(1);

//=============================================================================
// GPIO Assignment - Analog Inputs
//=============================================================================

//----------------------------
// Bus Voltage Measurements
//----------------------------

#define V132_BUS      1      // 132 kV Bus Voltage
#define V33_BUS       2      // 33 kV Bus Voltage

//----------------------------
// 132 kV Feeder Current Inputs
//----------------------------

#define FDR1_I       10      // Feeder-1 Current
#define FDR2_I       11      // Feeder-2 Current

//----------------------------
// Power Transformer HV Side
//----------------------------

#define HV1_I        12      // PTR-1 HV Current
#define HV2_I        13      // PTR-2 HV Current

//----------------------------
// Power Transformer LV Side
//----------------------------

#define LV1_I        14      // PTR-1 LV Current
#define LV2_I        20      // PTR-2 LV Current

//----------------------------
// 33 kV Feeder Current Inputs
//----------------------------

#define F33_1_I       8      // 33 kV Feeder-1 Current
#define F33_2_I       9      // 33 kV Feeder-2 Current

//=============================================================================
// GPIO Assignment - Relay Outputs
//=============================================================================

#define RELAY1        4
#define RELAY2        5
#define RELAY3        6
#define RELAY4        7
#define RELAY5       15
#define RELAY6       16
#define RELAY7       17
#define RELAY8       18

//=============================================================================
// GPIO Assignment - Protection Annunciation Outputs
//=============================================================================

//----------------------------
// Distance Protection
//----------------------------

#define ANN_FDR1_Z1     19
#define ANN_FDR1_Z2     21
#define ANN_FDR1_Z3     35

//----------------------------
// Feeder Over Current
//----------------------------

#define ANN_FDR1_OC     36
#define ANN_FDR2_OC     37

//----------------------------
// Transformer-1 Protection
//----------------------------

#define ANN_PTR1_DIFF   38
#define ANN_PTR1_HVOC   39
#define ANN_PTR1_LVOC   40

//----------------------------
// Transformer-2 Protection
//----------------------------

#define ANN_PTR2_DIFF   41
#define ANN_PTR2_HVOC   42
#define ANN_PTR2_LVOC   45

//----------------------------
// 33 kV Feeder Protection
//----------------------------

#define ANN_33FDR1_OC   46
#define ANN_33FDR2_OC   48

//=============================================================================
// UART Configuration
//=============================================================================

#define RX_PIN         47
#define TX_PIN          3

//=============================================================================
// Breaker Status Inputs
//=============================================================================

#define FDR1_CBO       30      // Breaker Open Status
#define FDR1_CBC       31      // Breaker Closed Status
#define FDR1_TNC_OPEN  32      // TNC Open Command
#define FDR1_TNC_CLOSE 33      // TNC Close Command
//=============================================================================
// Global Variables
// Description : Stores all real-time measurements, protection status, and
//               event notification flags used throughout the application.
//=============================================================================

//=============================================================================
// Electrical Measurements
//=============================================================================

//-----------------------------
// Bus Voltages (kV)
//-----------------------------

float Bus132 = 0.0f;      // 132 kV Bus Voltage
float Bus33  = 0.0f;      // 33 kV Bus Voltage

//-----------------------------
// 132 kV Feeder Currents (A)
//-----------------------------

float FDR1 = 0.0f;        // Feeder-1 Current
float FDR2 = 0.0f;        // Feeder-2 Current

//-----------------------------
// Power Transformer HV Currents (A)
//-----------------------------

float HV1 = 0.0f;         // PTR-1 High Voltage Side
float HV2 = 0.0f;         // PTR-2 High Voltage Side

//-----------------------------
// Power Transformer LV Currents (A)
//-----------------------------

float LV1 = 0.0f;         // PTR-1 Low Voltage Side
float LV2 = 0.0f;         // PTR-2 Low Voltage Side

//-----------------------------
// 33 kV Feeder Currents (A)
//-----------------------------

float F33_1 = 0.0f;       // 33 kV Feeder-1 Current
float F33_2 = 0.0f;       // 33 kV Feeder-2 Current

//=============================================================================
// Distance Protection
//=============================================================================

// Apparent Line Impedance (Ohms)

float Z_FDR1 = 999.0f;    // Feeder-1 Impedance
float Z_FDR2 = 999.0f;    // Feeder-2 Impedance

//=============================================================================
// Protection Trip Status Flags
//=============================================================================

//-----------------------------
// 132 kV Feeder Protection
//-----------------------------

bool FDR1_TRIPPED = false;
bool FDR2_TRIPPED = false;

//-----------------------------
// Transformer Earth Fault Protection
//-----------------------------

bool PTR1_HVEF_TRIPPED = false;
bool PTR2_HVEF_TRIPPED = false;

//-----------------------------
// Transformer Over Current Protection
//-----------------------------

bool PTR1_HV_TRIP = false;
bool PTR1_LV_TRIP = false;

bool PTR2_HV_TRIP = false;
bool PTR2_LV_TRIP = false;

//-----------------------------
// 33 kV Feeder Protection
//-----------------------------

bool F33_1_TRIPPED = false;
bool F33_2_TRIPPED = false;

//-----------------------------
// Feeder Over Current Protection
//-----------------------------

bool FDR1_OC_TRIPPED = false;
bool FDR2_OC_TRIPPED = false;

//=============================================================================
// Event Notification Flags
// Prevents duplicate Blynk notifications and annunciations.
//=============================================================================

//-----------------------------
// Distance Protection Messages
//-----------------------------

bool FDR1_ZONE1_SENT = false;
bool FDR1_ZONE2_SENT = false;
bool FDR1_ZONE3_SENT = false;

bool FDR2_ZONE1_SENT = false;
bool FDR2_ZONE2_SENT = false;
bool FDR2_ZONE3_SENT = false;

//-----------------------------
// Over Current Messages
//-----------------------------

bool FDR1_OC_SENT = false;
bool FDR2_OC_SENT = false;

//-----------------------------
// Differential Protection Messages
//-----------------------------

bool PTR1_DIFF_SENT = false;
bool PTR2_DIFF_SENT = false;

//-----------------------------
// Transformer HV Over Current Messages
//-----------------------------

bool PTR1_HVOC_SENT = false;
bool PTR2_HVOC_SENT = false;

//-----------------------------
// Transformer LV Over Current Messages
//-----------------------------

bool PTR1_LVOC_SENT = false;
bool PTR2_LVOC_SENT = false;

//-----------------------------
// 33 kV Feeder Over Current Messages
//-----------------------------

bool F33_1_OC_SENT = false;
bool F33_2_OC_SENT = false;
/******************************************************************************
 * Function    : sendMeasurements()
 *
 * Description :
 * ----------------------------------------------------------------------------
 * Acquires analog measurements from all bus voltage and current sensors,
 * converts the ADC values into engineering units, calculates the approximate
 * power flow for feeders and transformers, and updates the Blynk IoT dashboard
 * with real-time operating parameters.
 *
 * Measured Parameters:
 *   • 132 kV Bus Voltage
 *   • 33 kV Bus Voltage
 *   • 132 kV Feeder Currents
 *   • Power Transformer HV Currents
 *   • Power Transformer LV Currents
 *   • 33 kV Outgoing Feeder Currents
 *
 * Calculated Parameters:
 *   • Feeder Power
 *   • Transformer HV Power
 *   • Transformer LV Power
 *
 * Returns :
 *      None
 ******************************************************************************/
void sendMeasurements()
{
    //=========================================================================
    // Acquire Bus Voltage Measurements (kV)
    //=========================================================================

    Bus132 = map(analogRead(V132_BUS), 0, 4095, 1200, 1400) / 10.0;
    Bus33  = map(analogRead(V33_BUS),  0, 4095,  300,  350) / 10.0;

    //=========================================================================
    // Acquire 132 kV Feeder Currents (A)
    //=========================================================================

    FDR1 = map(analogRead(FDR1_I), 0, 4095, 0, 600);
    FDR2 = map(analogRead(FDR2_I), 0, 4095, 0, 600);

    //=========================================================================
    // Acquire Transformer HV Side Currents (A)
    //=========================================================================

    HV1 = map(analogRead(HV1_I), 0, 4095, 0, 100);
    HV2 = map(analogRead(HV2_I), 0, 4095, 0, 100);

    //=========================================================================
    // Acquire Transformer LV Side Currents (A)
    //=========================================================================

    LV1 = map(analogRead(LV1_I), 0, 4095, 0, 400);
    LV2 = map(analogRead(LV2_I), 0, 4095, 0, 400);

    //=========================================================================
    // Acquire 33 kV Outgoing Feeder Currents (A)
    //=========================================================================

    F33_1 = map(analogRead(F33_1_I), 0, 4095, 0, 400);
    F33_2 = map(analogRead(F33_2_I), 0, 4095, 0, 400);

    //=========================================================================
    // Calculate Approximate Active Power (MW)
    //=========================================================================
    // Formula:
    //      P ≈ Voltage × Current
    //
    // Note:
    // This is a simplified calculation intended for demonstration purposes.
    // In practical substations, active power is calculated using:
    //
    //      P = √3 × V × I × PF
    //
    // where PF is the power factor.
    //=========================================================================

    float P1      = Bus132 * FDR1  / 1000.0;
    float P2      = Bus132 * FDR2  / 1000.0;

    float PHV1    = Bus132 * HV1   / 1000.0;
    float PHV2    = Bus132 * HV2   / 1000.0;

    float PLV1    = Bus33  * LV1   / 1000.0;
    float PLV2    = Bus33  * LV2   / 1000.0;

    float PF33_1  = Bus33  * F33_1 / 1000.0;
    float PF33_2  = Bus33  * F33_2 / 1000.0;

    //=========================================================================
    // Update Blynk IoT Dashboard
    //=========================================================================

    //-----------------------------
    // 132 kV Feeder-1
    //-----------------------------

    Blynk.virtualWrite(V81, FDR1);
    Blynk.virtualWrite(V1,  Bus132);
    Blynk.virtualWrite(V2,  P1);

    //-----------------------------
    // 132 kV Feeder-2
    //-----------------------------

    Blynk.virtualWrite(V80, FDR2);
    Blynk.virtualWrite(V12, Bus132);
    Blynk.virtualWrite(V13, P2);

    //-----------------------------
    // Power Transformer-1 (HV Side)
    //-----------------------------

    Blynk.virtualWrite(V22, Bus132);
    Blynk.virtualWrite(V23, HV1);
    Blynk.virtualWrite(V24, Bus132);
    Blynk.virtualWrite(V25, PHV1);

    //-----------------------------
    // Power Transformer-2 (HV Side)
    //-----------------------------

    Blynk.virtualWrite(V34, HV2);
    Blynk.virtualWrite(V35, Bus132);
    Blynk.virtualWrite(V36, PHV2);

    //-----------------------------
    // Power Transformer-1 (LV Side)
    //-----------------------------

    Blynk.virtualWrite(V45, LV1);
    Blynk.virtualWrite(V46, Bus33);
    Blynk.virtualWrite(V47, PLV1);

    //-----------------------------
    // Power Transformer-2 (LV Side)
    //-----------------------------

    Blynk.virtualWrite(V54, LV2);
    Blynk.virtualWrite(V55, Bus33);
    Blynk.virtualWrite(V56, PLV2);

    //-----------------------------
    // 33 kV Bus
    //-----------------------------

    Blynk.virtualWrite(V63, Bus33);

    //-----------------------------
    // 33 kV Feeder-1
    //-----------------------------

    Blynk.virtualWrite(V64, F33_1);
    Blynk.virtualWrite(V65, Bus33);
    Blynk.virtualWrite(V66, PF33_1);

    //-----------------------------
    // 33 kV Feeder-2
    //-----------------------------

    Blynk.virtualWrite(V72, F33_2);
    Blynk.virtualWrite(V73, Bus33);
    Blynk.virtualWrite(V74, PF33_2);
}
/******************************************************************************
 * Function    : setup()
 *
 * Description :
 * ----------------------------------------------------------------------------
 * Performs system initialization during power-up or reset.
 *
 * Initialization Sequence:
 *   1. Initialize Serial Debug Interface
 *   2. Initialize UART Communication
 *   3. Configure ADC Resolution
 *   4. Configure Blynk Timer Tasks
 *   5. Configure Relay Output GPIOs
 *   6. Configure Protection Annunciation Outputs
 *   7. Set All Protection Relays to Safe State
 *   8. Display System Status
 *
 * Returns :
 *      None
 ******************************************************************************/
void setup()
{
    //=========================================================================
    // Initialize Debug Serial Port
    //=========================================================================

    Serial.begin(115200);

    //=========================================================================
    // Initialize UART Communication
    // Used for RTU / IED / Future SCADA Communication
    //=========================================================================

    DevSerial.begin(115200, SERIAL_8N1, RX_PIN, TX_PIN);

    //=========================================================================
    // Display Startup Information
    //=========================================================================

    Serial.println();
    Serial.println("======================================================");
    Serial.println("      132/33 kV DIGITAL SUBSTATION AUTOMATION");
    Serial.println("              ESP32-S3 INITIALIZED");
    Serial.println("======================================================");

    //=========================================================================
    // Configure ADC
    // 12-bit Resolution (0 - 4095)
    //=========================================================================

    analogReadResolution(12);

    //=========================================================================
    // Initialize Blynk (Enable when Wi-Fi is available)
    //=========================================================================

      Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

    //=========================================================================
    // Configure Periodic Tasks
    //=========================================================================

    timer.setInterval(2000L, sendMeasurements);
    timer.setInterval(2000L, protectionLogic);

    //=========================================================================
    // Configure Relay Output GPIOs
    //=========================================================================

    pinMode(RELAY1, OUTPUT);
    pinMode(RELAY2, OUTPUT);
    pinMode(RELAY3, OUTPUT);
    pinMode(RELAY4, OUTPUT);
    pinMode(RELAY5, OUTPUT);
    pinMode(RELAY6, OUTPUT);
    pinMode(RELAY7, OUTPUT);
    pinMode(RELAY8, OUTPUT);

    //=========================================================================
    // Set Relay Outputs to Safe State
    // Active LOW Relay Module
    // HIGH = Relay OFF
    // LOW  = Relay ON
    //=========================================================================

    digitalWrite(RELAY1, HIGH);
    digitalWrite(RELAY2, HIGH);
    digitalWrite(RELAY3, HIGH);
    digitalWrite(RELAY4, HIGH);
    digitalWrite(RELAY5, HIGH);
    digitalWrite(RELAY6, HIGH);
    digitalWrite(RELAY7, HIGH);
    digitalWrite(RELAY8, HIGH);

    //=========================================================================
    // Configure Protection Annunciation Outputs
    //=========================================================================

    pinMode(ANN_FDR1_Z1, OUTPUT);
    pinMode(ANN_FDR1_Z2, OUTPUT);
    pinMode(ANN_FDR1_Z3, OUTPUT);

    pinMode(ANN_FDR1_OC, OUTPUT);
    pinMode(ANN_FDR2_OC, OUTPUT);

    pinMode(ANN_PTR1_DIFF, OUTPUT);
    pinMode(ANN_PTR1_HVOC, OUTPUT);
    pinMode(ANN_PTR1_LVOC, OUTPUT);

    pinMode(ANN_PTR2_DIFF, OUTPUT);
    pinMode(ANN_PTR2_HVOC, OUTPUT);
    pinMode(ANN_PTR2_LVOC, OUTPUT);

    pinMode(ANN_33FDR1_OC, OUTPUT);
    pinMode(ANN_33FDR2_OC, OUTPUT);

    //=========================================================================
    // System Initialization Complete
    //=========================================================================

    Serial.println("System Initialization Completed Successfully.");
    Serial.println("Protection System Ready.");
}
/******************************************************************************
 * Blynk Virtual Pin Handlers
 *
 * Description :
 * ----------------------------------------------------------------------------
 * These callback functions receive commands from the Blynk IoT dashboard and
 * operate the corresponding circuit breaker relay outputs.
 *
 * Relay Logic:
 *      LOW  = Relay Energized (Breaker CLOSED)
 *      HIGH = Relay De-energized (Breaker OPEN)
 *
 * Safety Note:
 * These functions provide remote breaker operation for demonstration purposes.
 * In practical substation automation systems, breaker control should be
 * interlocked with protection relays, SCADA permissions, and local/remote
 * control selection to prevent unsafe switching operations.
 ******************************************************************************/

//=============================================================================
// 132 kV Feeder-1 Breaker Control
// Virtual Pin : V3
// Relay       : RELAY1
//=============================================================================

BLYNK_WRITE(V3)
{
    digitalWrite(RELAY1, param.asInt() ? LOW : HIGH);
}

//=============================================================================
// 132 kV Feeder-2 Breaker Control
// Virtual Pin : V14
// Relay       : RELAY2
//=============================================================================

BLYNK_WRITE(V14)
{
    digitalWrite(RELAY2, param.asInt() ? LOW : HIGH);
}

//=============================================================================
// Power Transformer-1 HV Breaker Control
// Virtual Pin : V27
// Relay       : RELAY3
//=============================================================================

BLYNK_WRITE(V27)
{
    digitalWrite(RELAY3, param.asInt() ? LOW : HIGH);
}

//=============================================================================
// Power Transformer-2 HV Breaker Control
// Virtual Pin : V38
// Relay       : RELAY4
//=============================================================================

BLYNK_WRITE(V38)
{
    digitalWrite(RELAY4, param.asInt() ? LOW : HIGH);
}

//=============================================================================
// Power Transformer-1 LV Breaker Control
// Virtual Pin : V48
// Relay       : RELAY5
//=============================================================================

BLYNK_WRITE(V48)
{
    digitalWrite(RELAY5, param.asInt() ? LOW : HIGH);
}

//=============================================================================
// Power Transformer-2 LV Breaker Control
// Virtual Pin : V57
// Relay       : RELAY6
//=============================================================================

BLYNK_WRITE(V57)
{
    digitalWrite(RELAY6, param.asInt() ? LOW : HIGH);
}

//=============================================================================
// 33 kV Feeder-1 Breaker Control
// Virtual Pin : V67
// Relay       : RELAY7
//=============================================================================

BLYNK_WRITE(V67)
{
    digitalWrite(RELAY7, param.asInt() ? LOW : HIGH);
}

//=============================================================================
// 33 kV Feeder-2 Breaker Control
// Virtual Pin : V75
// Relay       : RELAY8
//=============================================================================

BLYNK_WRITE(V75)
{
    digitalWrite(RELAY8, param.asInt() ? LOW : HIGH);
}

//=============================================================================
// 33 kV Feeder-1 Breaker Control
// Virtual Pin : V67
// Relay       : RELAY7
//=============================================================================
/*****************************************************************************
 * Function    : protectionLogic()
 *
 * Description :
 * ----------------------------------------------------------------------------
 * Executes the primary protection algorithms for the Digital Substation.
 *
 * Implemented Protection Functions:
 *   • Distance Protection (Zone-1, Zone-2, Zone-3)
 *   • Over Current Protection
 *   • Differential Protection
 *   • Earth Fault Protection
 *
 * This section implements the Distance Protection logic for
 * 132 kV Feeder-1.
 *
 * Distance Relay Characteristics
 * ------------------------------
 * Zone-1 : 80% Line Reach (Instantaneous)
 * Zone-2 : 100% Line Reach (1 Second Delay)
 * Zone-3 : Remote Backup (2 Second Delay)
 *
 * Apparent Impedance:
 *
 *      Z = V / I
 *
 ******************************************************************************/
void protectionLogic()
{
    //=========================================================================
    // 132 kV Feeder-1 Distance Protection
    //=========================================================================

    // If the feeder has already tripped, maintain the breaker in
    // the OPEN state and skip further protection processing.

    if (FDR1_TRIPPED)
    {
        digitalWrite(RELAY1, LOW);
        return;
    }

    //=========================================================================
    // Calculate Apparent Line Impedance
    //=========================================================================

    if (FDR1 > 1)
        Z_FDR1 = Bus132 / FDR1;
    else
        Z_FDR1 = 999.0;

    //=========================================================================
    // Zone-1 Protection
    // Reach : 80%
    // Operation : Instantaneous
    //=========================================================================

    if (Z_FDR1 <= 0.264)
    {
        // Trip Breaker

        digitalWrite(RELAY1, LOW);

        // Annunciation

        digitalWrite(ANN_FDR1_Z1, HIGH);
        digitalWrite(ANN_FDR1_Z2, LOW);
        digitalWrite(ANN_FDR1_Z3, LOW);

        // Blynk Dashboard

        Blynk.virtualWrite(V4, 1);
        Blynk.virtualWrite(V5, 0);
        Blynk.virtualWrite(V6, 0);

        // Send Event Only Once

        if (!FDR1_ZONE1_SENT)
        {
            DevSerial.println("ANN_FDR1_Z1");
            Serial.println("Distance Protection : Zone-1 Operated");

            FDR1_ZONE1_SENT = true;
        }
    }

    //=========================================================================
    // Zone-2 Protection
    // Reach : 120%
    // Time Delay : 1 Second
    //=========================================================================

    else if (Z_FDR1 <= 0.330)
    {
        delay(1000);

        digitalWrite(RELAY1, LOW);

        digitalWrite(ANN_FDR1_Z1, LOW);
        digitalWrite(ANN_FDR1_Z2, HIGH);
        digitalWrite(ANN_FDR1_Z3, LOW);

        Blynk.virtualWrite(V4, 0);
        Blynk.virtualWrite(V5, 1);
        Blynk.virtualWrite(V6, 0);

        if (!FDR1_ZONE2_SENT)
        {
            DevSerial.println("ANN_FDR1_Z2");
            Serial.println("Distance Protection : Zone-2 Operated");

            FDR1_ZONE2_SENT = true;
        }
    }

    //=========================================================================
    // Zone-3 Protection
    // Remote Backup Protection
    // Time Delay : 2 Seconds
    //=========================================================================

    else if (Z_FDR1 <= 0.377)
    {
        delay(2000);

        digitalWrite(RELAY1, LOW);

        digitalWrite(ANN_FDR1_Z1, LOW);
        digitalWrite(ANN_FDR1_Z2, LOW);
        digitalWrite(ANN_FDR1_Z3, HIGH);

        Blynk.virtualWrite(V4, 0);
        Blynk.virtualWrite(V5, 0);
        Blynk.virtualWrite(V6, 1);

        if (!FDR1_ZONE3_SENT)
        {
            DevSerial.println("ANN_FDR1_Z3");
            Serial.println("Distance Protection : Zone-3 Operated");

            FDR1_ZONE3_SENT = true;
        }
    }

    //=========================================================================
    // Healthy Feeder
    //=========================================================================
    else
    {
        // Keep Breaker Closed

        digitalWrite(RELAY1, HIGH);

        // Clear Annunciation

        digitalWrite(ANN_FDR1_Z1, LOW);
        digitalWrite(ANN_FDR1_Z2, LOW);
        digitalWrite(ANN_FDR1_Z3, LOW);

        // Update Dashboard

        Blynk.virtualWrite(V4, 0);
        Blynk.virtualWrite(V5, 0);
        Blynk.virtualWrite(V6, 0);

        // Clear Event Flags

        if (FDR1_ZONE1_SENT)
        {
            DevSerial.println("CLR_FDR1_Z1");
            Serial.println("Distance Protection : Zone-1 Reset");

            FDR1_ZONE1_SENT = false;
        }

        if (FDR1_ZONE2_SENT)
        {
            DevSerial.println("CLR_FDR1_Z2");
            Serial.println("Distance Protection : Zone-2 Reset");

            FDR1_ZONE2_SENT = false;
        }

        if (FDR1_ZONE3_SENT)
        {
            DevSerial.println("CLR_FDR1_Z3");
            Serial.println("Distance Protection : Zone-3 Reset");

            FDR1_ZONE3_SENT = false;
        }
    }

    //=========================================================================
    // Debug Information
    //=========================================================================

    Serial.print("Feeder-1 Apparent Impedance : ");
    Serial.println(Z_FDR1);
/******************************************************************************
 * 132 kV Feeder-2 Distance Protection
 * ----------------------------------------------------------------------------
 * Implements three-zone distance protection for Feeder-2.
 *
 * Protection Zones:
 *
 *   Zone-1 : 80% Line Reach
 *            Instantaneous Trip
 *
 *   Zone-2 : 120% Line Reach
 *            1 Second Time Delay
 *
 *   Zone-3 : Remote Backup Protection
 *            2 Second Time Delay
 *
 * Apparent Impedance:
 *
 *      Z = V / I
 *
 ******************************************************************************/

//=========================================================================
// Maintain Breaker Trip State
//=========================================================================

if (FDR2_TRIPPED)
{
    digitalWrite(RELAY2, LOW);
    return;
}

//=========================================================================
// Calculate Apparent Line Impedance
//=========================================================================

if (FDR2 > 1)
    Z_FDR2 = Bus132 / FDR2;
else
    Z_FDR2 = 999.0;

//=========================================================================
// Zone-1 Protection
// Reach : 80%
// Operation : Instantaneous
//=========================================================================

if (Z_FDR2 <= 0.264)
{
    // Trip Breaker

    digitalWrite(RELAY2, LOW);

    // Update Dashboard

    Blynk.virtualWrite(V15, 1);
    Blynk.virtualWrite(V16, 0);
    Blynk.virtualWrite(V17, 0);

    // Send Event Only Once

    if (!FDR2_ZONE1_SENT)
    {
        DevSerial.println("ANN_FDR2_Z1");
        Serial.println("Distance Protection : Feeder-2 Zone-1 Operated");

        FDR2_ZONE1_SENT = true;
    }
}

//=========================================================================
// Zone-2 Protection
// Reach : 120%
// Time Delay : 1 Second
//=========================================================================

else if (Z_FDR2 <= 0.330)
{
    // NOTE:
    // delay() is used only for demonstration.
    // Industrial systems should use non-blocking timers.

    delay(1000);

    digitalWrite(RELAY2, LOW);

    Blynk.virtualWrite(V15, 0);
    Blynk.virtualWrite(V16, 1);
    Blynk.virtualWrite(V17, 0);

    if (!FDR2_ZONE2_SENT)
    {
        DevSerial.println("ANN_FDR2_Z2");
        Serial.println("Distance Protection : Feeder-2 Zone-2 Operated");

        FDR2_ZONE2_SENT = true;
    }
}

//=========================================================================
// Zone-3 Protection
// Remote Backup Protection
// Time Delay : 2 Seconds
//=========================================================================

else if (Z_FDR2 <= 0.377)
{
    delay(2000);

    digitalWrite(RELAY2, LOW);

    Blynk.virtualWrite(V15, 0);
    Blynk.virtualWrite(V16, 0);
    Blynk.virtualWrite(V17, 1);

    if (!FDR2_ZONE3_SENT)
    {
        DevSerial.println("ANN_FDR2_Z3");
        Serial.println("Distance Protection : Feeder-2 Zone-3 Operated");

        FDR2_ZONE3_SENT = true;
    }
}

//=========================================================================
// Healthy Feeder Condition
//=========================================================================

else
{
    // Keep Breaker Closed

    digitalWrite(RELAY2, HIGH);

    // Clear Dashboard Indications

    Blynk.virtualWrite(V15, 0);
    Blynk.virtualWrite(V16, 0);
    Blynk.virtualWrite(V17, 0);

    // Reset Event Flags

    if (FDR2_ZONE1_SENT)
    {
        DevSerial.println("CLR_FDR2_Z1");
        Serial.println("Distance Protection : Feeder-2 Zone-1 Reset");

        FDR2_ZONE1_SENT = false;
    }

    if (FDR2_ZONE2_SENT)
    {
        DevSerial.println("CLR_FDR2_Z2");
        Serial.println("Distance Protection : Feeder-2 Zone-2 Reset");

        FDR2_ZONE2_SENT = false;
    }

    if (FDR2_ZONE3_SENT)
    {
        DevSerial.println("CLR_FDR2_Z3");
        Serial.println("Distance Protection : Feeder-2 Zone-3 Reset");

        FDR2_ZONE3_SENT = false;
    }
}

//=========================================================================
// Debug Information
//=========================================================================

Serial.print("Feeder-2 Apparent Impedance : ");
Serial.println(Z_FDR2);
/******************************************************************************
 * 132 kV Feeder-1 Over Current Protection
 * ----------------------------------------------------------------------------
 * Implements instantaneous over current protection for Feeder-1.
 *
 * Pickup Setting :
 *      300 A
 *
 * Protection Operation :
 *      Current > 300 A
 *          → Trip Breaker
 *          → Activate Annunciation
 *          → Update Blynk Dashboard
 *          → Send Event Notification
 *
 ******************************************************************************/

//=========================================================================
// Skip Processing if Distance Protection Already Tripped the Feeder
//=========================================================================

if (!FDR1_TRIPPED)
{
    // Debug Information

    Serial.print("Feeder-1 Current : ");
    Serial.println(FDR1);

    //=====================================================================
    // Over Current Pickup Detection
    //=====================================================================

    if (FDR1 > 300)
    {
        // Set Protection Flag

        FDR1_OC_TRIPPED = true;

        // Trip Circuit Breaker

        digitalWrite(RELAY1, LOW);

        // Activate Annunciation

        digitalWrite(ANN_FDR1_OC, HIGH);

        // Update Blynk Dashboard

        Blynk.virtualWrite(V9, 1);

        // Send Event Only Once

        if (!FDR1_OC_SENT)
        {
            DevSerial.println("ANN_FDR1_OC");
            Serial.println("Over Current Protection : Feeder-1 Operated");

            FDR1_OC_SENT = true;
        }
    }

    //=====================================================================
    // Healthy Condition
    //=====================================================================

    else
    {
        FDR1_OC_TRIPPED = false;

        digitalWrite(ANN_FDR1_OC, LOW);

        Blynk.virtualWrite(V9, 0);

        if (FDR1_OC_SENT)
        {
            DevSerial.println("CLR_FDR1_OC");
            Serial.println("Over Current Protection : Feeder-1 Reset");

            FDR1_OC_SENT = false;
        }
    }
}
else
{
    // Distance Protection has already tripped the feeder.
    // Maintain breaker in OPEN state.

    digitalWrite(RELAY1, LOW);
}

//=========================================================================
// Final Breaker Trip Decision
//=========================================================================
//
// Breaker remains OPEN if ANY protection has operated.
//
// Trip Sources:
//      • Distance Protection
//      • Over Current Protection
//
//=========================================================================

if (FDR1_TRIPPED || FDR1_OC_TRIPPED)
{
    digitalWrite(RELAY1, LOW);
}
else
{
    digitalWrite(RELAY1, HIGH);
}

//=========================================================================
// Debug Information
//=========================================================================

Serial.print("Distance Trip Status : ");
Serial.println(FDR1_TRIPPED);

Serial.print("Over Current Trip Status : ");
Serial.println(FDR1_OC_TRIPPED);
/******************************************************************************
 * 132 kV Feeder-2 Over Current Protection
 * ----------------------------------------------------------------------------
 * Implements instantaneous over current protection for Feeder-2.
 *
 * Pickup Setting :
 *      300 A
 *
 * Protection Operation :
 *      Current > 300 A
 *          → Trip Circuit Breaker
 *          → Activate Protection Annunciation
 *          → Update Blynk Dashboard
 *          → Send Protection Event
 *
 ******************************************************************************/

//=========================================================================
// Skip Over Current Processing if Distance Protection has Already Tripped
//=========================================================================

if (!FDR2_TRIPPED)
{
    // Debug Information

    Serial.print("Feeder-2 Current : ");
    Serial.println(FDR2);

    //=====================================================================
    // Over Current Pickup Detection
    //=====================================================================

    if (FDR2 > 300)
    {
        // Set Protection Flag

        FDR2_OC_TRIPPED = true;

        // Trip Circuit Breaker

        digitalWrite(RELAY2, LOW);

        // Activate Protection Annunciation

        digitalWrite(ANN_FDR2_OC, HIGH);

        // Update Blynk Dashboard

        Blynk.virtualWrite(V20, 1);

        // Send Event Notification Only Once

        if (!FDR2_OC_SENT)
        {
            DevSerial.println("ANN_FDR2_OC");
            Serial.println("Over Current Protection : Feeder-2 Operated");

            FDR2_OC_SENT = true;
        }
    }

    //=====================================================================
    // Healthy Operating Condition
    //=====================================================================

    else
    {
        FDR2_OC_TRIPPED = false;

        // Clear Protection Annunciation

        digitalWrite(ANN_FDR2_OC, LOW);

        // Update Dashboard

        Blynk.virtualWrite(V20, 0);

        // Reset Event Notification

        if (FDR2_OC_SENT)
        {
            DevSerial.println("CLR_FDR2_OC");
            Serial.println("Over Current Protection : Feeder-2 Reset");

            FDR2_OC_SENT = false;
        }
    }
}
else
{
    // Distance Protection has already tripped the feeder.
    // Maintain breaker in OPEN state.

    digitalWrite(RELAY2, LOW);
}

//=========================================================================
// Final Breaker Trip Decision
//=========================================================================
//
// Breaker remains OPEN if any protection function has operated.
//
// Trip Sources:
//      • Distance Protection
//      • Over Current Protection
//
//=========================================================================

if (FDR2_TRIPPED || FDR2_OC_TRIPPED)
{
    digitalWrite(RELAY2, LOW);
}
else
{
    digitalWrite(RELAY2, HIGH);
}

//=========================================================================
// Debug Information
//=========================================================================

Serial.print("Feeder-2 Distance Trip Status : ");
Serial.println(FDR2_TRIPPED);

Serial.print("Feeder-2 Over Current Trip Status : ");
Serial.println(FDR2_OC_TRIPPED);
/******************************************************************************
 * Power Transformer-1 Protection
 * ----------------------------------------------------------------------------
 * Implemented Protection Functions:
 *
 *   • Differential Protection (87T)
 *   • HV Side Over Current Protection (50/51)
 *   • LV Side Over Current Protection (50/51)
 *
 ******************************************************************************/

//=============================================================================
// Power Transformer-1 Differential Protection (87T)
//=============================================================================
//
// Operating Principle
//
//      Differential Current = HV Current × CT Ratio - LV Current
//
// Protection Operates When:
//
//      |IHV × CT_RATIO − ILV| > 30% of LV Current
//
// CT Ratio Used:
//
//      HV : LV = 1 : 4
//
//=============================================================================

if (HV1 > 10 && LV1 > 40)
{
    // Calculate Differential Current

    if (abs(HV1 * 4 - LV1) > (0.30 * LV1))
    {
        // Trip Both HV and LV Breakers

        digitalWrite(RELAY3, LOW);
        digitalWrite(RELAY5, LOW);

        // Activate Annunciation

        digitalWrite(ANN_PTR1_DIFF, HIGH);

        // Update Dashboard

        Blynk.virtualWrite(V28, 1);

        // Send Event Only Once

        if (!PTR1_DIFF_SENT)
        {
            DevSerial.println("ANN_PTR1_DIFF");
            Serial.println("Transformer-1 Differential Protection Operated");

            PTR1_DIFF_SENT = true;
        }
    }
    else
    {
        // Healthy Transformer

        digitalWrite(ANN_PTR1_DIFF, LOW);

        Blynk.virtualWrite(V28, 0);

        if (PTR1_DIFF_SENT)
        {
            DevSerial.println("CLR_PTR1_DIFF");
            Serial.println("Transformer-1 Differential Protection Reset");

            PTR1_DIFF_SENT = false;
        }
    }
}
else
{
    // Current Below Operating Threshold

    digitalWrite(ANN_PTR1_DIFF, LOW);

    if (PTR1_DIFF_SENT)
    {
        DevSerial.println("CLR_PTR1_DIFF");
        Serial.println("Transformer-1 Differential Protection Reset");

        PTR1_DIFF_SENT = false;
    }
}

//=============================================================================
// Power Transformer-1 HV Side Over Current Protection (50/51)
//=============================================================================
//
// Pickup Current : 80 A
//
//=============================================================================

if (HV1 > 80)
{
    // Trip HV and LV Breakers

    digitalWrite(RELAY3, LOW);
    digitalWrite(RELAY5, LOW);

    // Activate Annunciation

    digitalWrite(ANN_PTR1_HVOC, HIGH);

    // Update Dashboard

    Blynk.virtualWrite(V31, 1);

    // Send Event

    if (!PTR1_HVOC_SENT)
    {
        DevSerial.println("ANN_PTR1_HVOC");
        Serial.println("Transformer-1 HV Over Current Operated");

        PTR1_HVOC_SENT = true;
    }
}
else
{
    // Restore Breakers if no other protection is active

    if (!PTR1_HV_TRIP)
    {
        digitalWrite(RELAY3, HIGH);
    }

    if (!PTR1_LV_TRIP)
    {
        digitalWrite(RELAY5, HIGH);
    }

    // Clear Annunciation

    digitalWrite(ANN_PTR1_HVOC, LOW);

    // Update Dashboard

    Blynk.virtualWrite(V31, 0);

    // Reset Event Flag

    if (PTR1_HVOC_SENT)
    {
        DevSerial.println("CLR_PTR1_HVOC");
        Serial.println("Transformer-1 HV Over Current Reset");

        PTR1_HVOC_SENT = false;
    }
}

//=============================================================================
// Power Transformer-1 LV Side Over Current Protection (50/51)
//=============================================================================
//
// Pickup Current : 320 A
//
//=============================================================================

if (LV1 > 320)
{
    // Trip LV Breaker

    digitalWrite(RELAY5, LOW);

    // Activate Annunciation

    digitalWrite(ANN_PTR1_LVOC, HIGH);

    // Update Dashboard

    Blynk.virtualWrite(V42, 1);

    // Send Event

    if (!PTR1_LVOC_SENT)
    {
        DevSerial.println("ANN_PTR1_LVOC");
        Serial.println("Transformer-1 LV Over Current Operated");

        PTR1_LVOC_SENT = true;
    }
}
else
{
    // Clear Annunciation

    digitalWrite(ANN_PTR1_LVOC, LOW);

    // Update Dashboard

    Blynk.virtualWrite(V42, 0);

    // Reset Event Flag

    if (PTR1_LVOC_SENT)
    {
        DevSerial.println("CLR_PTR1_LVOC");
        Serial.println("Transformer-1 LV Over Current Reset");

        PTR1_LVOC_SENT = false;
    }
}
/******************************************************************************
 * Power Transformer-2 Protection
 * ----------------------------------------------------------------------------
 * Implemented Protection Functions:
 *
 *   • Differential Protection (87T)
 *   • HV Side Over Current Protection (50/51)
 *   • LV Side Over Current Protection (50/51)
 *
 ******************************************************************************/

//=============================================================================
// Power Transformer-2 Differential Protection (87T)
//=============================================================================
//
// Operating Principle
//
//      Differential Current = HV Current × CT Ratio − LV Current
//
// Protection Operates When:
//
//      |IHV × CT_RATIO − ILV| > 30% of LV Current
//
// CT Ratio:
//
//      HV : LV = 1 : 4
//
//=============================================================================

if (HV2 > 10 && LV2 > 40)
{
    if (abs(HV2 * 4 - LV2) > (0.30 * LV2))
    {
        // Trip Both HV and LV Breakers

        digitalWrite(RELAY4, LOW);
        digitalWrite(RELAY6, LOW);

        // Activate Protection Annunciation

        digitalWrite(ANN_PTR2_DIFF, HIGH);

        // Update Blynk Dashboard

        Blynk.virtualWrite(V39, 1);

        // Send Event Notification Only Once

        if (!PTR2_DIFF_SENT)
        {
            DevSerial.println("ANN_PTR2_DIFF");
            Serial.println("Transformer-2 Differential Protection Operated");

            PTR2_DIFF_SENT = true;
        }
    }
    else
    {
        // Healthy Transformer

        digitalWrite(ANN_PTR2_DIFF, LOW);

        Blynk.virtualWrite(V39, 0);

        if (PTR2_DIFF_SENT)
        {
            DevSerial.println("CLR_PTR2_DIFF");
            Serial.println("Transformer-2 Differential Protection Reset");

            PTR2_DIFF_SENT = false;
        }
    }
}
else
{
    // Current Below Differential Pickup

    digitalWrite(ANN_PTR2_DIFF, LOW);

    if (PTR2_DIFF_SENT)
    {
        DevSerial.println("CLR_PTR2_DIFF");
        Serial.println("Transformer-2 Differential Protection Reset");

        PTR2_DIFF_SENT = false;
    }
}

//=============================================================================
// Power Transformer-2 HV Side Over Current Protection (50/51)
//=============================================================================
//
// Pickup Current : 80 A
//
//=============================================================================

if (HV2 > 80)
{
    // Trip Both HV and LV Breakers

    digitalWrite(RELAY4, LOW);
    digitalWrite(RELAY6, LOW);

    // Activate Annunciation

    digitalWrite(ANN_PTR2_HVOC, HIGH);

    // Update Dashboard

    Blynk.virtualWrite(V42, 1);

    // Send Event Notification

    if (!PTR2_HVOC_SENT)
    {
        DevSerial.println("ANN_PTR2_HVOC");
        Serial.println("Transformer-2 HV Over Current Operated");

        PTR2_HVOC_SENT = true;
    }
}
else
{
    // Restore Breakers if no other protection remains active

    if (!PTR2_HVEF_TRIPPED)
    {
        digitalWrite(RELAY4, HIGH);
    }

    if (!PTR2_LV_TRIP)
    {
        digitalWrite(RELAY6, HIGH);
    }

    // Clear Annunciation

    digitalWrite(ANN_PTR2_HVOC, LOW);

    // Update Dashboard

    Blynk.virtualWrite(V42, 0);

    if (PTR2_HVOC_SENT)
    {
        DevSerial.println("CLR_PTR2_HVOC");
        Serial.println("Transformer-2 HV Over Current Reset");

        PTR2_HVOC_SENT = false;
    }
}

//=============================================================================
// Power Transformer-2 LV Side Over Current Protection (50/51)
//=============================================================================
//
// Pickup Current : 320 A
//
//=============================================================================

if (LV2 > 320)
{
    // Trip LV Breaker

    digitalWrite(RELAY6, LOW);

    // Activate Annunciation

    digitalWrite(ANN_PTR2_LVOC, HIGH);

    // Update Dashboard

    Blynk.virtualWrite(V60, 1);

    // Send Event Notification

    if (!PTR2_LVOC_SENT)
    {
        DevSerial.println("ANN_PTR2_LVOC");
        Serial.println("Transformer-2 LV Over Current Operated");

        PTR2_LVOC_SENT = true;
    }
}
else
{
    // Clear Annunciation

    digitalWrite(ANN_PTR2_LVOC, LOW);

    // Update Dashboard

    Blynk.virtualWrite(V60, 0);

    // Reset Event Flag

    if (PTR2_LVOC_SENT)
    {
        DevSerial.println("CLR_PTR2_LVOC");
        Serial.println("Transformer-2 LV Over Current Reset");

        PTR2_LVOC_SENT = false;
    }
}

//=============================================================================
// Debug Information
//=============================================================================

Serial.println("\n================ Protection Status ================");

Serial.print("PTR2_HVEF_TRIPPED : ");
Serial.println(PTR2_HVEF_TRIPPED);

Serial.print("F33_1_TRIPPED     : ");
Serial.println(F33_1_TRIPPED);

Serial.print("F33_2_TRIPPED     : ");
Serial.println(F33_2_TRIPPED);

Serial.print("PTR1_HV_TRIP      : ");
Serial.println(PTR1_HV_TRIP);

Serial.print("PTR1_LV_TRIP      : ");
Serial.println(PTR1_LV_TRIP);

Serial.print("PTR2_HV_TRIP      : ");
Serial.println(PTR2_HV_TRIP);

Serial.print("PTR2_LV_TRIP      : ");
Serial.println(PTR2_LV_TRIP);

Serial.println("\n================ Electrical Measurements ================");

Serial.print("PTR1 HV Current : ");
Serial.println(HV1);

Serial.print("PTR2 HV Current : ");
Serial.println(HV2);

Serial.print("PTR1 LV Current : ");
Serial.println(LV1);

Serial.print("PTR2 LV Current : ");
Serial.println(LV2);

Serial.print("33 kV Feeder-1 Current : ");
Serial.println(F33_1);

Serial.print("33 kV Feeder-2 Current : ");
Serial.println(F33_2);
/******************************************************************************
 * 33 kV Outgoing Feeder Protection
 * ----------------------------------------------------------------------------
 * Implemented Protection:
 *
 *   • Feeder-1 Over Current Protection (50/51)
 *   • Feeder-2 Over Current Protection (50/51)
 *
 ******************************************************************************/

//=============================================================================
// 33 kV Feeder-1 Over Current Protection (50/51)
//=============================================================================
//
// Pickup Current : 300 A
//
//=============================================================================

if (F33_1 > 300)
{
    // Trip Circuit Breaker

    digitalWrite(RELAY7, LOW);

    // Activate Protection Annunciation

    digitalWrite(ANN_33FDR1_OC, HIGH);

    // Update Blynk Dashboard

    Blynk.virtualWrite(V70, 1);

    // Send Event Notification Only Once

    if (!F33_1_OC_SENT)
    {
        DevSerial.println("ANN_33FDR1_OC");
        Serial.println("33 kV Feeder-1 Over Current Operated");

        F33_1_OC_SENT = true;
    }
}
else
{
    // Restore Breaker if no other protection has operated

    if (!F33_1_TRIPPED)
    {
        digitalWrite(RELAY7, HIGH);
    }

    // Clear Annunciation

    digitalWrite(ANN_33FDR1_OC, LOW);

    // Update Dashboard

    Blynk.virtualWrite(V70, 0);

    // Reset Event Flag

    if (F33_1_OC_SENT)
    {
        DevSerial.println("CLR_33FDR1_OC");
        Serial.println("33 kV Feeder-1 Over Current Reset");

        F33_1_OC_SENT = false;
    }
}

//=============================================================================
// 33 kV Feeder-2 Over Current Protection (50/51)
//=============================================================================
//
// Pickup Current : 300 A
//
//=============================================================================

if (F33_2 > 300)
{
    // Trip Circuit Breaker

    digitalWrite(RELAY8, LOW);

    // Activate Protection Annunciation

    digitalWrite(ANN_33FDR2_OC, HIGH);

    // Update Blynk Dashboard

    Blynk.virtualWrite(V78, 1);

    // Send Event Notification Only Once

    if (!F33_2_OC_SENT)
    {
        DevSerial.println("ANN_33FDR2_OC");
        Serial.println("33 kV Feeder-2 Over Current Operated");

        F33_2_OC_SENT = true;
    }
}
else
{
    // Restore Breaker if no other protection has operated

    if (!F33_2_TRIPPED)
    {
        digitalWrite(RELAY8, HIGH);
    }

    // Clear Annunciation

    digitalWrite(ANN_33FDR2_OC, LOW);

    // Update Dashboard

    Blynk.virtualWrite(V78, 0);

    // Reset Event Flag

    if (F33_2_OC_SENT)
    {
        DevSerial.println("CLR_33FDR2_OC");
        Serial.println("33 kV Feeder-2 Over Current Reset");

        F33_2_OC_SENT = false;
    }
}

//=============================================================================
// Update Breaker Status
//=============================================================================
//
// Refresh all breaker indications on the Blynk dashboard.
//
//=============================================================================

updateBreakerStatus();

}   // End of protectionLogic()
/******************************************************************************
 * Function    : loop()
 *
 * Description :
 * ----------------------------------------------------------------------------
 * Main execution loop of the Digital Substation Controller.
 *
 * Responsibilities:
 *   • Receive protection events from the ESP32-DEV controller
 *   • Process Earth Fault protection messages
 *   • Update breaker status indications
 *   • Execute periodic timer-based tasks
 *   • Maintain protection relay latching
 *
 * Communication:
 *   UART (ESP32-DEV  <---->  ESP32-S3)
 *
 ******************************************************************************/
void loop()
{
    //=========================================================================
    // Receive Protection Messages from ESP32-DEV
    //=========================================================================

    if (DevSerial.available())
    {
        String msg = DevSerial.readStringUntil('\n');
        msg.trim();

        Serial.print("UART RX : ");
        Serial.println(msg);

        //=====================================================================
        // 132 kV Feeder-1 Earth Fault Protection
        //=====================================================================

        if (msg.indexOf("FDR1_EF_TRIP") >= 0)
        {
            FDR1_TRIPPED = true;

            digitalWrite(RELAY1, LOW);

            Blynk.virtualWrite(V10, 1);

            Serial.println("Feeder-1 Earth Fault Trip");
        }

        else if (msg.indexOf("FDR1_EF_RESET") >= 0)
        {
            FDR1_TRIPPED = false;

            digitalWrite(RELAY1, HIGH);

            Blynk.virtualWrite(V10, 0);

            Serial.println("Feeder-1 Earth Fault Reset");
        }

        //=====================================================================
        // 132 kV Feeder-2 Earth Fault Protection
        //=====================================================================

        else if (msg.indexOf("FDR2_EF_TRIP") >= 0)
        {
            FDR2_TRIPPED = true;

            digitalWrite(RELAY2, LOW);

            Blynk.virtualWrite(V11, 1);

            Serial.println("Feeder-2 Earth Fault Trip");
        }

        else if (msg.indexOf("FDR2_EF_RESET") >= 0)
        {
            FDR2_TRIPPED = false;

            digitalWrite(RELAY2, HIGH);

            Blynk.virtualWrite(V11, 0);

            Serial.println("Feeder-2 Earth Fault Reset");
        }

        //=====================================================================
        // Power Transformer-1 HV Earth Fault
        //=====================================================================

        else if (msg.indexOf("PTR1_HVEF_TRIP") >= 0)
        {
            PTR1_HV_TRIP = true;

            digitalWrite(RELAY3, LOW);

            Blynk.virtualWrite(V12, 1);

            Serial.println("Transformer-1 HV Earth Fault");
        }

        else if (msg.indexOf("PTR1_HVEF_RESET") >= 0)
        {
            PTR1_HV_TRIP = false;

            digitalWrite(RELAY3, HIGH);

            Blynk.virtualWrite(V12, 0);

            Serial.println("Transformer-1 HV Earth Fault Reset");
        }

        //=====================================================================
        // Power Transformer-1 LV Earth Fault
        //=====================================================================

        else if (msg.indexOf("PTR1_LVEF_TRIP") >= 0)
        {
            PTR1_LV_TRIP = true;

            digitalWrite(RELAY5, LOW);

            Blynk.virtualWrite(V13, 1);

            Serial.println("Transformer-1 LV Earth Fault");
        }

        else if (msg.indexOf("PTR1_LVEF_RESET") >= 0)
        {
            PTR1_LV_TRIP = false;

            digitalWrite(RELAY5, HIGH);

            Blynk.virtualWrite(V13, 0);

            Serial.println("Transformer-1 LV Earth Fault Reset");
        }

        //=====================================================================
        // Power Transformer-2 HV Earth Fault
        //=====================================================================

        else if (msg.indexOf("PTR2_HVEF_TRIP") >= 0)
        {
            PTR2_HVEF_TRIPPED = true;

            digitalWrite(RELAY4, LOW);

            Blynk.virtualWrite(V14, 1);

            Serial.println("Transformer-2 HV Earth Fault");
        }

        else if (msg.indexOf("PTR2_HVEF_RESET") >= 0)
        {
            PTR2_HVEF_TRIPPED = false;

            digitalWrite(RELAY4, HIGH);

            Blynk.virtualWrite(V14, 0);

            Serial.println("Transformer-2 HV Earth Fault Reset");
        }

        //=====================================================================
        // Power Transformer-2 LV Earth Fault
        //=====================================================================

        else if (msg.indexOf("PTR2_LVEF_TRIP") >= 0)
        {
            PTR2_LV_TRIP = true;

            digitalWrite(RELAY6, LOW);

            Blynk.virtualWrite(V15, 1);

            Serial.println("Transformer-2 LV Earth Fault");
        }

        else if (msg.indexOf("PTR2_LVEF_RESET") >= 0)
        {
            PTR2_LV_TRIP = false;

            digitalWrite(RELAY6, HIGH);

            Blynk.virtualWrite(V15, 0);

            Serial.println("Transformer-2 LV Earth Fault Reset");
        }

        //=====================================================================
        // 33 kV Feeder-1 Earth Fault
        //=====================================================================

        else if (msg.indexOf("F33_1_EF_TRIP") >= 0)
        {
            F33_1_TRIPPED = true;

            digitalWrite(RELAY7, LOW);

            Blynk.virtualWrite(V16, 1);

            Serial.println("33 kV Feeder-1 Earth Fault");
        }

        else if (msg.indexOf("F33_1_EF_RESET") >= 0)
        {
            F33_1_TRIPPED = false;

            digitalWrite(RELAY7, HIGH);

            Blynk.virtualWrite(V16, 0);

            Serial.println("33 kV Feeder-1 Earth Fault Reset");
        }

        //=====================================================================
        // 33 kV Feeder-2 Earth Fault
        //=====================================================================

        else if (msg.indexOf("F33_2_EF_TRIP") >= 0)
        {
            F33_2_TRIPPED = true;

            digitalWrite(RELAY8, LOW);

            Blynk.virtualWrite(V17, 1);

            Serial.println("33 kV Feeder-2 Earth Fault");
        }

        else if (msg.indexOf("F33_2_EF_RESET") >= 0)
        {
            F33_2_TRIPPED = false;

            digitalWrite(RELAY8, HIGH);

            Blynk.virtualWrite(V17, 0);

            Serial.println("33 kV Feeder-2 Earth Fault Reset");
        }
    }

    //=========================================================================
    // Update Breaker Status
    //=========================================================================

    updateBreakerStatus();

    //=========================================================================
    // Execute Periodic Tasks
    //=========================================================================

    timer.run();

    //=========================================================================
    // Maintain Protection Relay Latching
    //=========================================================================

    if (PTR2_HVEF_TRIPPED)
    {
        digitalWrite(RELAY4, LOW);
        digitalWrite(RELAY6, LOW);
    }

    if (PTR2_LV_TRIP)
    {
        digitalWrite(RELAY6, LOW);
    }
}
/******************************************************************************
 * Function    : updateBreakerStatus()
 *
 * Description :
 * ----------------------------------------------------------------------------
 * Updates the real-time status of all circuit breakers on the Blynk IoT
 * dashboard.
 *
 * The function reads the relay output state and updates the corresponding
 * breaker indications, including:
 *
 *      • Circuit Breaker Open (CBO)
 *      • Circuit Breaker Closed (CBC)
 *      • TNC Control Lamp
 *
 * Relay Logic
 * ----------------------------------------------------------------------------
 * Relay LOW   → Breaker OPEN (Tripped)
 * Relay HIGH  → Breaker CLOSED (Healthy)
 *
 * Returns :
 *      None
 ******************************************************************************/
void updateBreakerStatus()
{
    //=========================================================================
    // 132 kV Feeder-1 Breaker Status
    //=========================================================================

    if (digitalRead(RELAY1) == LOW)
    {
        Blynk.virtualWrite(V7, 1);      // Breaker Open (CBO)
        Blynk.virtualWrite(V8, 0);      // Breaker Closed (CBC)
        Blynk.virtualWrite(V3, 1);      // TNC Indication
    }
    else
    {
        Blynk.virtualWrite(V7, 0);
        Blynk.virtualWrite(V8, 1);
        Blynk.virtualWrite(V3, 0);
    }

    //=========================================================================
    // 132 kV Feeder-2 Breaker Status
    //=========================================================================

    if (digitalRead(RELAY2) == LOW)
    {
        Blynk.virtualWrite(V18, 1);
        Blynk.virtualWrite(V19, 0);
        Blynk.virtualWrite(V14, 1);
    }
    else
    {
        Blynk.virtualWrite(V18, 0);
        Blynk.virtualWrite(V19, 1);
        Blynk.virtualWrite(V14, 0);
    }

    //=========================================================================
    // Power Transformer-1 HV Breaker Status
    //=========================================================================

    if (digitalRead(RELAY3) == LOW)
    {
        Blynk.virtualWrite(V29, 1);
        Blynk.virtualWrite(V30, 0);
        Blynk.virtualWrite(V27, 1);
    }
    else
    {
        Blynk.virtualWrite(V29, 0);
        Blynk.virtualWrite(V30, 1);
        Blynk.virtualWrite(V27, 0);
    }

    //=========================================================================
    // Power Transformer-1 LV Breaker Status
    //=========================================================================

    if (digitalRead(RELAY5) == LOW)
    {
        Blynk.virtualWrite(V49, 1);
        Blynk.virtualWrite(V50, 0);
        Blynk.virtualWrite(V48, 1);
    }
    else
    {
        Blynk.virtualWrite(V49, 0);
        Blynk.virtualWrite(V50, 1);
        Blynk.virtualWrite(V48, 0);
    }

    //=========================================================================
    // Power Transformer-2 HV Breaker Status
    //=========================================================================

    if (digitalRead(RELAY4) == LOW)
    {
        Blynk.virtualWrite(V40, 1);
        Blynk.virtualWrite(V41, 0);
        Blynk.virtualWrite(V38, 1);
    }
    else
    {
        Blynk.virtualWrite(V40, 0);
        Blynk.virtualWrite(V41, 1);
        Blynk.virtualWrite(V38, 0);
    }

    //=========================================================================
    // Power Transformer-2 LV Breaker Status
    //=========================================================================

    if (digitalRead(RELAY6) == LOW)
    {
        Blynk.virtualWrite(V58, 1);
        Blynk.virtualWrite(V59, 0);
        Blynk.virtualWrite(V57, 1);
    }
    else
    {
        Blynk.virtualWrite(V58, 0);
        Blynk.virtualWrite(V59, 1);
        Blynk.virtualWrite(V57, 0);
    }

    //=========================================================================
    // 33 kV Feeder-1 Breaker Status
    //=========================================================================

    if (digitalRead(RELAY7) == LOW)
    {
        Blynk.virtualWrite(V68, 1);
        Blynk.virtualWrite(V69, 0);
        Blynk.virtualWrite(V67, 1);
    }
    else
    {
        Blynk.virtualWrite(V68, 0);
        Blynk.virtualWrite(V69, 1);
        Blynk.virtualWrite(V67, 0);
    }

    //=========================================================================
    // 33 kV Feeder-2 Breaker Status
    //=========================================================================

    if (digitalRead(RELAY8) == LOW)
    {
        Blynk.virtualWrite(V76, 1);
        Blynk.virtualWrite(V77, 0);
        Blynk.virtualWrite(V75, 1);
    }
    else
    {
        Blynk.virtualWrite(V76, 0);
        Blynk.virtualWrite(V77, 1);
        Blynk.virtualWrite(V75, 0);
    }
}
