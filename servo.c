#include <Servo.h>
#include <time.h>
#define horzservoPin 9
#define vertservoPin 8
#define updateFreq 1000*60*5 // 5 minutes

void init_servo(Servo servo);
void transition_angle(int *oldhAngle, int *oldvAngle, int newHorzAngle, int newVertAngle, Servo horzservo, Servo vertServo);

// some test movement for the servos, I think it looks cool
void init_servo(Servo servo)
{
  servo.write(90);
  delay(100);
  servo.write(180);
  delay(100);
  servo.write(0);
  delay(100);
  // Sweep from 0 to 180 degrees:
  for (angle = 0; angle <= 180; angle += 1) {
    servo.write(angle);
    delay(15);
  }
  // And back from 180 to 0 degrees:
  for (angle = 180; angle >= 0; angle -= 1) {
    servo.write(angle);
    delay(30);
  }
  delay(1000);
}

// Transition between angles function (so that transition is smooth)
void transition_angle(int *oldhAngle, int *oldvAngle, int newHorzAngle, int newVertAngle, Servo horzservo, Servo vertServo)
{
  int hangle = oldhAngle;
  int vangle = oldvAngle;

  while (hangle <= newHorzAngle) || (vangle <= newVertAngle)
  {
    if (hangle <= newHorzAngle)
    {
      servo.write(hangle);
      hangle++;
    }
    if (vangle <= newVertAngle)
    {
      servo.write(vangle);
      vangle++;
    }
    delay(15);
  }
  *oldhAngle = newHorzAngle;
  *oldvAngle = newVertAngle;
}


// Create a new servo object:
Servo horzServo;
Servo vertServo;

// variables to store servo positions
int oldHorzAngle = 0;
int newHorzAngle = 0;
int oldVertAngle = 0;
int newVertAngle = 0;


void setup() {
  printf("---- Setting up ---- \n")
  time_t start_time = time(NULL);

  // Attach the Servo variable to a pin:
  horzServo.attach(horzservoPin);
  vertServo.attach(vertservoPin);

  // test movement in Servos
  init_servo(horzServo);
  init_servo(vertServo);

  time_t finish_time = time(NULL);
  printf("---- Finished set up %f s ----\n", difftime(finish_time, start_time));
}

void loop() {
  /* TODO: get angles from curlapp through a call to the function
   if it can be run in the Arduino, or through Serial connection (then need to decode)
   */

  transition_angle(&oldHorzAngle, &oldVertAngle, newHorzAngle, newVertAngle, horzServo, vertServo);
  delay(updateFreq);
}