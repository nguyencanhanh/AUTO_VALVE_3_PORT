/*****************************************************************************
 * File Name   : drv_display.c
 * Layer       : Drivers - Hardware Abstraction
 * Description : Driver 7-segment 4 digit - NOI TRUC TIEP GPIO (khong HC595)
 *
 * Segment layout (Common Cathode / Common Cathode):
 *   bit:  dp    g     f     e     d     c     b     a
 *   pin:  PB4   PB8   PB5   PB3   PB1   PB2   PA12  PB13
 *
 * Digit select (Active LOW):
 *   DIG1(left)=PB12 | DIG2=PB6 | DIG3=PB0 | DIG4(right)=PB9
 *
 * Multiplex: 4 digit × 3ms = 12ms/cycle ≈ 83Hz (khong blink)
 *****************************************************************************/
#include "drv_display.h"

/*============================================================================
 * BANG FONT 7-DOAN
 * Bit layout: bit7=dp | bit6=g | bit5=f | bit4=e | bit3=d | bit2=c | bit1=b |
 *bit0=a
 *============================================================================*/
static const u8 s_font[] = {
    /* 0-9 */
    0x3F, /* 0: abcdef  */
    0x06, /* 1: bc      */
    0x5B, /* 2: abdeg   */
    0x4F, /* 3: abcdg   */
    0x66, /* 4: bcfg    */
    0x6D, /* 5: acdfg   */
    0x7D, /* 6: acdefg  */
    0x07, /* 7: abc     */
    0x7F, /* 8: abcdefg */
    0x6F, /* 9: abcdfg  */
    /* 0x0A-0x0F: hex */
    0x77,
    /* A */ 0x7C,
    /* b */ 0x39, /* C */
    0x5E,
    /* d */ 0x79,
    /* E */ 0x71, /* F */
    /* 0x10 = BLANK  */ 0x00,
    /* 0x11 = '-'    */ 0x40,
    /* 0x12 = 'E'    */ 0x79,
    /* 0x13 = 'H'    */ 0x76,
    /* 0x14 = 'L'    */ 0x38,
    /* 0x15 = 'O'    */ 0x3F,
    /* 0x16 = 'C'    */ 0x39,
    /* 0x17 = 'P'    */ 0x73,
    /* 0x18 = 'r'    */ 0x50,
    /* 0x19 = 'n'    */ 0x54,
    /* 0x1A = ALL ON */ 0x7F,
};
#define FONT_SIZE ((u8)(sizeof(s_font) / sizeof(s_font[0])))

typedef struct {
  GPIO_TypeDef *port;
  uint32_t pin;
} SegPin_t;

/* Array segment pins tuong ung bit 0(a)→bit7(dp) */
static const SegPin_t s_seg_pins[8] = {
    {SEG_PORT, SEG_A_PIN}, {GPIOA, SEG_B_PIN}, /* SEG_B o GPIOA */
    {SEG_PORT, SEG_C_PIN}, {SEG_PORT, SEG_D_PIN}, {SEG_PORT, SEG_E_PIN},
    {SEG_PORT, SEG_F_PIN}, {SEG_PORT, SEG_G_PIN}, {SEG_PORT, SEG_DP_PIN}};

/* Array digit pins: index 0=DIG1(left) .. 3=DIG4(right) */
static const uint32_t s_dig_pins[4] = {DIG1_PIN, DIG2_PIN, DIG3_PIN, DIG4_PIN};

/*============================================================================
 * PRIVATE VARIABLES
 *============================================================================*/
static u8 s_seg_buf[DISPLAY_NUM_DIGITS]; /* Ma font every digit */
static u8 s_dp_buf[DISPLAY_NUM_DIGITS];  /* Decimal point thap phan */
static u8 s_cur_digit = 0;
static u8  s_blink_mask         = 0;
static u16 s_blink_cnt          = 0;
static u8  s_blink_phase        = 1; /* 1 = turn on, 0 = turn off */
static u16 s_blink_complete_cnt = 0; /* Dem so lan blink hoan chinh (OFF->ON) */
/*============================================================================
 * PRIVATE: Export ma doan ra GPIOB
 *============================================================================*/
static void prv_OutputSegments(u8 code) {
  u8 i;
  /* Turn off all segments (Common Cathode: turn off = LOW) */
  GPIO_SetBits(SEG_PORT, SEG_A_PIN | SEG_C_PIN | SEG_D_PIN | SEG_E_PIN |
                             SEG_F_PIN | SEG_G_PIN | SEG_DP_PIN);
  GPIO_SetBits(GPIOA, SEG_B_PIN);

  for (i = 0; i < 8; i++) {
    /* Turn on segment tuong ung voi bit = 1 (Common Cathode: turn on = HIGH) */
    if (code & (1u << i))
      GPIO_ResetBits(s_seg_pins[i].port, s_seg_pins[i].pin);
  }
}

/*============================================================================
 * DRV_Display_Init
 *============================================================================*/
void DRV_Display_Init(void) {
  u8 i;
  for (i = 0; i < DISPLAY_NUM_DIGITS; i++) {
    s_seg_buf[i] = SEG_CHAR_BLANK;
    s_dp_buf[i] = 0;
  }
  s_cur_digit = 0;
  s_blink_mask = 0;
  s_blink_cnt = 0;
  s_blink_phase = 1;

  GPIO_ResetBits(SEG_PORT, SEG_A_PIN | SEG_C_PIN | SEG_D_PIN | SEG_E_PIN |
                               SEG_F_PIN | SEG_G_PIN | SEG_DP_PIN);
  GPIO_ResetBits(GPIOA, SEG_B_PIN);
  GPIO_SetBits(DIG_PORT, DIG_ALL_PINS); /* Turn off all digit (HIGH) */
}

/*============================================================================
 * DRV_Display_Refresh - CALLED FROM TIM2 ISR every ~3ms
 *============================================================================*/
void DRV_Display_Refresh(void) {
  u8 idx, code;

  /* Blink: cycle ~500ms (167 × 3ms ≈ 500ms) */
  s_blink_cnt++;
  if (s_blink_cnt >= 167) {
    s_blink_cnt  = 0;
    s_blink_phase = !s_blink_phase;
    /* Dem khi phase lat tu OFF(0) sang ON(1) = ket thuc 1 lan blink hoan chinh */
    if (s_blink_phase == 1) {
      if (s_blink_complete_cnt < 0xFFFF) s_blink_complete_cnt++;
    }
  }

  /* Turn off digit old + clear segments before changing (Common Cathode) */
  GPIO_SetBits(DIG_PORT, DIG_ALL_PINS);
  GPIO_ResetBits(SEG_PORT, SEG_A_PIN | SEG_C_PIN | SEG_D_PIN | SEG_E_PIN |
                               SEG_F_PIN | SEG_G_PIN | SEG_DP_PIN);
  GPIO_ResetBits(GPIOA, SEG_B_PIN);

  /* Get code segment */
  idx = s_seg_buf[s_cur_digit];
  if ((s_blink_mask & (1u << s_cur_digit)) && !s_blink_phase)
    idx = SEG_CHAR_BLANK;
  code = (idx < FONT_SIZE) ? s_font[idx] : 0x00;

  /* Them decimal point */
  if (s_dp_buf[s_cur_digit] == 1)
    code |= 0x80;
  else if (s_dp_buf[s_cur_digit] == 2 && s_blink_phase)
    code |= 0x80;

  /* Export segments + select digit (Common Cathode → ResetBits = turn on Digit)
   */
  prv_OutputSegments(code);
  GPIO_ResetBits(DIG_PORT, s_dig_pins[s_cur_digit]);

  /* Switch digit next */
  s_cur_digit = (s_cur_digit + 1) % DISPLAY_NUM_DIGITS;
}

/*============================================================================
 * API Display
 *============================================================================*/
void DRV_Display_Text(u8 d3, u8 d2, u8 d1, u8 d0) {
  s_seg_buf[0] = d3;
  s_seg_buf[1] = d2;
  s_seg_buf[2] = d1;
  s_seg_buf[3] = d0;
  s_dp_buf[0] = s_dp_buf[1] = s_dp_buf[2] = s_dp_buf[3] = 0;
}

void DRV_Display_Number(u16 number, u8 leading_zero) {
  u8 d[4], i, j;
  if (number > 9999)
    number = 9999;
  d[0] = (u8)(number / 1000);
  d[1] = (u8)((number / 100) % 10);
  d[2] = (u8)((number / 10) % 10);
  d[3] = (u8)(number % 10);

  for (i = 0; i < 4; i++) {
    s_dp_buf[i] = 0;
    if (!leading_zero && i < 3) {
      u8 all_zero = 1;
      for (j = 0; j <= i; j++)
        if (d[j]) {
          all_zero = 0;
          break;
        }
      s_seg_buf[i] = all_zero ? SEG_CHAR_BLANK : d[i];
    } else {
      s_seg_buf[i] = d[i];
    }
  }
}

void DRV_Display_Time(u8 hour, u8 minute) {
  if (hour > 23)
    hour = 0;
  if (minute > 59)
    minute = 0;
  s_seg_buf[0] = hour / 10;
  s_seg_buf[1] = hour % 10;
  s_seg_buf[2] = minute / 10;
  s_seg_buf[3] = minute % 10;
  /* Decimal point 2 va 3 light up (index 1 & 2), turn off dau 1 va 4 (index 0 &
   * 3) */
  s_dp_buf[0] = 0;
  s_dp_buf[1] = 2; /* 2 = blink DP */
  s_dp_buf[2] = 0;
  s_dp_buf[3] = 0;
}

void DRV_Display_AllOn(void) {
  u8 i;
  for (i = 0; i < DISPLAY_NUM_DIGITS; i++) {
    s_seg_buf[i] = SEG_CHAR_ALL;
    s_dp_buf[i] = 1;
  }
}

void DRV_Display_Blank(void) {
  u8 i;
  for (i = 0; i < DISPLAY_NUM_DIGITS; i++) {
    s_seg_buf[i] = SEG_CHAR_BLANK;
    s_dp_buf[i] = 0;
  }
}

void DRV_Display_SetDP(u8 digit, u8 on) {
  if (digit < DISPLAY_NUM_DIGITS)
    s_dp_buf[digit] = on;
}

void DRV_Display_SetBlink(u8 digit_mask, u8 enable) {
  if (enable)
    s_blink_mask |= digit_mask;
  else
    s_blink_mask &= ~digit_mask;
}

/* Tra ve so lan blink da hoan chinh ke tu lan goi ResetBlinkCount() cuoi cung */
u16 DRV_Display_GetBlinkCount(void) {
  return s_blink_complete_cnt;
}

/* Reset bo dem ve 0, goi truoc khi bat dau dem so lan blink */
void DRV_Display_ResetBlinkCount(void) {
  s_blink_complete_cnt = 0;
}