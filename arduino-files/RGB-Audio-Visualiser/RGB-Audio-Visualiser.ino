#include <string.h>
#include <arduinoFFT.h>
#include <Adafruit_NeoPixel.h>

#define LBTN_PIN 4
#define MBTN_PIN 3
#define RBTN_PIN 2

#define MIC_PIN A7
arduinoFFT FFT = arduinoFFT();

#define LED_PIN 10
#define LED_COUNT 64
Adafruit_NeoPixel pixels(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

// Button Press State Variables
int buttonStateLeft = 1;      // Button states for the "Left" command
int lastButtonStateLeft = 1;
int buttonStateMid = 1;       // Button states for the "Mid" command
int lastButtonStateMid = 1;       
int buttonStateRight = 1;     // Button states for the "Right" command
int lastButtonStateRight = 1;         

// Lighting Profile Variables
int type = 0;

// Sampling Variables
const int samples = 64;     // Samples must be in powers of 2
double samplingFrequency;

double vReal[samples];      // Real Values
double vImag[samples];      // Imaginary Values
double rawMag[samples];     // Raw Magnitude Values
double rawFreq[samples];    // Raw Frequency Values

// Sampling Frequency Variables
unsigned long startTime;
unsigned long endTime;
unsigned long timeLapsed;

// Set Num of Bands
int band;                   // Assign Frequency to Bands
const int bands = 8;        // Number of Frequency Bands - [128, 256, 512, 1024, 2048, 4096, 8192, 16384]

double finalMag[bands];     // Final Magnitude Values [Average]
int normalisedMag[bands] = {0, 0, 0, 0, 0, 0, 0, 0};   // Normalised Magnitude Values [0~7]

// Set minMag for Each Band - Calibrated Average Minimum Magnitude
double minMag[] = {1350, 707, 492, 403, 357, 292, 251, 390}; // {1256, 646, 450, 376, 328, 281, 246, 375}
int my_cal_result;  // For setting the LED Row Height

// Assign LED Matrix Index
int ledMatrix[8][8] = {
  { 0,  1,  2, 3,  4,  5,  6,  7},
  { 8,  9, 10, 11, 12, 13, 14, 15},
  {16, 17, 18, 19, 20, 21, 22, 23},
  {24, 25, 26, 27, 28, 29, 30, 31},
  {32, 33, 34, 35, 36, 37, 38, 39},
  {40, 41, 42, 43, 44, 45, 46, 47},
  {48, 49, 50, 51, 52, 53, 54, 55},
  {56, 57, 58, 59, 60, 61, 62, 63} };

// Setup RGB Values & Brightness
uint16_t pixelHue;
uint16_t ledMatrixRGB[8][8] = {
  { 0, 0, 0, 0, 0, 0, 0, 0},
  { 0, 0, 0, 0, 0, 0, 0, 0},
  { 0, 0, 0, 0, 0, 0, 0, 0},
  { 0, 0, 0, 0, 0, 0, 0, 0},
  { 0, 0, 0, 0, 0, 0, 0, 0},
  { 0, 0, 0, 0, 0, 0, 0, 0},
  { 0, 0, 0, 0, 0, 0, 0, 0},
  { 0, 0, 0, 0, 0, 0, 0, 0} };

uint8_t ledMatrixBrightness[8][8] = {
  { 0, 0, 0, 0, 0, 0, 0, 0},
  { 0, 0, 0, 0, 0, 0, 0, 0},
  { 0, 0, 0, 0, 0, 0, 0, 0},
  { 0, 0, 0, 0, 0, 0, 0, 0},
  { 0, 0, 0, 0, 0, 0, 0, 0},
  { 0, 0, 0, 0, 0, 0, 0, 0},
  { 0, 0, 0, 0, 0, 0, 0, 0},
  { 0, 0, 0, 0, 0, 0, 0, 0} };

void setup() {
  pinMode(LBTN_PIN, INPUT_PULLUP);
  pinMode(MBTN_PIN, INPUT_PULLUP);
  pinMode(RBTN_PIN, INPUT_PULLUP);
  pinMode(MIC_PIN, INPUT);
  pixels.begin();
  pixels.show();
  // Serial.begin(115200);
}

void loop() {
  getLightingType();              // Get Lighting Type (RGB/Single)
  getRawFreqMag(rawFreq, rawMag); // (output double rawFreq[] as rawFreq, output double vReal[] as rawMag)
  getFinalMag();                  // Get Final Magnitude using Frequency Bands
  getNormalisedMag();             // Get Normalised Magnitude sine there are 8 LEDs
  getColour();                    // Get Colours & Assign LED to Display
  performRGB();                   // Display Colours to Specific LEDs
}

void getRawFreqMag(double* rawFreq, double* vReal) {
  // Measure time lapsed from sampling
  startTime = micros();
  for(int i = 0; i < samples; i++) {
    vReal[i] = analogRead(MIC_PIN); // Sampling real signals from microphone
    vImag[i] = 0.0;                 // Imaginary part must be zeroed in case of looping to avoid wrong calculations and overflows
  }
  endTime = micros();
  timeLapsed = endTime - startTime;
  samplingFrequency = 1e6 * samples / timeLapsed; 

  // Calculate rawMag values
  FFT.Windowing(vReal, samples, FFT_WIN_TYP_HANN, FFT_FORWARD);  // Reduce the amplitude of the discontinuities at the boundaries of each signal
  FFT.Compute(vReal, vImag, samples, FFT_FORWARD);               // Compute FFT
  FFT.ComplexToMagnitude(vReal, vImag, samples);                 // Compute magnitudes

  // Calculate rawFreq values
  for(int i = 0; i < samples; i++) {
    rawFreq[i] = ((i * 1.0 * samplingFrequency) / samples);
  }
}

void getFinalMag() {
  // Divide the Raw Frequency into eight groups
  double minFreq = rawFreq[1];
  double maxFreq = rawFreq[samples - 1];
  double divFreq = (maxFreq - minFreq) / bands;
  
  // Update Sum of Magnitude & Magnitude Count Per Band
  double sumMag[] = {0,0,0,0,0,0,0,0};
  double countMag[] = {0,0,0,0,0,0,0,0};
  
  // Read Each Sample Frequency & Sum Each Magnitude in Their Respective Band
  for(int i = 1; i < samples; i++) {  //Skip the first frequency as it represents 0hz
    // Each Sample Frequency & Magnitude
    double sampleFreq = rawFreq[i];
    double sampleMag = rawMag[i];

    // Assign each frequency to their bands
    if      (sampleFreq <   divFreq + minFreq) band = 0;
    else if (sampleFreq < 2*divFreq + minFreq) band = 1;
    else if (sampleFreq < 3*divFreq + minFreq) band = 2;
    else if (sampleFreq < 4*divFreq + minFreq) band = 3;
    else if (sampleFreq < 5*divFreq + minFreq) band = 4;
    else if (sampleFreq < 6*divFreq + minFreq) band = 5;
    else if (sampleFreq < 7*divFreq + minFreq) band = 6;
    else                                       band = 7;

    // Sum Each Magnitude in Their Respective Bands & Update Magnitude Count Per Band
    switch(band) {
      case 0:
        sumMag[0] += sampleMag;
        countMag[0]++;
      case 1:
        sumMag[1] += sampleMag;
        countMag[1]++;
      case 2:
        sumMag[2] += sampleMag;
        countMag[2]++;
      case 3:
        sumMag[3] += sampleMag;
        countMag[3]++;
      case 4:
        sumMag[4] += sampleMag;
        countMag[4]++;
      case 5:
        sumMag[5] += sampleMag;
        countMag[5]++;
      case 6:
        sumMag[6] += sampleMag;
        countMag[6]++;
      case 7:
        sumMag[7] += sampleMag;
        countMag[7]++;
    }
  }
  // Find the Final Magnitude (Average) in Each Band
  for(int i = 0; i < bands; i++) {
    if (countMag[i] == 0) finalMag[i] = 0;
    else                  finalMag[i] = sumMag[i]/countMag[i];
  }
}

//Update Normalised Magnitude in Each Band
void getNormalisedMag() {
  for(int i = 0; i < bands; i++) {
    // Use the Fraction of the Difference over the Min Mag Value as the LED Column Height
    my_cal_result = floor(10 * (finalMag[i] - minMag[i]) / finalMag[i]);

    if (my_cal_result > 8) my_cal_result = 8; // Prevent Getting Above the Highest LED Column 
    if (my_cal_result < 0) my_cal_result = 0; // Prevent Getting Below the Lowest LED Column
    
    if (type == 0 || type == 2) normalisedMag[i] = my_cal_result - 1;
    else                        normalisedMag[i] = my_cal_result;
  }
}

void resetLED() {
  memset(ledMatrixBrightness, 0, sizeof(ledMatrixBrightness)); // Zero Brightness on all the LEDs
}

void getLightingType() {
  // Check change in middle button state
  buttonStateMid = digitalRead(MBTN_PIN);
  if (buttonStateMid != lastButtonStateMid && buttonStateMid == 0) {
    type++;
    if (type > 3) type = 0; // 4 Lighting Types
  }
  lastButtonStateMid = buttonStateMid;  // Update Button State
}

// Get RGB Colour and Assign to LED
void getColour() {
  resetLED(); // Clear all the LED

  // RGB Cycle & Single Colour
  if (type == 0 || type == 1) pixelHue += 29;
  else {
    buttonStateLeft = digitalRead(LBTN_PIN);
    buttonStateRight = digitalRead(RBTN_PIN);
    if (buttonStateLeft == 0)  pixelHue -= 269;
    if (buttonStateRight == 0) pixelHue += 269;
  }

  // Find Peak LED in each band
  for(int ledRow = 0; ledRow < bands; ledRow++) {
    // Assign Colour to Peak LED (Highest Per Band)
    int ledCol = normalisedMag[ledRow];

    if (type == 0 || type == 2) {
      for (ledCol; ledCol >= 0; ledCol--) {
        ledMatrixBrightness[ledRow][ledCol] = 16;  // Selected LED Will Light Up
        ledMatrixRGB[ledRow][ledCol] = pixelHue;   // Set LED to Hue Colour Value
      }
    }
    else {
      ledMatrixBrightness[ledRow][ledCol] = 16;
      ledMatrixRGB[ledRow][ledCol] = pixelHue;
    }
  }
}

// Output settings to each of the LEDs
void performRGB() {
  for (int row = 0; row < 8; row++) {
    for (int col = 0; col < 8; col++) {
      // setPixelColor Parameters (LED Index, 16-bit Colour Value)
      // ColorHSV Parameters (Hue, Saturation, Brightness)
      pixels.setPixelColor(ledMatrix[row][col], pixels.ColorHSV(ledMatrixRGB[row][col], 255, ledMatrixBrightness[row][col]));
    }
  }
  pixels.show();  // Update LEDs
}