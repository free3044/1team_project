/**
   상생과일 분류 모델

   DC모터 : 레일을 움직임
   서보모터 : 제품을 색상에 따라 분류
   적외선 IR센서 : 물품이 적재됨을 감지.
   serial : 분류 결과값에 따라 서보모터의 각도가 결정
   RGB : 측정된 제품 분류 결과 값 색상을 표시

   동작 순서
   1. 컨베이어 벨트 레일 작동
   2. 적외선 센서로부터 레일에 제품이 적재됨을 감지하면 일시 정지
   3. 레일이 카메라 센서까지 움직이기 시작
   4. 컬러센서에서 정지 후 해당 제품 품질 분석
   5. 분석에 따른 결론에 따라 ok, notok, co가 결정되고, 해당되는 곳으로 서보모터가 동작
   6. 레일이 다시 움직이며 서보모터가 가리키는 방향으로 제품 분류
*/

/* 라이브러리 불러오기 */
#include <Adafruit_NeoPixel.h>  // LED 모듈 라이브러리
#include <Servo.h>              // 서보모터 라이브러리 불러오기
#include <Wire.h>               // MQTT 통신에 사용되는 라이브러리
#include <ArduinoJson.h>

/* 상수 선언 : 핀 번호, 속도제어, 서보모터의 각도*/
#define PIN_DC_DIRECTION 12  // DC모터(레일) 방향을 정하는 핀(현재 A모터 사용) B모터: 13
#define PIN_DC_SPEED 10      // DC모터(레일) 속도를 정하는 핀(현재 A모터 사용) B모터: 11
#define PIN_SERVO 9          // 서보모터 연결 핀
#define PIN_LED 5            // LED 연결 핀
#define PIN_IR A0            // 적외선 IR센서 연결 핀

#define POS_BLUE 90           // 정상품을 분류할 서보모터의 각도
#define POS_GREEN 120         // 상생 물품을 분류할 서보모터의 각도
#define POS_RED 155           // 불량품을 분류할 서보모터의 각도
#define NUM_PIXELS 3          // 네오픽셀의 픽셀 수: 3

/* 변수 선언 : HW객체, 측정값, 기타 변수, ...*/
Servo servo;
Adafruit_NeoPixel pixels(NUM_PIXELS, PIN_LED, NEO_GRB + NEO_KHZ800);

//String apple, apple_type;     // 분류값 사용하기 위한 변환값
int railSpeed = 0;            // 레일 기본 속도, 초기값은 160
bool isRailMoving = true;     // 레일 가동/정지 변수 유지
String data = Serial.readStringUntil(0x0d);

void setup() {
  /* 모터 설정 */
  Serial.begin(115200);
  pinMode(PIN_DC_DIRECTION, OUTPUT);     // dc모터의 방향을 제어하는 핀을 output으로 설정
  digitalWrite(PIN_DC_DIRECTION, HIGH);  // 방향은 전진. 의도한 방향과 반대일 경우 HIGH -> LOW로 변경
  analogWrite(PIN_DC_SPEED, railSpeed);  // 레일 작동 시작
  servo.attach(PIN_SERVO);               // 서보모터를 아두이노와 연결
  servo.write(180);                      // 초기 서보모터가 가리키는 각도는 0도
  delay(500);                            // 서보모터가 완전히 동작을 끝낸 후 detach를 위해 delay를 부여
  servo.detach();                        // 서보모터와 아두이노 분리

  /* 적외선 센서 설정 */
  pinMode(PIN_IR, INPUT);  // 적외선 센서 핀을 INPUT으로 설정

  /* LED 모듈 설정 */
  pixels.begin();             // LED 모듈 작동 시작
  pixels.setPixelColor(0, pixels.Color(0, 0, 0));
  pixels.setPixelColor(1, pixels.Color(0, 0, 0));
  pixels.setPixelColor(2, pixels.Color(0, 0, 0));
  pixels.show();
  pixels.setBrightness(255);  // 밝기(0~255) 설정. 최대 밝기로 출력

}

void loop() {
    if (Serial.available()) {
      String data = Serial.readStringUntil(0x0d);
      //1이면 on
      //0이면 off
      servo.attach(PIN_SERVO);
      if(data == "on"){
        analogWrite(PIN_DC_SPEED, railSpeed = 100);
        Serial.println("시스템 작동중");
      }else if(data == "off"){
        analogWrite(PIN_DC_SPEED, railSpeed = 0);
        servo.write(180);
        Serial.println("시스템 정지");
      }
    }
}
    /* 제품 적재여부 확인 */
void apple_sort(){
     if (Serial.available(),digitalRead(PIN_IR) == LOW,data == "0") {          // 적외선 센서는 물건 감지 시 LOW값을 전달. HIGH라는 것은 감지되지 않았음
        analogWrite(PIN_DC_SPEED, 0);          // 적외선 센서에서 제품을 감지하여 일시 정지
        toneDetected();                        // 감지되었을 때 나오는 소리를 부저에 출력
        delay(5000);                           // 5초간 정지
        analogWrite(PIN_DC_SPEED, railSpeed);  // 레일을 카메라까지 움직이기 시작

        servo.attach(PIN_SERVO);       // 서보모터를 아두이노와 연결
        servo.write(POS_BLUE);        // 서보모터를 정상품(파란색) 분류방향으로 회전
        /* LED모듈에 있는 3개의 LED모듈에 모두 파란색에 해당하는 값(#0000ff) 입력*/
        pixels.setPixelColor(0, pixels.Color(0, 0, 255));
        pixels.setPixelColor(1, pixels.Color(0, 0, 255));
        pixels.setPixelColor(2, pixels.Color(0, 0, 255));
        pixels.show();  // 입력한 색상 값을 LED 모듈에 출력
        delay(2000);   // 측정 결과 표기 후 2초 간 레일 대기
        
        pixels.setPixelColor(0, pixels.Color(0, 0, 0));
        pixels.setPixelColor(1, pixels.Color(0, 0, 0));
        pixels.setPixelColor(2, pixels.Color(0, 0, 0));
        pixels.show();
        servo.detach();   // 서보모터와 아두이노 연결 해제
        delay(3000);
        analogWrite(PIN_DC_SPEED,railSpeed = 0);
    }else if(Serial.available(),digitalRead(PIN_IR) == LOW,data == "1"){
        analogWrite(PIN_DC_SPEED, 0);          // 적외선 센서에서 제품을 감지하여 일시 정지
        toneDetected();                        // 감지되었을 때 나오는 소리를 부저에 출력
        delay(5000);                           // 5초간 정지
        analogWrite(PIN_DC_SPEED, railSpeed);  // 레일을 카메라까지 움직이기 시작

        servo.attach(PIN_SERVO);       // 서보모터를 아두이노와 연결
        servo.write(POS_GREEN);        // 서보모터를 정상품(파란색) 분류방향으로 회전
        /* LED모듈에 있는 3개의 LED모듈에 모두 초록색에 해당하는 값(#00ff00) 입력*/
        pixels.setPixelColor(0, pixels.Color(0, 255, 0));
        pixels.setPixelColor(1, pixels.Color(0, 255, 0));
        pixels.setPixelColor(2, pixels.Color(0, 255, 0));
        pixels.show();  // 입력한 색상 값을 LED 모듈에 출력
        delay(2000);   // 측정 결과 표기 후 2초 간 레일 대기
        
        pixels.setPixelColor(0, pixels.Color(0, 0, 0));
        pixels.setPixelColor(1, pixels.Color(0, 0, 0));
        pixels.setPixelColor(2, pixels.Color(0, 0, 0));
        pixels.show();
        servo.detach();   // 서보모터와 아두이노 연결 해제
        delay(3000);
        analogWrite(PIN_DC_SPEED,railSpeed = 0);
    }else if(Serial.available(),digitalRead(PIN_IR) == LOW,data == "2"){
        analogWrite(PIN_DC_SPEED, 0);          // 적외선 센서에서 제품을 감지하여 일시 정지
        toneDetected();                        // 감지되었을 때 나오는 소리를 부저에 출력
        delay(5000);                           // 5초간 정지
        analogWrite(PIN_DC_SPEED, railSpeed);  // 레일을 카메라까지 움직이기 시작

        servo.attach(PIN_SERVO);       // 서보모터를 아두이노와 연결
        servo.write(POS_RED);        // 서보모터를 정상품(파란색) 분류방향으로 회전
        /* LED모듈에 있는 3개의 LED모듈에 모두 빨간색에 해당하는 값(#ff0000) 입력*/
        pixels.setPixelColor(0, pixels.Color(255, 0, 0));
        pixels.setPixelColor(1, pixels.Color(255, 0, 0));
        pixels.setPixelColor(2, pixels.Color(255, 0, 0));
        pixels.show();  // 입력한 색상 값을 LED 모듈에 출력
        delay(2000);   // 측정 결과 표기 후 2초 간 레일 대기
        
        pixels.setPixelColor(0, pixels.Color(0, 0, 0));
        pixels.setPixelColor(1, pixels.Color(0, 0, 0));
        pixels.setPixelColor(2, pixels.Color(0, 0, 0));
        pixels.show();
        servo.detach();   // 서보모터와 아두이노 연결 해제
        delay(3000);
        analogWrite(PIN_DC_SPEED,railSpeed = 0);
    }
     return;                                    // loop에 대한 return문장은 그 즉시 loop문을 종료하고, 처음부터 loop을 시작하게 함
  }

void toneDetected() {
  tone(4, 523, 50);  // '도'에 해당. 0.05초간 출력
  delay(100);        // 0.1초간 대기(출력시간 0.05초 + 대기시간 0.05초 = 0.1초)
  tone(4, 784, 50);  // '미'에 해당. 0.05초간 출력
  delay(100);        // 0.1초간 대기(출력시간 0.05초 + 대기시간 0.05초 = 0.1초)
}  // void toneDetected()
