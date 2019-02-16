typedef enum
{
  BLK = 1 << 0, // 黒
  BLU = 1 << 1, // 青
  GRN = 1 << 2, // 緑
  CYA = 1 << 3, // シアン
  RED = 1 << 4, // 赤
  MAG = 1 << 5, // マゼンタ
  YEL = 1 << 6, // 黄
  WHT = 1 << 7, // 白
  RTP = 1 << 8, // 右押す
  RTR = 1 << 9, // 右離す
  LTP = 1 << 10, // 左押す
  LTR = 1 << 11, // 左離す
  DIS = 1 << 12, // 移動距離
  POS = 1 << 13, // アーム位置
  // 以下ライトセンサー?
} EBits;

static int COL_THRES[] = {400, 350, 320};
S16 col[3];
U8 CBits = 0;

ecrobot_get_nxtcolorsensor_rgb(Color, col);
CBits = bin(col[0], COL_THRES[0], 2) |
bin(col[1], COL_THRES[1], 1) |
bin(col[2], COL_THRES[2], 0);

// フラッグをクリアしてからセットする
clr_flg(Fsens, ~(BLK | BLU | GRN | CYA |
  RED | MAG | YEL | WHT)); (?)

  switch (CBits)
  {
    case 0:
    set_flg(Fsens, BLK);
    break;
    case 1:
    set_flg(Fsens, BLU);
    break;
    case 2:
    set_flg(Fsens, GRN);
    break;
    case 3:
    set_flg(Fsens, CYA);
    break;
    case 4:
    set_flg(Fsens, RED);
    break;
    case 5:
    set_flg(Fsens, MAG);
    break;
    case 6:
    set_flg(Fsens, YEL);
    break;
    case 7:
    set_flg(Fsens, WHT);
    break;
  }
