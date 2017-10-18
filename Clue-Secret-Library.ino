/***
  Concept: Smart Library in the CLUE Game

  Functionalities:
  -Uploading a book to an online recomendation list when the book
  is placed on a specific area of the bookshelf
  -Secret door leading to a hidden room
  -Warning buzzer for earthquake when alert is issued
  -Access to zombie-tolerant hidden room when zombies alert is issued

  Attribution: The following libraries were created by Andy Davidson
  -SoftwareSerial.h
  -AlertNodeLib.h
  -Button.h
  -Servo.h
***/

// initialize libraries
#include <SoftwareSerial.h>
#include <AlertNodeLib.h>
#include <Button.h>
#include <Servo.h>

// call a constant to identify my node to the library object
const String myNodeName = "Node Clem";

// connect this program to the XBee module
// must use pin 2 for RX and pin 3 for TX
// set XBee to 9600 baud
AlertNode myNode;

// variables that won't change:
const int buzzPin = 4;
const int insideBtn = 5;
const int uploadLED = 6;
const int secretLED = 8;
const int secretBtn = 10;
const int alertLED = 11;
const int hallPin = 12;

// variables that will change:
int field;
int action1;
int action2;
int pos = 0;
int hallPrev = HIGH;
int occupiedPrev = 0;

// set initial occupancy state to be none
bool isOccupied = 0;

// set initial upload state to nothing
bool isUploaded = 0;

// initialize button object
Button btn1 (secretBtn);
Button btn2 (insideBtn);

// initialize servo object
Servo myservo;

// function for playing warning tone when earthquake alert is detected
void playSound () {
  int i;
  // number of times the buzzer will sound in response to earthquake
  int soundDuration = 8;
  for (i = 0; i <= soundDuration; i++) {
    tone(buzzPin, 330, 250);
    delay(250);
    tone(buzzPin, 1047, 250);
    delay(250);
  }
}

// function for Wizard of Oz-ing data upload when
// the magnet (aka book) is placed next to Hall Effect sensor
void uploadEffect() {
  Serial.println("Uploading your recommended book...");
  int i;
  for (i = 0; i < 5; i++) {
    digitalWrite(uploadLED, HIGH);
    delay(100);
    digitalWrite(uploadLED, LOW);
    delay(100);
  }
  Serial.println("\n\n\nThis book is added to your recommended list.");
}

// default debugging to true
boolean myDebugging = true;

// function for logging the alerts when system receive them
void logAlert (String myName, int alert) {

  // just blink the led and write a message out to the serial monitor
  digitalWrite(alertLED, HIGH);
  delay(100);

  if (myDebugging) {
    Serial.print("*** alert received at ");
    float sec = millis() / 1000.0;
    Serial.println(sec);
    Serial.print("***   node = ^");
    Serial.print(myName);
    Serial.println("^");
    Serial.print("***   alert = ");
    Serial.print(alert);
    Serial.print(": ");
    Serial.println(myNode.alertName(alert));
  }

  // end of the blink
  digitalWrite(alertLED, LOW);

}

// function for opening secret door
void openSecretDoor () {
  digitalWrite(secretLED, HIGH);
  for (pos = 0; pos <= 90; pos += 1) {
    myservo.write(pos);
    delay(50);
  }
  Serial.println("Secret door is open!");
  delay(1000);
}

// function for closing secret door
void closeSecretDoor () {
  digitalWrite(secretLED, LOW);
  for (pos = 90; pos >= 0; pos -= 1) {
    myservo.write(pos);
    delay(50);
  }
  Serial.println("Secret door is closed!");
  delay(1000);
}

void setup() {

  // attach servo object to pin 9 to start out
  myservo.attach(9);

  // initialize the following as input:
  pinMode(hallPin, INPUT);
  pinMode(insideBtn, INPUT);
  pinMode(secretBtn, INPUT);

  // initialize the following as output:
  pinMode(buzzPin, OUTPUT);
  pinMode(alertLED, OUTPUT);
  pinMode(uploadLED, OUTPUT);
  pinMode(secretLED, OUTPUT);

  // initialize Serial Monitor at 9600 baud
  Serial.begin(9600);
  Serial.println("**Serial monitor is running**");
  Serial.println("\n\n*** Starting AlertNode demo");

  // this starts up your XBee and does other Alert initialization stuff.
  // the argument is just to identify your node by a name.
  myNode.begin(myNodeName);

  // let me know it's time to test all equipment
  Serial.println("\n\nTesting equipment now");

  // execute playSound() to make sure the buzzer has initialized
  playSound();

  // flash alertLED to make sure it has initialized
  digitalWrite(alertLED, HIGH);
  delay(1000);
  digitalWrite(alertLED, LOW);

  // execute uploadEffect() to make sure the function has initialized
  uploadEffect();

  // flash tiltLED to make sure it has initialized
  digitalWrite(secretLED, HIGH);
  delay(1000);
  digitalWrite(secretLED, LOW);

  // reset motor to starting position
  myservo.write(pos);

  Serial.println("\n\nEnd of equipment testing");
  delay(5000);
}

void loop () {

  //****READ INPUTS****

  // check if the button is pressed from inside the secret room
  action1 = btn1.checkButtonAction();
  Serial.print("\n\ncurrent secret button status: ");
  Serial.println(action1);
  delay(500);

  // read the state of the tilt sensor into the local variable
  action2 = btn2.checkButtonAction();
  Serial.print("\ncurrent inside button status: ");
  Serial.println(action2);
  delay(500);

  // read Hall Effect sensor
  field = digitalRead(hallPin);
  Serial.print("\ncurrent Hall Effect sensor status: ");
  Serial.println(field);
  delay(500);

  // check occupancy status
  Serial.print("\n\nIs it occupied?: ");
  Serial.println(isOccupied);
  delay(500);

  //****ANALYZE INPUTS****

  // check if the room status has changed
  if (isOccupied != occupiedPrev) {
    // update from last occupancy status
    // if someone enters the room
    occupiedPrev = isOccupied;
    Serial.print("\nNew room state: ");
    Serial.println(occupiedPrev);
  } else {
    // tell me nothing has changed
    Serial.println("Nothing's changed.");
  }

  // check if the magnetic field has been detected
  if (field == LOW) {
    // check if current status is different from previous
    if (field != hallPrev) {
      // update status of upload if someone
      // added/removed book from recommended section
      isUploaded != isUploaded;
      // tell me about the state change
      Serial.print("\nIs it uploaded?: ");
      Serial.println(isUploaded);
    } else {
      // tell me nothing has changed
      Serial.println("\nYou haven't uploaded a NEW book yet.");
    }
  }

  //****EXECUTE OUTPUTS****

  // secret button has been HELD and
  // there's no one in the secret room
  if (action1 == Button::HELD && occupiedPrev == 0) {

    // attach servo motor first after being "disabled"
    myservo.attach(9);

    // tell me that the motor is attached
    Serial.println("\n\n**servo is attached**");

    // open and close secret door
    openSecretDoor();
    delay(3000);
    closeSecretDoor();

    // change status of the secret room
    isOccupied = 1;

    // if someone is in the secret room
  } else if (occupiedPrev == 1) {

    // disable the servo motor
    myservo.attach(7);

    // tell me that the motor disabled
    Serial.println("\n**servo detached**");

    // tell me why nothing is happening
    Serial.println("The secret room is occupied so SECRET button doesn't work.");
    delay(500);

    // if someone has left the secret room
  } else if (occupiedPrev == 0) {

    // let me know the secret button works again
    Serial.println("The secret room isn't occupied so SECRET button works.");
  }

  // inside button has been HELD
  if (action2 == Button::HELD && occupiedPrev == 1) {

    // re-enable the servo motor
    myservo.attach(9);

    // tell me that the motor is attached
    Serial.println("\n\n**servo is attached**");

    // open and close secret door
    openSecretDoor();
    delay(3000);
    closeSecretDoor();

    // change the occupancy state of the room
    isOccupied = 0;

    // if someone has left the secret room
  } else if (occupiedPrev == 0) {

    // disable servo motor if no one is in the room
    myservo.attach(7);

    // tell me that the motor is disabled
    Serial.println("\n**servo detached**");

    // tell me why nothing is happening
    Serial.println("No one is in the secret room so INSIDE button doesn't work.");

    // if someone has entered the secret room
    // but they haven't pressed the inside button
  } else if (occupiedPrev == 1) {

    // let me know the inside button works again
    Serial.println("The secret room is occupied so INSIDE button works.");
  }

  // magnetic field is detected and state changed
  // from last time
  if (field != hallPrev) {

    // check if a book is already uploaded
    if (isUploaded == 0) {

      // if not, show book recommendation is being uploaded
      uploadEffect();

      // if the book is sitting on the recommended
      // area of the bookshelf
    } else if (isUploaded == 1) {

      // if the book is already uploaded
      // turn LED off
      digitalWrite(uploadLED, LOW);
    }

    // save the state
    hallPrev = field;
  }

  // check for an incoming alert
  // alertReceived() returns 0 (zero) if none have been sent to your zone.
  // it returns a code as follows if there is one:
  //       AlertNode::EARTHQUAKE
  //       AlertNode::ZOMBIE
  //       AlertNode::CLUE_LIB_CANDLE_MUSTARD
  //       AlertNode::CLUE_LIB_ROPE_PLUM

  // log the alert if the system detects it
  int alert = myNode.alertReceived();

  // when alert has been sent
  if (alert != 0) {
    // print out a message about what alert is detected
    logAlert(myNodeName, alert);

    // system reaction when EARTHQUAKE alert is detected:
    if (alert == AlertNode::EARTHQUAKE) {

      // blink alertLED to show alert was detected
      Serial.println("Earthquake! Take Cover!");
      digitalWrite(alertLED, HIGH);
      delay(1000);
      digitalWrite(alertLED, LOW);

      // play buzzer tone to warn people
      playSound ();
    }

    // system reaction when ZOMBIE alert is detected:
    if (alert == AlertNode::ZOMBIE) {

      // blink to show alert was detected
      digitalWrite(alertLED, HIGH);
      delay(1000);
      digitalWrite(alertLED, LOW);

      // opens and closes the Secret door
      openSecretDoor ();
      delay(3000);
      closeSecretDoor ();

      // change status of the secret room
      isOccupied = 1;

      // make sure that the system knows the room is now occupied
      Serial.print("\nIs it occupied?: ");
      Serial.println(isOccupied);
      delay(500);
    }

    // system reaction when CLUE_LIB_CANDLE_MUSTARD alert is detected:
    if (alert == AlertNode::CLUE_LIB_CANDLE_MUSTARD) {

      // blink alertLED to show alert was detected
      Serial.println("COLONEL MUSTARD is in the LIBRARY with a CANDLEHOLDER");
      digitalWrite(alertLED, HIGH);
      delay(1000);
      digitalWrite(alertLED, LOW);
    }

    // system reaction when CLUE_LIB_ROPE_PLUM alert is detected:
    if (alert == AlertNode::CLUE_LIB_ROPE_PLUM) {

      // blink alertLED to show alert was detected
      Serial.println("PROFESSOR PLUM is in the LIBRARY with a ROPE");
      digitalWrite(alertLED, HIGH);
      delay(1000);
      digitalWrite(alertLED, LOW);
    }
  }

}
