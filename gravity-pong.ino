#include "functions.h" // Energia will compile the functions.cpp file for us

#define UP 1
#define DOWN -1
#define LEFT 1
#define RIGHT 0

#define GAME_MODE_INFINITE 1
#define GAME_MODE_NORMAL 0

const int button1 = PUSH1;
const int button2 = PUSH2;

int yBdir = 1; //1 up, 0 down
int xBdir = 1; //1 right, 0 left

// Switch statues
long lSwt1;
long lSwt2;
long lBtn2;

int dirPaddle = UP; // -1 is up, 1 is down

// The height of the bitmap we're about to create
const unsigned int bitmapHeight = 10;  

// The width of the bitmap we're about to create
const unsigned int bitmapWidth = 2; 

const int bitBallX = 3;
const int bitBallY = 3;
int last_accel_val = 0;

char bmp[][2] = { 
  {1, 1},
  {1, 1},
  {1, 1},
  {1, 1},
  {1, 1},
  {1, 1},
  {1, 1},
  {1, 1},
  {1, 1},
  {1, 1} 
};
  
char right_wall[][2] = {
  {1, 1}, {1, 1}, {1, 1}, {1, 1}, {1, 1},
  {1, 1}, {1, 1}, {1, 1}, {1, 1}, {1, 1},
  {1, 1}, {1, 1}, {1, 1}, {1, 1}, {1, 1},
  {1, 1}, {1, 1}, {1, 1}, {1, 1}, {1, 1},
  {1, 1}, {1, 1}, {1, 1}, {1, 1}, {1, 1},
  {1, 1}, {1, 1}, {1, 1}, {1, 1}, {1, 1},
  {1, 1}, {1, 1}
};

 char ball[][3] = { 
  {0,1,0},
  {1,1,1},
  {0,1,0} 
};

char text_score[] = { 
  'S', 'c', 'o', 'r', 'e', '\0'
};

char game_over[] = {
  'G', 'A', 'M', 'E', ' ', 'O', 'V', 'E', 'R', '\0'
};
         
int game_mode = GAME_MODE_NORMAL;      
int score = 0;
char score_text[10];
int lives = 3;
int ball_path_mod = 0;

unsigned int paddle_y1_posn = 0;
unsigned int paddle_y2_posn = 10;
unsigned int paddle_x_posn = 0;
int right_wall_posn = 80;

unsigned int ball_x_posn = 15;
unsigned int ball_y_posn = 28;

char* formattedBitmap(char* input, unsigned int width, unsigned int height) {
  
  unsigned int h = ceil(height / 8.0);
  char *output = (char*)calloc(h * width, sizeof(char));
  char b, temp;
  
  for (unsigned int hbyte = 0; hbyte < h; ++hbyte) {
    for (unsigned int i = 0; i < width; ++i) {
      b = 0;
      for (unsigned int j = 0; j < ((height - hbyte * 8)/8 ? 8 : (height%8)); ++j) {
        temp = input[(8*hbyte+j)*width+i];
        if (temp) {
          b |= 1 << j;
        }
      }
      output[hbyte*width+i]|=b;
    }
  }

  return output;
}

char* bitmap = formattedBitmap((char*)bmp, bitmapWidth, bitmapHeight);
char* bitBall = formattedBitmap((char*)ball, bitBallY, bitBallX);
char* right_wall_bm = formattedBitmap((char*)right_wall, 2, 32);

void setup() {

  // Helper function found in functions.cpp

  deviceInit(); 
  pinMode(button1, INPUT_PULLUP);
  pinMode(button2, INPUT_PULLUP);

  // Reset OLED (function from functions.cpp)

  oledReset();
  OrbitOledUpdate();
}

void loop() {
  
  is_diff_game_mode(); // different game modes

  movePaddle();
  render_ball();
  render_right_wall();
  render_score();
  render_lives();

  lBtn2 = GPIOPinRead(BTN2Port, BTN2);

  if (lBtn2 == BTN2) 
  {
    render_credits();
  }

  ballMove();
 
  OrbitOledUpdate();
  oledReset();
}

void render_credits() {

  int title_delay = 750;
  int name_delay = 500;

  char pong[] = {
    'P', 'O', 'N', 'G', ' ', 'B', 'Y', '\0'
  };
  char kevin[] = { 
    'K', 'e', 'v', 'i', 'n', '\0'
  };
  char michael[] = { 
    'M', 'i', 'c', 'h', 'a', 'e', 'l', '\0'
  };
  char rohan[] = { 
    'R', 'o', 'h', 'a', 'n', '\0'
  };
  char yang[] = {
    'Y', 'a', 'n', 'g', '\0'
  };

  oledReset();
  oledPrintText(pong, 2, 1);
  delay(title_delay);
  oledReset();
  oledPrintText(rohan, 2, 1);
  delay(name_delay);
  oledReset();
  oledPrintText(kevin, 2, 1);
  delay(name_delay);
  oledReset();
  oledPrintText(michael, 2, 1);
  delay(name_delay);
  oledReset();
  oledPrintText(yang, 2, 1);
  delay(name_delay);
  oledReset();

}

void is_diff_game_mode() {

  if (is_switch1() && game_mode != GAME_MODE_INFINITE) {

    game_mode = GAME_MODE_INFINITE;
    new_game();
  } 

  else if (!is_switch1() && game_mode != GAME_MODE_NORMAL) {

    game_mode = GAME_MODE_NORMAL;
    new_game();
  }

  return;
}

void ballMove() {

    collision();

    // moving up
    if(yBdir == 1) {

       ball_y_posn -= 1 + ball_path_mod;
    }

    // moving down
    else {

        ball_y_posn += 1 + ball_path_mod;
    }

    // moving right
    if(xBdir == 1) {

        ball_x_posn++;
    }

    // moving left
    else {

        ball_x_posn--;
    }
}

void collision() {

  
  if ((ball_x_posn <= 3) && (ball_y_posn >= (paddle_y1_posn - 1)) && (ball_y_posn <= (paddle_y2_posn + 1))) { 
     
      xBdir = 1;	// move it right
      score += 1;
  }
  
  // hits ceiling
  if(ball_y_posn <= 0) {

    mod_ball_path();
    yBdir = 0;
    ball_y_posn = 0;

  }

  // hits floor
  else if(ball_y_posn >= 28) {

    yBdir = 1;
    mod_ball_path();
    ball_y_posn = 28;

  }

  // hits paddle left wall
  // Paddle width 2, so we have it like this
  if(ball_x_posn <= 1) {

    red_led_on(250);
    lose();
  }

  // hits right wall
  else if(ball_x_posn >= (right_wall_posn - 2)) 
  {
    mod_ball_path();
    ball_x_posn = right_wall_posn - 2;
    xBdir = 0;
  }

}

void mod_ball_path() {

  ball_path_mod = rand() % 2;

  return;
}

void movePaddle() {

  int curr_accel_val = getAccelValue();
  int accel_delta = curr_accel_val - last_accel_val;
  last_accel_val = curr_accel_val;


  // if switch1 is on, use absolute position
  // else use relative position
  if (is_switch2()) {

    if (getAccelValue() < -8) {

      rand();
      dirPaddle = -1;
    } 

    else if (getAccelValue() > 8) {

      dirPaddle = 1;
    } 

    else {

      dirPaddle = 0; // no movement
    }
 } 

  else {

    if (digitalRead(button1) == HIGH && digitalRead(button2) == LOW) {

      rand();
      dirPaddle = 1;
    } 

    else if (digitalRead(button2) == HIGH && digitalRead(button1) == LOW) {
     
      dirPaddle = -1;
    } 

    else {

      dirPaddle = 0;
    }
 }

  int paddle_limit = is_switch2() ? 2 : 1;
  for (int i = 0; i < paddle_limit; i++) {

    if (dirPaddle == 1) {

      if (paddle_y1_posn <= 20) {

        paddle_y1_posn += 1;
        paddle_y2_posn += 1;
      } 

      else {

        dirPaddle = -1;
      }
    } 

    else if (dirPaddle == -1) {

      if (paddle_y1_posn >= 1) {

        paddle_y1_posn -= 1;
        paddle_y2_posn -= 1;
      } 

      else {

        dirPaddle = 1;
      }
    }

    render_paddle();
  }
}

void lose() {

  delay(500);

  reset_posns();

  if (game_mode == GAME_MODE_NORMAL) {

    if (lives == 0) {
      score = 0;
      lives = 3;

      render_game_over();
    }

    else {

      lives -= 1;  
    }  
  } 

  else if (score <= 0) {

      score = 0;
      render_game_over();
  } 

  else {

    score -= 2;
  }
  
  return;  
}

void new_game() {

  delay(500);
  reset_posns();
  score = 0;
  lives = 3;

  return;
}


void reset_posns() {

  ball_x_posn = 5;
  ball_y_posn = 5;
  ball_path_mod = 0;

  paddle_x_posn = 0;
  paddle_y1_posn = 0;
  paddle_y2_posn = 10;

  xBdir = 1;
  yBdir = 1;

  return;
}

void render_score() {

  oledPrintText(text_score, 11, 0);
  sprintf(score_text, "%d", (int)score);
  oledPrintText(score_text, 11, 1);

  return;
}

void render_game_over() {

  int led_delay = 200;

  oledReset();
  oledPrintText(game_over, 2, 1);
  
  for (int i = 0; i < 3; i++) {
    red_led_on(led_delay);
    delay(led_delay);
  }

  //OrbitOledUpdate();
  delay(750);

  return;
}

void render_lives() {

  if (game_mode == GAME_MODE_INFINITE) {
    return;
  }

  int padding = 5;
  for (int i = 0; i < lives; i++) 
  {
    oledDraw(bitBall, (right_wall_posn + 10) + (i * padding), 23, bitBallY, bitBallX);
  }

  return;
}

void render_paddle() {

  oledDraw(bitmap, paddle_x_posn, paddle_y1_posn, bitmapWidth, bitmapHeight);

  return;
}

void render_ball() {

  oledDraw(bitBall, ball_x_posn, ball_y_posn, bitBallY, bitBallX);

  return;
}

void render_right_wall() {

  oledDraw(right_wall_bm, right_wall_posn, 0, 2, 32);

  return; 
}

void red_led_on(int delay_ms) {

  digitalWrite(RED_LED, HIGH);
  delay(delay_ms);
  digitalWrite(RED_LED, LOW);

  return;
}

int check_switches() {

  long lSwt1 = GPIOPinRead(SWT1Port, SWT1);
  long lSwt2 = GPIOPinRead(SWT2Port, SWT2);
  int chSwtCur;

  chSwtCur = (lSwt1 | lSwt2) >> 6;

  return chSwtCur;
}

bool is_switch1() {

  if (GPIOPinRead(SWT1Port, SWT1)) {

    return true;
  } 

  else {

    return false;
  }

}

bool is_switch2() {

  if (GPIOPinRead(SWT2Port, SWT2)) {

    return true;
  } 

  else {

    return false;
  }
}

int getAccelValue() {

  short	dataX;
  short dataY;
  short dataZ;

  char printVal[10];

  char 	chPwrCtlReg = 0x2D;
  char 	chX0Addr = 0x32;
  char  chY0Addr = 0x34;
  char  chZ0Addr = 0x36;

  char 	rgchReadAccl[] = { 0, 0, 0 };
  char 	rgchWriteAccl[] = { 0, 0 };

  char rgchReadAccl2[] = { 0, 0, 0 };

  char rgchReadAccl3[] = { 0, 0, 0 };

  int xDirThreshPos = 50;
  int	xDirThreshNeg = -50;

  bool fDir = true;
  
  bool fClearOled = true;

  /*
   * If applicable, reset OLED
   */
  if(fClearOled == true) {
    
    
    //Enable I2C Peripheral
     
    SysCtlPeripheralEnable(SYSCTL_PERIPH_I2C0);
    SysCtlPeripheralReset(SYSCTL_PERIPH_I2C0);

    
    //Set I2C GPIO pins
     
    GPIOPinTypeI2C(I2CSDAPort, I2CSDA_PIN);
    GPIOPinTypeI2CSCL(I2CSCLPort, I2CSCL_PIN);
    GPIOPinConfigure(I2CSCL);
    GPIOPinConfigure(I2CSDA);

    //Setup I2C
     
    I2CMasterInitExpClk(I2C0_BASE, SysCtlClockGet(), false);

    //Initialize the Accelerometer    
     
    GPIOPinTypeGPIOInput(ACCL_INT2Port, ACCL_INT2);

    rgchWriteAccl[0] = chPwrCtlReg;
    rgchWriteAccl[1] = 1 << 3;		// sets Accl in measurement mode

    I2CGenTransmit(rgchWriteAccl, 1, WRITE, ACCLADDR);
  }  

    rgchReadAccl[0] = chX0Addr;
    rgchReadAccl2[0] = chY0Addr;
    rgchReadAccl3[0] = chZ0Addr;
    
    I2CGenTransmit(rgchReadAccl, 2, READ, ACCLADDR);
    I2CGenTransmit(rgchReadAccl2, 2, READ, ACCLADDR);
    I2CGenTransmit(rgchReadAccl3, 2, READ, ACCLADDR);
    
    dataX = (rgchReadAccl[2] << 8) | rgchReadAccl[1];
    dataY = (rgchReadAccl2[2] << 8) | rgchReadAccl2[1];
    dataZ = (rgchReadAccl3[2] << 8) | rgchReadAccl2[1];
    
    return (int)dataY; 
}
