#include <IRremote.h>

// Define for interrupt:
#define interruptPin1 2
#define interruptPin2 3
#define interruptPin3 7
// Define stepper motor connections and steps per revolution:
#define dirPin_R 4
#define stepPin_R 5
#define dirPin_L 8
#define stepPin_L 9
#define stepsPerRevolution 6*1600 // if ratio is 1/16, 1600 steps per 0.5 rev
#define stepLength 26000
#define minLength 0
#define middleStep 15000
#define GAP 2000
// 초기화 한 이후의 고정된 최대 최소값. 기기의 최대최소
#define max_val_default 20000
#define min_val_default 5000
// limit switch(end_stop_sw)
#define sw1 12
#define sw2 13
#define default_delay 500
// Define receiver pin
// IR receiver에서 Y가 센서의 output R: vcc G: Ground
#define RECV_PIN A0 
IRrecv irrecv (RECV_PIN);
decode_results results;

// limit sw(endstop) 을 위한 변수
bool sw1_state = false;
bool sw2_state = false;
// 적외선센서에서 제대로된값이 수신되었을경우에만 인터럽트
bool inter_state=HIGH; 
// 회전할때의 위치. 최대 최소 비교할때 씀. 
int r_state=0; 
int l_state=0;
// 회전할 때 방향. True=High=clock=down, False=LOW=counterclock=up
bool r_dir=HIGH;
bool l_dir=LOW;
// 적외선 리모트 컨트롤러로 변화시킬 변수
int state=0;
// 스텝모터 스피드
const int motor_delay[9] = {150,200,250,375,500,750,1000,1250,1500};
int delay_i=4;
// 초기화한 이후의 최대 최소값. 고정 무빙의 최대최소
int max_val=0;
int min_val=0;
int mid_val=middleStep;
// 비교를 위한 값
int r_max_val=0;
int l_max_val=0;
int r_min_val=0;
int l_min_val=0;

// 현재 입력하고 있는 것
int input_state = 0; // 0은 아무것도 입력받지 않음. 1은 max 2는




/////////////////////// setup function ////////////////////////////
void setup() {
  // Declare pins as input and output:
  pinMode(stepPin_R, OUTPUT);
  pinMode(dirPin_R, OUTPUT);
  pinMode(stepPin_L, OUTPUT);
  pinMode(dirPin_L, OUTPUT);
  pinMode(sw1,INPUT);
  pinMode(sw2,INPUT);
 
  // Declare pins as Interrupt 0,1
  pinMode(interruptPin1,INPUT_PULLUP); 
  pinMode(interruptPin2,INPUT_PULLUP); 
  pinMode(interruptPin3,OUTPUT);
  // 해당 pin(2,3)이 Falling이 되면 test_interrupt 실행, 나중에 digitalWrite로 바꿔보자.
  attachInterrupt(digitalPinToInterrupt(interruptPin1), test_interrupt, FALLING);
  attachInterrupt(digitalPinToInterrupt(interruptPin2), test_interrupt, FALLING);
  
  Serial.begin(9600);
  irrecv.enableIRIn(); //start receiver
  initialize(); // 나중에 다시 주석 취소 해라
}

////////////////////// loop function ////////////////////////////////
void loop() {
  irreceiver();
  if (state==0){      //0 stop, 초기화 되었을때로 돌아감.
    // 처음설정한 중앙값으로 돌아감
    motor_stop();
  }else if(state==1){ //1 start, 움직임
    //motor_start();
    motor_move();    
  }else if(state==2){ //2 Pause, 현재상태로 멈춤
    //motor_pause();
  }else{
    //error
  }
}
/////////////////////////////////////////////////////////////////////////////////
////////////////////////////// Other Functions  /////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
void initialize(){ // 한발만 실행
    
  Serial.println("initialize start");
  sw1_state=false;
  sw2_state=false;
  // Set the spinning direction counter clockwise:(LOW), UP
  digitalWrite(dirPin_R, LOW);
  digitalWrite(dirPin_L, LOW);

  for (int i = 0; i < stepLength; i++) {
    if(digitalRead(sw1)==LOW){
      sw1_state = true;
    }
    if(digitalRead(sw2)==LOW){
      sw2_state = true;
    }
    if(sw1_state == false){
      digitalWrite(stepPin_L, HIGH);
    }
    if(sw2_state == false){
      digitalWrite(stepPin_R, HIGH);
    }
    delayMicroseconds(default_delay);
    digitalWrite(stepPin_R, LOW);
    digitalWrite(stepPin_L, LOW);
    if(sw1_state == true && sw2_state==true){
      break;
    }
  } //clock wise for statemend end.
  
  state=0;
  r_state=stepLength;
  l_state=stepLength;
  max_val=max_val_default;           // # define max_val_default 26000
  min_val=min_val_default;            // # define min_val_default 0
  r_dir = LOW;
  l_dir = HIGH;
  Serial.println("initialize done");
}
//////////////////////////// 함 수 분 리 선 ///////////////////////////////////
void motor_stop(){ //중앙으로 돌아가서 멈춰있음.
  //중앙이 아니면, 중앙으로 가는 함수
  if (l_state==mid_val){
    //nothing
  }else if(l_state>mid_val){// 중앙값보다 크면 가운데로 돌아감
    digitalWrite(dirPin_L, HIGH);
    digitalWrite(stepPin_L, HIGH);
    delayMicroseconds(motor_delay[delay_i]);
    digitalWrite(stepPin_L, LOW);
    delayMicroseconds(motor_delay[delay_i]);
    l_state--;
  }else{  // l_state<mid_val, low 가 up
    digitalWrite(dirPin_L, LOW);
    digitalWrite(stepPin_L, HIGH);
    delayMicroseconds(motor_delay[delay_i]);
    digitalWrite(stepPin_L, LOW);
    delayMicroseconds(motor_delay[delay_i]);
    l_state++;
  }
  
    if (r_state==mid_val){
    //nothing
  }else if(r_state>mid_val){// 중앙값보다 크면 가운데로 돌아감
    digitalWrite(dirPin_R, HIGH);
    digitalWrite(stepPin_R, HIGH);
    delayMicroseconds(motor_delay[delay_i]);
    digitalWrite(stepPin_R, LOW);
    delayMicroseconds(motor_delay[delay_i]);
    r_state--;
  }else{  // l_state<mid_val, low 가 up
    digitalWrite(dirPin_R, LOW);
    digitalWrite(stepPin_R, HIGH);
    delayMicroseconds(motor_delay[delay_i]);
    digitalWrite(stepPin_R, LOW);
    delayMicroseconds(motor_delay[delay_i]);
    r_state++;
  }
}
//////////////////////////// 함 수 분 리 선 ///////////////////////////////////

void motor_move(){ // 1 step move
   // state를 계속 비교하면서 max, min 값이면 반대방향
  if (r_state>=max_val){
    r_dir = HIGH;
    r_state--;
    digitalWrite(dirPin_R, r_dir);
    digitalWrite(stepPin_R, HIGH);
  }else if(r_state<=min_val){
    r_dir = LOW;
    r_state++;
    digitalWrite(dirPin_R, r_dir);
    digitalWrite(stepPin_R, HIGH);
  }else{
    if(r_dir != l_dir){
      if(r_dir){
        r_state--; 
      }else{ 
        r_state++;
      }
      digitalWrite(dirPin_R, r_dir);
      digitalWrite(stepPin_R, HIGH);
    }else{
      r_dir = !r_dir;
    }
  }

  if (l_state>=max_val){
    l_dir = HIGH;
    l_state--;
    digitalWrite(dirPin_L, l_dir);
    digitalWrite(stepPin_L, HIGH);
  }else if(l_state<=min_val){
    l_dir = LOW;
    l_state++;
    digitalWrite(dirPin_L, l_dir);
    digitalWrite(stepPin_L, HIGH);
  }else{
    if(r_dir != l_dir){
      if(l_dir){
        l_state--; 
      }else{ 
        l_state++;
      }
      digitalWrite(dirPin_L, l_dir);
      digitalWrite(stepPin_L, HIGH);
    }  
  }
  delayMicroseconds(motor_delay[delay_i]);
  digitalWrite(stepPin_R, LOW);
  digitalWrite(stepPin_L, LOW);
  delayMicroseconds(motor_delay[delay_i]);
  
}
//////////////////////////// 함 수 분 리 선 ///////////////////////////////////

void motor_pause(){
  //아무것도 안함.
}
//////////////////////////// 함 수 분 리 선 ///////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////// 검증안된 기능 ///////////////////////////
///////////////////////////////////////////////////////////////////////////////

void motor_start(){
  // 회전할 때 방향. True=High=clock, False=LOW=counterclock
  //중앙에서 양발 다른 방향으로 회전 시작함.
  //한쪽 끝으로 가면 반대 방향으로 다시 회전
  digitalWrite(dirPin_R, r_dir);
  digitalWrite(dirPin_L, l_dir);
  for (int i = 0; i < max_val; i++) {
    digitalWrite(stepPin_R, HIGH);
    digitalWrite(stepPin_L, HIGH);
    delayMicroseconds(motor_delay[delay_i]);
    digitalWrite(stepPin_R, LOW);
    digitalWrite(stepPin_L, LOW);
    delayMicroseconds(motor_delay[delay_i]);
  }
  
  digitalWrite(dirPin_R, LOW);
  digitalWrite(dirPin_L, HIGH);
  for (int i = 0; i < max_val; i++) {
    digitalWrite(stepPin_R, HIGH);
    digitalWrite(stepPin_L, HIGH);
    delayMicroseconds(motor_delay[delay_i]);
    digitalWrite(stepPin_R, LOW);
    digitalWrite(stepPin_L, LOW);
    delayMicroseconds(motor_delay[delay_i]);
  }
}

/////////////////////// interrupt ////////////////////////////
void test_interrupt(){//수신부가 2번에 있음
  Serial.println("\tinterrupt!\n");
  digitalWrite(inter_state,HIGH);

}
void irreceiver(){
  //state는 적외선 리모트컨트롤러로 작동, 기본값 state= 0;
  //아무 값이 안들어오면 irrecv.decode(&results) 의 값은 0
  if(irrecv.decode(&results)){
    if(results.value==0xFFFFFFFF){
      //Serial.println(results.value,HEX); // 버튼 값 확인용
      // 오래누른거 확인용
    }else if ((results.value>>16)!= 0xFF){
      //nothing
    }else{
      Serial.println(results.value,HEX); // 버튼 값 확인용
      //digitalWrite(inter_state,LOW);  
      switch(results.value){ 
        case 0xFFA25D: 
          Serial.print("Stop\t");
          state=0;
          Serial.print("State: ");
          Serial.println(state);
          break; //전원       state 0, stop
        case 0xFFE21D: break; //메뉴      
        case 0xFF22DD:
          initialize();
          break; //테스트     initialize
        case 0xFFC23D: break; //뒤로가기
        case 0xFF02FD: 
        if(input_state == 1){
          Serial.println("add max_val");
          if(max_val < 26000){
            max_val += 1000;
          }
          Serial.println(max_val);
          Serial.println("\n");
        }else if(input_state == 2){
          Serial.println("add min_val");
          if(min_val >= 0 && min_val < max_val - GAP){
            min_val += 1000;
          }
          Serial.println(min_val);
          Serial.println("\n");
        }
        break; //+         speed, max, min up
        case 0xFF9867: 
        if(input_state == 1){
          Serial.println("subs max_val");
          if(max_val <= 26000 && max_val > min_val + GAP){
            max_val -= 1000;
          }
          Serial.println(max_val);
          Serial.println("\n");
        }else if(input_state == 2){
          Serial.println("subs min_val");
          if(min_val > 0){
            min_val -= 1000;
          }
          Serial.println(min_val);
          Serial.println("\n");
        }
        break; //-         speed, max, min down
        case 0xFFA857: 
          if(state == 1){ //if state is 1, change state into 2(pause)
            Serial.print("Pause");
            state=2;
            Serial.print("State: ");
            Serial.println(state);
          }else{        // if state is 0 or 2, change state into 1(start or move)
            Serial.print("Start");
            state=1;
            Serial.print("State: ");
            Serial.println(state);
          }
          break; //재생       
        case 0xFFE01F: //뒤로점프    speed down #define default_delay 500
          if(delay_i<8){ //motor_delay[delay_i] = {100,125,250,375,500,750,1000,1500,2000};
            delay_i++;
          }
          break; 
        case 0xFF906F:  //앞으로점프  speed
          if(delay_i>0){
            delay_i--;
          }
          break;
        case 0xFFB04F: break; //C   
        case 0xFF6897: break; //0         
        case 0xFF30CF: 
          input_state = 1;
          Serial.println("input_state : ");
          Serial.println(input_state);
          Serial.println("\n");
          break; //1         max
        case 0xFF18E7: 
          input_state = 2;
          Serial.println("input_state : ");
          Serial.println(input_state);
          Serial.println("\n");
          break; //2         min
        case 0xFF7A85: 
          break;
        case 0xFF10EF: break; //4
        case 0xFF38C7: break; //5
        case 0xFF5AA5: break; //6
        case 0xFF42BD: break; //7
        case 0xFF4AB5: break; //8
        case 0xFF52AD: break; //9
        case 0xFFFFFFFF: break; //꾹누르고 있으면
        
        default: break;
      }  
    }
  delayMicroseconds(1000);
  irrecv.resume();//이걸해야 다시 수신 가능
  }
}

/////////////////////////// speed  /////////////////////////
void speed_change(){ //button 2,3
  
}
/////////////////////////// max,min ////////////////////////
void max_val_change(){ // button 4,5
  
}
void min_val_change(){ // button 6,7
  
}
