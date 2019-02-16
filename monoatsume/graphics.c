/*
 * graphics.c
 *	NXT画面に図形等を描画するルーチン群
 *	display.cに定義された一部のstatic値を参照するため
 *	display.cにも修正が必要
 */

#include "nxt_lcd.h"
#include "display.h"
#include "graphics.h"

#define N_CHARS 128
#define FONT_WIDTH 5
#define FONT_HEIGHT 7

extern U8(*display_buffer)[NXT_LCD_WIDTH];
extern const U8 font[N_CHARS][FONT_WIDTH];

static const U8 bit_pattern[8] = {
	1 << 0,
	1 << 1,
	1 << 2,
	1 << 3,
	1 << 4,
	1 << 5,
	1 << 6,
	1 << 7
};

U8
nxt_display_pixel(nxt_display_op op, int x, int y)
{
  if (x < 0 || x >= NXT_DISPLAY_WIDTH || y < 0 || y >= NXT_DISPLAY_HEIGHT)
    return 0;
  switch (op) {
  case nxt_display_set:
  case nxt_display_or:
    display_buffer[y / 8][x] |= bit_pattern[y % 8];
    break;
  case nxt_display_clear:
    display_buffer[y / 8][x] &= ~(bit_pattern[y % 8]);
    break;
  case nxt_display_xor:
    display_buffer[y / 8][x] ^= bit_pattern[y % 8];
    break;
  case nxt_display_get:
    return display_buffer[y / 8][x] & bit_pattern[y % 8];
  case nxt_display_none:
    return 0;
  }
  return 1;
}

U8
nxt_display_horizontal(nxt_display_op op, int x1, int y, int x2)
{
  int t;

  if (op == nxt_display_get || y < 0 || y >= NXT_DISPLAY_HEIGHT) return 0;
  if (x1 > x2) {
    t = x1;
    x1 = x2;
    x2 = t;
  }
  if (x1 < 0) x1 = 0;
  if (x2 >= NXT_LCD_WIDTH) x2 = NXT_LCD_WIDTH - 1;
  for (t = x1; t <= x2; t++) {
    nxt_display_pixel(op, t, y);
  }
  return 1;
}

U8
nxt_display_vertical(nxt_display_op op, int x, int y1, int y2)
{
  int t;

  if (op == nxt_display_get || x < 0 || x >= NXT_DISPLAY_WIDTH) return 0;
  if (y1 > y2) {
    t = y1;
    y1 = y2;
    y2 = t;
  }
  if (y1 < 0) y1 = 0;
  if (y2 >= NXT_DISPLAY_HEIGHT) y2 = NXT_DISPLAY_HEIGHT - 1;
  for (t = y1; t <= y2; t++) {
    nxt_display_pixel(op, x, t);
  }
  return 1;
}

U8
nxt_display_rectangle(nxt_display_op op, int x1, int y1, int x2, int y2)
{
  if (op == nxt_display_get) return 0;
  nxt_display_horizontal(op, x1, y1, x2);
  nxt_display_horizontal(op, x1, y2, x2);
  nxt_display_vertical(op, x1, y1, y2);
  nxt_display_vertical(op, x2, y1, y2);
  return 1;
}

U8
nxt_display_tile(nxt_display_op op, int x1, int y1, int x2, int y2)
{
  int t;

  if (x1 > x2) {
    t = x1;
    x1 = x2;
    x2 = t;
  }
  for (t = x1; t <= x2; t++) {
    nxt_display_vertical(op, t, y1, y2);
  }
  return 1;
}

U8
nxt_display_line(nxt_display_op op, int x1, int y1, int x2, int y2)
{
  int x0, y0, t;

  if (x1 > x2) {
    x0 = x1 - x2;
  } else {
    x0 = x2 - x1;
  }
  if (y1 > y2) {
    y0 = y1 - y2;
  } else {
    y0 = y2 - y1;
  }
  if (x0 == 0) {
    if (y0 == 0) {
      return nxt_display_pixel(op, x1, y1);
    } else {
      return nxt_display_vertical(op, x1, y1, y2);
    }
  } else if (y0 == 0) {
    return nxt_display_horizontal(op, x1, y1, x2);
  } else if (x0 > y0) {
    if (x1 > x2) {
      t = x1;
      x1 = x2;
      x2 = t;
      t = y1;
      y1 = y2;
      y2 = t;
    }
    y0 = y2 - y1;
    for (t = 0; t <= x0; t++) {
      nxt_display_pixel(op, x1 + t, y1 + (t * y0 * 2 + x0) / x0 / 2);
    }
  } else {
    if (y1 > y2) {
      t = x1;
      x1 = x2;
      x2 = t;
      t = y1;
      y1 = y2;
      y2 = t;
    }
    x0 = x2 - x1;
    for (t = 0; t <= y0; t++) {
      nxt_display_pixel(op, x1 + (t * x0 * 2 + y0) / y0 / 2, y1 + t);
    }
  }
  return 1;
}

U8
nxt_display_ellipse(nxt_display_op op, int x, int y, int a, int b)
{
  int xx = a * 64;
  int yy = 0;
  int dx = 0;
  int dy = 0;

  while (xx >= 0) {
    dx = xx / 64;
    dy = yy / 64;
    nxt_display_pixel(op, x + dx, y + dy);
    nxt_display_pixel(op, x - dx, y - dy);
    nxt_display_pixel(op, x + dx, y - dy);
    nxt_display_pixel(op, x - dx, y + dy);
    yy += xx * b / a / 64;
    xx -= yy * a / b / 64;
  }
  return 1;
}

U8
nxt_display_ellipsoid(nxt_display_op op, int x, int y, int a, int b)
{
  int xx = a * 64;
  int yy = 0;
  int dx = 0;
  int dy = 0;

  while (xx >= 0) {
    dx = xx / 64;
    dy = yy / 64;
    nxt_display_line(op, x + dx, y - dy, x + dx, y + dy);
    nxt_display_line(op, x - dx, y - dy, x - dx, y + dy);
    yy += xx * b / a / 64;
    xx -= yy * a / b / 64;
  }
  return 1;
}

U8
nxt_display_char_mn(nxt_display_op op, int x, int y, int c, int m, int n)
{
  int i, j;
  int k, l;
  int o0, o1;

  if (c < 0 || c >= N_CHARS) return 0;
  switch (op) {
  case nxt_display_clear:
    o1 = op;
    o0 = nxt_display_set;
    break;
  case nxt_display_set:
    o1 = op;
    o0 = nxt_display_clear;
    break;
  case nxt_display_or:
    o1 = nxt_display_set;
    o0 = nxt_display_none;
    break;
  case nxt_display_xor:
    o1 = op;
    o0 = nxt_display_none;
    break;
  default:
    o0 = o1 = op;
  }
  nxt_display_tile(o0, x, y, x + m / 2, y + (FONT_HEIGHT + 1) * n - 1);
  nxt_display_tile(o0, x + m / 2, y, x + (FONT_WIDTH + 1) * m - 1, y + n / 2);
  x += m / 2;
  y += n / 2;
  for (i = 0; i < FONT_WIDTH; i++) {
    for (j = 0; j < FONT_HEIGHT; j++) {
      for (k = 0; k < m; k++) {
	for (l = 0; l < n; l++) {
          if (font[c][i] & (1 << j)){
            nxt_display_pixel(o1, x + i * m + k, y + j * n + l);
	  } else {
            nxt_display_pixel(o0, x + i * m + k, y + j * n + l);
	  }
	}
      }
    }
  }
  nxt_display_tile(o0, x, y + FONT_HEIGHT * n,
	x + FONT_WIDTH * m - 1, y + (FONT_HEIGHT + 1) * n - n / 2 - 1);
  nxt_display_tile(o0, x + FONT_WIDTH * m, y,
	x + (FONT_WIDTH + 1) * m - m / 2 - 1,
	y + (FONT_HEIGHT + 1) * n - n / 2 - 1);
  return 1;
}

U8
nxt_display_string_mn(nxt_display_op op, int x, int y, char *s, int m, int n)
{
  while (*s) {
    nxt_display_char_mn(op, x, y, *s++, m, n);
    x += (FONT_WIDTH + 1) * m;
  }
  return 1;
}

#ifdef DEBUG
extern void go_next(void);

void
graphics_test(void)
{
  display_clear(0);
  nxt_display_line(nxt_display_set, NXT_DISPLAY_WIDTH, NXT_DISPLAY_HEIGHT, 0, 0);
  go_next();
  nxt_display_STRING(nxt_display_set, 2 * 6, 2 * 8, "ABCDEFG");
  go_next();
  nxt_display_rectangle(nxt_display_set, 30, 30, 50, 60);
  go_next();
  nxt_display_tile(nxt_display_set, 90, 30, 70, 20);
  go_next();
  nxt_display_tile(nxt_display_xor, 40, 25, 80, 50);
  go_next();
  nxt_display_string(nxt_display_set, 6 * 6, 5 * 8, "ABCDEFG");
  go_next();
  nxt_display_string_mn(nxt_display_clear, 1 * 6, 4 * 8, "DEFG", 3, 3);
  go_next();
  nxt_display_string_mn(nxt_display_xor, 1 * 6, 4 * 8, "DEFG", 3, 3);
  go_next();
  nxt_display_tile(nxt_display_clear, 50, 35, 60, 40);
  go_next();
  nxt_display_ellipse(nxt_display_set, 50, 32, 50, 32);
  go_next();
  nxt_display_ellipsoid(nxt_display_set, 50, 32, 25, 20);
  go_next();
  nxt_display_ellipsoid(nxt_display_clear, 50, 32, 15, 10);
  go_next();
  nxt_display_tile(nxt_display_xor, 0, 0, NXT_DISPLAY_WIDTH, NXT_DISPLAY_HEIGHT);
  go_next();
  nxt_display_tile(nxt_display_xor, 0, 0, NXT_DISPLAY_WIDTH, NXT_DISPLAY_HEIGHT);
  display_update();
}
#endif
