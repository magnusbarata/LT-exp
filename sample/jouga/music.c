/*
 *	NXTで音楽を扱うためのコード
 *		次のようにコンパイルすることで音楽を単独でテスト可能
 *			gcc -DMUSIC_DEBUG music.c -lwinmm
 */

#include "music.h"
#ifndef MUSIC_DEBUG
#include <t_services.h>
#include "ecrobot_interface.h"
#define play_tone(f, t, l, v, f1, f2)	ecrobot_sound_tone(f, l, v)
#else
#include <windows.h>
#include <unistd.h>

#define dly_tsk(t)	usleep((t) * 1000)
#define MIDIMSG(s,c,d1,d2)	(((s)<<4) | (c) | ((d1)<<8) | ((d2)<<16))
#define play_tone(f, t, l, v, f1, f2)	do\
  {\
    midi_play(t, l, v, f1, f2);\
    tie = f1;\
    len2 -= l;\
  } while (0)

HMIDIOUT hMidiOut;

void
midi_play(int t, int l, int v, int tie, int cont)
{
  if (!cont) midiOutShortMsg(hMidiOut, MIDIMSG(0x9, 0x0, t, v));
  usleep(l * 1000);
  if (!tie) midiOutShortMsg(hMidiOut, MIDIMSG(0x9, 0x0, t, 0));
}
#endif

/* 音名から番号を得る */
int C2tone[] = {
  10, 12, 1, 3, 5, 6, 8
// A,  B, C, D, E, F, G
};

/* 番号から周波数を得る */
int tone2freq[] = {
  247, 262, 277, 294, 311, 330, 349, 370, 392, 415, 440, 466, 494, 523
//  B,   C,  C+,   D,  D+,   E,   F,  F+,   G,  G+,   A,  A+,   B,   C
};
// BとCが2度出てくるのは、♯や♭の処理を簡単にするため

/*
 * play_notes: 文字列として渡された音符を演奏する
 *	len: 全音符の長さ(ms単位)
 *	vol: ボリューム
 *	notes: 音符文字列
 *		[A-G][+-]?[^v]*num.*(/num)?[:=]?
 *			A-G: 音名
 *			+-: ♯、♭
 *			^v: オクターブ上げ、下げ
 *			num: 音符
 *			.: 付点
 *			/num: 連符
 *			:=: スタッカート、タイ
 *		Rnum.*
 *			R: 休符を示す
 *			num: 休符
 *			.: 付点
 *			/num: 連符
 *		pp|p|mp|mf|f|ff
 *			ボリュームの調整
 */

int
play_notes(int len, int vol, char *notes)
{
  int count = 0;
  int len2, len3;
  int t, f;
  int i;
  int v = 8;
  int tie = 0;

  // 音符が残っている間は続ける
  while (notes && *notes) {
    if (*notes == 'm') {
      if (*++notes == 'p') {
	v = 7;		// mp
      } else if (*notes == 'f') {
	v = 8;		// mf
      }
      notes++;
    } else if (*notes == 'f') {
      if (*++notes == 'f') {
	notes++;
	v = 10;		// ff
      } else {
	v = 9;		// f
      }
    } else if (*notes == 'p') {
      if (*++notes == 'p') {
	notes++;
	v = 5;		// pp
      } else {
	v = 6;		// p
      }
    } else if (*notes == 'R') {	// 休符の処理
      notes++;
      // 長さの読み込み
      i = 0;
      while (*notes >= '0' && *notes <= '9') {
	i *= 10;
	i += *notes++ - '0';
      }
      if (i == 0) i = 1;
      len3 = len2 = len / i;
      // 付点の処理
      while (*notes == '.') {
	notes++;
	len3 /= 2;
	len2 += len3;
      }
      // 連符の処理
      if (*notes == '/') {
	notes++;
	i = 0;
	while (*notes >= '0' && *notes <= '9') {
	  i *= 10;
	  i += *notes++ - '0';
	}
	len2 /= i;
      }
      tie = 0;
      // 指定した時間だけ休む
      dly_tsk(len2);
    } else if (*notes >= 'A' && *notes <= 'G') {	// 音符の処理
      // 音名からtone番号を得る
      t = C2tone[*notes++ - 'A'];
      // ♯、♭の処理
      switch(*notes) {
      case '+':
	t++;
	notes++;
	break;
      case '-':
	--t;
	notes++;
	break;
      }
      f = tone2freq[t];
      // オクターブ変換
      while (*notes == '^') {
	notes++;
	f *= 2;
	t += 12;
      }
      while (*notes == 'v') {
	notes++;
	f /= 2;
	t -= 12;
      }
      // 長さの読み込み
      i = 0;
      while (*notes >= '0' && *notes <= '9') {
	i *= 10;
	i += *notes++ - '0';
      }
      len3 = len2 = len / i;
      // 付点の処理
      while (*notes == '.') {
	notes++;
	len3 /= 2;
	len2 += len3;
      }
      // 連符の処理
      if (*notes == '/') {
	notes++;
	i = 0;
	while (*notes >= '0' && *notes <= '9') {
	  i *= 10;
	  i += *notes++ - '0';
	}
	len2 /= i;
      }
      count++;
      t += 0x3B;
      if (*notes == '=') {	// タイなら時間いっぱい演奏
	notes++;
	play_tone(f, t, len2, vol * v, t, tie == t);
      } else if (*notes == ':') {	// スタッカート
	notes++;
	play_tone(f, t, len2 / 3, vol * v, 0, tie == t);
      } else {	// 普通の音符は8割まで
	play_tone(f, t, len2 * 4 / 5, vol * v, 0, tie == t);
      }
      // 音符の本来の時間分待つ
      dly_tsk(len2);
    } else  {
      // Error
      break;
    }
  }
  // 演奏した音符数を返す
  return count;
}

// 千葉大学歌
char *chiba_univ =
	"D4B4.A8G4G4E8.A16G8E8D4R16E8G4.A8B4B4"
	"C^8.C^16B8A8G4F+8G8A2.R4B8.C^16D^8E^8D^8D^8B4"
	"A8.A16A8A8G8B8A8R8B8.B16B8A8G4F+8G8A8.A16A8A8B4C+^4"
	"D^2=D^8R8E^4D^2=D^8B8B8.C^16B2G2"
	"E8.E16D8G8A4B4G2=G8R8G4G4.C^8C^4E^4"
	"D^2=D^8R8D^8.E^16D^4B4C^8B4A8G2=G8R8"
;

#ifdef MUSIC_DEBUG
int
main(void)
{
  midiOutOpen(&hMidiOut, MIDIMAPPER, 0, 0, CALLBACK_NULL);
  midiOutShortMsg(hMidiOut, MIDIMSG(0xC, 0, 22, 0));
  play_notes(TIMING_chiba_univ, 10, chiba_univ);
  midiOutReset(hMidiOut);
  midiOutClose(hMidiOut);
  return 0;
}
#endif
