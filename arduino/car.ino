#include <FlexiTimer2.h>
#include <SoftwareSerial.h>

#define NOP do { __asm__ __volatile__ ("nop"); } while (0)
#define LED0 22
#define LED1 23
#define LED2 24
#define LED3 25
#define LEDright 32
#define IN1M 28  //左轮
#define IN2M 29
#define IN3M 26  //右轮
#define IN4M 27
#define STBY 30
#define PWMA 2   //左轮
#define PWMB 3  //右轮
#define servoPin1 6   //爪子伸缩  90-30
#define servoPin2 12   //爪子开合 100开-140关
#define servoPin3 4   //云台  30-170
#define myTx 11    //软串口
#define myRx 10
#define velocity1 80
#define velocity2 20
#define velocity3 100

SoftwareSerial mySerial(myRx, myTx); // RX, TX
//1ms跳一次中断
const int INTERVAL = 5;
//接收灰度信息
int value_1 = 0;
int value_2 = 0;
int value_3 = 0;
int value_4 = 0;
int value_right = 0;
//中断标识符
int temp = 0;
//舵机闭合角度
int close_servo = 0;

//延时函数
void delay1()//自制软件延时1ms
{
  for(int j=0;j<1985;j++) NOP;    
}
void delay_(int ms)//自制软件延时
{
  for(int i=0; i<ms; i++)
  {
     for(int j=0;j<1985;j++) NOP;
  }        
}

void servo(int Pin, int angle) 
{ //定义一个脉冲函数  
  //发送50个脉冲  
  for(int i=0;i<50;i++){  
    int pulsewidth = (angle * 11) + 500; //将角度转化为500-2480的脉宽值  
    digitalWrite(Pin, HIGH);   //将舵机接口电平至高  
    delayMicroseconds(pulsewidth);  //延时脉宽值的微秒数  
    digitalWrite(Pin, LOW);    //将舵机接口电平至低  
    delayMicroseconds(20000 - pulsewidth);  
  }  
  delay_(100); //可能要删掉 
}  
//******爪子部分start*****//
void catch_block(int c)//抓取物块
{
  servo(servoPin3,25);//云台旋到物块一侧
  delay_(1000);
  servo(servoPin2,110);//打开爪子
  delay_(1000);
  servo(servoPin1,95);//伸出爪子
  delay_(1000);
  c=140;
  servo(servoPin2,c);//关上爪子
  delay_(1000);
  servo(servoPin1,35);//回收爪子
  delay_(1000);
  //servo(servoPin3,179);//云台旋到放置物块一侧
  go_right(velocity3);
  delay_(300);
}
void put_block()//放置物块
{
  servo(servoPin1,95);//伸出爪子
  delay_(1000);
  servo(servoPin2,110);//打开爪子
  delay_(1000);
  servo(servoPin1,35);//回收爪子
  delay_(1000);
  servo(servoPin2,140);//关上爪子
  delay_(1000);
  go_right(velocity3);
  delay_(300);
  servo(servoPin3,179);
  delay_(1000);
}
//******爪子部分end******//

//********运动控制start********//
void rotate_right(int v)//顺时针自旋
{
  digitalWrite(IN1M, 1);//左轮
  digitalWrite(IN2M, 0);
  analogWrite(PWMA, v);
  digitalWrite(IN3M, 0);//右轮
  digitalWrite(IN4M, 1);
  analogWrite(PWMB, v);
}
void go_stop()
{
  digitalWrite(IN1M, 0);//左轮
  digitalWrite(IN2M, 0);
  analogWrite(PWMA, 0);
  digitalWrite(IN3M, 0);//右轮
  digitalWrite(IN4M, 0);
  analogWrite(PWMB, 0);
}
void go_straight(int v)//直行
{
  digitalWrite(IN1M, 1);//左轮
  digitalWrite(IN2M, 0);
  analogWrite(PWMA, v);
  digitalWrite(IN3M, 1);//右轮
  digitalWrite(IN4M, 0);
  analogWrite(PWMB, v);
}
void go_left(int v)//右转
{
  digitalWrite(IN1M, 1);//左轮
  digitalWrite(IN2M, 0);
  analogWrite(PWMA, v);
  digitalWrite(IN3M, 0);//右轮
  digitalWrite(IN4M, 1);
  analogWrite(PWMB, v);
}
void go_right(int v)//左转
{
  digitalWrite(IN1M, 0);//左轮
  digitalWrite(IN2M, 1);
  analogWrite(PWMA, v);
  digitalWrite(IN3M, 1);//右轮
  digitalWrite(IN4M, 0);
  analogWrite(PWMB, v);
}
void motor_control(int value1,int value2,int value3,int value4,int value_right)//逻辑寻线
{
  //黑线高电平，白色低电平
  if(value_1==0&&value_2==0&&value_3==0) {
    go_right(velocity1);
  }
  else
  {
    if(value1==1){
      go_left(velocity1);
    }
    else if(value3==1) {
      go_right(velocity1);
    }  
    else if(value1==0&&value3==0) {
      go_straight(velocity1);
    }
  }
}
//********运动控制end********//

void interrupt_fun()
{
  if(temp==0)//标识符为0，一直循线
  {
    value_1 = digitalRead(LED0);   // read the input pin //digitalWrite(IN3M, 0);//反转
    value_2 = digitalRead(LED1);   // read the input pin //digitalWrite(IN4M, 1);
    value_3 = digitalRead(LED2);   // read the input pin //analogWrite(PWMB, velocity);
    value_4 = digitalRead(LED3);   // read the input pin //digitalWrite(IN1M, 0);//反转
    value_right = digitalRead(LEDright);
    //Serial.println(value_1);
    motor_control(value_1,value_2,value_3,value_4,value_right);
    temp=2;
  }
  //抓取圆柱部分start
  if(temp==1)
  {
    FlexiTimer2::stop();
    catch_block(close_servo);
    put_block();
    temp=0;
    close_servo=0;
    while(mySerial.available())
    {Serial.println(mySerial.read()); }
    //mySerial.print(3);
    FlexiTimer2::start();
  }
  //抓取圆柱部分end
  if(temp==2)//标识符为2，获取摄像头数据
  {
    int value_cam=0;
    /*if(mySerial.available()) 
    {
      value_cam=(int)mySerial.read();
      //Serial.println(value_cam);
    }*/
    if(mySerial.available()) 
    {
      value_cam=(int)mySerial.read();
      Serial.println(value_cam);
    }
    //检测到圆，停下进行抓取 
      /*if(value_cam==49) 
      {
        go_stop();
        close_servo=115;
        temp=1;
      }
      else if(value_cam==50) 
      {
        go_stop();
        close_servo=110;
        temp=1;
      }
      else if(value_cam==51) 
      {
        go_stop();
        close_servo=105;
        temp=1;
      }
      else if(value_cam==52) 
      {
        go_stop();
        close_servo=100;
        temp=1;
      }*/
      if(value_cam!=0) 
      {
        go_stop();
        close_servo=135;
        temp=1;
      }
      else 
        temp=0;
  }
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  mySerial.begin(115200);
  //读取灰度信息
  pinMode(LED0, INPUT);
  pinMode(LED1, INPUT); 
  pinMode(LED2, INPUT);   
  pinMode(LED3, INPUT);
  pinMode(LEDright, INPUT);
  //舵机控制引脚
  pinMode(servoPin1, OUTPUT);//控制舵机伸缩
  servo(servoPin1,30);//复位
  pinMode(servoPin2, OUTPUT);//控制爪子开合
  servo(servoPin2,140);//复位
  pinMode(servoPin3, OUTPUT);//控制云台旋转
  servo(servoPin3,170);//复位
  //电机驱动引脚
  pinMode(IN1M, OUTPUT);//控制左电机1的方向，01为正转，10为反转
  pinMode(IN2M, OUTPUT);
  pinMode(IN3M, OUTPUT);//控制右电机2的方向，01为正转，10为反转
  pinMode(IN4M, OUTPUT);
  pinMode(PWMA, OUTPUT);//左电机PWM
  pinMode(PWMB, OUTPUT);//右电机PWM
  pinMode(STBY, OUTPUT);//驱动模块使能
  //初始化电机驱动模块
  digitalWrite(IN1M, 1);
  digitalWrite(IN2M, 0);
  digitalWrite(IN3M, 1);
  digitalWrite(IN4M, 0);
  digitalWrite(STBY, 1);
  analogWrite(PWMA, 0);
  analogWrite(PWMB, 0);
  FlexiTimer2::set(INTERVAL,interrupt_fun);//设置中断
  FlexiTimer2::start();//开启中断
}

void loop() {
  // put your main code here, to run repeatedly:
  /*{
    servo(servoPin1,30);
    //最近30,最远100//Pin1
    //最宽65,最窄100/Pin2
    delay(100);
  }*/
  /*for(int i=0;i<100;i++)
  { servo(servoPin3,i);//打开爪子
    delay(1000);
  }*/

  //Pin 1:20-90
  //Pin 3:30-170

 // delay(1000)
  //delay(1000);
          //直走
         //rotate_right(velocity1);
          //go_straight(velocity1);
          //servo(servoPin3,10);//云台旋到放置物块一侧
          //servo(servoPin2,110);//关上爪子
          //servo(servoPin1,95);//回收爪子
        /*catch_block(140);
    delay_(300);
    put_block();*/
    /*int value_cam=0;
    if(mySerial.available()) 
    {
      go_straight(velocity1);
      value_cam=(int)mySerial.read();
      Serial.println(value_cam);
    }*/
}
