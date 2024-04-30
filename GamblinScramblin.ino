// Code Inspiration largely based off of https://github.com/makeabilitylab/arduino/blob/master/OLED/BallBounceWithSoundAndHaptics/BallBounceWithSoundAndHaptics.ino
// Certain spacing attempts for the text was using Claude by Anthropic, although largely unsuccessful.

#include <Wire.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET 4
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

const int LEFT_BUTTON_PIN = 8;
const int MIDDLE_BUTTON_PIN = 9;
const int RIGHT_BUTTON_PIN = 10;
const int TONE_OUTPUT_PIN = 4;
const int GREEN_LED_PIN = 6;
const int RED_LED_PIN = 11;

const int DELAY_LOOP_MS = 5;
const int WALL_COLLISION_TONE_FREQUENCY = 100;
const int CEILING_COLLISION_TONE_FREQUENCY = 200;
const int PLAY_TONE_DURATION_MS = 200;
const int VIBROMOTOR_OUTPUT_PIN = 5;
const int VIBROMOTOR_DURATION_MS = 200;

unsigned long vibroMotorStartTimeStamp = -1;

int menuOption = 1;
const int NUM_OPTIONS = 3;
int playerClass = -1;

int money = 1000;
bool gameOver = false;
bool loanTaken = false;

void setup() {
    Serial.begin(9600);

    pinMode(TONE_OUTPUT_PIN, OUTPUT);
    pinMode(VIBROMOTOR_OUTPUT_PIN, OUTPUT);
    pinMode(GREEN_LED_PIN, OUTPUT);
    pinMode(RED_LED_PIN, OUTPUT);

    initializeOledAndShowStartupScreen();

    pinMode(LEFT_BUTTON_PIN, INPUT_PULLUP);
    pinMode(MIDDLE_BUTTON_PIN, INPUT_PULLUP);
    pinMode(RIGHT_BUTTON_PIN, INPUT_PULLUP);

    selectPlayerClass();
}

void loop() {
    int leftButtonState = digitalRead(LEFT_BUTTON_PIN);
    int middleButtonState = digitalRead(MIDDLE_BUTTON_PIN);
    int rightButtonState = digitalRead(RIGHT_BUTTON_PIN);

    static int currentLocation = 0;

    if (gameOver) {
        displayGameOverScreen(middleButtonState);
    } else {
        handleLocationChange(leftButtonState, rightButtonState, &currentLocation);
        displayLocationScreen(currentLocation, middleButtonState);
        checkGameOver();
    }
}

void resetGame() {
    money = 1000;
    gameOver = false;
    playerClass = -1;
    loanTaken = false;  // Reset loan status
    initializeOledAndShowStartupScreen();
    selectPlayerClass();
}

void selectPlayerClass() {
    int leftButtonState, middleButtonState, rightButtonState;

    while (playerClass == -1) {
        leftButtonState = digitalRead(LEFT_BUTTON_PIN);
        middleButtonState = digitalRead(MIDDLE_BUTTON_PIN);
        rightButtonState = digitalRead(RIGHT_BUTTON_PIN);

        handleMenuOptionChange(leftButtonState, rightButtonState);

        if (middleButtonState == 1) {
            playerClass = menuOption;
            tone(TONE_OUTPUT_PIN, WALL_COLLISION_TONE_FREQUENCY, PLAY_TONE_DURATION_MS);
            if (playerClass == 1) {
                money = 1500;
            }
            delay(1000);
            break;
        } else {
            displayClassDescription();
            delay(100); // Add a small delay to allow the display to refresh
        }
    }
}

void displayClassDescription() {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE, BLACK);

    int16_t x1, y1;
    uint16_t textWidth, textHeight;

    String className;
    switch (menuOption) {
        case 1:
            className = "The Business Man - Start out with extra cash.";
            break;
        case 2:
            className = "Golden Fingers - Gain a boost in luck.";
            break;
        case 3:
            className = "Persuasive Prodigy - Better loan success.";
            break;
    }

    display.getTextBounds(className, 0, 0, &x1, &y1, &textWidth, &textHeight);
    int xCursor = display.width() / 2 - textWidth / 2;
    int yCursor = display.height() / 4;

    display.setCursor(xCursor, yCursor);
    display.println(className);
    display.display();
}


void selectDrink() {
    int leftButtonState, middleButtonState, rightButtonState;
    int drinkOption = 1;
    const int NUM_DRINK_OPTIONS = 3;

    delay(200);

    while (true) {
        leftButtonState = digitalRead(LEFT_BUTTON_PIN);
        middleButtonState = digitalRead(MIDDLE_BUTTON_PIN);
        rightButtonState = digitalRead(RIGHT_BUTTON_PIN);

        handleMenuOptionChange(leftButtonState, rightButtonState, &drinkOption, NUM_DRINK_OPTIONS);

        if (middleButtonState == 1) {
            tone(TONE_OUTPUT_PIN, WALL_COLLISION_TONE_FREQUENCY, PLAY_TONE_DURATION_MS);
            orderDrink(drinkOption);
            break;
        }

        displayDrinkOptions(drinkOption);
    }
}

void orderDrink(int drinkOption) {
    int drinkCost;
    bool doubleMoney = random(2);

    switch (drinkOption) {
        case 1:
            drinkCost = 100;
            break;
        case 2:
            drinkCost = 500;
            break;
        case 3:
            drinkCost = 1000;
            break;
    }

    if (money >= drinkCost) {
        money -= drinkCost;
        if (doubleMoney) {
            money += drinkCost * 2;
        }

        displayDrinkOrderResult(drinkOption, doubleMoney);
    } else {
        displayNotEnoughMoney();
    }
}

void displayDrinkOrderResult(int drinkOption, bool doubleMoney) {
    display.clearDisplay();
    display.setCursor(0, 0);

    String drinkName;
    switch (drinkOption) {
        case 1:
            drinkName = "beer";
            break;
        case 2:
            drinkName = "wine";
            break;
        case 3:
            drinkName = "vodka";
            break;
    }

    display.println();
    display.println("   You ordered " + drinkName + "!   ");
    display.println();
    digitalWrite(VIBROMOTOR_OUTPUT_PIN, HIGH);
    delay(1000);
    digitalWrite(VIBROMOTOR_OUTPUT_PIN, LOW);

    if (doubleMoney) {
        display.println(" Lucky! Money doubled! ");
        display.println();
        digitalWrite(GREEN_LED_PIN, HIGH);
        delay(1000);
        digitalWrite(GREEN_LED_PIN, LOW);
    } else {
        display.println("Nothing exciting happened.");
        display.println();
        digitalWrite(RED_LED_PIN, HIGH);
        delay(1000);
        digitalWrite(RED_LED_PIN, LOW);
    }

    display.println("   Money left: $" + String(money) + "   ");
    display.display();
    delay(2000);
}

void displayNotEnoughMoney() {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println();
    display.println("   Not enough money!   ");
    display.println();
    display.display();
    delay(2000);
}

void displayDrinkOptions(int drinkOption) {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println();
    display.println("   Select Drink:   ");
    display.println();
    display.println(drinkOption == 1 ? "> Beer - $100" : "  Beer - $100");
    display.println(drinkOption == 2 ? "> Wine - $500" : "  Wine - $500");
    display.println(drinkOption == 3 ? "> Vodka - $1000" : "  Vodka - $1000");
    display.display();
}

void handleMenuOptionChange(int leftButtonState, int rightButtonState, int* option, int numOptions) {
    if (leftButtonState == 1) {
        (*option)--;
        tone(TONE_OUTPUT_PIN, WALL_COLLISION_TONE_FREQUENCY, PLAY_TONE_DURATION_MS);
        if (*option < 1) {
            *option = numOptions;
        }
        delay(200);
    } else if (rightButtonState == 1) {
        (*option)++;
        tone(TONE_OUTPUT_PIN, WALL_COLLISION_TONE_FREQUENCY, PLAY_TONE_DURATION_MS);
        if (*option > numOptions) {
            *option = 1;
        }
        delay(200);
    }
}

void handleMenuOptionChange(int leftButtonState, int rightButtonState) {
    handleMenuOptionChange(leftButtonState, rightButtonState, &menuOption, NUM_OPTIONS);
}

void displayGameOverScreen(int middleButtonState) {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println();
    display.println("     Game Over!     ");
    display.println();
    display.println("   You ran out of money.   ");
    display.println();
    display.println("  Middle: Restart  ");
    display.display();

    if (middleButtonState == 1) {
        resetGame();
        delay(200);
    }
}

void handleLocationChange(int leftButtonState, int rightButtonState, int* currentLocation) {
    if (leftButtonState == 1) {
        (*currentLocation)--;
        if (*currentLocation < 0) {
            *currentLocation = 2;
        }
        delay(200);
    } else if (rightButtonState == 1) {
        (*currentLocation)++;
        if (*currentLocation > 2) {
            *currentLocation = 0;
        }
        delay(200);
    }
}

void displayLocationScreen(int currentLocation, int middleButtonState) {
    switch (currentLocation) {
        case 0:
            display.clearDisplay();
            display.setCursor(0, 0);
            display.println();
            display.println("     Main Menu     ");
            display.println();
            display.println("  Left: Bar - Right: Bank  ");
            display.println();
            display.println("   Money: $" + String(money) + "   ");
            display.display();
            break;
        case 1:
            display.clearDisplay();
            display.setCursor(0, 0);
            display.println();
            display.println("       Bank       ");
            display.println();
            display.println("  Middle: Apply for Loan  ");
            display.display();
            if (middleButtonState == 1 && !loanTaken) {
                applyForLoan();
                delay(200);
            } else if (middleButtonState == 1){
              display.clearDisplay();
              display.setCursor(0, 0);
              display.println();
              display.println();
              display.println("You already applied for a loan! ");
              display.display();
              delay(1000);
            }
            break;
        case 2:
            display.clearDisplay();
            display.setCursor(0, 0);
            display.println();
            display.println("       Bar        ");
            display.println();
            display.println("  Middle: Order Drink  ");
            display.println();
            display.println("   Money: $" + String(money) + "   ");
            display.display();
            if (middleButtonState == 1) {
                selectDrink();
                delay(200);
            }
            break;
    }
}

void checkGameOver() {
    if (money <= 0) {
        gameOver = true;
    }
}

void applyForLoan() {
    int loanChance;
    switch (playerClass) {
        case 1:  // Business Man
            loanChance = 50;
            break;
        case 2:  // Golden Fingers
            loanChance = 75;
            break;
        case 3:  // Persuasive Prodigy
            loanChance = 90;
            break;
        default:
            loanChance = 0;
            break;
    }

    if (random(100) < loanChance) {
        display.clearDisplay();
        display.setCursor(0, 0);
        display.println();
        display.println("   Loan approved!   ");
        display.println();
        display.println("  $500 added to balance.  ");
        display.display();
        money += 500;
        loanTaken = true;
        delay(2000);
    } else {
        display.clearDisplay();
        display.setCursor(0, 0);
        display.println();
        display.println("   Loan denied!   ");
        display.println();
        display.display();
        delay(2000);
    }
}

void initializeOledAndShowStartupScreen() {
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3D)) {
        Serial.println(F("SSD1306 allocation failed"));
        for (;;);
    }

    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE, BLACK);
    int16_t x1, y1;
    uint16_t textWidth, textHeight;
    String strOpeningScreen = "Gamblin' n Scramblin";
    display.getTextBounds(strOpeningScreen, 0, 0, &x1, &y1, &textWidth, &textHeight);
    uint16_t xText = display.width() / 2 - textWidth / 2;
    uint16_t yText = display.height() / 2 - textHeight / 2;
    display.setCursor(xText, yText);
    display.println(strOpeningScreen);
    display.setTextSize(0.5);
    display.setCursor(xText, yText + 10);
    display.println("By Jacob Kieser");
    display.display();
    delay(2000);

    display.clearDisplay();
    display.setCursor(0, 0);
    display.println();
    display.println("   Choose your class:   ");
    display.println();
    display.display();
    delay(1000);
}