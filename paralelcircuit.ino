// Paralel Circuit
// === KODE ARDUINO: PENGUKURAN TANPA RELAY, HENTI JIKA TEGANGAN ≤ 10.5V ===
// === DENGAN FILTER MOVING AVERAGE ===

const int currentPin = A0;
const int voltagePin = A1;

const float ACS_ZERO = 2.5;        // Tegangan tengah sensor arus (bias)
const float MV_PER_AMP = 0.1;      // Skala sensor arus
const float VOLTAGE_SCALE = 5.0;   // Skala pembagi tegangan

const unsigned long intervalRead = 1000; // interval pembacaan dalam ms
const float voltageCutoff = 10.5;        // batas cutoff voltase

// === Parameter Filter Moving Average ===
const int filterSamples = 10;
float voltageBuffer[filterSamples];
float currentBuffer[filterSamples];
int filterIndex = 0;
bool bufferFilled = false;

unsigned long lastReadMillis = 0;

void setup() {
  Serial.begin(9600);
  delay(1000);

  Serial.println("Time(s),Voltage(V),Current(A)");
  lastReadMillis = millis();
}

void loop() {
  unsigned long currentMillis = millis();

  if (currentMillis - lastReadMillis >= intervalRead) {
    lastReadMillis = currentMillis;

    float rawVoltage = analogRead(voltagePin) * (5.0 / 1024.0);
    float rawCurrent = analogRead(currentPin) * (5.0 / 1024.0);

    // Simpan ke buffer filter
    voltageBuffer[filterIndex] = rawVoltage;
    currentBuffer[filterIndex] = rawCurrent;
    filterIndex++;

    if (filterIndex >= filterSamples) {
      filterIndex = 0;
      bufferFilled = true;
    }

    int count = bufferFilled ? filterSamples : filterIndex;
    float voltageSum = 0, currentSum = 0;

    for (int i = 0; i < count; i++) {
      voltageSum += voltageBuffer[i];
      currentSum += currentBuffer[i];
    }

    float avgVoltage = voltageSum / count;
    float avgCurrent = currentSum / count;

    float inputVoltage = avgVoltage * VOLTAGE_SCALE;
    float current = (avgCurrent - ACS_ZERO) / MV_PER_AMP;

    unsigned long seconds = currentMillis / 1000;

    Serial.print(seconds);
    Serial.print(",");
    Serial.print(inputVoltage, 2);
    Serial.print(",");
    Serial.println(current, 2);

    // === Hentikan program jika tegangan di bawah batas ===
    if (inputVoltage <= voltageCutoff) {
      Serial.println("Tegangan ≤ 10.5V. Pengukuran dihentikan.");
      Serial.flush();
      while (true); // Hentikan loop
    }
  }
}
