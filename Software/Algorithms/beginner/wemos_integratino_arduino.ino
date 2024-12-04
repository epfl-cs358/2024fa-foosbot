#include <SimpleFOC.h> // Enables advanced motor control FOC

StepperMotor motor = StepperMotor(50); // Always 50 pole pairs
StepperDriver4PWM driver = StepperDriver4PWM(16, 27, 12, 13, 14, 5);
Encoder encoder = Encoder(39, 36, 2048);

void doA() { encoder.handleA(); }
void doB() { encoder.handleB(); }

Commander command = Commander(Serial);

// Use HardwareSerial for communication with Arduino Uno
HardwareSerial arduinoSerial(1); // Use UART1 (HardwareSerial instance)

void onMotor(char* cmd) { command.motor(&motor, cmd); }

void setup() {
    encoder.quadrature = Quadrature::ON; // Default
    encoder.pullup = Pullup::USE_EXTERN;
    encoder.init();
    encoder.enableInterrupts(doA, doB);
    motor.linkSensor(&encoder);

    motor.foc_modulation = FOCModulationType::SpaceVectorPWM;
    driver.voltage_power_supply = 12;
    driver.init();
    motor.linkDriver(&driver);

    motor.controller = MotionControlType::angle;

    motor.PID_velocity.P = 0.2;
    motor.PID_velocity.I = 20;
    motor.PID_velocity.D = 0;
    motor.LPF_velocity.Tf = 0.01;

    motor.voltage_limit = 12;
    motor.current_limit = 18;
    motor.P_angle.P = 20;
    motor.velocity_limit = 100;

    Serial.begin(9600); // Serial Monitor for debugging
    arduinoSerial.begin(9600, SERIAL_8N1, 25, -1); // RX = IO25, TX = IO17
    motor.useMonitoring(Serial);

    motor.init();
    motor.initFOC(); // Initializes the motor position as the starting point
    motor.target = 0;

    command.add('M', onMotor, "motor");
    delay(1000);
}

void loop() {
    motor.loopFOC();
    motor.move();
    command.run();

    // Read and process commands from Arduino Uno
    if (arduinoSerial.available() > 0) {
        String commandFromArduino = arduinoSerial.readStringUntil('\n'); // Read the message
        Serial.println("Received from Arduino: " + commandFromArduino); // Print to Serial Monitor

        // Example: Parse and execute a MOVE command
        if (commandFromArduino.startsWith("MOVE")) {
            float angle = commandFromArduino.substring(5).toFloat(); // Extract angle value
            motor.target = angle; // Set the motor's target angle
        }
    }
}
