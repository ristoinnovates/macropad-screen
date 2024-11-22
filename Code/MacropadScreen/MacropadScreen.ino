// Risto Innovates
// Macropad Screen v1

#include <Arduino.h>
#include "Keyboard.h"
#include "Mouse.h"
#include <Encoder.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h> 

#define TFT_CS     8    // Chip select line to pin 8
#define TFT_RST    14   // Reset line to pin 14
#define TFT_DC     7    // Data/command line to pin 7
#define TFT_LED    9    // LED line to pin 9
#define ENCODER_BUTTON_PIN 4 // Encoder button connected to pin 4

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

#define ICON_SIZE 46    // Size of each icon
#define ICON_SPACING 8  // Space between icons
#define BORDER_RADIUS 12 // Larger radius for smoother rounded corners

int cursorPosition = 0;      // Tracks current cursor position (0-3 on each page)
int previousCursorPosition = 0; // Tracks the previous cursor position for partial redraw
int selectedIcon = 0;        // Tracks the selected icon across pages
int currentPage = 0;         // Tracks the current page (0 for first, 1 for second)
int previousPage = 0;        // Tracks the previous page for partial redraw
int encoderPinA = 2;
int encoderPinB = 3;
Encoder myEncoder(encoderPinA, encoderPinB); // Use the Encoder library for easy handling
int oldEncoderPos = 0;       // To track changes in encoder position

// Function Prototypes
void displayIcons(bool fullRedraw = true);
void updateCursor(int direction);
void selectIcon();
void updateEncoder();
void drawPaginationDots();
void drawIcon(int x, int y, uint16_t startColor, uint16_t endColor, const char* label);


// MUX
// Outputs
int muxChannel1 = A0;
int muxChannel2 = A1;
int muxChannel3 = A2;
int muxChannel4 = A3;

// Inputs
int muxSwitchesInput = 6;

// MULTIPLEXER 1 BUTTONS - 16 CH
const int NUMBER_MUX_BUTTONS = 16;
bool muxButtonsCurrentState[NUMBER_MUX_BUTTONS] = {0};
bool muxButtonsPreviousState[NUMBER_MUX_BUTTONS] = {0};

unsigned long lastDebounceTimeMUX[NUMBER_MUX_BUTTONS] = {0};
unsigned long debounceDelayMUX = 5;

void setup() {
  Serial.begin(9600);

  Keyboard.begin();
  Mouse.begin();

  // Initialize TFT display
  tft.initR(INITR_144GREENTAB); // Use correct initialization for 1.44" display
  tft.setRotation(0); // Rotate if needed, 1 for landscape
  
  // Set max SPI frequency
  SPI.beginTransaction(SPISettings(24000000, MSBFIRST, SPI_MODE0));

  // Turn on the display backlight
  pinMode(TFT_LED, OUTPUT);
  digitalWrite(TFT_LED, HIGH);

  
  tft.fillScreen(ST7735_BLACK); // Clear screen

  // Initialize encoder button pin
  pinMode(ENCODER_BUTTON_PIN, INPUT_PULLUP);
  
  // Set up encoder pins
  pinMode(encoderPinA, INPUT_PULLUP);
  pinMode(encoderPinB, INPUT_PULLUP);

  pinMode(muxChannel1, OUTPUT);
  pinMode(muxChannel2, OUTPUT);
  pinMode(muxChannel3, OUTPUT);
  pinMode(muxChannel4, OUTPUT);
  digitalWrite(muxChannel1, LOW);
  digitalWrite(muxChannel2, LOW);
  digitalWrite(muxChannel3, LOW);
  digitalWrite(muxChannel4, LOW);

  pinMode(muxSwitchesInput, INPUT_PULLUP); // Digital input for the first set of switches

  String label = "Risto";
  tft.setCursor(35, 45);
  tft.setTextColor(ST7735_ORANGE); // Set text color to white
  tft.setTextSize(2);
  tft.print(label);

  label = "Innovates";
  tft.setCursor(10, 65);
  tft.print(label);
  delay(2000);
  tft.fillScreen(ST7735_BLACK); // Clear screen

  // Initialize and display icons with the first icon selected
  displayIcons(true);
}

void loop() {
  updateEncoder();  // Handle encoder updates for navigation and selection

  // Check if encoder button is pressed
  if (digitalRead(ENCODER_BUTTON_PIN) == LOW) {
    selectIcon();
    delay(200); // Simple debounce
  }

  updateMUXButtons(muxSwitchesInput, NUMBER_MUX_BUTTONS, muxButtonsCurrentState, muxButtonsPreviousState, lastDebounceTimeMUX, debounceDelayMUX);
}

void updateMUXButtons(int muxInputPin, int numberOfButtons, bool *currentState, bool *previousState, unsigned long *lastDebounceTime, unsigned long debounceDelay) {
  for (int i = 0; i < numberOfButtons; i++) {
    int A = bitRead(i, 0);
    int B = bitRead(i, 1);
    int C = bitRead(i, 2);
    int D = bitRead(i, 3);
    digitalWrite(muxChannel1, A);
    digitalWrite(muxChannel2, B);
    digitalWrite(muxChannel3, C);
    digitalWrite(muxChannel4, D);
    delay(1);
    currentState[i] = digitalRead(muxInputPin);

    if ((millis() - lastDebounceTime[i]) > debounceDelay) {
      if (currentState[i] != previousState[i]) {
        lastDebounceTime[i] = millis();

        if (currentState[i] == LOW) {
          pressButton(i, 1); 
        } else {
          pressButton(i, 0);
        }
        previousState[i] = currentState[i];
      }
    }
  }
}


// Function to draw each icon with a solid color and label
void drawIcon(int x, int y, uint16_t color, const int item ) {
  // Draw solid color background with rounded corners
  tft.fillRoundRect(x, y, ICON_SIZE, ICON_SIZE, BORDER_RADIUS, color);
  char* label = "PS";
  switch(item) {
    case 0:
      label = "PS";
      // Draw label text in the center of the icon
      tft.setCursor(x + 7, y + 13);
      tft.setTextColor(ST7735_WHITE); // Set text color to white
      tft.setTextSize(3);
      tft.print(label);
      break;
    case 1:
      label = "LR";
      // Draw label text in the center of the icon
      tft.setCursor(x + 7, y + 13);
      tft.setTextColor(ST7735_WHITE); // Set text color to white
      tft.setTextSize(3);
      tft.print(label);
      break;
    case 2:
      label = "DaVinci";
      // Draw label text in the center of the icon
      tft.setCursor(x + 3, y + 13);
      tft.setTextColor(ST7735_WHITE); // Set text color to white
      tft.setTextSize(1);
      tft.print(label);

      label = "Resolve";
      // Draw label text in the center of the icon
      tft.setCursor(x + 3, y + 26);
      tft.setTextColor(ST7735_WHITE); // Set text color to white
      tft.setTextSize(1);
      tft.print(label);
      break;
    case 3:
      label = "Logic";
      // Draw label text in the center of the icon
      tft.setCursor(x + 9, y + 13);
      tft.setTextColor(ST7735_BLACK); // Set text color to white
      tft.setTextSize(1);
      tft.print(label);

      label = "Pro X";
      // Draw label text in the center of the icon
      tft.setCursor(x + 9, y + 26);
      tft.setTextColor(ST7735_BLACK); // Set text color to white
      tft.setTextSize(1);
      tft.print(label);
      break;
    case 4:
      label = "VS";
      // Draw label text in the center of the icon
      tft.setCursor(x + 13, y + 7);
      tft.setTextColor(ST7735_WHITE); // Set text color to white
      tft.setTextSize(2);
      tft.print(label);

      label = "Code";
      // Draw label text in the center of the icon
      tft.setCursor(x + 1, y + 23);
      tft.setTextColor(ST7735_WHITE); // Set text color to white
      tft.setTextSize(2);
      tft.print(label);
      break;
    case 5:
      label = "Arduino";
      // Draw label text in the center of the icon
      tft.setCursor(x + 3, y + 10);
      tft.setTextColor(ST7735_WHITE); // Set text color to white
      tft.setTextSize(1);
      tft.print(label);

      label = "IDE";
      // Draw label text in the center of the icon
      tft.setCursor(x + 5, y + 23);
      tft.setTextColor(ST7735_WHITE); // Set text color to white
      tft.setTextSize(2);
      tft.print(label);
      break;
    case 6:
      label = "Fusion";
      // Draw label text in the center of the icon
      tft.setCursor(x + 5, y + 9);
      tft.setTextColor(ST7735_WHITE); // Set text color to white
      tft.setTextSize(1);
      tft.print(label);

      label = "360";
      // Draw label text in the center of the icon
      tft.setCursor(x + 5, y + 21);
      tft.setTextColor(ST7735_WHITE); // Set text color to white
      tft.setTextSize(2);
      tft.print(label);
      break;
    case 7:
    label = "Custom";
      // Draw label text in the center of the icon
      tft.setCursor(x + 5, y + 19);
      tft.setTextColor(ST7735_WHITE); // Set text color to white
      tft.setTextSize(1);
      tft.print(label);
      break;

  }

  
}


// Function to display icons with pagination and cursor navigation
void displayIcons(bool fullRedraw) {
  // Define icon positions for a 2x2 grid
  int iconX[4] = {14, 14 + ICON_SIZE + ICON_SPACING, 14, 14 + ICON_SIZE + ICON_SPACING};
  int iconY[4] = {10, 10, 10 + ICON_SIZE + ICON_SPACING, 10 + ICON_SIZE + ICON_SPACING};

  if (fullRedraw || currentPage != previousPage) {
    // Clear the screen and redraw all icons if full redraw or page change
    tft.fillScreen(ST7735_BLACK);
    for (int i = 0; i < 4; i++) {
      int iconIndex = currentPage * 4 + i;

      // Choose color and label based on iconIndex
      switch (iconIndex) {
        case 0: drawIcon(iconX[i], iconY[i], tft.color565(0, 153, 204), 0); break;
        case 1: drawIcon(iconX[i], iconY[i], tft.color565(51, 153, 204), 1); break;
        case 2: drawIcon(iconX[i], iconY[i], tft.color565(204, 153, 0), 2); break;
        case 3: drawIcon(iconX[i], iconY[i], tft.color565(192, 192, 192), 3); break;
        case 4: drawIcon(iconX[i], iconY[i], tft.color565(0, 153, 255), 4); break;
        case 5: drawIcon(iconX[i], iconY[i], tft.color565(0, 153, 51), 5); break;
        case 6: drawIcon(iconX[i], iconY[i], tft.color565(255, 102, 0), 6); break;
        case 7: drawIcon(iconX[i], iconY[i], tft.color565(255, 128, 0), 7); break;

      }

      // Draw outlines for the selected icon and cursor position
      if (iconIndex == selectedIcon) {
        tft.drawRoundRect(iconX[i] - 2, iconY[i] - 2, ICON_SIZE + 4, ICON_SIZE + 4, BORDER_RADIUS, ST7735_RED);
      } else if (i == cursorPosition) {
        tft.drawRoundRect(iconX[i] - 2, iconY[i] - 2, ICON_SIZE + 4, ICON_SIZE + 4, BORDER_RADIUS, ST7735_WHITE);
      }
    }
    drawPaginationDots(); // Draw pagination dots for full redraw
    previousPage = currentPage; // Update previous page
  } else {
    // Partial redraw for cursor movement within the same page
    for (int i = 0; i < 4; i++) {
      int iconIndex = currentPage * 4 + i;
      int x = iconX[i];
      int y = iconY[i];

      // Clear previous cursor outline if necessary
      if (i == previousCursorPosition && iconIndex != selectedIcon) {
        tft.drawRoundRect(x - 2, y - 2, ICON_SIZE + 4, ICON_SIZE + 4, BORDER_RADIUS, ST7735_BLACK);
        switch (iconIndex) {
         case 0: drawIcon(iconX[i], iconY[i], tft.color565(0, 153, 204), 0); break;
        case 1: drawIcon(iconX[i], iconY[i], tft.color565(51, 153, 204), 1); break;
        case 2: drawIcon(iconX[i], iconY[i], tft.color565(204, 153, 0), 2); break;
        case 3: drawIcon(iconX[i], iconY[i], tft.color565(192, 192, 192), 3); break;
        case 4: drawIcon(iconX[i], iconY[i], tft.color565(0, 153, 255), 4); break;
        case 5: drawIcon(iconX[i], iconY[i], tft.color565(0, 153, 51), 5); break;
        case 6: drawIcon(iconX[i], iconY[i], tft.color565(255, 102, 0), 6); break;
        case 7: drawIcon(iconX[i], iconY[i], tft.color565(255, 128, 0), 7); break;

        }
      }

      // Redraw the current cursor or selected outline
      if (iconIndex == selectedIcon) {
        tft.drawRoundRect(x - 2, y - 2, ICON_SIZE + 4, ICON_SIZE + 4, BORDER_RADIUS, ST7735_RED);
      } else if (i == cursorPosition) {
        tft.drawRoundRect(x - 2, y - 2, ICON_SIZE + 4, ICON_SIZE + 4, BORDER_RADIUS, ST7735_WHITE);
      }
    }
  }
}

// Function to draw pagination dots
void drawPaginationDots() {
  int dotX = 64; // Center position for dots
  int dotY = tft.height() - 10;

  // Clear both dots before redrawing them with correct sizes
  tft.fillCircle(dotX - 6, dotY, 3, ST7735_BLACK); // Clear left dot
  tft.fillCircle(dotX + 6, dotY, 3, ST7735_BLACK); // Clear right dot

  // Draw the current page's dot as larger and the other as smaller
  tft.fillCircle(dotX - 6, dotY, (currentPage == 0) ? 3 : 2, ST7735_WHITE); // Left dot
  tft.fillCircle(dotX + 6, dotY, (currentPage == 1) ? 3 : 2, ST7735_WHITE); // Right dot
}

// Function to update cursor position based on encoder rotation
void updateCursor(int direction) {
  previousCursorPosition = cursorPosition; // Store current cursor position
  cursorPosition += direction;

  // Check if cursor has moved past the bounds of the page
  if (cursorPosition < 0) {
    cursorPosition = 3;
    currentPage = (currentPage == 0) ? 1 : 0;  // Move to last icon on the previous page
    displayIcons(true); // Full redraw if page has changed
  } else if (cursorPosition > 3) {
    cursorPosition = 0;
    currentPage = (currentPage == 0) ? 1 : 0;  // Move to first icon on the next page
    displayIcons(true); // Full redraw if page has changed
  } else {
    displayIcons(false); // Full redraw if page has changed
  }

  
}

// Function to select the current icon
void selectIcon() {
  selectedIcon = currentPage * 4 + cursorPosition; // Update selected icon to the current cursor position
  displayIcons(); // Full redraw with updated selection
}

// Function to update encoder and handle navigation and selection
void updateEncoder() {
  int newEncoderPos = myEncoder.read() / 4; // Adjust encoder sensitivity if needed
  if (newEncoderPos != oldEncoderPos) {
    int direction = (newEncoderPos > oldEncoderPos) ? 1 : -1; // Determine rotation direction
    updateCursor(direction); // Update cursor based on direction
    oldEncoderPos = newEncoderPos; // Update encoder position
  }
}


void pressButton(int buttonNumber, int state) {
  if (state){
    Serial.print("Button: ");
    Serial.println(buttonNumber);
  }

  switch (selectedIcon) {
    case 0:
    // Photoshop
      switch (buttonNumber) {
        case 0:
          // SAVE
          if (state) {
            Keyboard.press(KEY_LEFT_GUI);
            Keyboard.press('S');
          } else {
            Keyboard.release(KEY_LEFT_GUI);
            Keyboard.release('S');
          }
          break;
        case 1:
          // New Layer
          if (state) {
            Keyboard.press(KEY_LEFT_GUI);
            Keyboard.press(KEY_LEFT_SHIFT);
            Keyboard.press('N');
          } else {
            Keyboard.release(KEY_LEFT_GUI);
            Keyboard.release(KEY_LEFT_SHIFT);
            Keyboard.release('N');
          }
          break;
        case 2:
          // Duplicate Layer
          if (state) {
            Keyboard.press(KEY_LEFT_GUI);
            Keyboard.press('J');
          } else {
            Keyboard.release(KEY_LEFT_GUI);
            Keyboard.release('J');
          }
          break;
        case 3:
          // Merge Layers
          if (state) {
            Keyboard.press(KEY_LEFT_GUI);
            Keyboard.press('E');
          } else {
            Keyboard.release(KEY_LEFT_GUI);
            Keyboard.release('E');
          }
          break;
        case 4:
          // ERASER TOOL
          if (state) {
            Keyboard.press('E');
          } else {
            Keyboard.release('E');
          }
          break;
        case 5:
          // COPY
          if (state) {
            Keyboard.press(KEY_LEFT_GUI);
            Keyboard.press('C');
          } else {
            Keyboard.release(KEY_LEFT_GUI);
            Keyboard.release('C');
          }
          break;
        case 6:
          // PASTE
          if (state) {
            Keyboard.press(KEY_LEFT_GUI);
            Keyboard.press('V');
          } else {
            Keyboard.release(KEY_LEFT_GUI);
            Keyboard.release('V');
          }
          break;
        case 7:
          // CUT
          if (state) {
            Keyboard.press(KEY_LEFT_GUI);
            Keyboard.press('X');
          } else {
            Keyboard.release(KEY_LEFT_GUI);
            Keyboard.release('X');
          }
          break;
        case 8:
          // Select All
          if (state) {
            Keyboard.press(KEY_LEFT_GUI);
            Keyboard.press('A');
          } else {
            Keyboard.release(KEY_LEFT_GUI);
            Keyboard.release('A');
          }
          break;
        case 9:
          // Deselect
          if (state) {
            Keyboard.press(KEY_LEFT_GUI);
            Keyboard.press('D');
          } else {
            Keyboard.release(KEY_LEFT_GUI);
            Keyboard.release('D');
          }
          break;
        case 10:
          // MOVE TOOL
          if (state) {
            Keyboard.press('V');
          } else {
            Keyboard.release('V');
          }
          break;
        case 11:
          // BRUSH TOOL
          if (state) {
            Keyboard.press('B');
          } else {
            Keyboard.release('B');
          }
          break;
        case 12:
          // HAND TOOL
          if (state) {
            Keyboard.press('H');
          } else {
            Keyboard.release('H');
          }
          break;
        case 13:
          // TRANSFORM
          if (state) {
            Keyboard.press(KEY_LEFT_GUI);
            Keyboard.press('T');
          } else {
            Keyboard.release(KEY_LEFT_GUI);
            Keyboard.release('T');
          }
          break;
        case 14:
          // UNDO
          if (state) {
            Keyboard.press(KEY_LEFT_GUI);
            Keyboard.press('Z');
          } else {
            Keyboard.release(KEY_LEFT_GUI);
            Keyboard.release('Z');
          }
          break;
        case 15:
          // REDO
          if (state) {
            Keyboard.press(KEY_LEFT_GUI);
            Keyboard.press(KEY_LEFT_SHIFT);
            Keyboard.press('Z');
          } else {
            Keyboard.release(KEY_LEFT_GUI);
            Keyboard.release(KEY_LEFT_SHIFT);
            Keyboard.release('Z');
          }
          break;
      }
      break;
    case 1:
    // Lightroom
      switch (buttonNumber) {
        case 0:
          // Flag as Picked
          if (state) {
            Keyboard.press('P');
          } else {
            Keyboard.release('P');
          }
          break;
        case 1:
          // Reject Photo
          if (state) {
            Keyboard.press('X');
          } else {
            Keyboard.release('X');
          }
          break;
        case 2:
          // Unflag Photo
          if (state) {
            Keyboard.press('U');
          } else {
            Keyboard.release('U');
          }
          break;
        case 3:
          // Quick Crop Tool 
          if (state) {
            Keyboard.press('R');
          } else {
            Keyboard.release('R');
          }
          break;
        case 4:
          // Reset Settings
          if (state) {
            Keyboard.press(KEY_LEFT_GUI);
            Keyboard.press(KEY_LEFT_SHIFT);
            Keyboard.press('R');
          } else {
            Keyboard.release(KEY_LEFT_GUI);
            Keyboard.release(KEY_LEFT_SHIFT);
            Keyboard.release('R');
          }
          break;
        case 5:
          // Copy Settings
          if (state) {
            Keyboard.press(KEY_LEFT_GUI);
            Keyboard.press(KEY_LEFT_SHIFT);
            Keyboard.press('C');
          } else {
            Keyboard.release(KEY_LEFT_GUI);
            Keyboard.release(KEY_LEFT_SHIFT);
            Keyboard.release('C');
          }
          break;
        case 6:
          // Paste Settings
          if (state) {
            Keyboard.press(KEY_LEFT_GUI);
            Keyboard.press(KEY_LEFT_SHIFT);
            Keyboard.press('V');
          } else {
            Keyboard.release(KEY_LEFT_GUI);
            Keyboard.release(KEY_LEFT_SHIFT);
            Keyboard.release('V');
          }
          break;
        case 7:
          // Show/Hide Clipping
          if (state) {
            Keyboard.press('J');
          } else {
            Keyboard.release('J');
          }
          break;
        case 8:
          // Before/After
          if (state) {
            Keyboard.press('\\');
          } else {
            Keyboard.release('\\');
          }
          break;
        case 9:
          // Toggle Edit Brush
          if (state) {
            Keyboard.press('K');
          } else {
            Keyboard.release('K');
          }
          break;
        case 10:
          // Decrease Exposure
          if (state) {
            Keyboard.press(KEY_LEFT_SHIFT);
            Keyboard.press('-');
          } else {
            Keyboard.release(KEY_LEFT_SHIFT);
            Keyboard.release('-');
          }
          break;
        case 11:
          // Increase Exposure
          if (state) {
            Keyboard.press(KEY_LEFT_SHIFT);
            Keyboard.press('+');
          } else {
            Keyboard.release(KEY_LEFT_SHIFT);
            Keyboard.release('+');
          }
          break;
        case 12:
          // Decrease Contrast
          if (state) {
            Keyboard.press(KEY_LEFT_SHIFT);
            Keyboard.press('[');
          } else {
            Keyboard.release(KEY_LEFT_SHIFT);
            Keyboard.release('[');
          }
          break;
        case 13:
          // Increase Contrast
          if (state) {
            Keyboard.press(KEY_LEFT_SHIFT);
            Keyboard.press(']');
          } else {
            Keyboard.release(KEY_LEFT_SHIFT);
            Keyboard.release(']');
          }
          break;
        case 14:
          // Linear Gradient Mask
          if (state) {
            Keyboard.press(KEY_LEFT_SHIFT);
            Keyboard.press('L');
          } else {
            Keyboard.release(KEY_LEFT_SHIFT);
            Keyboard.release('L');
          }
          break;
        case 15:
          // Radial Gradient Mask
          if (state) {
            Keyboard.press(KEY_LEFT_SHIFT);
            Keyboard.press('M');
          } else {
            Keyboard.release(KEY_LEFT_SHIFT);
            Keyboard.release('M');
          }
          break;
      }
      break;
    case 2:
    // DaVinci Resolve
      switch (buttonNumber) {
        case 0:
          // Toggle Fullscreen
          if (state) {
            Keyboard.press(KEY_LEFT_GUI);
            Keyboard.press('F');
          } else {
            Keyboard.release(KEY_LEFT_GUI);
            Keyboard.release('F');
          }
          break;
        case 1:
          // Clear In/Out Points
          if (state) {
            Keyboard.press(KEY_LEFT_ALT);
            Keyboard.press('X');
          } else {
            Keyboard.release(KEY_LEFT_ALT);
            Keyboard.release('X');
          }
          break;
        case 2:
          // Add Market
          if (state) {
            Keyboard.press('M');
          } else {
            Keyboard.release('M');
          }
          break;
        case 3:
          // Trim Start to Playhead
          if (state) {
            Keyboard.press(KEY_LEFT_ALT);
            Keyboard.press('[');
          } else {
            Keyboard.release(KEY_LEFT_ALT);
            Keyboard.release('[');
          }
          break;
        case 4:
          // Trim End to Playhead
          if (state) {
            Keyboard.press(KEY_LEFT_ALT);
            Keyboard.press(']');
          } else {
            Keyboard.release(KEY_LEFT_ALT);
            Keyboard.release(']');
          }
          break;
        case 5:
          // Zoom Out on Timeline
          if (state) {
            Keyboard.press(KEY_LEFT_GUI);
            Keyboard.press('-');
          } else {
            Keyboard.release(KEY_LEFT_GUI);
            Keyboard.release('-');
          }
          break;
        case 6:
          // Zoom In on Timeline
          if (state) {
            Keyboard.press(KEY_LEFT_GUI);
            Keyboard.press('+');
          } else {
            Keyboard.release(KEY_LEFT_GUI);
            Keyboard.release('+');
          }
          break;
        case 7:
          // Go To Previous Edit Point
          if (state) {
            Keyboard.press(KEY_UP_ARROW);
          } else {
            Keyboard.release(KEY_UP_ARROW);
          }
          break;
        case 8:
          // Go To Next Edit Point
          if (state) {
            Keyboard.press(KEY_DOWN_ARROW);
          } else {
            Keyboard.release(KEY_DOWN_ARROW);
          }
          break;
        case 9:
          // Link/Unlink Clips
          if (state) {
            Keyboard.press(KEY_LEFT_GUI);
            Keyboard.press(KEY_LEFT_SHIFT);
            Keyboard.press('L');
          } else {
            Keyboard.release(KEY_LEFT_GUI);
            Keyboard.release(KEY_LEFT_SHIFT);
            Keyboard.release('L');
          }
          break;
        case 10:
          // Snap Toggle
          if (state) {
            Keyboard.press('N');
          } else {
            Keyboard.release('N');
          }
          break;
        case 11:
          // Overwrite Clip
          if (state) {
            Keyboard.press(KEY_F10);
          } else {
            Keyboard.release(KEY_F10);
          }
          break;
        case 12:
          // Insert Clip
          if (state) {
            Keyboard.press(KEY_F9);
          } else {
            Keyboard.release(KEY_F9);
          }
          break;
        case 13:
          // Cut Clip
          if (state) {
            Keyboard.press(KEY_LEFT_GUI);
            Keyboard.press('B');
          } else {
            Keyboard.release(KEY_LEFT_GUI);
            Keyboard.release('B');
          }
          break;
        case 14:
          // Ripple Delete
          if (state) {
            Keyboard.press(KEY_LEFT_SHIFT);
            Keyboard.press(KEY_DELETE);
          } else {
            Keyboard.release(KEY_LEFT_SHIFT);
            Keyboard.release(KEY_DELETE);
          }
          break;
        case 15:
          // Play/Pause
          if (state) {
            Keyboard.press(' ');
          } else {
            Keyboard.release(' ');
          }
          break;
      }
      break;
    case 3:
    // Logic Pro X
      switch (buttonNumber) {
        case 0:
          // Duplicate Region/Note
          if (state) {
            Keyboard.press(KEY_LEFT_GUI);
            Keyboard.press('D');
          } else {
            Keyboard.release(KEY_LEFT_GUI);
            Keyboard.release('D');
          }
          break;
        case 1:
          // Open Inspector
          if (state) {
            Keyboard.press('I');
          } else {
            Keyboard.release('I');
          }
          break;
        case 2:
          // Open Piano Roll
          if (state) {
            Keyboard.press('P');
          } else {
            Keyboard.release('P');
          }
          break;
        case 3:
          // Open Mixer
          if (state) {
            Keyboard.press('X');
          } else {
            Keyboard.release('X');
          }
          break;
        case 4:
          // Quantize Selected Notes
          if (state) {
            Keyboard.press('Q');
          } else {
            Keyboard.release('Q');
          }
          break;
        case 5:
          // Solo Selected Track
          if (state) {
            Keyboard.press(KEY_LEFT_GUI);
            Keyboard.press('S');
          } else {
            Keyboard.release(KEY_LEFT_GUI);
            Keyboard.release('S');
          }
          break;
        case 6:
          // Mute Selected Track
          if (state) {
            Keyboard.press(KEY_LEFT_GUI);
            Keyboard.press('M');
          } else {
            Keyboard.release(KEY_LEFT_GUI);
            Keyboard.release('M');
          }
          break;
        case 7:
          // Create New Track
          if (state) {
            Keyboard.press(KEY_LEFT_GUI);
            Keyboard.press(KEY_LEFT_ALT);
            Keyboard.press('N');
          } else {
            Keyboard.release(KEY_LEFT_GUI);
            Keyboard.release(KEY_LEFT_ALT);
            Keyboard.release('N');
          }
          break;
        case 8:
          // Zoom Out
          if (state) {
            Keyboard.press(KEY_LEFT_GUI);
            Keyboard.press(KEY_DOWN_ARROW);
          } else {
            Keyboard.release(KEY_LEFT_GUI);
            Keyboard.release(KEY_DOWN_ARROW);
          }
          break;
        case 9:
          // Zoom In
          if (state) {
            Keyboard.press(KEY_LEFT_GUI);
            Keyboard.press(KEY_UP_ARROW);
          } else {
            Keyboard.release(KEY_LEFT_GUI);
            Keyboard.release(KEY_UP_ARROW);
          }
          break;
        case 10:
          // Toggle Cycle Mode
          if (state) {
            Keyboard.press('C');
          } else {
            Keyboard.release('C');
          }
          break;
        case 11:
          // Undo
          if (state) {
            Keyboard.press(KEY_LEFT_GUI);
            Keyboard.press('Z');
          } else {
            Keyboard.release(KEY_LEFT_GUI);
            Keyboard.release('Z');
          }
          break;
        case 12:
          // Redo
          if (state) {
            Keyboard.press(KEY_LEFT_GUI);
            Keyboard.press(KEY_LEFT_SHIFT);
            Keyboard.press('Z');
          } else {
            Keyboard.release(KEY_LEFT_GUI);
            Keyboard.release(KEY_LEFT_SHIFT);
            Keyboard.release('Z');
          }
          break;
        case 13:
          // Cut/Split Regions/Events at Playhead
          if (state) {
            Keyboard.press(KEY_LEFT_GUI);
            Keyboard.press('T');
          } else {
            Keyboard.release(KEY_LEFT_GUI);
            Keyboard.release('T');
          }
          break;
        case 14:
          // Record
          if (state) {
            Keyboard.press('R');
          } else {
            Keyboard.release('R');
          }
          break;
        case 15:
          // Play/Stop
          if (state) {
            Keyboard.press(' ');
          } else {
            Keyboard.release(' ');
          }
          break;
      }
      break;
    case 4:
    // VS Code
      switch (buttonNumber) {
        case 0:
          // Open Command Pallete
          if (state) {
            Keyboard.press(KEY_LEFT_GUI);
            Keyboard.press(KEY_LEFT_SHIFT);
            Keyboard.press('P');
          } else {
            Keyboard.release(KEY_LEFT_GUI);
            Keyboard.release(KEY_LEFT_SHIFT);
            Keyboard.release('P');
          }
          break;
        case 1:
          // Toggle Sidebar Visibility
          if (state) {
            Keyboard.press(KEY_LEFT_GUI);
            Keyboard.press('B');
          } else {
            Keyboard.release(KEY_LEFT_GUI);
            Keyboard.release('B');
          }
          break;
        case 2:
          // Search Files by Name
          if (state) {
            Keyboard.press(KEY_LEFT_GUI);
            Keyboard.press('P');
          } else {
            Keyboard.release(KEY_LEFT_GUI);
            Keyboard.release('P');
          }
          break;
        case 3:
          // Find in Files
          if (state) {
            Keyboard.press(KEY_LEFT_GUI);
            Keyboard.press(KEY_LEFT_SHIFT);
            Keyboard.press('F');
          } else {
            Keyboard.release(KEY_LEFT_GUI);
            Keyboard.release(KEY_LEFT_SHIFT);
            Keyboard.release('F');
          }
          break;
        case 4:
          // Replace in Files
          if (state) {
            Keyboard.press(KEY_LEFT_GUI);
            Keyboard.press(KEY_LEFT_ALT);
            Keyboard.press('F');
          } else {
            Keyboard.release(KEY_LEFT_GUI);
            Keyboard.release(KEY_LEFT_ALT);
            Keyboard.release('F');
          }
          break;
        case 5:
          // Open new terminal
          if (state) {
            Keyboard.press(KEY_LEFT_CTRL);
            Keyboard.press('`');
          } else {
            Keyboard.release(KEY_LEFT_CTRL);
            Keyboard.release('`');
          }
          break;
        case 6:
          // Split Editor
          if (state) {
            Keyboard.press(KEY_LEFT_GUI);
            Keyboard.press('\\');
          } else {
            Keyboard.release(KEY_LEFT_GUI);
            Keyboard.release('\\');
          }
          break;
        case 7:
          // Close Editor
          if (state) {
            Keyboard.press(KEY_LEFT_GUI);
            Keyboard.press('W');
          } else {
            Keyboard.release(KEY_LEFT_GUI);
            Keyboard.release('W');
          }
          break;
        case 8:
          // Navigate between open files
          if (state) {
            Keyboard.press(KEY_LEFT_GUI);
            Keyboard.press(KEY_TAB);
          } else {
            Keyboard.release(KEY_LEFT_GUI);
            Keyboard.release(KEY_TAB);
          }
          break;
        case 9:
          // Go to Definition
          if (state) {
            Keyboard.press(KEY_F12);
          } else {
            Keyboard.release(KEY_F12);
          }
          break;
        case 10:
          // Toggle Comment
          if (state) {
            Keyboard.press(KEY_LEFT_GUI);
            Keyboard.press('/');
          } else {
            Keyboard.release(KEY_LEFT_GUI);
            Keyboard.release('/');
          }
          break;
        case 11:
          // Format Document
          if (state) {
            Keyboard.press(KEY_LEFT_ALT);
            Keyboard.press(KEY_LEFT_SHIFT);
            Keyboard.press('F');
          } else {
            Keyboard.release(KEY_LEFT_ALT);
            Keyboard.release(KEY_LEFT_SHIFT);
            Keyboard.release('F');
          }
          break;
        case 12:
          // Multi Cursor
          if (state) {
            Keyboard.press(KEY_LEFT_GUI);
            Mouse.press();
          } else {
            Keyboard.release(KEY_LEFT_GUI);
            Mouse.release();
          }
          break;
        case 13:
          // Show Extensions
          if (state) {
            Keyboard.press(KEY_LEFT_GUI);
            Keyboard.press(KEY_LEFT_SHIFT);
            Keyboard.press('X');
          } else {
            Keyboard.release(KEY_LEFT_GUI);
            Keyboard.release(KEY_LEFT_SHIFT);
            Keyboard.release('X');
          }
          break;
        case 14:
          // Move Line Down
          if (state) {
            Keyboard.press(KEY_LEFT_ALT);
            Keyboard.press(KEY_DOWN_ARROW);
          } else {
            Keyboard.release(KEY_LEFT_ALT);
            Keyboard.release(KEY_DOWN_ARROW);
          }
          break;
        case 15:
          // Move Line Up
          if (state) {
            Keyboard.press(KEY_LEFT_ALT);
            Keyboard.press(KEY_UP_ARROW);
          } else {
            Keyboard.release(KEY_LEFT_ALT);
            Keyboard.release(KEY_UP_ARROW);
          }
          break;
      }
      break;
    case 5:
    // Arduino IDE
      switch (buttonNumber) {
        case 0:
          // New Sketch
          if (state) {
            Keyboard.press(KEY_LEFT_GUI);
            Keyboard.press('N');
          } else {
            Keyboard.release(KEY_LEFT_GUI);
            Keyboard.release('N');
          }
          break;
        case 1:
          // Open Sketch
          if (state) {
            Keyboard.press(KEY_LEFT_GUI);
            Keyboard.press('O');
          } else {
            Keyboard.release(KEY_LEFT_GUI);
            Keyboard.release('O');
          }
          break;
        case 2:
          // Save Sketch
          if (state) {
            Keyboard.press(KEY_LEFT_GUI);
            Keyboard.press('S');
          } else {
            Keyboard.release(KEY_LEFT_GUI);
            Keyboard.release('S');
          }
          break;
        case 3:
          // Close Sketch
          if (state) {
            Keyboard.press(KEY_LEFT_GUI);
            Keyboard.press('W');
          } else {
            Keyboard.release(KEY_LEFT_GUI);
            Keyboard.release('W');
          }
          break;
        case 4:
          // Copy
          if (state) {
            Keyboard.press(KEY_LEFT_GUI);
            Keyboard.press('C');
          } else {
            Keyboard.release(KEY_LEFT_GUI);
            Keyboard.release('C');
          }
          break;
        case 5:
          // Paste
          if (state) {
            Keyboard.press(KEY_LEFT_GUI);
            Keyboard.press('V');
          } else {
            Keyboard.release(KEY_LEFT_GUI);
            Keyboard.release('V');
          }
          break;
        case 6:
          // Cut
          if (state) {
            Keyboard.press(KEY_LEFT_GUI);
            Keyboard.press('X');
          } else {
            Keyboard.release(KEY_LEFT_GUI);
            Keyboard.release('X');
          }
          break;
        case 7:
          // Find
          if (state) {
            Keyboard.press(KEY_LEFT_GUI);
            Keyboard.press('F');
          } else {
            Keyboard.release(KEY_LEFT_GUI);
            Keyboard.release('F');
          }
          break;
        case 8:
          // Replace
          if (state) {
            Keyboard.press(KEY_LEFT_GUI);
            Keyboard.press(KEY_LEFT_SHIFT);
            Keyboard.press('F');
          } else {
            Keyboard.release(KEY_LEFT_GUI);
            Keyboard.release(KEY_LEFT_SHIFT);
            Keyboard.release('F');
          }
          break;
        case 9:
          // Comment/Uncomment
          if (state) {
            Keyboard.press(KEY_LEFT_GUI);
            Keyboard.press('/');
          } else {
            Keyboard.release(KEY_LEFT_GUI);
            Keyboard.release('/');
          }
          break;
        case 10:
          // Select All
          if (state) {
            Keyboard.press(KEY_LEFT_GUI);
            Keyboard.press('A');
          } else {
            Keyboard.release(KEY_LEFT_GUI);
            Keyboard.release('A');
          }
          break;
        case 11:
          // Auto Format
          if (state) {
            Keyboard.press(KEY_LEFT_GUI);
            Keyboard.press('T');
          } else {
            Keyboard.release(KEY_LEFT_GUI);
            Keyboard.release('T');
          }
          break;
        case 12:
          // Open Serial Plotter
          if (state) {
            Keyboard.press(KEY_LEFT_GUI);
            Keyboard.press(KEY_LEFT_SHIFT);
            Keyboard.press('L');
          } else {
            Keyboard.release(KEY_LEFT_GUI);
            Keyboard.release(KEY_LEFT_SHIFT);
            Keyboard.release('L');
          }
          break;
        case 13:
          // Open Serial Monitor
          if (state) {
            Keyboard.press(KEY_LEFT_GUI);
            Keyboard.press(KEY_LEFT_SHIFT);
            Keyboard.press('M');
          } else {
            Keyboard.release(KEY_LEFT_GUI);
            Keyboard.release(KEY_LEFT_SHIFT);
            Keyboard.release('M');
          }
          break;
        case 14:
          // Verify/Compile
          if (state) {
            Keyboard.press(KEY_LEFT_GUI);
            Keyboard.press('R');
          } else {
            Keyboard.release(KEY_LEFT_GUI);
            Keyboard.release('R');
          }
          break;
        case 15:
          // Upload
          if (state) {
            Keyboard.press(KEY_LEFT_GUI);
            Keyboard.press('U');
          } else {
            Keyboard.release(KEY_LEFT_GUI);
            Keyboard.release('U');
          }
          break;
      }
      break;
    case 6:
    // Fusion 360
      switch (buttonNumber) {
        case 0:
          // Create Sketch
          if (state) {
            Keyboard.press('C');
          } else {
            Keyboard.release('C');
          }
          break;
        case 1:
          // Create Line
          if (state) {
            Keyboard.press('L');
          } else {
            Keyboard.release('L');
          }
          break;
        case 2:
          // Create Circle
          if (state) {
            Keyboard.press('C');
          } else {
            Keyboard.release('C');
          }
          break;
        case 3:
          // Create Rectangle
          if (state) {
            Keyboard.press('R');
          } else {
            Keyboard.release('R');
          }
          break;
        case 4:
          // Extrude
          if (state) {
            Keyboard.press('E');
          } else {
            Keyboard.release('E');
          }
          break;
        case 5:
          // Fillet
          if (state) {
            Keyboard.press('F');
          } else {
            Keyboard.release('F');
          }
          break;
        case 6:
          // Offset
          if (state) {
            Keyboard.press('O');
          } else {
            Keyboard.release('O');
          }
          break;
        case 7:
          // Dimension
          if (state) {
            Keyboard.press('D');
          } else {
            Keyboard.release('D');
          }
          break;
        case 8:
          // Trim
          if (state) {
            Keyboard.press('T');
          } else {
            Keyboard.release('T');
          }
          break;
        case 9:
          // Move
          if (state) {
            Keyboard.press('M');
          } else {
            Keyboard.release('M');
          }
          break;
        case 10:
          // Mirror
          if (state) {
            Keyboard.press('S');
          } else {
            Keyboard.release('S');
          }
          break;
        case 11:
          // Measure
          if (state) {
            Keyboard.press('I');
          } else {
            Keyboard.release('I');
          }
          break;
        case 12:
          // Press/Pull
          if (state) {
            Keyboard.press('Q');
          } else {
            Keyboard.release('Q');
          }
          break;
        case 13:
          // Project
          if (state) {
            Keyboard.press('P');
          } else {
            Keyboard.release('P');
          }
          break;
        case 14:
          // Revolve
          if (state) {
            Keyboard.press('R');
          } else {
            Keyboard.release('R');
          }
          break;
        case 15:
          // Pan
          if (state) {
            Keyboard.press(KEY_LEFT_SHIFT);
            Mouse.press();
          } else {
            Keyboard.release(KEY_LEFT_SHIFT);
            Mouse.release();
          }
          break;
      }
      break;
    case 7:
    // Custom
      switch (buttonNumber) {
        case 0:
          // Screenshot Full
          if (state) {
            Keyboard.press(KEY_LEFT_GUI);
            Keyboard.press(KEY_LEFT_SHIFT);
            Keyboard.press('3');
          } else {
            Keyboard.release(KEY_LEFT_GUI);
            Keyboard.release(KEY_LEFT_SHIFT);
            Keyboard.release('3');
          }
          break;
        case 1:
          // Screenshot Area
          if (state) {
            Keyboard.press(KEY_LEFT_GUI);
            Keyboard.press(KEY_LEFT_SHIFT);
            Keyboard.press('4');
          } else {
            Keyboard.release(KEY_LEFT_GUI);
            Keyboard.release(KEY_LEFT_SHIFT);
            Keyboard.release('4');
          }
          break;
        case 2:
          // Cycle Through Open Tabs
          if (state) {
            Keyboard.press(KEY_LEFT_CTRL);
            Keyboard.press(KEY_TAB);
          } else {
            Keyboard.release(KEY_LEFT_CTRL);
            Keyboard.release(KEY_TAB);
          }
          break;
        case 3:
          // Lock Screen
          if (state) {
            Keyboard.press(KEY_LEFT_GUI);
            Keyboard.press(KEY_LEFT_CTRL);
            Keyboard.press('Q');
          } else {
            Keyboard.release(KEY_LEFT_GUI);
            Keyboard.release(KEY_LEFT_CTRL);
            Keyboard.release('Q');
          }
          break;
        case 4:
          // Notification Center
          if (state) {
            Keyboard.press(KEY_LEFT_ALT);
            Keyboard.press(KEY_F8);
          } else {
            Keyboard.release(KEY_LEFT_ALT);
            Keyboard.release(KEY_F8);
          }
          break;
        case 5:
          // Mission Control
          if (state) {
            Keyboard.press(KEY_F3);
          } else {
            Keyboard.release(KEY_F3);
          }
          break;
        case 6:
          // Brightness Down
          if (state) {
            Keyboard.press(KEY_F1);
          } else {
            Keyboard.release(KEY_F1);
          }
          break;
        case 7:
          // Brightness Up
          if (state) {
            Keyboard.press(KEY_F2);
          } else {
            Keyboard.release(KEY_F2);
          }
          break;
        case 8:
          // Volume Down
          if (state) {
            Keyboard.press(KEY_F11);
          } else {
            Keyboard.release(KEY_F11);
          }
          break;
        case 9:
          // Volume Up
          if (state) {
            Keyboard.press(KEY_F12);
          } else {
            Keyboard.release(KEY_F12);
          }
          break;
        case 10:
          // Mute/Unmute Volume
          if (state) {
            Keyboard.press(KEY_F10);
          } else {
            Keyboard.release(KEY_F10);
          }
          break;
        case 11:
          // Open Finder
          if (state) {
            Keyboard.press(KEY_LEFT_GUI);
            Keyboard.press(KEY_LEFT_ALT);
            Keyboard.press(' ');
          } else {
            Keyboard.release(KEY_LEFT_GUI);
            Keyboard.release(KEY_LEFT_SHIFT);
            Keyboard.release(' ');
          }
          break;
        case 12:
          // Open Applications Folder
          if (state) {
            Keyboard.press(KEY_LEFT_GUI);
            Keyboard.press(KEY_LEFT_SHIFT);
            Keyboard.press('A');
          } else {
            Keyboard.release(KEY_LEFT_GUI);
            Keyboard.release(KEY_LEFT_SHIFT);
            Keyboard.release('A');
          }
          break;
        case 13:
          // Open Launchpad
          if (state) {
            Keyboard.press(KEY_F4);
          } else {
            Keyboard.release(KEY_F4);
          }
          break;
        case 14:
          // Show Desktop
          if (state) {
            Keyboard.press(KEY_F11);
          } else {
            Keyboard.release(KEY_F11);
          }
          break;
        case 15:
          // Spotlight Search
          if (state) {
            Keyboard.press(KEY_LEFT_GUI);
            Keyboard.press(' ');
          } else {
            Keyboard.release(KEY_LEFT_GUI);
            Keyboard.release(' ');
          }
          break;
      }
      break;
   }
  
}
