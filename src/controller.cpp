#include <Arduino.h>
#include <SPI.h>

#include <nRF24L01.h>
#include <RF24.h> //NRF24L01+ - 송신기

// 핀 선언
#define RADIO_CE 2
#define RADIO_CSN 5
#define RADIO_MOSI 16 //고정
#define RADIO_MISO 14 // 고정
#define RADIO_SCK 15 //고정     // NRF24L01+ - 송신기


#define Stick1_X A3 //아날로그
#define Stick1_Y A2 //아날로그
#define Stick2_X A1 //아날로그
#define Stick2_Y A0 //아날로그   // 조이스틱

#define SWITCH 6               // 스위치

const int center1 = 474;
const int center2 = 488;
const int center3 = 492;
const int center4 = 488;

// 아날로그 입력의 최소/최대값
const int analogMin = 0;
const int analogMax = 1023;

// 변환 범위
const int outputMin = -100;
const int outputMax = 100;

// NRF24L01+
RF24 radio(RADIO_CE, RADIO_CSN); 
const uint64_t pipe = 0xE8E8F0F0E1LL; // 송신 주소 설정

// 변수 선언
int initialDelay = 1000;
int delayTime = 100;

struct controllerData {
  int s1_X;
  int s1_Y;
  float s2_rad;
  float s2_distance;
  bool switchState;
};


// 함수
int mapJoy(int rawValue, int centerValue) {
  // 중심값을 0으로 맞추고, 좌우 최대치(중심에서의 거리)를 계산
  int distance = (rawValue - centerValue);

  // 중심에서 최대 거리(양수/음수 중 더 큰 쪽)를 구함
  int maxDistance = max(centerValue - analogMin, analogMax - centerValue);

  // -100~100으로 맵핑
  int mapped = map(distance, -maxDistance, maxDistance, outputMin, outputMax);

  // 범위 제한(클램프)
  if (mapped < outputMin) mapped = outputMin;
  if (mapped > outputMax) mapped = outputMax;

  return mapped;
}


//스위치 확인
bool checkSwitch() {
  int switchState = digitalRead(SWITCH);
  if (switchState == LOW) {
    return true;
  } else {
    return false;
  }
}


void setup() {
  Serial.begin(115200);

  // NRF24L01+
  radio.begin();
  radio.setPALevel(RF24_PA_LOW);
  radio.openWritingPipe(pipe);
  radio.stopListening();

  // 조이스틱
  
  // 스위치
  pinMode(SWITCH, INPUT_PULLUP);

  delay(initialDelay);
}


void loop() {





  // 조이스틱
  int X1 = analogRead(Stick1_X);
  int Y1 = analogRead(Stick1_Y);
  int X2 = analogRead(Stick2_X);
  int Y2 = analogRead(Stick2_Y);

  int s1_X = mapJoy(X1, center1);
  int s1_Y = mapJoy(Y1, center2);
  float s2_X = mapJoy(X2, center3);
  float s2_Y = mapJoy(Y2, center4);

  // Serial.print("s1_X: ");
  // Serial.print(s1_X);
  // Serial.print(" s1_Y: ");
  // Serial.print(s1_Y);
  // Serial.print(" s2_X: ");
  // Serial.print(s2_X);
  // Serial.print(" s2_Y: ");
  // Serial.println(s2_Y);
  


  float s2_rad = atan2(s2_Y, s2_X); // 조이스틱 각도 - 라디안
  float s2_distance = sqrt(s2_X * s2_X + s2_Y * s2_Y); // 조이스틱 거리
  // 거리 매핑
  // s2_distance = s2.distance / ( sqrt(2) * 100 ) * MAPPING_VALUE

  controllerData data;
  data.s1_X = s1_X;
  data.s1_Y = s1_Y;
  data.s2_rad = s2_rad;
  data.s2_distance = s2_distance;
  data.switchState = checkSwitch();

  Serial.print("Switch: ");
  Serial.println(data.switchState);
  Serial.print("s2_rad: ");
  Serial.print(data.s2_rad);
  Serial.print(" s2_distance: ");
  Serial.println(data.s2_distance);
  Serial.print("s1_X: ");
  Serial.print(data.s1_X);
  Serial.print(" s1_Y: ");
  Serial.print(data.s1_Y);
  Serial.println();

  // NRF24L01+
  radio.write(&data, sizeof(data));


  delay(delayTime);
}




