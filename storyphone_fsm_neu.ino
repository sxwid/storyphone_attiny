/* STORYPHONE 
 *  
 *  Plays Line Sound on POTS
 *  Plays selectable MP3 from USB Stick
 *  allows to change Volume
 *  
 *  Use AttinyCore (http://drazzy.com/package_drazzy.com_index.json) for tone() functionality
 *  
 *  PB0: LED for Debugging
 *  PB1: Output for tone()
 *  PB2: detects Fork States and Inputs of Dial Wheel
 *  PB3 / PB4: Serial Communication with DFPlayer
 *  
 *  (c) 2022 simon.widmer@mailbox.org
 *  Schematics: https://github.com/sxwid/storyphone
 *  
 *  Issues: Slight Distortions on audio signal... Better shielding needed?
 */

#include <Fsm.h>
#include <SoftwareSerial.h>
#include "DFRobotDFPlayerMini.h"

// Events FSM
#define FORK_LIFT 0
#define FORK_RISING 1
#define FORK_FALLING 2
#define HANGUP 3
#define DIAL_COMPLETE 4
#define ERROR_TR 5
#define LED_TOGGLE 6

#define LED_OFF 0
#define LED_1 50
#define LED_2 100
#define LED_3 400
#define LED_4 800
#define LED_5 2000

//#define DEBUG 1

SoftwareSerial mySoftwareSerial(4, 3); // RX, TX
DFRobotDFPlayerMini myDFPlayer;

const int O_LED = 0;     // LED Out auf PB0
const int O_AUDIO = 1;   // Signalausgabe auf PB1
const int I_FORK = 2;    // Auswertung Gabel auf PB2

bool last_forkstate; // last state of fork
int  dial_counter = 0;
unsigned long lastchange = 0; //last time fork changed

int led_delay = LED_1;
unsigned long led_lastchange = 0; //last time led changed

// Timings
int pulse = 60; //ms
int pulse_min = pulse*0.9; //ms
int pulse_max = pulse*1.2; //ms
int pause = 40; //ms
int pause_min = pause*0.9; //ms
int pause_max = pause*1.1; //ms
int period_max = 10* (pulse_max + pause_max);

//DF Player Volume (possibly to low?!)
int df_vol = 20;

//FSM Setup
State state_idle(&on_idle_enter, &check_fork_falling, NULL);
State state_forklift(&on_forklift_enter, &check_fork_rising, &on_forklift_exit);
State state_play(&on_play_enter, &check_fork_rising, &on_play_exit);
State state_impuls(&on_impuls_enter, &check_fork_state, NULL);
Fsm fsm(&state_idle);

State state_LED_off(&LED_off, &check_LED, NULL);
State state_LED_on(&LED_on, &check_LED, NULL);
Fsm fsm_led(&state_LED_on);

// regular Functions **********************************************
void signal_line(){
  tone(O_AUDIO,440);
}

void signal_error(){
  tone(O_AUDIO,800);
}

void blink_led(int nb = 1){
  digitalWrite(O_LED,LOW);
  delay(200);
  while(nb>=1){
    digitalWrite(O_LED,HIGH);
    delay(100);
    digitalWrite(O_LED,LOW);
    delay(150);
    nb -= 1;    
  }
}


//FSM LED Functions **********************************************
void LED_off(){
  digitalWrite(O_LED, LOW);
}

void LED_on(){
  if(led_delay > 0){
    digitalWrite(O_LED, HIGH);
  }
}

void check_LED(){
  if(millis() > (led_lastchange + led_delay)){
    led_lastchange = millis();
    fsm_led.trigger(LED_TOGGLE);
  }
}


//FSM  Functions **********************************************
void check_fork_state(){
  //Handle Timeout means completed action
  if(millis() > (lastchange + pulse_max)){
    // No complete pulse detected, hanged up
    if (dial_counter == 0){
      #ifdef DEBUG 
        Serial.println("TRIGGER HANGUP");
      #endif
      fsm.trigger(HANGUP);
    }
    if(dial_counter > 0 && dial_counter <=10){
      #ifdef DEBUG
        blink_led(dial_counter);
        Serial.print("DIAL COMPLETE, Playing: ");
        Serial.println(dial_counter);
      #endif
      fsm.trigger(DIAL_COMPLETE);
    }
    if((dial_counter > 10) && digitalRead(I_FORK)== HIGH){
      #ifdef DEBUG
        Serial.print("ERROR");
        Serial.println(dial_counter);
      #endif
      fsm.trigger(ERROR_TR);
    }
  }
  else{
    int fork_state = digitalRead(I_FORK);
    // Only take action if forkstate changed
    if (fork_state != last_forkstate){
      last_forkstate = fork_state;
      lastchange = millis();
      if (fork_state == LOW) {
        // We saw a falling edge, add 1 to dial_counter
        dial_counter += 1;
      }  
      if (fork_state == HIGH) {
        // We saw a rising edge, do nothing because we count on falling edge
      } 
    }
  }
}

void check_fork_falling(){
  int fork_state = digitalRead(I_FORK);
  if (fork_state == LOW) {
    fsm.trigger(FORK_LIFT);
  }
}

void check_fork_rising(){
  int fork_state = digitalRead(I_FORK);
  if (fork_state == HIGH) {
    fsm.trigger(FORK_RISING);
  }
}

void on_idle_enter(){
  #ifdef DEBUG
    Serial.println("Enter IDLE");
  #endif
  led_delay = LED_4;
  noTone(O_AUDIO);
  myDFPlayer.pause();
}

void on_forklift_enter(){
  #ifdef DEBUG
    Serial.println("Enter FORKLIFT");
  #endif
  led_delay = LED_OFF;
  signal_line();
  myDFPlayer.pause();
}

void on_forklift_exit(){
  #ifdef DEBUG
    Serial.println("Exit FORKLIFT");
  #endif
  lastchange = millis();
  dial_counter = 0;
}

void on_play_enter(){
  #ifdef DEBUG
    Serial.println("Enter PLAY");
  #endif
  led_delay = LED_5;
  myDFPlayer.play(dial_counter);
}

void on_play_exit(){
  #ifdef DEBUG
    Serial.println("Exit PLAY");
  #endif
  led_delay = LED_5;
  lastchange = millis();
  dial_counter = 0;
}

void on_impuls_enter(){
  //Serial.println("Enter IMPULS");
  led_delay = LED_OFF;
  noTone(O_AUDIO);
  myDFPlayer.pause();
}

void setup() {
  #ifdef DEBUG
    Serial.begin(115200);
    Serial.println("");
    Serial.println("Start");
  #endif
  
  mySoftwareSerial.begin(9600);
  
  pinMode(O_LED, OUTPUT);
  pinMode(I_FORK, INPUT);
  pinMode(O_AUDIO, OUTPUT);

  blink_led(1);
  
  //todo necessary? DFPlayer needs some time...
  delay(2000);

  //FSM Transistions 
  fsm.add_transition(&state_idle, &state_forklift,FORK_LIFT, NULL);
  fsm.add_transition(&state_forklift, &state_impuls,FORK_RISING, NULL);
  fsm.add_transition(&state_play, &state_impuls, FORK_RISING, NULL);
  fsm.add_transition(&state_impuls, &state_idle, HANGUP, NULL);
  fsm.add_transition(&state_impuls, &state_play, DIAL_COMPLETE, NULL);
  fsm.add_transition(&state_impuls, &state_forklift, ERROR_TR, NULL);

  //FSM Transistions 
  fsm_led.add_transition(&state_LED_off, &state_LED_on, LED_TOGGLE, NULL);
  fsm_led.add_transition(&state_LED_on, &state_LED_off, LED_TOGGLE, NULL);


  //Initialize Forkstate
  last_forkstate = digitalRead(I_FORK);

  blink_led(2);
  
  //Initialize DF Player
  if (!myDFPlayer.begin(mySoftwareSerial)) {  //Use softwareSerial to communicate with mp3.
    while(true){
      delay(0); // Code to compatible with ESP8266 watch dog.
    }
  }

  blink_led(3);
  
  myDFPlayer.outputDevice(DFPLAYER_DEVICE_U_DISK);
  myDFPlayer.volume(df_vol);
  myDFPlayer.EQ(DFPLAYER_EQ_CLASSIC);

}


void loop() {
  fsm_led.run_machine();
  fsm.run_machine();
  delay(1);
}
