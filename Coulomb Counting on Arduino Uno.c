/* ═══════════════════════════════════════════════
   Coulomb Counting — Arduino Uno
   Hardware: NCV84045 CS pin → A0
             5kΩ pull down on A0 to GND
   Battery:  Exide 9Ah 12V
   ═══════════════════════════════════════════════ */

/* ── Constants ─────────────────────────────── */
#define BATTERY_CAPACITY_AH     9.0f
#define BATTERY_CAPACITY_AS     (BATTERY_CAPACITY_AH * 3600.0f)  /* 32400 As */

#define VREF                    5.0f      /* Arduino Uno VREF = 5V        */
#define ADC_RES                 1023.0f   /* Arduino Uno = 10 bit         */
#define R_SENSE                 150.0f    /* R6 on PDM schematic          */
#define CS_RATIO                1415.0f   /* NCV84045 K4/K5 typical       */
#define NOISE_THRESHOLD_V       0.015f    /* 15mV noise floor             */
#define ADC_PIN                 A0        /* CS pin connected here        */

/* ── Battery State Variables ───────────────── */
float current_a           = 0.0f;
float soc_percent         = 100.0f;
float remaining_charge_as = BATTERY_CAPACITY_AS;
float voltage_cs          = 0.0f;
int   raw_adc             = 0;

unsigned long last_tick   = 0;

/* ══════════════════════════════════════════════
   SETUP
   ══════════════════════════════════════════════ */
void setup()
{
    Serial.begin(9600);

    /* Set ADC reference to default 5V */
    analogReference(DEFAULT);

    /* Initialize timing */
    last_tick = millis();

    Serial.println("=============================");
    Serial.println("  Coulomb Counter Started    ");
    Serial.println("  Battery: Exide 9Ah 12V     ");
    Serial.println("=============================");
    Serial.println("ADC | Voltage | Current | SoC% | Charge");
}

/* ══════════════════════════════════════════════
   LOOP
   ══════════════════════════════════════════════ */
void loop()
{
    unsigned long now   = millis();
    unsigned long dt_ms = now - last_tick;

    if (dt_ms == 0)
        return;

    last_tick = now;

    /* ── 1. Read ADC ── */
    raw_adc = analogRead(ADC_PIN);

    /* Clamp */
    if (raw_adc > 1023)
        raw_adc = 1023;

    /* ── 2. Convert to voltage ── */
    voltage_cs = ((float)raw_adc / ADC_RES) * VREF;

    /* ── 3. Noise floor ── */
    if (voltage_cs < NOISE_THRESHOLD_V)
    {
        voltage_cs = 0.0f;
        current_a  = 0.0f;
    }
    else
    {
        /* ── 4. Real current from NCV84045 ── */
        current_a = (voltage_cs / R_SENSE) * CS_RATIO;
    }

    /* ── 5. Coulomb counting ── */
    float dt_s        = (float)dt_ms / 1000.0f;
    float used_charge = current_a * dt_s;

    if (remaining_charge_as > used_charge)
        remaining_charge_as -= used_charge;
    else
        remaining_charge_as = 0.0f;

    /* ── 6. SoC ── */
    soc_percent = (remaining_charge_as / BATTERY_CAPACITY_AS) * 100.0f;

    /* ── 7. Print to Serial Monitor ── */
    Serial.print(raw_adc);
    Serial.print("  |  ");
    Serial.print(voltage_cs, 4);
    Serial.print("V  |  ");
    Serial.print(current_a, 3);
    Serial.print("A  |  ");
    Serial.print(soc_percent, 2);
    Serial.print("%  |  ");
    Serial.print(remaining_charge_as, 1);
    Serial.println(" As");

    delay(1000);
}