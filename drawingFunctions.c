#include <stdint.h>
#include "GameObject.h"

extern uint8_t screen_data[512];
extern uint8_t screen_background[512];
extern uint8_t imageSizes[][2];
extern char* images[2];

uint8_t *canvases[] = { screen_data, screen_background };

void setPixel(int x, int y, char value, int canvasIndex)
{
  if(x < 0 || x > 127 || y < 0 || y > 31)
    return;

  y = 32 - y;
	int blockPosition = x + ((y / 8) * 128);
	char block = canvases[canvasIndex][blockPosition];

	block &= ~(1 << (y % 8));

  if(value > 0)
  {
    block |= 1 << (y % 8);
  }

	canvases[canvasIndex][blockPosition] = block;
}

void clearScreen()
{
  int i;
  for (i = 0; i < 512; i++)
  {
    screen_data[i] = screen_background[i];
  }
}

void drawGraphic(int x, int y, int width, int height, char* graphic, char mirrored, int canvasIndex)
{
  int i;
  int j;
  for (i = 0; i < width; i++)
  {
    int drawX;
    
    if(mirrored == 0)
        drawX = x + i;
      else
        drawX = (x + width - i);
        
    for (j = 0; j < height; j++)
    {
      setPixel(drawX, (y + height) - j, graphic[i + j * (width)], canvasIndex);
    }  
  } 
}

void drawGameObject(GameObject *gameObject, int canvasIndex)
{
  drawGraphic((int)gameObject->xPosition, (int)gameObject->yPosition, imageSizes[gameObject->graphicIndex][0], imageSizes[gameObject->graphicIndex][1], images[gameObject->graphicIndex], gameObject->is_mirrored, canvasIndex);  
}

int getGameObjectWidth(GameObject *gameObject)
{
  return imageSizes[gameObject->graphicIndex][0];
}

int getGameObjectHeight(GameObject *gameObject)
{
  return imageSizes[gameObject->graphicIndex][1];
}