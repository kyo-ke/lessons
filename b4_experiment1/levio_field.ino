#include <Wire.h> //for mpu6050 i2c
#include <Servo.h>
#include "MPU6050.h"
#include "MadgwickAHRS.h"

#define REDLED 13   //HIGH = ON
#define GREENLED 30 //LOW = ON

Servo Motor_R1;
Servo Motor_R2;
Servo Motor_R3;
Servo Motor_R4;
Servo Motor_L1;
Servo Motor_L2;
Servo Motor_L3;
Servo Motor_L4;

#define PIN_R1 5
#define PIN_R2 8
#define PIN_R3 6
#define PIN_R4 9
#define PIN_L1 2
#define PIN_L2 10
#define PIN_L3 3
#define PIN_L4 7

#define MOTORNUM 8

Madgwick filter;
MPU6050 mpu6050;

int MotorVal[MOTORNUM];
int MotorMemory[MOTORNUM];

int16_t THROTTLE = 1000;
int16_t ROLL = 1500;
int16_t PITCH = 1500;
int16_t YAW = 1500;

float roll, pitch, yaw;

//経路の何個めか
bool rootmode = false;
int rootnum = 0;
bool rootSW = false;

//速度の決定
int pitch_num = 2;
int turncounter = 1;

//turntimeは偶数
int turntime;//*100ms
int straighttime;//*100ms

unsigned long intervalTime;
unsigned long pMillis_Motor;
unsigned long pMillis_Serial;
unsigned long pMillis_Generator;


bool PW_ON = false;
bool SafeLimit = true;
bool ResetIMU_SW = true;

float timemanager;

bool Roll_FB = true;
bool Pitch_FB = true;
bool Yaw_FB = true;

bool JumpMode = false;
bool JumpSW = false;

bool WLMode = false;
bool InversedMode = false;

bool PaddleMode = false;
int EffectValue[MOTORNUM];

unsigned int HapticPatternNo = 0;
unsigned int HapticCounter;

unsigned int WeightPatternNo = 0;

void setup() {
  Serial.begin(115200);
  
  pinMode(REDLED, OUTPUT);
  pinMode(GREENLED, OUTPUT);
  digitalWrite(REDLED, LOW);
  digitalWrite(GREENLED, HIGH);

  InitIMU();

  intervalTime = 10; //10[ms], 100[Hz]
  
  

  MotorInit();
  delay(500);
  
  digitalWrite(GREENLED, LOW);
  timemanager = -1;
  turncounter = 0;
}

void InitIMU() {
  Wire.begin();
  mpu6050.init();
  filter.begin(100); //madjwick filter begin
}


float pitchFB;           
float rollFB;
float yawFB;
float Base_V;
float y_V;

float pPitch;
float pitchS;
float pYaw;
float yawS;
float pRoll;
float rollS;

void loop() {
   
  unsigned long currentMillis = millis();
  
  if ((unsigned long)(currentMillis - pMillis_Generator) >= (intervalTime*10)) {  //100[ms], 10[Hz]
    pMillis_Generator = currentMillis;
    
    //HapticGenerator()
    
    
  }
  if ((unsigned long)(currentMillis - pMillis_Motor) >= intervalTime) { //10[ms], 100[Hz]
    pMillis_Motor = currentMillis;
    ReadMPU6050();
    ForceGenerater();
    MotorWrite();
  }
  if ((unsigned long)(currentMillis - pMillis_Serial) >= (intervalTime*2)) {  //20[ms], 50[Hz]
    pMillis_Serial = currentMillis;
    CommandRead();
    if(!JumpMode) SendCondtions(roll, pitch, yaw, pitchFB, rollFB, yawFB, Base_V, y_V);
    //if(!JumpMode) SendCondtions(pPitch, pitchS, pYaw, yawS, pRoll, rollS, Base_V, y_V);
    //sendMotorData();
  }
  
}

void MotorInit(){
  for(int i=0; i<8; i++) MotorVal[i] = 1000;
  Motor_R1.attach(PIN_R1, 1000, 2000);
  Motor_R2.attach(PIN_R2, 1000, 2000);
  Motor_R3.attach(PIN_R3, 1000, 2000);
  Motor_R4.attach(PIN_R4, 1000, 2000);
  Motor_L1.attach(PIN_L1, 1000, 2000);
  Motor_L2.attach(PIN_L2, 1000, 2000);
  Motor_L3.attach(PIN_L3, 1000, 2000);
  Motor_L4.attach(PIN_L4, 1000, 2000);
  MotorWrite();
}

void MotorStop(){
  for(int i=0; i<8; i++) MotorVal[i] = 1000;
  MotorWrite();
}

void SystemInit(){
  InitIMU();
  
  PW_ON = false;
  SafeLimit = true;
  ResetIMU_SW = true;
  
  THROTTLE = 1000;
  ROLL = 1500;
  PITCH = 1500;
  YAW = 1500;
  
  Roll_FB = true;
  Pitch_FB = true;
  Yaw_FB = true;

  JumpMode = false;
  JumpSW = false;

  WLMode = false;
  InversedMode = false;

  PaddleMode = false;
  for(int i=0; i<8; i++) EffectValue[i] = 0;
}

void MotorWrite(){
  Motor_R1.writeMicroseconds(MotorVal[0]);
  Motor_R2.writeMicroseconds(MotorVal[1]);
  Motor_R3.writeMicroseconds(MotorVal[2]);
  Motor_R4.writeMicroseconds(MotorVal[3]);
  Motor_L1.writeMicroseconds(MotorVal[4]);
  Motor_L2.writeMicroseconds(MotorVal[5]);
  Motor_L3.writeMicroseconds(MotorVal[6]);
  Motor_L4.writeMicroseconds(MotorVal[7]);
}

void ReadMPU6050(){
  float imuData[6];
  mpu6050.getData(imuData);
  //ボードが逆さに取り付けられているため、値を入れ替えています
  filter.updateIMU(imuData[1], imuData[0], -imuData[2], imuData[4], imuData[3], -imuData[5]);
  
  if(!InversedMode){
    roll = filter.getRoll();
    pitch = filter.getPitch();
    yaw = filter.getYaw();
  }else{
    roll = AngleOffset(filter.getRoll(), 180);
    pitch = filter.getPitch();
    yaw = filter.getYaw();
  }

  if(JumpMode) {
    JumpDitection(imuData[5]);
  } else {
    //SendRotation(roll, pitch, yaw);
  }
  /*
  for(int i=3; i<6; i++) {
    Serial.print(imuData[i]);
    Serial.print("\t");
  }
  Serial.println();
  */
  
}

float AngleOffset(float deg, float offset) {
  float output = deg + offset;
  if(output < -180.0f){
    output = output + 360.0f;
  } else if(180.0f < output){
    output = output - 360.0f;
  }
  return output;
}

const int JUMP_SH = -2100;
void JumpDitection(float az) {
  int JumpingVal = (int)(az*1000);
  //Serial.println(JumpingVal);

  if((JumpingVal<JUMP_SH)&&(JumpSW==false)) {
    //Serial.println(JumpingVal);
    JumpSW = true;
    HapticPatternNo = 1;
    HapticCounter = 0;
    Serial.println("j");  //send command to Unity
  }
  
}

void ForceGenerater(){
  /*--- define values ---*/
  unsigned long millisPerReading = 1000 / 200;
  int baseValue;
  int escValue[MOTORNUM];
  float imuData[6];
  static const float KpP = 1.0f, KiP = 0.5f, KdP = 0.3f;
  static const float KpR = 2.0f, KiR = 0.1f, KdR = 0.1f;
  static const float KpY = 1.5f, KiY = 0.0f, KdY = 0.0f;
  static float prePitch, pitchSum;
  static float preYaw, yawSum;
  static float preRoll, rollSum;
  //float targetYaw = 180.0f;
  float targetYaw = yaw;
  static int count;
  /*--- define values end ---*/

  if(ResetIMU_SW) {
    ResetIMU_SW = false;
    prePitch = 0;
    pitchSum = 0;
    yawSum = 0;
    preRoll = 0;
    rollSum = 0;
  }
  
  baseValue = THROTTLE;
  /*--- caluculate feedback value ---*/
  //float pitchFeedBack = KpP*pitch + KiP*pitchSum + KdP*(pitch - prePitch)*1000/millisPerReading + (PITCH - 1500)/6;
  float pitchFeedBack = (PITCH - 1500)/6;
  //float rollFeedBack = KpR*roll + KiR*rollSum + KdR*(roll - preRoll)*1000/millisPerReading;
  float rollFeedBack = 0;
  //float yawFeedBack = KpY*(yaw-targetYaw) + KiY*yawSum + KdY*(yaw - preYaw)*1000/millisPerReading + (YAW - 1500)/2;  
  float yawFeedBack = (YAW - 1500)/2;
  float y = (ROLL - 1500)/2;
  /*--- finish calculating feedback value ^^^*/

  pitchFB = pitchFeedBack;           
  rollFB = rollFeedBack;
  yawFB = yawFeedBack;
  Base_V = (float)baseValue;
  y_V = y;

  if(!Pitch_FB) pitchFeedBack = 0;
  if(!Roll_FB) rollFeedBack = 0;
  if(!Yaw_FB) yawFeedBack = 0;

  if(PW_ON){
    if(!InversedMode){
      MotorVal[0] = baseValue + pitchFeedBack + rollFeedBack + yawFeedBack + y;
      //MotorVal[0] = 0;
      MotorVal[1] = baseValue + pitchFeedBack + rollFeedBack + yawFeedBack + y;
      MotorVal[2] = baseValue - pitchFeedBack + rollFeedBack - yawFeedBack + y;
      MotorVal[3] = baseValue - pitchFeedBack + rollFeedBack - yawFeedBack + y;
      MotorVal[4] = baseValue - pitchFeedBack - rollFeedBack + yawFeedBack - y;
      MotorVal[5] = baseValue - pitchFeedBack - rollFeedBack + yawFeedBack - y;
      MotorVal[6] = baseValue + pitchFeedBack - rollFeedBack - yawFeedBack - y;
      MotorVal[7] = baseValue + pitchFeedBack - rollFeedBack - yawFeedBack - y;
    } else {
      MotorVal[0] = baseValue - pitchFeedBack + rollFeedBack - yawFeedBack - y;
      MotorVal[1] = baseValue - pitchFeedBack + rollFeedBack - yawFeedBack - y;
      MotorVal[2] = baseValue + pitchFeedBack + rollFeedBack + yawFeedBack - y;
      MotorVal[3] = baseValue + pitchFeedBack + rollFeedBack + yawFeedBack - y;
      MotorVal[4] = baseValue + pitchFeedBack - rollFeedBack - yawFeedBack + y;
      MotorVal[5] = baseValue + pitchFeedBack - rollFeedBack - yawFeedBack + y;
      MotorVal[6] = baseValue - pitchFeedBack - rollFeedBack + yawFeedBack + y;
      MotorVal[7] = baseValue - pitchFeedBack - rollFeedBack + yawFeedBack + y;
    }
    
  } else {
    MotorStop();
  }

  if(PaddleMode) {
    for(int i=0; i<8; i++) MotorVal[i] += EffectValue[i];
  }

  if(SafeLimit) {
    for(int i=0; i<8; i++) if(1800<MotorVal[i]) MotorVal[i] = 0;
  }

  /*---deal with pid values---*/
  prePitch = pitch;
  pitchSum += pitch * millisPerReading/1000;
  preYaw = yaw;
  yawSum += yaw * millisPerReading/1000;
  preRoll = roll;
  rollSum += roll * millisPerReading/1000;
  /*---pid values---*/

  pPitch = prePitch;
  pitchS = pitchSum;
  pYaw = preYaw;
  yawS = yawSum;
  pRoll = preRoll;
  rollS = rollSum;



  
}



void SendCondtions(float r, float p, float y, float rFB, float pFB, float yFB, float BV, float yV) {
  int r_ = (int)(18000+r*100);
  int p_ = (int)(18000+p*100);
  int y_ = (int)(18000+y*100);
  int rFB_ = (int)(18000+rFB*100);
  int pFB_ = (int)(18000+pFB*100);
  int yFB_ = (int)(18000+yFB*100);
  int BV_ = (int)(18000+BV*100);
  int yV_ = (int)(18000+yV*100);
  String data;
  data += "H";
  data += "\t";
  data += r_;
  data += "\t";
  data += p_;
  data += "\t";
  data += y_;
  data += "\t";
  data += rFB_;
  data += "\t";
  data += pFB_;
  data += "\t";
  data += yFB_;
  data += "\t";
  data += BV_;
  data += "\t";
  data += yV_;
  Serial.println(data);
  /*
  Serial.write('H');
  Serial.write(highByte(r_));
  Serial.write(lowByte(r_));
  Serial.write(highByte(p_));
  Serial.write(lowByte(p_));
  Serial.write(highByte(y_));
  Serial.write(lowByte(y_));
  */
}


void sendMotorData() {
  for(int i=0; i<8; i++) {
    Serial.print(MotorVal[i]);
    Serial.print("\t");
  }
  Serial.println();
}
//sボタンで直進スタート
void CommandRead() {
  if(Serial.available()){
    char mode = Serial.read();
          
    switch(mode){
      case '0':
        MotorStop();
        SystemInit();
        break;
        
      case '1':
        PW_ON = true;
        THROTTLE = 1100;
        break;

      case '2':
        PW_ON = true;
        timemanager =2;
        HapticCounter = 1;
        straighttime = 3000;
        HapticPatternNo = 2;
        break;
      case '3':
        PW_ON = true;
        timemanager =3;
        HapticPatternNo = 3;
        turntime = 86;
        break;

      case '4':
        PW_ON = true;
        HapticCounter = 1;
        timemanager =4;
        straighttime = 3000;
        HapticPatternNo = 4;
        
        break;
      case '5':
        PW_ON = false;
        timemanager =4;
        turncounter = 0;
        break;


//回転
      case '6':
        if (ROLL < 1850){
            ROLL += 10;
        }
        
        break;
      case '7':
        if (ROLL > 1150){
            ROLL -= 10;
        }
        break;

//加速
      case '8':
        if (THROTTLE < 1700){
            THROTTLE += 10;
        }
        if(PITCH < 1800){
            PITCH += 10;
        }
        break;
      
      case '9':
        THROTTLE -= 10;
        break;
//並進
      case 'f':
        //no difference
        ROLL = 1500
        break;

      case 'z':
        MotorStop();
        SystemInit();
        InversedMode = true;
        //Roll_FB = false;
        //Pitch_FB = false;
        //Yaw_FB = false;
        break;
      
      case 'a':
        if(HapticPatternNo == 0){
          HapticPatternNo = 6;
          HapticCounter = 0;
        }
        break;

      case 's':
        HapticPatternNo = 2;
        PW_ON = true;
        rootmode = true;
    }
  }
}


void HapticGenerator() {
  if (HapticPatternNo == 1){

  }
  
  
}


void next_TPRY(float dt, float dp,float dr, float  dy){

}