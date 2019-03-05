#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <cab202_graphics.h>
#include <cab202_sprites.h>
#include <cab202_timers.h>
#include <curses.h>

#define DIAMOND_WIDTH (5)
#define DIAMOND_HEIGHT (5)
#define SPACECRAFT_WIDTH (5)
#define SPACECRAFT_HEIGHT (3)
#define MENU_DIALOG_WIDTH (40)
#define MENU_DIALOG_HEIGHT (14)
#define DELAY (10)


bool update_screen = true;
bool game_over = false;
int lives = 10;

sprite_id diamond;
sprite_id spacecraft;
sprite_id displayOption;

char * RegularDiamond = "  D  "
                        " DDD "
                        "DDDDD"
                        " DDD "
                        "  D  ";
char * SmallD = " D "
                "DDD"
                " D ";

char * SmallerD = "C";

char * Spacecraft_image = "^|||^"
                          "^|O|^"
                          "||_||";

char * menuOption =
/**/               "########################################"
/**/               "#         CAB202 Assignment 1          #"
/**/               "#        The Diamonds of Doom          #"
/**/               "#             John Layson              #"
/**/               "#              n9823239                #"
/**/               "########################################"
/**/               "#              Controls                #"
/**/               "#      q         : quit                #"
/**/               "#      h         : help                #"
/**/               "#      Arrow keys: move left/right     #"
/**/               "# Space, z, x, c : shoot bullet        #"
/**/               "########################################"
/**/               "#       Press a key to play...         #"
/**/               "########################################";

char * msg_image =
/**/	"Goodbye and thank-you for playing ZombieDash Jr."
/**/	"            Press any key to exit...            ";


void draw_menu( void ){
  int w = screen_width(), h = screen_height();
  int mx = (w - MENU_DIALOG_WIDTH) / 2, my = (h - MENU_DIALOG_HEIGHT) / 2;
  sprite_id displayOption = sprite_create(mx, my, MENU_DIALOG_WIDTH, MENU_DIALOG_HEIGHT, menuOption);
  sprite_draw(displayOption);
}

void draw_border( void ) {
  int top = 0;
  int left = 0;
  int right = screen_width() - 1;
  int bottom = screen_height() - 1;


  draw_line(left, top, right, top, '|');
  draw_line(left + 1, top + 3, right - 1, top + 3, '|');
  draw_line(right, top, right, bottom, '|');
  draw_line(right, bottom, left, bottom, '|');
  draw_line(left, bottom, top, left, '|');

  draw_formatted( left + 1, top + 2,
      "Lives =  %d", lives
      );
  draw_formatted( screen_width() * 0.20, top + 2,
      "| Score = %d", 0
      );
  draw_formatted( screen_width() * 0.40, top + 2,
      "| Time = %d", 90
      );
}

void play_game(void){
  clear_screen();

  draw_border();

  sprite_draw(diamond);

  sprite_draw(spacecraft);
}

void setup(void){
  int w = screen_width(), h = screen_height();

  draw_menu();
  show_screen();
  wait_char();


//  draw_border();

//  show_screen();


  //Set up Spacecraft
  int sw = SPACECRAFT_WIDTH, sh = SPACECRAFT_HEIGHT;
  int x = (w - sw)/2;
  int y = ((h - sh)-1);
  spacecraft = sprite_create(x, y, sw, sh, Spacecraft_image);

  //Draw Spacecraft_image
  //sprite_draw(spacecraft);

  //show_screen();

  //Setup Diamonds
  srand(get_current_time());
  int dx = 1 + rand() % (screen_width() - DIAMOND_WIDTH - 2);
  int dy = 4;
  diamond = sprite_create(dx, dy, DIAMOND_WIDTH, DIAMOND_HEIGHT, RegularDiamond);
  //Draw diamond
  //sprite_draw(diamond);

  //Set diamond in motion
//  srand(21345678);
  int angle = rand() % 360;
  sprite_turn_to(diamond, 0, 0.05);
  sprite_turn(diamond, angle);

  show_screen();
}

void cleanup(void) {
    // STATEMENTS
}

bool collision(void) {
    bool collided = true;

    int spacecraft_top = round(sprite_y(spacecraft)), spacecraft_base = spacecraft_top + SPACECRAFT_HEIGHT - 1;
    int spacecraft_left = round(sprite_x(spacecraft)), spacecraft_right = spacecraft_left + SPACECRAFT_WIDTH - 1;

    int diamond_top = round(sprite_y(diamond)), diamond_base = diamond_top + DIAMOND_HEIGHT - 1;
    int diamond_left = round(sprite_x(diamond)), diamond_right = diamond_left + DIAMOND_WIDTH - 1;

    if ( spacecraft_base < diamond_top ) collided = false;
    else if ( spacecraft_top > diamond_base ) collided = false;
    else if ( spacecraft_right < diamond_left ) collided = false;
    else if ( spacecraft_left > diamond_right ) collided = false;

    return collided;
}

void process(void){
  int w = screen_width(), h = screen_height();
  int key = get_char();

  if ( key == 'q' ) {
    clear_screen();
    int message_width = strlen(msg_image) / 2;
    sprite_id msg = sprite_create((w - message_width) / 2, (h - 2) / 2, message_width, 2, msg_image);
    sprite_draw(msg);
    show_screen();
    game_over = true;
    wait_char();
    return;
  }
  if (key < 0){
    sprite_step(diamond);
    // (s) Get screen location of the diamond.
    int zx = sprite_x(diamond);
    int zy = sprite_y(diamond);

    // (t) Get the displacement vector of the zombie.
    double zdx = sprite_dx(diamond);
    double zdy = sprite_dy(diamond);

    // (u) Test to see if the zombie hit the left or right border.
    if ( zx <= 0 ) {
      zdx = fabs(zdx);
    }
    else if ( zx >= w - 1 - DIAMOND_WIDTH ) {
      zdx = -fabs(zdx);
    }

    // (v) Test to see if the zombie hit the top or bottom border.
    if ( zy <= 3 ) {
      zdy = fabs(zdy);
    }
    else if ( zy >= h - 1 - DIAMOND_HEIGHT ) {
      zdy = -fabs(zdy);
    }

    // (w) Test to see if the diamond needs to step back and change direction.
    if ( zdx != sprite_dx(diamond) || zdy != sprite_dy(diamond) ) {
      sprite_back(diamond);
      sprite_turn_to(diamond, zdx, zdy);
    }

  }
  // (g)	Get the current screen coordinates of the spacecraft in integer variables
  //		by rounding the actual coordinates.
  int sx = round(sprite_x(spacecraft));

  if ( key == KEY_LEFT && sx > 1 ) sprite_move(spacecraft, -1, 0);

  if ( key == KEY_RIGHT && sx < w - sprite_width(spacecraft) - 1 ) sprite_move(spacecraft, +1, 0);

  //UPDATE THE LIVES COUNT
  if ( collision() ) {
    sprite_destroy(diamond);
    lives = lives - 1;
    return;
  }

  play_game();

}


int main(void){
  setup_screen();
  setup();
  show_screen();
//  draw_menu();
//  wait_char();


  while ( !game_over ) {
      process();

      if ( update_screen ) {
          show_screen();
      }
      	timer_pause(DELAY);
    }

  cleanup();

  return 0;
}
