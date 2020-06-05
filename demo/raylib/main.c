#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <math.h>
#include <limits.h>
#include <time.h>

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_IMPLEMENTATION
#define NK_RAYLIB_IMPLEMENTATION
#include "../../nuklear.h"
#include "nuklear_raylib.h"

const int screenWidth = 1280;
const int screenHeight = 768;
enum
{
  EASY,
  HARD
};

int main(int argc, char *argv[])
{
  SetConfigFlags(FLAG_MSAA_4X_HINT);
  InitWindow(screenWidth, screenHeight, "Nuklear + Raylib");
  SetWindowMinSize(screenWidth, screenHeight);
  SetTargetFPS(60);

  struct nk_context *ctx = nk_raylib_init("default");

  while (!WindowShouldClose())
  {
    // --------------------
    // NUKLEAR INPUT
    // --------------------
    nk_raylib_handle_input();

    static int op = EASY;
    static float value = 0.6f;
    static int i = 20;

    static char message[64] = "Esto es Nuklear!!!";
    static int message_len;

    if (nk_begin(ctx, "Show", nk_rect(50, 50, 220, 220),
                 NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_CLOSABLE))
    {
      /* fixed widget pixel width */
      nk_layout_row_static(ctx, 30, 80, 1);
      if (nk_button_label(ctx, "button"))
      {
        /* event handling */
      }

      /* fixed widget window ratio width */
      nk_layout_row_dynamic(ctx, 30, 2);
      if (nk_option_label(ctx, "easy", op == EASY))
        op = EASY;
      if (nk_option_label(ctx, "hard", op == HARD))
        op = HARD;

      /* custom widget pixel width */
      nk_layout_row_begin(ctx, NK_STATIC, 30, 2);
      {
        nk_layout_row_push(ctx, 50);
        nk_label(ctx, "Volume:", NK_TEXT_LEFT);
        nk_layout_row_push(ctx, 110);
        nk_slider_float(ctx, 0, &value, 1.0f, 0.1f);
      }
      nk_layout_row_end(ctx);

      nk_layout_row_dynamic(ctx, 30, 2);
      nk_label(ctx, "Message:", NK_TEXT_LEFT);
      nk_edit_string_zero_terminated(ctx, NK_EDIT_DEFAULT, &message, sizeof(message), nk_filter_default);
    }
    nk_end(ctx);
    
    BeginDrawing();
    {
      ClearBackground(RAYWHITE);

      DrawText(message, 400, 300, 30, BLACK);

      // --------------------
      // NUKLEAR RENDER
      // --------------------
      nk_raylib_render();
    }
    EndDrawing();
  }

  CloseWindow();

  return 0;
}
