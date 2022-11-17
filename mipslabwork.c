#include <stdint.h>  /* Declarations of uint_32 and the like */
#include <pic32mx.h> /* Declarations of system-specific addresses etc */
#include "mipslab.h" /* Declatations for these labs */
#include "drawingFunctions.h"
#include "Collision.h"
#include "mapFunctions.h"

unsigned int tickValue = 0x1;
volatile int *trise = (volatile int *)0xbf886100;
volatile int *porte = (volatile int *)0xbf886110;
int ticks = 0;
int displayUpdateCounter = 0;
extern uint8_t screen_data[512];
extern char player_default[88];
extern char baby_block_01[32];
extern char* images[2];
extern uint8_t imageSizes[][2];

GameObject *player;

GameObject gameObjects[16];
int gameObjectsLength;

Collision collisions[100];
int collisionsLength;

void apply_physics();
void find_collisions();
void handle_collisions();

void user_isr(void)
{
  /*if (IFS(0))
  {
    IFSCLR(0) = 0x100;
    count++;
    if (count == 10)
    {
    count = 0;
    ticks++;
    tick(&mytime);
    }
    
  }*/
  return;
}

void LoadScene(int level, int scene)
{
  gameObjectsLength = GetLevelSceneLength(0, 0);
  gameObjectsLength = 16;
  LoadLevelScene(gameObjects, level, scene);
  
  player = &gameObjects[0];

  player->yVelocity = 0;

  int i;
  for (i = 0; i < gameObjectsLength; i++) //render to background
  {
    if(i != 0 && gameObjects[i].graphicIndex == 0) break;
    
    if(gameObjects[i].graphicIndex == 1)
      drawGameObject(&gameObjects[i], 1);
  }
}

void start(void)
{
  T2CON = 0b111 << 4;                               // sätter prescale till 256
  PR2 = (80000000 / 256) / 100;                     // sätter period
  TMR2 = 0;                                         // nollar timer 2
  IECSET(0) = 0x00000100;                           // sätter så interupt är enable på timer 2
  IPCSET(0) = 0b11111;                              // sätter prioritet priritet 3 och sub prio 1
  enable_interrupt();                               // sätter interupt global
  T2CONSET = 0x8000;                                // aktiverar timer 2

  *trise = 0x00;
  //*porte = 0xff;

  TRISD |= 0x7f;
  TRISF |= 0x2;

  LoadScene(0, 0);
}

void game_update(void) //will run every time the timer ticks
{
  int buttons = getbtns();
  int btn1 = getbtn1();

  if(btn1 & 0x2) //button 4
  {
    //jump
    LoadScene(0,1);
  }

  if (buttons & 0x1) //button 3 is pressed
  {
    //attack
  }

  if (buttons & 0x2) //button 2 is pressed
  {
    player->xPosition += 0.002; 
  }

  if (buttons & 0x4) //button 1 is pressed
  {
    player->xPosition -= 0.002;   
  }
}

void draw_update(void) //will run every 100th time timer ticks
{
  clearScreen();

  int i;
  for (i = 0; i < gameObjectsLength; i++)
  {
    if(i != 0 && gameObjects[i].graphicIndex == 0) break;
    
    if(gameObjects[i].graphicIndex != 1)
      drawGameObject(&gameObjects[i], 0);
  }

  if(displayUpdateCounter == 2000000)
  {
    displayUpdateCounter = 0; 
  }
  display_image(0, screen_data); //draw image to screen
}

void master_update(void)
{
  if(!IFS(0)) //only update on timer
    return;

  displayUpdateCounter++;
  ticks++;
  game_update();
  IFSCLR(0) = 0x100;

  if ((displayUpdateCounter % 100) == 0)
  {
    draw_update();
    apply_physics();
    
    if((displayUpdateCounter % 20000) == 0)
    {
      find_collisions();
      handle_collisions();
    }
  }
}

void apply_physics()
{
  int index = 0;
  for(index = 0; index < gameObjectsLength; index++)
  {
    if(gameObjects[index].usePhysics == 1)
    {
      gameObjects[index].yVelocity -= 0.0001; //apply gravity

      gameObjects[index].xPosition += gameObjects[index].xVelocity; //move object with x velocity
      gameObjects[index].yPosition += gameObjects[index].yVelocity; //move object with y velocity
    }
  }
}

void find_collisions()
{
  collisionsLength = 0;
  
  int i = 0;
  for(i = 0; i < gameObjectsLength; i++)
  {
    if(gameObjects[i].usePhysics == 1)
    {    
      int j = 0;
      for(j = 0; j < gameObjectsLength; j++)
      {
        collisionsLength += get_collision(&gameObjects[i], &gameObjects[j], &collisions[collisionsLength]);
      }
    }
  }
}

int get_collision(GameObject *object1, GameObject *object2, Collision *collision)
{
  double thisLeft = object1->xPosition;
  double thisRight = object1->xPosition + getGameObjectWidth(object1);
  double thisTop = object1->yPosition + getGameObjectHeight(object1);
  double thisBottom = object1->yPosition;

  double otherLeft = object2->xPosition;
  double otherRight = object2->xPosition + getGameObjectWidth(object2);
  double otherTop = object2->yPosition + getGameObjectHeight(object2);
  double otherBottom = object2->yPosition;
  
  //*porte += 1;
  *porte |= object1->graphicIndex + object2->graphicIndex;

  if (otherLeft >= thisLeft && otherLeft <= thisRight && otherRight >= thisRight)
  {
      double xOverlap = thisRight - otherLeft;
      if (otherBottom >= thisBottom && otherBottom <= thisTop)
      {
          double yOverlap = thisTop - otherBottom;
          collision->xPosition = otherLeft;
          collision->yPosition = otherBottom;
          collision->width = xOverlap;
          collision->height = yOverlap;
          collision->objectOne = object1;
          collision->objectTwo = object2;
          return 1;
      }
      else if (otherTop >= thisBottom && otherTop <= thisTop)
      {
          double yOverlap = otherTop - thisBottom;
          collision->xPosition = otherLeft;
          collision->yPosition = thisBottom;
          collision->width = xOverlap;
          collision->height = yOverlap;
          collision->objectOne = object1;
          collision->objectTwo = object2;
          return 1;
      }
  }
  else if (otherRight >= thisLeft && otherRight <= thisRight)
  {
      double xOverlap = otherRight - thisLeft;
      if (otherTop >= thisBottom && otherTop <= thisTop)
      {
          double yOverlap = otherTop - thisBottom;
          collision->xPosition = thisLeft;
          collision->yPosition = thisBottom;
          collision->width = xOverlap;
          collision->height = yOverlap;
          collision->objectOne = object1;
          collision->objectTwo = object2;
          return 1;
      }
      else if (otherBottom >= thisBottom && otherBottom <= thisTop)
      {
          double yOverlap = thisTop - otherBottom;
          collision->xPosition = thisLeft;
          collision->yPosition = otherBottom;
          collision->width = xOverlap;
          collision->height = yOverlap;
          collision->objectOne = object1;
          collision->objectTwo = object2;
          return 1;
      }
  }
  else if (thisLeft >= otherLeft && thisRight <= otherRight)
  {
      if (thisTop <= otherTop)
      {
          if(thisBottom >= otherBottom) 
          {
            collision->xPosition = thisLeft;
            collision->yPosition = otherBottom;
            collision->width = 3;
            collision->height = 2;
            collision->objectOne = object1;
            collision->objectTwo = object2;
            return 1;
          }
      }
  }
  else if (otherLeft >= thisLeft  && otherRight <= thisRight)
  {
      if (otherTop <= thisTop)
      {
          if (otherBottom >= thisBottom)
          {
            collision->xPosition = thisLeft;
            collision->yPosition = otherBottom;
            collision->width = 3;
            collision->height = 2;
            collision->objectOne = object1;
            collision->objectTwo = object2;
            return 1;
          }
      }
  }
  
  return 0;
}

void handle_collisions()
{
  *porte = collisionsLength;

  int i = 0;
  for (i = 0; i < collisionsLength; i++)
  {
    collisions[i].objectOne->yVelocity = 0;
    collisions[i].objectTwo->yVelocity = 0;
    collisions[i].objectOne->yPosition = collisions[i].height + 1;
  }
}