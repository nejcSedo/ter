#include <LiquidCrystal.h>

uint8_t DegreeBitmap[]= { 0x6, 0x9, 0x9, 0x6, 0x0, 0, 0, 0 };
float voltage = 3.3;
const int in_array_size = 150;
float base_temp_res_value = 100.0;
int R1 = 671;
float alpha = 0.385;
float in[in_array_size];
int temp_sens = A0;
int up_temp_btn = 3;
int down_temp_btn = 2;
const int rs = 4, en = 5, d4 = 6, d5 = 7, d6 = 8, d7 = 9;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
int avr_samp = 20;
int set_temp = 60;
int btn_state_up = HIGH;
int btn_state_down = HIGH;
int speakerPin = 10;
int numTones = 10;
int tones[] = {261, 277, 294, 311, 330, 349, 370, 392, 415, 440};
//            mid C  C#   D    D#   E    F    F#   G    G#   A
bool temp_reached = false;
int pt100;
uint8_t pos;
float r1;
float r2;
int c1;
int c2;
float Vout;
float R2;
float c;
float c_avr;

float MultiMap(float val, float* _in, uint8_t size) {
  if (val < _in[0] ) return -999999;
  if (val > _in[size-1] ) return 999999;
  pos = 0;
  while(val > _in[pos]) pos++;  
  if (val == _in[pos]) return pos;
  r1 = _in[pos-1];
  r2 = _in[pos];
  c1 = pos-1;
  c2 = pos;
  return c1 + (val - r1) / (r2-r1) * (c2-c1);
}

void ReadTemp() {
  c_avr = 0;
  for(int i(0); i < avr_samp; i++) {
    pt100 = analogRead(temp_sens);
    Vout = (pt100 * voltage) / 1023;
    R2 = R1 * 1/(voltage / Vout - 1);
    c =  MultiMap(R2,in,in_array_size);
    c_avr += c;
    delay(5);
  }
  c = c_avr / avr_samp;
}

void SetTempBtn() {
  btn_state_up = digitalRead(up_temp_btn);
  if(btn_state_up == LOW) {
    if(set_temp == -10) {
      set_temp = c + 10;
    }
    if(set_temp < 200) {
      set_temp += 1;
      delay(300);
      btn_state_up = digitalRead(up_temp_btn);
      bool first = true;
      while(btn_state_up == LOW) {
        if(first) {
          set_temp -= 1;
          first = false;
        }
        set_temp += 10;
        if(set_temp >= 200) {
          set_temp = 200;
        }
        LCD_print();
        delay(400);
        btn_state_up = digitalRead(up_temp_btn);
      }
    }
    else if(set_temp > 200){
      set_temp = 200;
    }
  }
  btn_state_down = digitalRead(down_temp_btn);
  if(btn_state_down == LOW) {
    if(set_temp == -10) {
      set_temp = c + 10;
    }
    if(set_temp > 0) {
      set_temp -= 1;
      delay(300);
      btn_state_down = digitalRead(down_temp_btn);
      bool first = true;
      while(btn_state_down == LOW) {
        if(first) {
          set_temp += 1;
          first = false;
        }
        set_temp -= 10;
        if(set_temp <= 0) {
          set_temp = 0;
        }
        LCD_print();
        delay(400);
        btn_state_down = digitalRead(down_temp_btn);
      }
    }
    else if(set_temp < 0) {
      set_temp = 0;
    }
  }
  ReadTemp();
  LCD_print();
}

void LCD_print() {
  if(set_temp == -10) {
    lcd.clear();
    lcd.setCursor(5,0);
    lcd.println("RESET       ");
    set_temp = (c + 10);
    delay(2000);
    lcd.clear();
  }
  else if(!temp_reached) {
    lcd.setCursor(0,0);
    lcd.print("Alarm: ");
    if(set_temp == 200) {
      lcd.setCursor(7,0);
      lcd.print("MAX ");
    }
    else if(set_temp == 0) {
      lcd.setCursor(7,0);
      lcd.print("MIN ");
    }
    else {
      lcd.print(set_temp);
    }
    lcd.print("C");
    lcd.print(char(1));
    if(set_temp < 10) {
      lcd.setCursor(10,0);
      lcd.print(" ");
    }
    else if(set_temp < 100) {
      lcd.setCursor(11,0);
      lcd.print(" ");
    }
    else if(set_temp < 200) {
      lcd.setCursor(12,0);
      lcd.print(" ");
    }
    lcd.setCursor(0,1);
    lcd.print("Temp.: ");
    lcd.print(c);
    lcd.print("C");
    lcd.print(char(1));
    if(c < 10) {
      lcd.setCursor(13,1);
      lcd.print(" ");
    }
    else if(c < 100) {
      lcd.setCursor(14,1);
      lcd.print(" ");
    }
  }
}

void setup() {
  pinMode(up_temp_btn, INPUT_PULLUP);
  pinMode(down_temp_btn, INPUT_PULLUP);
  analogReference(EXTERNAL);
  in[0] = base_temp_res_value;
  for(int i(1); i < in_array_size; i++) {
    in[i] = in[i - 1] + alpha;
  }
  lcd.begin(16,2);
  lcd.createChar(1, DegreeBitmap);
  lcd.setCursor(2,0);
  lcd.print("TEMPERATURNI");
  lcd.setCursor(5,1);
  delay(1000);
  lcd.print("SENZOR");
  delay(2000);
  lcd.clear();
  lcd.setCursor(1,0);
  lcd.print("Nejc Sedovnik");
  delay(1000);
  lcd.setCursor(3,1);
  lcd.print("Maj 2018");
  delay(2000);
  lcd.clear();
}

void loop() {
  SetTempBtn();
  ReadTemp();
  if(c > set_temp && set_temp != -10) {
    temp_reached = true;
    while(temp_reached) {
      lcd.clear();
      lcd.setCursor(2,0);
      lcd.print("TEMPERATURA");
      lcd.setCursor(4,1);
      lcd.print("DOSEZENA");
      bool sound = true;
      while(sound) {
        for(int hz = 440; hz < 1000; hz++){
          tone(speakerPin, hz, 10);
          delay(1);
        }
        for(int hz = 1000; hz > 440; hz--){
          tone(speakerPin, hz, 10);
          delay(1);
        }
        btn_state_up = digitalRead(up_temp_btn);
        btn_state_down = digitalRead(down_temp_btn);
        if(btn_state_up == LOW && btn_state_down == LOW) {
          noTone(speakerPin);
          set_temp = -10;
          temp_reached = false;
          btn_state_up = digitalRead(up_temp_btn);
          btn_state_down = digitalRead(down_temp_btn);
          bool first = true;
          while(btn_state_up == LOW || btn_state_down == LOW) {
            if(first) {
              lcd.clear();
              lcd.setCursor(2,0);
              lcd.print("SPUSTI TIPKO");
              first = false;
            }
            delay(500);
            btn_state_up = digitalRead(up_temp_btn);
            btn_state_down = digitalRead(down_temp_btn);
          }
          sound = false;
        }
      }
    }
  }
  delay(5);
}
