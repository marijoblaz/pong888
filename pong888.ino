//Include

#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library for ST7735
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
#include <SPI.h>

//LCD pins
#define TFT_CS        A0
#define TFT_RST       10 // Or set to -1 and connect to Arduino RESET pin
#define TFT_DC         2

//1.8" TFT with ST7735 use:
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

//Keys
#define UP_KEY  3
#define DOWN_KEY 5
#define LEFT_KEY 4
#define RIGHT_KEY 6
#define A_KEY 7
#define B_KEY 8
#define BAT_PIN A3
#define SPEAKER_OUTPUT_PIN  9 //buzzer to arduino pin 9

//PressTone
#define PTONE 500

int oldPercInt = 100;
auto batTextColor = ST77XX_WHITE;
bool batFirtPowerOn = true;

char printout[4];
char printout2[5];

const int PADDLE_WIDTH = 25; // The width of the paddle.
const int PADDLE_HEIGHT = 2; // The height of the paddle.
const int BALL_DIMENSIONS = 3; // The length/width of the ball.
const int MAX_SCORE_TO_WIN = 5; // The maximum score a player needs until it's game over.
bool autoPlay = false;
int ballDirectionX = 1; // The X velocity of the game ball.
int ballDirectionY = 1; // The Y velocity of the game ball.

int paddleX = 0; // The X location of the player's paddle.
int paddleY = 0; // The Y location of the player's paddle.
int oldPaddleX, oldPaddleY; // The old X and Y locations of the player's paddle.

int menuKeyPosition = 15;

int opponentX = 0; // The X location of the opponent's paddle.
int opponentY = 0; // The Y location of the opponent's paddle.
int oldOpponentX, oldOpponentY; // The old X and Y locations of the opponent's paddle.

int ballX, ballY, oldBallX, oldBallY; // The ball's locations variables.
int playerScore, opponentScore; // The score of the player and the opponents.

boolean gameOver = false; // States whether the game is over or not.
boolean gameOverScreenDrawn = false; // States whether the game over screen has been drawn or not.
boolean opponentWon = true; // States whether the opponent won or not.

long interval = 10000;           // interval at which to blink (milliseconds)
long previousMillis = 0;        // will store last time LED was updated

//Menu
bool showMenu = true;
bool showGame = false;

void setup() {
  Serial.begin(9600);
  tft.initR(INITR_BLACKTAB); // Init ST7735S chip, black tab
  tft.fillScreen(ST77XX_BLACK);
  tft.setRotation(1);

  // buzzer init
  pinMode(SPEAKER_OUTPUT_PIN, OUTPUT); // Set buzzer - pin 9 as an output
  
  //Buttons init
  analogReference(INTERNAL);
  pinMode(UP_KEY,INPUT_PULLUP);
  pinMode(DOWN_KEY,INPUT_PULLUP);
  pinMode(LEFT_KEY,INPUT_PULLUP);
  pinMode(RIGHT_KEY,INPUT_PULLUP);
  pinMode(A_KEY,INPUT_PULLUP);
  pinMode(B_KEY,INPUT_PULLUP);

  pinMode(BAT_PIN,INPUT);
    
  centerPaddles();
  centerBall();
}
void loop() {
  randomSeed(millis());

  if(showMenu){
    drawBat();
    int width = tft.width();
    int height = tft.height();
     
    tft.setTextSize(1.8);
    tft.setCursor( 80, 20);
    tft.println("PLAY Game ->");
    
    tft.setTextSize(2);
    tft.setCursor( 15, 55);
    tft.println("PONG!");

    tft.setTextSize(1.8);
    tft.setCursor( 80, 100);
    tft.println("EXIT Game ->");

     if(digitalRead(A_KEY) == LOW){
      tft.fillScreen(ST77XX_BLACK);
      showMenu = false;
      showGame = true;
     }
    
  }

  if(showGame){
    checkScores();
    drawCourt();
    drawPaddles();
    drawScores();
    moveOpponentPaddle();
    moveBall();
    checkKeys();

    if(digitalRead(B_KEY) == LOW){
      tft.fillScreen(ST77XX_BLACK);
      showMenu = true;
      showGame = false;
      opponentScore = 0;
      playerScore = 0;
      centerPaddles();
      centerBall();
     }

    if(digitalRead(LEFT_KEY) == LOW){
      autoPlay = true;
     }
    if(digitalRead(RIGHT_KEY) == LOW){
      autoPlay = false;
     }
     
    if (gameOver)
    {
      drawGameOverScreen();
    }
  }

  
}

void centerPaddles(){
  paddleX = 2;
  opponentX = tft.width()-7;
  
  paddleY = (tft.width() / 2 - 30);
  opponentY = (tft.width() / 2 - 30);
}
void centerBall(){
  
  ballX = (tft.width() / 2) - BALL_DIMENSIONS;
  ballY = (tft.height() / 2) - BALL_DIMENSIONS;

  int ballDirX = random(-1, 2);
  if(ballDirX != 0){
    ballDirectionX = ballDirX;  
  }
  int ballDirY = random(-1, 2);
  if(ballDirY != 0){
    ballDirectionY = ballDirY;  
  }
}
void checkKeys(){
  if(autoPlay){
    int moveSpeed = 1;
  
     if(ballX < tft.width()/2){
        if (ballY < paddleY + 12.5)
        {
          paddleY -= moveSpeed;
          
          if (paddleY < 0)
          {
              paddleY = 0;
          }
        }
        
        if(ballY > paddleY + 12.5)
        {
          paddleY += moveSpeed;
          
          if (paddleY + 25 > tft.height())
          {
            paddleY = tft.height() - 25;
          }
    
        }
     }
  }

  if(digitalRead(UP_KEY) == LOW){
    paddleY -= 2;
    if (paddleY < 0)
      {
          paddleY = 0;
      }
  }else if(digitalRead(DOWN_KEY) == LOW){
    paddleY += 2;
    if(paddleY > tft.height() -25){
      paddleY = tft.height() - 25;
    }
    }
   else{
   //noTone(SPEAKER_OUTPUT_PIN);     // Stop sound... 
  }
  
}
float mapfloat(float x, float in_min, float in_max, float out_min, float out_max){
 return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
void drawBat(){

    unsigned long currentMillis = millis();

    String percs = String(oldPercInt);
    percs.toCharArray(printout2,5);
    tft.setTextSize(1);
    tft.setTextColor(batTextColor);
    tft.setCursor(135 , 2);
    tft.print(printout2);
    tft.print("%");
      
    if(currentMillis - previousMillis > interval || batFirtPowerOn) {
      previousMillis = currentMillis;  

      int sensorValue = analogRead(BAT_PIN); //read the pin value
      float voltage = sensorValue * (2.3 / 1023.00) * 2; //convert the value to a true voltage.
      float perc = mapfloat(voltage, 3.5, 4.1, 1, 100);

      if(perc>100){
        perc = 100;
      }

      int percInt = (int)perc;
      if(percInt < oldPercInt){
        tft.fillRect(130, 1, 30,12, ST77XX_BLACK);
        oldPercInt = percInt;
        if(oldPercInt < 10) batTextColor = ST77XX_RED;
      }
      
      batFirtPowerOn = false;
    }
}
void drawPaddles(){
  // Erease the old positions of the both the player and the opponents paddle on the screen:
 if (oldPaddleY != paddleY) {
  // Draw player paddle:
  tft.drawFastVLine(2, oldPaddleY, PADDLE_WIDTH, ST77XX_BLACK);
 }
  
 if (oldOpponentY != opponentY) {
  // Draw the opponents paddle:
  tft.drawFastVLine(157, oldOpponentY, PADDLE_WIDTH, ST77XX_BLACK);
 }
 
  tft.drawFastVLine(2, paddleY, PADDLE_WIDTH, ST77XX_WHITE);
  tft.drawFastVLine(157, opponentY, PADDLE_WIDTH, ST77XX_WHITE);
  oldPaddleY = paddleY;
  oldOpponentY = opponentY;
}
void checkScores(){ 
  if (playerScore >= MAX_SCORE_TO_WIN)
  {
      gameOver = true;
      gameOverScreenDrawn = false;
      opponentScore = 0;
      playerScore = 0;
      opponentWon = false;
  }

  if (opponentScore >= MAX_SCORE_TO_WIN)
  {
      gameOver = true;
      gameOverScreenDrawn = false;
      opponentScore = 0;
      playerScore = 0;
      opponentWon = true;
  }
}
void drawGameOverScreen(){
  if (!gameOverScreenDrawn) // We don't want to always draw the game over screen because that will cause flickering...
  {
    tft.fillScreen(ST77XX_BLACK);
    tft.setTextSize(2);
    tft.setTextWrap(false);
    
    if (opponentWon)
    {
      tft.setCursor(tft.width() / 2 - 50, tft.height() / 2 - 10);
      tft.println("YOU LOST!");
     
      
      delay(1000);

      // Play the "you lost" tune:
      tone(SPEAKER_OUTPUT_PIN, 700);
      delay(150);
      noTone(SPEAKER_OUTPUT_PIN);
      tone(SPEAKER_OUTPUT_PIN, 600);
      delay(130);
      noTone(SPEAKER_OUTPUT_PIN);
      tone(SPEAKER_OUTPUT_PIN, 500);
      delay(100);
      noTone(SPEAKER_OUTPUT_PIN);
      delay(2000);
      
    }
    else
    {
      tft.setCursor(tft.width() / 2 - 50, tft.height() / 2 - 10);
      tft.println("YOU WON!");

      delay(1000);
      
      // Play the "you won" tune:
      tone(SPEAKER_OUTPUT_PIN, 1000);
      delay(125);
      noTone(SPEAKER_OUTPUT_PIN);
      tone(SPEAKER_OUTPUT_PIN, 1300);
      delay(125);
      noTone(SPEAKER_OUTPUT_PIN);
      tone(SPEAKER_OUTPUT_PIN, 1600);
      delay(125);
      noTone(SPEAKER_OUTPUT_PIN);
      tone(SPEAKER_OUTPUT_PIN, 1900);
      delay(125);
      noTone(SPEAKER_OUTPUT_PIN);
      delay(2000);
    }
    tft.fillScreen(ST77XX_BLACK);
    gameOverScreenDrawn = true;
    showMenu = true;
    showGame = false;
  }
}
void drawCourt(){
  // Draw the dividing line between the two players:
  tft.drawFastVLine(tft.width()/2, 0 , tft.height(), ST77XX_WHITE);
}
void drawScores(){
  int width = tft.width();
  int height = tft.height();
  String score = String(playerScore);

  // Draw the opponents score onto the screebL
  score.toCharArray(printout,4);
  tft.setTextSize(2);
  tft.setCursor( height / 2 - 10, width / 2 - 70);
  tft.println(printout);

  score = String(opponentScore);
  score.toCharArray(printout,4);
  tft.setCursor( height / 2 + 30, width / 2 - 70);
  tft.println(printout);

}
void moveBall() {
  // Move the ball according to its respective x and y velocity:
  ballX += ballDirectionX;
  ballY += ballDirectionY;

  // Check if the ball intersects with either the player paddle or opponent paddle. If it does, then bounce the ball back.
  if (intersects(ballX, ballY, paddleX, paddleY, PADDLE_WIDTH, PADDLE_HEIGHT)) 
  {

    //ballY = paddleY + BALL_DIMENSIONS* 2;
    ballDirectionX = 1;

    tone(SPEAKER_OUTPUT_PIN, 500);
    delay(40);
    noTone(SPEAKER_OUTPUT_PIN);
  }

  if (intersects(ballX, ballY, opponentX, opponentY, PADDLE_WIDTH, PADDLE_HEIGHT)) 
  {

    //ballY = opponentY - BALL_DIMENSIONS * 2;
    ballDirectionX = -1;

    tone(SPEAKER_OUTPUT_PIN, 500);
    delay(40);
    noTone(SPEAKER_OUTPUT_PIN);
  }



  // Ball hit the bottom wall:
  if (ballY + BALL_DIMENSIONS >= tft.height()) 
  {
    ballDirectionY = -ballDirectionY;
    tone(SPEAKER_OUTPUT_PIN, 500);
    delay(20);
    noTone(SPEAKER_OUTPUT_PIN);
  }
    // Ball hit the right wall:
  if (ballX + BALL_DIMENSIONS >= tft.width()) 
  {
    ballDirectionX = -ballDirectionX;
    score(false);
  }
  // Ball hit the top wall:
  if (ballY <= 0)
  {
    ballDirectionY = -ballDirectionY;
    tone(SPEAKER_OUTPUT_PIN, 500);
    delay(20);
    noTone(SPEAKER_OUTPUT_PIN);
  }
  //Ball hit the left wall
  if (ballX <= 0)
  {
    ballDirectionX = -ballDirectionX;
    score(true);
  }

  // Draw the ball new location:
  tft.fillCircle(oldBallX, oldBallY, BALL_DIMENSIONS - 1, ST77XX_RED);
  tft.fillCircle(ballX, ballY, BALL_DIMENSIONS - 1, ST77XX_WHITE);
  oldBallX = ballX;
  oldBallY = ballY;
}
void moveOpponentPaddle(){
  
    int moveSpeed = 1;

   if(ballX > tft.width()/2){
      if (ballY < opponentY + 8)
      {
        
          opponentY -= moveSpeed;
          
        
        if (opponentY < 0)
        {
            opponentY = 0;
        }
      }
      
      if(ballY > opponentY + 18)
      {
        
          opponentY += moveSpeed;
          
        
        if (opponentY + 25 > tft.height())
        {
          opponentY = tft.height() - 25;
        }
  
      }
    
   }
   
      
    
    
}
boolean intersects(int x, int y, int rectX, int rectY, int rectWidth, int rectHeight) {
  boolean result = false;

  if ((x >= rectX && x <= (rectX + rectHeight)) && (y >= rectY && y <= (rectY + rectWidth))) 
  {
    result = true;
  }

  return result;
}
void score(boolean opponentScored){ 
  // Add 1 point to the player who scored:
  if (opponentScored)
  {
    opponentScore++;
  }
  else
  {
    playerScore++;
  }


  // Play the "score" tune:
  tone(SPEAKER_OUTPUT_PIN, 1000);
  delay(50);
  noTone(SPEAKER_OUTPUT_PIN);
  tone(SPEAKER_OUTPUT_PIN, 1100);
  delay(50);
  noTone(SPEAKER_OUTPUT_PIN);
  tone(SPEAKER_OUTPUT_PIN, 1200);
  delay(50);
  noTone(SPEAKER_OUTPUT_PIN);
  
  delay(1500);
  
  centerPaddles();
  centerBall(); 
  tft.fillScreen(0x0000);
 
}
