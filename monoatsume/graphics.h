/*
 * graphics.h
 *	NXT画面に図形等を描画するルーチン群の宣言
 */

#ifndef __GRAPHICS_H__
#define __GRAPHICS_H__

#include "mytypes.h"

#define NXT_DISPLAY_WIDTH (NXT_LCD_WIDTH)
#define NXT_DISPLAY_HEIGHT ((NXT_LCD_DEPTH) * 8)

typedef enum {
  nxt_display_none,
  nxt_display_clear,
  nxt_display_set,
  nxt_display_xor,
  nxt_display_or,
  nxt_display_get
} nxt_display_op;

// (x, y)に点を打つ
extern U8 nxt_display_pixel(nxt_display_op, int, int);
// (x, y) --- (x, y)で線を引く
extern U8 nxt_display_line(nxt_display_op, int, int, int, int);
// (x, y) 「」 (x, y)に矩形を描く
extern U8 nxt_display_rectangle(nxt_display_op, int, int, int, int);
// (x, y) 「」 (x, y)に塗りつぶした矩形を描く
extern U8 nxt_display_tile(nxt_display_op, int, int, int, int);
// (x, y) 横a縦bで楕円を描く
extern U8 nxt_display_ellipse(nxt_display_op, int, int, int, int);
// (x, y) 横a縦bで塗りつぶした楕円を描く
extern U8 nxt_display_ellipsoid(nxt_display_op, int, int, int, int);
// 左上を(x, y)として横m倍、縦n倍で文字を描く
extern U8 nxt_display_char_mn(nxt_display_op, int, int, int, int, int);
// 左上を(x, y)として横m倍、縦n倍で文字列を描く
extern U8 nxt_display_string_mn(nxt_display_op, int, int, char *, int, int);

// (x, y): r で円を描く
#define nxt_display_circle(op, x, y, r) nxt_display_ellipse((op), (x), (y), (r), (r))
// (x, y): r で塗りつぶした円を描く
#define nxt_display_dish(op, x, y, r) nxt_display_ellipsoid((op), (x), (y), (r), (r))
// 左上を(x, y)として文字を描く
#define nxt_display_char(op, x, y, c) nxt_display_char_mn((op), (x), (y), (c), 1, 1)
// 左上を(x, y)としてテキストを描く
#define nxt_display_string(op, x, y, s) nxt_display_string_mn((op), (x), (y), (s), 1, 1)
// 左上を(x, y)として大きな文字を描く
#define nxt_display_CHAR(op, x, y, c) nxt_display_char_mn((op), (x), (y), (c), 2, 2)
// 左上を(x, y)として大きなテキストを描く
#define nxt_display_STRING(op, x, y, s) nxt_display_string_mn((op), (x), (y), (s), 2, 2)

#endif
