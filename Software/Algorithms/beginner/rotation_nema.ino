#define IN1 13  
#define IN2 12  
#define IN3 23 
#define IN4 5   


int stepSequence[4][4] = {
  {1, 0, 1, 0},  // Step 1: A+ and B+
  {0, 1, 1, 0},  // Step 2: A- and B+
  {0, 1, 0, 1},  // Step 3: A- and B-
  {1, 0, 0, 1}   // Step 4: A+ and B-
};

int currentStep = 0;  // Track the current step

void setup() {
  // Initialize L298N motor driver pins
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  Serial.begin(115200);
  Serial.println("NEMA 17 Motor Control Initialized");
}

void loop() {
  rotateMotor(200);  // Rotate 360° clockwise
  delay(1000);       // Pause for 1 second
}

void rotateMotor(int steps) {
  int direction = (steps > 0) ? 1 : -1; 
  steps = abs(steps); 

  for (int i = 0; i < steps; i++) {
    stepMotor(direction > 0);  
    delay(50);   
  }
}

void stepMotor(bool forward) {
  digitalWrite(IN1, stepSequence[currentStep][0]);
  digitalWrite(IN2, stepSequence[currentStep][1]);
  digitalWrite(IN3, stepSequence[currentStep][2]);
  digitalWrite(IN4, stepSequence[currentStep][3]);

  if (forward) {
    currentStep = (currentStep + 1) % 4; 
  } else {
    currentStep = (currentStep - 1 + 4) % 4;  
  }
}
