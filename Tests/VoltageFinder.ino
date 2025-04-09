//int pin = A10;  // Pin 10 where voltage is read
float Vref = 3.3;  // Reference voltage for ADC
float R1 = 1000000.0;  // Resistor 1 (1M Ohm)
float R2 = 100000.0;  // Resistor 2 (100k Ohm)
float Vmin = 3.5;  // Min battery voltage
float Vmax = 4.2;  // Max battery voltage
float gammaMax = 3.0;
float gammaMin = 0.5;


int adcValue = 125;

void setup() {
  Serial.begin(9600);
}

void loop() {
  //int adcValue = analogRead(pin);
  
  // Calculate the voltage from ADC value
  float Vout = (adcValue / 1023.0) * Vref;  // Output voltage after voltage divider
  float Vbattery = Vout * (R1 + R2) / R2;  // Reversed voltage divider equation

  // Map the voltage to battery percentage
  float gamma = gammaMin + (gammaMax - gammaMin) * (Vbattery - Vmin) / (Vmax - Vmin);
  float batteryPercentage = pow((Vbattery - Vmin) / (Vmax - Vmin), gamma) * 100;

  if (batteryPercentage < 0) batteryPercentage = 0;
  if (batteryPercentage > 100) batteryPercentage = 100;

  // Print the result
  Serial.print("Voltage out: ");
  Serial.print(Vout);
  Serial.print("Battery Voltage: ");
  Serial.print(Vbattery);
  Serial.print("V, Battery Percentage: ");
  Serial.print(batteryPercentage);
  Serial.println("%");

  delay(1000);  // Wait 1 second before next reading
  adcValue = adcValue - 1;
}