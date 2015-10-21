// Tao Du
// taodu@csail.mit.edu
// Octo 18, 2015
//
// Implement a sample controller.

#include <AP_HAL/AP_HAL.h>
#include <AP_InertialSensor/AP_InertialSensor.h>
#include <PID/PID.h>

const AP_HAL::HAL& hal = AP_HAL::get_HAL();

// Inertial Sensor.
AP_InertialSensor ins;

// Yaw angles.
float yaw = 0.0;
// Timer.
int count = 0;
// PID controller.
PID pid;

// Constants.
// Number of propellers.
const int prop_num = 4;
// Minimal pwm value.
const uint16_t pwm_min = 1000;
// For safety reasons, we don't allow pwm to go beyond 1700.
const uint16_t pwm_max = 1700;
// Minimal and maximal yaw.
const uint16_t yaw_min = 1045;
const uint16_t yaw_max = 1865;

float map(float x, float in_min, float in_max, float out_min, float out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void setup(void) {
    // Initialize the inertial sensor.
    ins.init(AP_InertialSensor::RATE_100HZ);

    // Enable outputs to all the four rotors.
    for (int i = 0; i < prop_num; ++i) {
        hal.rcout->enable_ch(i);
        hal.rcout->write(i, pwm_min);
    }

    // PID configuration.
    pid.kP(0.7);
    pid.kI(1);
    pid.imax(50);
}

void loop(void) {
    // Update ins data.
    ins.update();

    Vector3f delta_angle;
    ins.get_delta_angle(delta_angle);
    // Update yaw.
    yaw += delta_angle.z;

    // Now get throttle and yaw input.
    // Channel 2: throttle.
    // Channel 3: yaw.
    const int ch_throttle = 2;
    const int ch_yaw = 3;
    const float desired_throttle = (float)hal.rcin->read(ch_throttle);
    const float desired_yaw = map((float)hal.rcin->read(ch_yaw),
            (float)yaw_min,
            (float)yaw_max,
            -180.0f, 180.0f);

    // Throw them into our PID controller.
    const float yaw_output = constrain_float(
        pid.get_pid((float)desired_yaw - (float)ToDeg(yaw), 1.0),
        -200.0f, 200.0f
    );

    // Send out signals to all the four rotors.
    // 2 (Red)          0 (Red)
    //           ^
    //           |
    //           |
    // 1 (White)        3 (White)
    uint16_t upper_right = (uint16_t)constrain_float(desired_throttle + yaw_output, pwm_min, pwm_max);
    uint16_t lower_left = (uint16_t)constrain_float(desired_throttle + yaw_output, pwm_min, pwm_max);
    uint16_t upper_left = (uint16_t)constrain_float(desired_throttle - yaw_output, pwm_min, pwm_max);
    uint16_t lower_right = (uint16_t)constrain_float(desired_throttle - yaw_output, pwm_min, pwm_max);

    hal.rcout->write(0, upper_right);
    hal.rcout->write(1, lower_left);
    hal.rcout->write(2, upper_left);
    hal.rcout->write(3, lower_right);

    // Display the states every two seconds.
    ++count;
    if (count % 1000 == 0) {
        hal.console->printf("yaw = %f, desired yaw = %f, desired throttle = %f. "
                "motors = [%u %u %u %u]\n",
                ToDeg(yaw), desired_yaw, desired_throttle,
                upper_right, lower_left, upper_left, lower_right);
    }
    hal.scheduler->delay(10);
}

AP_HAL_MAIN();
