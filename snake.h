#ifndef SNAKE_H__
#define SNAKE_H__

#include "pico/multicore.h"
#include "pico/stdlib.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

#define clear() printf("\033[H\033[J")
#define gotoxy(x, y) printf("\033[%d;%dH", (y), (x))

#define BUTTON 16
#define BUTTON2 19
#define BUTTON3 17
#define BUTTON4 20
#define BUTTON5 21
#define BUTTON6 22

#define LED 13 // red led
#define LED2 9 // green led

#define UP 'w'
#define DOWN 's'
#define LEFT 'a'
#define RIGHT 'd'

#define BORDER "o"
#define SNAKE "*"
#define FOOD "@"
#define WALL "|"

typedef struct {
  uint height;
  uint width;
  uint speed_in_ms;
  uint score;
  uint start;
} game;

typedef struct {
  uint x;
  uint y;
} wall;

typedef struct {
  uint x;
  uint y;
} food;

typedef struct {
  uint horz[100];
  uint vert[100];
  uint length;
  uint head_x;
  uint head_y;

} snake;

void blink_seq();
void button_callback(uint gpio, uint32_t events);
void game_over(game *game, snake *snake, food *food, wall *wall);
void food_gen(food *food, game *game);
void game_init(game *game, snake *snake, food *food, wall *wall);
void bend(snake *snake);
void move_snake(snake *snake);
void pixels_draw(game *game, snake *snake, food *food, wall *wall);
void game_logic(snake *snake, food *food, game *game, wall *wall);
void game_main(game *game, snake *snake, food *food, wall *wall);
void game_over(game *game, snake *snake, food *food, wall *wall);
void init_button(int *arr, size_t len);
void init_led(int *arr, size_t len);

#endif