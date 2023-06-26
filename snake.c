#include "snake.h"
#include "pico/multicore.h"
#include "pico/stdlib.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

volatile char direction, cont, pause, game_end, restart;
volatile uint high_score;

void GotoXY(int x, int y) {
  fflush(stdout);
  gotoxy(x, y);
}

void blink_seq() {

  while (true) {

    if (multicore_fifo_rvalid()) {
      int status = multicore_fifo_pop_blocking();
      if (status == 1) {
        for (int idx = 0; idx < 4; idx++) {
          gpio_put(LED2, 1);
          sleep_ms(125);
          gpio_put(LED2, 0);
          sleep_ms(125);
        }
      }
    }
  }
}

void button_callback(uint gpio, uint32_t events) {

  if (gpio == BUTTON) { // up
    direction = UP;
  }

  else if (gpio == BUTTON2) { // down
    direction = DOWN;
  }

  else if (gpio == BUTTON3) {
    direction = LEFT; // left
  }

  else if (gpio == BUTTON4) { // right
    if (!pause) {
      direction = RIGHT;
    } else {
      pause = false;
    }
  }

  else if (gpio ==
           BUTTON5) { // button 5 used for either continuing the game after it
                      // ends or pause the game while it is running
    if (game_end) {
      cont = true;
    } else {
      pause = true;
    }
  }

  else if (gpio == BUTTON6) { // button 6 used for restarting the game
    restart = true;
  }
}

void food_gen(food *food, game *game) { // spawn food
  food->x = rand() % (game->width - 4); // should be within the boundary
  food->y = rand() % (game->height - 4);

  if (food->x <= 0)
    food->x = 1;

  if (food->y <= 0)
    food->y = 1;

  gpio_put(LED2, 0);

  printf("%d %d", food->x, food->y);
}

void game_init(game *game, snake *snake, food *food,
               wall *wall) { // initialize all game variables

  restart = false;
  game->start = 8 + rand() % 8;
  game->height = 40;
  game->width = 80;
  game->speed_in_ms = 250;
  game->score = 0;

  cont = false;
  pause = false;
  game_end = false;
  direction = ' ';

  snake->length = 1;
  snake->head_x = game->width / 2; // generate snake
  snake->head_y = game->height / 2;

  food_gen(food, game); // spawn food

  gpio_put(LED2, 0);
  gpio_put(LED, 0);

  wall->x = rand() % game->width - 1;
  wall->y = game->start + rand() % game->height / 2;
}

void bend(snake *snake) {

  int prev_posx = snake->horz[0];
  int prev_posy = snake->vert[0];
  int temp2x, temp2y;
  snake->horz[0] = snake->head_x;
  snake->vert[0] = snake->head_y;

  for (int i = 1; i < snake->length; i++) {
    temp2x = snake->horz[i];
    temp2y = snake->vert[i];
    snake->horz[i] = prev_posx;
    snake->vert[i] = prev_posy;
    prev_posx = temp2x;
    prev_posy = temp2y;
  }
}

void move_snake(snake *snake) { // moving snake
  bend(snake);                  // run the bend algorithm

  switch (direction) {
  case UP:
    snake->head_y--;
    break;

  case DOWN:
    snake->head_y++;
    break;

  case LEFT:
    snake->head_x--;
    break;

  case RIGHT:
    snake->head_x++;
    break;

  default:
    break;
  }
}

void pixels_draw(game *game, snake *snake, food *food,
                 wall *wall) { // draw the snake
  clear();
  for (int i = 0; i < game->height; i++) {
    for (int j = 0; j < game->width; j++) {
      if (i == 0 || i == game->height - 1 || j == 0 || j == game->width - 1) {
        printf(BORDER);
      } else {
        if (i == snake->head_y && j == snake->head_x) {
          printf(SNAKE);
        } else if (i == food->y && j == food->x) {
          printf(FOOD);
        } else {
          int ind = 0;
          for (int k = 0; k < snake->length; k++) {
            if (i == snake->vert[k] && j == snake->horz[k]) {
              printf(SNAKE);
              ind = 1;
            }
          }
          if (ind == 0) {
            printf(" ");
          }
        }
      }
    }
    printf("\n");
  }
  printf("\nYour Score: %d", game->score);
  // wall generator
  for (int i = game->start; i < wall->y; i++) {
    gotoxy(wall->x, i); // six pixel wide wall
    printf(WALL);
  }
}

void game_logic(snake *snake, food *food, game *game, wall *wall) { // algorithm
  if (snake->head_x == food->x && snake->head_y == food->y) {
    game->score += 10;
    food_gen(food, game); // generate food
    int status = 1;
    multicore_fifo_push_blocking(status);
    snake->length += 1;      // increase snake size
    game->speed_in_ms -= 25; // increase speed

    if (game->score > high_score) {
      high_score = game->score;
    }
  }

  // if snake goes out of boundary then end the game
  else if (snake->head_x >= game->width - 1 ||
           snake->head_y >= game->height - 1 || snake->head_x <= 0 ||
           snake->head_y <= 0 ||
           (snake->head_x == wall->x - 1 && snake->head_y <= wall->y - 1 &&
            snake->head_y >= game->start)) {
    game_end = true;
  }

  // if snake touches his own body then end the game
  for (int i = 1; i < snake->length; i++) {
    if (snake->head_x == snake->horz[i] && snake->head_y == snake->vert[i]) {
      game_end = true;
    }
  }
}

// infinite loop for the game
void game_main(game *game, snake *snake, food *food, wall *wall) {

  while (!game_end && !pause) {
    cont = false;
    pixels_draw(game, snake, food, wall);
    game_logic(snake, food, game, wall);
    move_snake(snake);
    if (restart) {
      game_init(game, snake, food, wall);
    }
    if (game->speed_in_ms >= 0) {
      sleep_ms(game->speed_in_ms);
    }
  }

  if (game_end) {
    game_over(game, snake, food, wall);
  }
}

void game_over(game *game, snake *snake, food *food, wall *wall) {
  clear();
  printf("---GAME OVER---\n");
  printf("your score: %d\n", game->score);
  gpio_put(LED, 1);

  if (high_score == game->score) {
    printf("new high score set!\n");
  }
  printf("Press pusbutton5 to continue..\n");
  if (cont) {
    gpio_put(LED, 0);
    game_init(game, snake, food, wall);
    game_main(game, snake, food, wall);
  }
}

void init_button(int *arr, size_t len) {
  for (int i = 0; i < len; i++) {
    gpio_init(arr[i]);
    gpio_set_dir(arr[i], false);
    gpio_pull_down(arr[i]);

    if (i == 0) {
      gpio_set_irq_enabled_with_callback(
          arr[i], 0x04, 1, button_callback); // attach interrupt to button
    } else {
      gpio_set_irq_enabled(arr[i], 0x04,
                           true); // attach intr to button 6
    }
  }
}

void init_led(int *arr, size_t len) {
  for (int i = 0; i < len; i++) {
    gpio_init(arr[i]);
    gpio_set_dir(arr[i], true);
  }
}
