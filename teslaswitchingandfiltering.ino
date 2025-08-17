// === KODE ARDUINO DENGAN MOVING AVERAGE FILTER DAN FORMAT CSV UNTUK COOLTERM ===

const int currentPin = A0;
const int voltagePin = A1;
const int relay1 = A2;
const int relay2 = A3;

const float ACS_ZERO = 2.5;
const float MV_PER_AMP = 0.1;
const float VOLTAGE_SCALE = 5.0;

const unsigned long intervalRead = 1000;
const unsigned long switchingInterval = 10UL * 60UL * 1000UL; // 10 menit bisa diubah sesuai durasi yang diinginkan
const float voltageCutoff = 10.5; // Batas tegangan cutoff

const int filterSamples = 10;
float voltageBuffer[filterSamples];
float currentBuffer[filterSamples];
int filterIndex = 0;
bool bufferFilled = false;

unsigned long lastReadMillis = 0;
unsigned long lastSwitchMillis = 0;
unsigned long sessionStartMillis = 0;

bool relay1Active = true;

void setup() {
  Serial.begin(9600);
  delay(1000);

  pinMode(relay1, OUTPUT);
  pinMode(relay2, OUTPUT);
  digitalWrite(relay1, HIGH);
  digitalWrite(relay2, HIGH);

  Serial.println("Waktu (detik),Tegangan (V),Arus (A),Relay");

  sessionStartMillis = millis();
  lastSwitchMillis = millis();
  lastReadMillis = millis();

  if (relay1Active) {
    digitalWrite(relay1, LOW);
  } else {
    digitalWrite(relay2, LOW);
  }

  delay(3000);
}

void loop() {
  unsigned long currentMillis = millis();

  // === Switching relay setiap 10 menit ===
  if (currentMillis - lastSwitchMillis >= switchingInterval) {
    digitalWrite(relay1, HIGH);
    digitalWrite(relay2, HIGH);
    delay(1000);

    relay1Active = !relay1Active;

    if (relay1Active) {
      digitalWrite(relay1, LOW);
    } else {
      digitalWrite(relay2, LOW);
    }

    delay(3000);
    lastSwitchMillis = millis();
    lastReadMillis = millis();
    return;
  }

  // === Pembacaan sensor dengan filter setiap 1 detik ===
  if (currentMillis - lastReadMillis >= intervalRead) {
    lastReadMillis = currentMillis;

    float rawVoltage = analogRead(voltagePin) * (5.0 / 1024.0);
    float rawCurrent = analogRead(currentPin) * (5.0 / 1024.0);

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
    unsigned long seconds = (currentMillis - sessionStartMillis) / 1000;

    String activeRelay = relay1Active ? "A2" : "A3";

    Serial.print(seconds);
    Serial.print(",");
    Serial.print(inputVoltage, 2);
    Serial.print(",");
    Serial.print(current, 2);
    Serial.print(",");
    Serial.println(activeRelay);

    // === STOP jika Tegangan ≤ cutoff ===
    if (inputVoltage <= voltageCutoff) {
      digitalWrite(relay1, HIGH);
      digitalWrite(relay2, HIGH);
      Serial.println("!,Tegangan <= Batas cutoff. Sistem dihentikan.");
      Serial.flush();
      while (true); // Hentikan sistem
    }
  }
}
