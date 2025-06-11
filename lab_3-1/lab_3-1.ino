/*
    Lab 3.1: Lab 3.1: Working with Displays and Touchscreen Buttons
    Name: Levi Terry
    Date: 6/7/25
    Email: lterry80052@uat.edu
    Due Date: 6/11/25
*/

// Include the M5Core2 Library
#include <M5Core2.h>

void setup() {
    /* ---- PART 1 ----  */
    // Initialize the Core2
    M5.begin();

    /* --- Background Flashing --- */
    // Set BG to RED
    M5.Lcd.fillScreen(RED);
    // Wait 1s
    delay(1000);
    // Set BG to WHITE
    M5.Lcd.fillScreen(WHITE);
    // Wait 1s
    delay(1000);
    // Set BG to BLUE
    M5.Lcd.fillScreen(BLUE);
    // Wait 1s
    delay(1000);
    // Set BG to BLACK
    M5.Lcd.fillScreen(BLACK);

    M5.Lcd.draw

    /* --- Write name at (20,20) in white wwith size 2 --- */
    M5.Lcd.setCursor(20, 20);  // Move the cursor position to (x,y)
    M5.Lcd.setTextColor(WHITE);  // Set the font color to white.
    M5.Lcd.setTextSize(2);  // Set the font size.
    M5.Lcd.printf("Levi Terry!");  // Serial output format string.

    /* Wait 2s before part 2 */
    delay(2000);

    /* ---- PART 2 ---- */

    /* Screen white, text green, centered name, taking up as much space as possible */
    M5.Lcd.fillScreen(WHITE);
    M5.Lcd.setTextColor(GREEN);

    M5.Lcd.setCursor(15, 100);  // Move the cursor position to (x,y)
    M5.Lcd.setTextSize(5);  // Set the font size.
    M5.Lcd.printf("Levi Terry");  // Serial output format string
}

void loop() {
    M5.update();  // Read the press state of the key.

    /* ---- PART 3 ----*/
    if (M5.BtnA.wasReleased())
    {
        // Set text color and  BG to defaults
        M5.Lcd.clear(BLACK);
        M5.Lcd.setTextColor(WHITE);
        M5.Lcd.setCursor(15, 100);
        M5.Lcd.setTextSize(3);

        M5.Lcd.println("BtnA!");
    } else if (M5.BtnB.wasReleased())
    {
        // Set text color and BG to random colors
        // Array with all chosen colors
        int colors[] = { BLUE, GREEN, BLACK, OLIVE, PINK, DARKCYAN };

        // Pick random index from array
        int bgColor = random(0, 6);
        int textColor = random(0,6);

        // Set colors
        M5.Lcd.clear(colors[bgColor]);
        M5.Lcd.setTextColor(colors[textColor]);
        M5.Lcd.setCursor(15, 100);
        M5.Lcd.setTextSize(3);

        M5.Lcd.println("BtnB!");
    } else if (M5.BtnC.wasReleased())
    {
        // Set BG to GREEN and text color to BLUE
        M5.Lcd.clear(GREEN);
        M5.Lcd.setTextColor(BLUE);
        M5.Lcd.setCursor(15, 100);
        M5.Lcd.setTextSize(3);

        M5.Lcd.println("BtnC!");
    }
}
