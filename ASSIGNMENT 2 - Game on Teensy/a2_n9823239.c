/*Author: John Layson
Student number: 09823239
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "lcd.h"
#include "graphics.h"
#include "cpu_speed.h"
#include "sprite.h"

bool initialise = true;
bool gameover = false;
bool shoot = false;
int lives = 3;
int score = 0;
int minutes = 0;
double temp_seconds = 0;
int seconds = 0;

unsigned char spacecraft_image[8] = {
  0b00011000,
  0b00100100,
  0b01111110,
  0b01111110,
  0b11111111,
  0b11011011,
  0b10011001,
  0b10111101,
};

unsigned char crosshair_image[3] = {
  0b01000000,
  0b11100000,
  0b01000000,
};

unsigned char alien_image[5] = {
  0b00100000,
  0b11111000,
  0b01110000,
  0b10101000,
  0b10001000,
};

unsigned char missile_image[2] = {
  0b11000000,
  0b11000000,
};

Sprite Spacecraft;
Sprite Alien;
Sprite Crosshair;
Sprite Missile;

void setup_aim(){
  init_sprite(&Crosshair,Spacecraft.x + 2,Crosshair.y - 6,3,3,crosshair_image);
}

void draw_crosshair(){
  draw_sprite(&Crosshair);
}

void setup_spacecraft(){
  init_sprite(&Spacecraft,1 + rand()%(LCD_X - 10),10 + rand()%(LCD_Y - 18), 8, 8, spacecraft_image);
}

void draw_spacecraft(){
  draw_sprite(&Spacecraft);
}

void setup_alien(){
  int alien_x = 1 + rand()%(LCD_X-2-5);
  int alien_y = 10 + rand()%(LCD_Y-10-5);

  while ((alien_x+5 > Spacecraft.x && alien_x < Spacecraft.x+8) &&
  (alien_y+5 > Spacecraft.y && alien_y < Spacecraft.y+8)) {
    alien_x = 1 + rand()%(LCD_X-2-5);
    alien_y = 10 + rand()%(LCD_Y-9-5);
  }
  init_sprite(&Alien,alien_x,alien_y, 5, 5, alien_image);
}

void draw_alien(){
  draw_sprite(&Alien);
}

void initialise_hardware(void){
  set_clock_speed(CPU_8MHz);
  lcd_init(LCD_DEFAULT_CONTRAST);
  DDRF &= ~(0b01100000); //Switches as input
  DDRB &= ~(0b10000011); //Center left and Down
  DDRD &= ~(0b00000011); //Right and up
  // Setup TIMER0 in "normal" operation mode
  TCCR1B &= ~(1<<WGM02);
  //
  //TCCR1B |= (1<<CS02);
  //TCCR1B &= ~((1<<CS01) | (1<<CS00));
    TCCR1B |= ((1<<CS01) | (1<<CS00));
    TCCR1B &= ~(1<<CS02);
  // Enable the Timer Overflow Interrupt for TIMER0
  TIMSK1 |= (0b1 << TOIE0);
  //TIMER 0 stuff
  TCCR3B &= ~(1<<WGM02);
  TCCR3B |= ((1<<CS01) | (1<<CS00));
  TCCR3B &= ~(1<<CS02);
  TIMSK3 |= (0b1 << TOIE0);
  // Globally enable interrupts
  sei();
}

void setup_menu(void){
  draw_string(9.5,0, "Alien Advance");
  draw_string(14.5,9, "John Layson");
  draw_string(24.5,18, "9823239");
  draw_string(7,27, "Press a button");
  draw_string(14.5,36, "to continue");
}

void draw_border(void){
  char buff[100];

  sprintf(buff, "S:%d L:%d T:%02d:%02d ", score, lives,
  minutes, seconds);
  draw_string(0,0,buff);

  draw_line(0,9,LCD_X,9);
  draw_line(LCD_X-1,9,LCD_X-1,LCD_Y);
  draw_line(LCD_X,LCD_Y-1,0,LCD_Y-1);
  draw_line(0,LCD_Y,0,9);

  if (seconds == 60) {
    minutes++;
    seconds = 0;
  }
}

void countdown(void){
  draw_string(LCD_X/2-2.5, LCD_Y/2-4, "3");
  show_screen();
  _delay_ms(300);
  clear_screen();


  draw_string(LCD_X/2-2.5, LCD_Y/2-4, "2");
  show_screen();
  _delay_ms(300);
  clear_screen();


  draw_string(LCD_X/2-2.5, LCD_Y/2-4, "1");
  show_screen();
  _delay_ms(300);
  clear_screen();

  seconds = 0;
  minutes = 0;
}

void check_for_movement_press(void){
  //LEFT
  if(((PINB>>1)&0b1) && Spacecraft.x > 1){
    Spacecraft.x-=1;
    clear_screen();
    draw_border();
    draw_spacecraft();
    Crosshair.x = Spacecraft.x - 6;
    Crosshair.y = Spacecraft.y + 3;
  //  show_screen();
  }
  //UP
  if(((PIND>>1)&0b1) && Spacecraft.y > 10){
    Spacecraft.y-=1;
    clear_screen();
    draw_border();
    draw_spacecraft();
    Crosshair.x = Spacecraft.x + 2;
    Crosshair.y = Spacecraft.y - 6;
  //  show_screen();
  }
  //RIGHT
  if(((PIND>>0)&0b1) && Spacecraft.x < LCD_X - (1+8)){
    Spacecraft.x+=1;
    clear_screen();
    draw_border();
    draw_spacecraft();
    Crosshair.x = Spacecraft.x + 10;
    Crosshair.y = Spacecraft.y + 3;
  //  show_screen();
  }
  //DOWN
  if(((PINB>>7)&0b1) && Spacecraft.y < LCD_Y - (1+8)){
    Spacecraft.y+=1;
    clear_screen();
    draw_border();
    draw_spacecraft();
    Crosshair.x = Spacecraft.x + 2;
    Crosshair.y = Spacecraft.y + 10;
  //  show_screen();
  }
  draw_crosshair();
  show_screen();
}

void check_for_button_press(void){
  if(((PINF>>6)&0b1) || ((PINF>>5)&0b1)){
    while (((PINF>>6)&0b1) || ((PINF>>5)&0b1)) {
      _delay_ms(100);
    }
    clear_screen();
    countdown();
    initialise = false;
    gameover = false;
  }
}

bool spacecraft_collision(){
  bool collided = true;
  int spacecraft_top  = round(Spacecraft.y);
  int spacecraft_left = round(Spacecraft.x);
  int spacecraft_right = round(Spacecraft.x) + 8 -1;
  int spacecraft_bottom = round(Spacecraft.y) + 8 -1;

  int alien_top = round(Alien.y);
  int alien_left = round(Alien.x);
  int alien_right = round(Alien.x) + 5 -1;
  int alien_bottom = round(Alien.y) + 5 -1;

  if ( spacecraft_top > alien_bottom ) collided = false;
  else if ( spacecraft_right < alien_left ) collided = false;
  else if ( spacecraft_left > alien_right ) collided = false;
  else if ( spacecraft_bottom < alien_top ) collided = false;

  return collided;
}

bool alien_missile_collided(){
  bool collided1 = true;

  int missile_top  = round(Missile.y);
  int missile_left = round(Missile.x);
  int missile_right = round(Missile.x) + 2 - 1;
  int missile_bottom = round(Missile.y) + 2 - 1;

  int alien_top = round(Alien.y);
  int alien_left = round(Alien.x);
  int alien_right = round(Alien.x) + 5 -1;
  int alien_bottom = round(Alien.y) + 5 -1;

  if ( missile_top > alien_bottom ) collided1 = false;
  else if ( missile_right < alien_left ) collided1 = false;
  else if ( missile_left > alien_right ) collided1 = false;
  else if ( missile_bottom < alien_top ) collided1 = false;

  return collided1;
}

void game_over_screen(){
  clear_screen();
  draw_string(7,0, "##############");
  draw_string(7,9, "##GAMEOVER!!##");
  draw_string(7,18,"##############");
  draw_string(7,27, "Press a button");
  draw_string(9,36, "to play again");
  show_screen();
  if(((PINF>>6)&0b1) || ((PINF>>5)&0b1)){
    while (((PINF>>6)&0b1) || ((PINF>>5)&0b1)) {
      _delay_ms(100);
      lives = 3;
      seconds = 0;
      minutes = 0;
      score = 0;
    }
  clear_screen();
  initialise = true;
  }
}

void do_game_over(){
  if (lives == 0) {
    gameover = true;
  }
}

void spacecraft_alien_collision(void){
  if ( spacecraft_collision() ){
    if (lives >= 0) {
      lives = lives - 1;
      Spacecraft.is_visible = 0;
      setup_spacecraft();
      clear_screen();
      draw_spacecraft();
      Spacecraft.is_visible = 1;
      Missile.is_visible = 0;
    }
  }
}
void setup_missile(){
  init_sprite(&Missile,Spacecraft.x + 2,Spacecraft.y - 1,2,2,missile_image);
}

void draw_missile(){
  draw_sprite(&Missile);
}

void missile_alien_collision(void){
  if( alien_missile_collided() ){
    score++;
    shoot = false;
    Missile.is_visible = 0;
    setup_alien();
  }
}

void shoot_now(){
  if ((((PINF>>6)&0b1) && shoot == false) || (((PINF>>5)&0b1) && shoot == false)) {
    shoot = true;
    setup_missile();
    Missile.x = Spacecraft.x + 3;
    Missile.y = Spacecraft.y - 1;
  }
}

  void do_shoot(){
      Missile.y -= 1;
      clear_screen();
      draw_border();
      draw_spacecraft();
      draw_alien();
      draw_missile();

      if (Missile.y <= 9) {
        clear_screen();
        shoot = false;
        Missile.x = Spacecraft.x + 3;
        Missile.y = Spacecraft.y - 1;
      }
  }

  ISR(TIMER1_OVF_vect) {
    temp_seconds = (temp_seconds + 0.5);
    if (temp_seconds == 1) {
      seconds++;
      temp_seconds = 0;
    }
  }

  ISR(TIMER3_OVF_vect) {
    int previous_location_x;
    int previous_location_y;
    previous_location_x = Spacecraft.x;
    previous_location_y = Spacecraft.y;

    if (Alien.x < previous_location_x) {
      Alien.x = Alien.x + 1.5;
    }
    if (Alien.x > previous_location_x) {
      Alien.x = Alien.x - 1.5;
    }
    if (Alien.y < previous_location_y) {
      Alien.y = Alien.y + 1.5;
    }
    if (Alien.y > previous_location_y) {
      Alien.y = Alien.y - 1.5;
    }
    clear_screen();
  }

int main(void){
  initialise_hardware();
  while (1) {
    while ( initialise ) {
      setup_menu();
      show_screen();
      setup_spacecraft();
      setup_aim();
      setup_alien();
      check_for_button_press();
    }
    draw_spacecraft();
    while (!gameover) {
      shoot_now();
      draw_border();
      check_for_movement_press();
      draw_spacecraft();
      draw_alien();
      spacecraft_alien_collision();
      if (shoot == true) {
        do_shoot();
        Missile.is_visible = 1;
      }
      missile_alien_collision();
      do_game_over();
      show_screen();
    }
    game_over_screen();
  }
  return 0;
}
