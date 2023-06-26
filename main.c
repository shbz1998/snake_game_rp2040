#include "snake.h"
#include <stdio.h>

int main(void) {

  game Game;
  food Food;
  wall Wall;
  snake Snake;

  stdio_init_all();

  sleep_ms(50);

  int button[] = {BUTTON, BUTTON2, BUTTON3, BUTTON4, BUTTON5, BUTTON6};
  int led[] = {LED, LED2};

  size_t nelms_button = sizeof(button) / sizeof(button[0]);
  size_t nelms_led = sizeof(led) / sizeof(led[0]);

  init_button(button, nelms_button);
  init_led(led, nelms_led);

  sleep_ms(50);

  multicore_launch_core1(blink_seq);

  clear();

  game_init(&Game, &Snake, &Food, &Wall);

  while (true) {
    tight_loop_contents();

    sleep_ms(500);

    game_main(&Game, &Snake, &Food, &Wall);
  }

  return 0;
}