#include <Bridge.h>
#include <HttpClient.h>

String form = "http://studenthome.hku.nl/~tariq.bakhtali/IAD4/form_thing.php?";
String username = "og";
String password = "dank";

int buttonPin[] = {12, 13}; // {FALSE, TRUE}
int ledPin1[] = {3, 5, 6}; // {R, G, B} dit zijn de PWM pins
int ledPin2[] = {9, 10}; // {R, G} dit zijn de PWM pins
int r1, g1, b1;
int r2, g2;

unsigned long previousMillis = 0;

String sessid;
int votesFake, votesCorrect;
int votesTotal;
boolean loggedIn = false;
HttpClient http;

void setup() {
  r1 = g1 = b1 = 0;
  r2 = g2 = 0;

  Serial.begin(9600);
  //while (!Serial);

  Serial.println("Starting Bridge...");
  pinMode(13, OUTPUT);
  digitalWrite(13, LOW);
  Bridge.begin();
  digitalWrite(13, HIGH);
  Serial.println("Bridge started!");

  for (int i = 0; i < 2; i++) {
    pinMode(buttonPin[i], INPUT);
    pinMode(ledPin2[i], OUTPUT);
    digitalWrite(ledPin2[i], LOW);
  }
  for (int i = 0; i < 3; i++) {
    pinMode(ledPin1[i], OUTPUT);
    digitalWrite(ledPin1[i], LOW);
  }
}

void loop() {
  if (!loggedIn) {
    pending(); // terwijl hij aan het inloggen is laat hij een lampje knipperen
    loggedIn = login();
  }

  if (loggedIn) {
    succes();
    vote();
  }

  setColor1(r1, g1, b1);
  setColor2(r2, g2);
}

void vote() {
  // wanneer een knop wordt ingedrukt maken we een lampje blauw ...
  if (digitalRead(buttonPin[0]) == HIGH) { // FAKE NEWS
    b1 = 255;
    http.get(form + "sessid=" + sessid + "&value=" + 0); 
    Serial.println("fake");
  } else if (digitalRead(buttonPin[1]) == HIGH) { // TRUE NEWS
    b1 = 255;
    http.get(form + "sessid=" + sessid + "&value=" + 1); 
    Serial.println("true");
  }
  else { 
    // ... maar wanneer er geen knop wordt ingedrukt gebruiken we de lampjes om de score te laten zien
    voteStatus();
  }
}

void voteStatus() {
  // om de 5 sec vragen we de "score" van het artikel op, dus hoe positief of negatief staat men tegenover het artikel.
  
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= 5000) {
    previousMillis = currentMillis;

    http.get(form + "sessid=" + sessid + "&status=" + 0);
    String responseFake = "";
    getResponse(http, responseFake);
    votesFake = responseFake.toInt();

    http.get(form + "sessid=" + sessid + "&status=" + 1);
    String responseCorrect = "";
    getResponse(http, responseCorrect);
    votesCorrect = responseCorrect.toInt();
  }
  votesTotal = votesFake + votesCorrect;

  // ik had het geprobeerd te mappen, maar dat werkte niet. 
  // zelfs in donker licht en een score van 20/1 was er geen verschil te zien.
  // dus er gaat hier iets mis.
  r1 = map(votesFake, 0, votesTotal, 0, 255);
  g2 = map(votesCorrect, 0, votesTotal, 0, 255);
}

boolean login() {
  // deze login werkt niet 'echt', hij zal altijd inloggen
  
  http.get(form + "username=" + username + "&password=" + password);

  String response;
  getResponse(http, response);

  if (response == "login0") {
    Serial.println("unkown username and/or password");
    return false;
  } else {
    Serial.println("login succesfull");
    sessid = response;
    Serial.println(response);
    return true;
  }
}

void pending() {
  // laat een lampje knipperen
  
  Serial.println("pending");
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= 500) {
    previousMillis = currentMillis;
    if (r1 == 0) {
      r1 = 255;
      g1 = 65;
    } else {
      r1 = g1 = 0;
    }
  }
}

void succes() {
  resetColors();
  unsigned long currentMillis = millis();
  if (currentMillis <= 5000) {
    b1 = 255;
  }
}

void getResponse(HttpClient& http, String& response) {
  while (http.available()) {
    char c = http.read();
    response += c;
  }
}

void setColor1(int& r, int& g, int& b) {
  analogWrite(ledPin1[0], r);
  analogWrite(ledPin1[1], g);
  analogWrite(ledPin1[2], b);
}

void setColor2(int& r, int& g) {
  analogWrite(ledPin2[0], r);
  analogWrite(ledPin2[1], g);
}

void resetColors() {
  r1 = g1 = b1 = 0;
  r2 = g2 = 0;
}
