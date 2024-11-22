#include <Arduino.h>
#include <SPI.h>
#include <Servo.h>
#include <LiquidCrystal.h>

const int startBTN = 2;   
const int servoPin = 9;     
LiquidCrystal lcd(3, 4, 5, 6, 7, 8);
Servo gameServo;

int scorePlayer1 = 0, scorePlayer2 = 0;
bool player1Turn = true;
bool gameActive = false;
unsigned long gameStartTime = 0;
unsigned long roundStartTime = 0;
const unsigned long roundDuration = 3000;
const unsigned long gameDuration = 30000;
int servoPosition = 0;
const int servoMaxAngle = 180;
bool awaitingResponse = false;
char currentCommand;

enum RGBState { RED, GREEN, BLUE };
RGBState currentColor;

void setup();
void loop();
void startGame();
void handleGame();
void endGame();
void activateLED(bool player1Turn, RGBState color);
void deactivateAllLEDs();
char sendCommand(const char* command);
void updateScore(char rating);
void displayScores();
void displayWinner();
void resetGame();
void displayWelcome();

void setup() {
    Serial.begin(115200);
    SPI.begin();
    pinMode(SS, OUTPUT);
    digitalWrite(SS, HIGH);
    pinMode(startBTN, INPUT_PULLUP);

    lcd.begin(16, 2);
    gameServo.attach(servoPin);

    displayWelcome();
}

void loop() {
    if (!gameActive) {
        if (digitalRead(startBTN) == LOW) {
            delay(200);
            startGame();
        }
        return;
    }

    handleGame();

    if (millis() - gameStartTime >= gameDuration) {
        endGame();
    }
}

void startGame() {
    gameActive = true;
    scorePlayer1 = 0;
    scorePlayer2 = 0;
    gameStartTime = millis();
    roundStartTime = millis();
    servoPosition = 0;
    gameServo.write(0);
    player1Turn = true;
    lcd.clear();
    lcd.print("Game Start!");
    delay(1000);
    displayScores();
    currentColor = static_cast<RGBState>(random(0, 3));
    activateLED(player1Turn, currentColor);
}

void handleGame() {
    unsigned long currentMillis = millis();

    if (currentMillis - roundStartTime >= roundDuration) {
        deactivateAllLEDs();
        awaitingResponse = false;
        player1Turn = !player1Turn;
        roundStartTime = currentMillis;

        currentColor = static_cast<RGBState>(random(0, 3));
        activateLED(player1Turn, currentColor);
    }

    if (awaitingResponse) {
        char response = sendCommand("#");
        if (response == 'a' || response == 'b' || response == 'c' || response == 'i') {
            updateScore(response);
            awaitingResponse = false;
        }
    }

    int angle = map(currentMillis - gameStartTime, 0, gameDuration, 0, servoMaxAngle);
    gameServo.write(angle);
}

void endGame() {
    gameActive = false;
    deactivateAllLEDs();
    gameServo.write(0);
    displayWinner();
    resetGame();
}

void activateLED(bool player1Turn, RGBState color) {
    const char* cmd = (player1Turn) ? ((color == RED) ? "1r" : (color == GREEN) ? "1g" : "1b")
                                    : ((color == RED) ? "2r" : (color == GREEN) ? "2g" : "2b");
    currentCommand = cmd[1];
    sendCommand(cmd);
    lcd.clear();
    lcd.print(player1Turn ? "P1 Turn: " : "P2 Turn: ");
    lcd.print((color == RED) ? "Red" : (color == GREEN) ? "Green" : "Blue");
    awaitingResponse = true;
}

void deactivateAllLEDs() {
    sendCommand("0");
}

char sendCommand(const char* command) {
    digitalWrite(SS, LOW);
    for (size_t i = 0; i < strlen(command); i++) {
        SPI.transfer(command[i]);
    }
    digitalWrite(SS, HIGH);
    return SPI.transfer('#');
}

void updateScore(char rating) {
    int points = 0;
    switch (rating) {
        case 'a': points = 50; break;
        case 'b': points = 25; break;
        case 'c': points = 10; break;
        case 'i': points = 0; break;  
    }

    if (player1Turn) {
        scorePlayer1 += points;
    } else {
        scorePlayer2 += points;
    }

    displayScores();
}

void displayScores() {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("P1: ");
    lcd.print(scorePlayer1);
    lcd.setCursor(0, 1);
    lcd.print("P2: ");
    lcd.print(scorePlayer2);
}

void displayWinner() {
    lcd.clear();
    if (scorePlayer1 > scorePlayer2) {
        lcd.print("Winner: P1");
    } else if (scorePlayer2 > scorePlayer1) {
        lcd.print("Winner: P2");
    } else {
        lcd.print("It's a Draw!");
    }
    lcd.setCursor(0, 1);
    lcd.print("P1: ");
    lcd.print(scorePlayer1);
    lcd.print(" P2: ");
    lcd.print(scorePlayer2);
    delay(5000);
}

void resetGame() {
    lcd.clear();
    lcd.print("Press Start!");
    scorePlayer1 = 0;
    scorePlayer2 = 0;
}

void displayWelcome() {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Reflex Game!");
    lcd.setCursor(0, 1);
    lcd.print("Press Start");
}

// SLAVE

// #include <Arduino.h>
// #include <SPI.h>

// const int redRGB1 = 4, greenRGB1 = 9, blueRGB1 = 8;
// const int redRGB2 = 5, greenRGB2 = 6, blueRGB2 = 7;

// const int BTNP1 = A0;
// const int BTNP2 = A1;

// unsigned long pressStartTime = 0;
// bool waitingForPress = false;
// bool player1Turn = true;
// volatile char cmd = ' ';
// volatile boolean received = false;

// enum RGBState { RED, GREEN, BLUE, OFF };

// void setRGB1Col(RGBState state);
// void setRGB2Col(RGBState state);
// void resetLEDs();
// void startRound();
// void handleBtn();

// void setup() {
//     Serial.begin(9600);
//     pinMode(redRGB1, OUTPUT); pinMode(greenRGB1, OUTPUT); pinMode(blueRGB1, OUTPUT);
//     pinMode(redRGB2, OUTPUT); pinMode(greenRGB2, OUTPUT); pinMode(blueRGB2, OUTPUT);
//     pinMode(BTNP1, INPUT); pinMode(BTNP2, INPUT);

//     resetLEDs();
//     SPCR |= _BV(SPE);
//     SPI.attachInterrupt();
// }

// ISR(SPI_STC_vect) {
//     char receivedCmd = SPDR;
//     if (receivedCmd != '#') {
//         cmd = receivedCmd;
//         received = true;
//     }
// }

// void loop() {
//     if (received) {
//         received = false;

//         if (cmd == 'r' || cmd == 'g' || cmd == 'b') {
//             startRound();
//         } else if (cmd == '0') {
//             resetLEDs();
//         }
//     }

//     if (waitingForPress) {
//         handleBtn();
//     }

    
// }

// void startRound() {
//     pressStartTime = millis();
//     waitingForPress = true;
//     RGBState color = (cmd == 'r') ? RED : (cmd == 'g') ? GREEN : BLUE;

//     if (player1Turn) {
//         setRGB1Col(color);
//         setRGB2Col(OFF);
//     } else {
//         setRGB2Col(color);
//         setRGB1Col(OFF);
//     }
//     player1Turn = !player1Turn;
        

// }

// void handleBtn() {
//     int temp1 = analogRead(BTNP1);
//     int temp2 = analogRead(BTNP2);
//     unsigned long elapsedTime = millis() - pressStartTime;

//     if (player1Turn) {
//         if (temp1 < 200) return;
//         SPDR = (cmd == 'r' && temp1 < 350) || (cmd == 'g' && temp1 < 475) || (cmd == 'b') ? 'a' : 'i';
        
//     } else {
//         if (temp2 < 200) return;
//         SPDR = (cmd == 'r' && temp2 < 350) || (cmd == 'g' && temp2 < 475) || (cmd == 'b') ? 'a' : 'i';
//     }

//     waitingForPress = false;
//     resetLEDs();
// }

// void setRGB1Col(RGBState state) {
//     digitalWrite(redRGB1, state == RED);
//     digitalWrite(greenRGB1, state == GREEN);
//     digitalWrite(blueRGB1, state == BLUE);
// }

// void setRGB2Col(RGBState state) {
//     digitalWrite(redRGB2, state == RED);
//     digitalWrite(greenRGB2, state == GREEN);
//     digitalWrite(blueRGB2, state == BLUE);
// }

// void resetLEDs() {
//     setRGB1Col(OFF);
//     setRGB2Col(OFF);
// }