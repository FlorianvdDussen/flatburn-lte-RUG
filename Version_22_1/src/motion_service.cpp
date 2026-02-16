#include "Particle.h"
#include "motion_service.h"
#include "CS_core.h"  //

#define LOW_POWER

float   sampleRate = 6.25;  // HZ - Samples per second - 0.781, 1.563, 3.125, 6.25, 12.5, 25, 50, 100, 200, 400, 800, 1600Hz
uint8_t accelRange = 2;     // Accelerometer range = 2, 4, 8, 16g

int32_t result;

KXTJ3 myIMU(0x0E); // Address can be 0x0E or 0x0F

MotionService *MotionService::_instance = nullptr;
int MotionService::inactivity_counter = 0;

namespace {
    const uint32_t STATIONARY_ALERT_MAGIC = 0x53544154; // "STAT"
    const int EEPROM_ADDR_STATIONARY_ALERT = 32;
    const uint32_t IMU_HEALTH_MAGIC = 0x494D5548; // "IMUH"
    const int EEPROM_ADDR_IMU_HEALTH = 96;

    struct StationaryAlertRecord {
        uint32_t magic;
        uint16_t batt_mv;
        uint16_t reserved;
        uint32_t epoch;
        uint32_t count;
        uint32_t inactivity_sec;
    };

    struct ImuHealthAlertRecord {
        uint32_t magic;
        uint16_t batt_mv;
        uint16_t reserved;
        uint32_t epoch;
        uint32_t count;
        uint8_t whoami;
        uint8_t status;
        uint16_t pad;
    };

    bool readStationaryAlert(StationaryAlertRecord &rec) {
        EEPROM.get(EEPROM_ADDR_STATIONARY_ALERT, rec);
        return (rec.magic == STATIONARY_ALERT_MAGIC);
    }

    void writeStationaryAlert(uint16_t batt_mv, uint32_t epoch, uint32_t inactivity_sec) {
        StationaryAlertRecord rec = {};
        if (readStationaryAlert(rec)) {
            rec.count++;
        } else {
            rec.magic = STATIONARY_ALERT_MAGIC;
            rec.count = 1;
        }
        rec.batt_mv = batt_mv;
        rec.epoch = epoch;
        rec.inactivity_sec = inactivity_sec;
        EEPROM.put(EEPROM_ADDR_STATIONARY_ALERT, rec);
    }

    void clearStationaryAlert() {
        StationaryAlertRecord rec = {};
        EEPROM.put(EEPROM_ADDR_STATIONARY_ALERT, rec);
    }

    bool readImuHealthAlert(ImuHealthAlertRecord &rec) {
        EEPROM.get(EEPROM_ADDR_IMU_HEALTH, rec);
        return (rec.magic == IMU_HEALTH_MAGIC);
    }

    void writeImuHealthAlert(uint16_t batt_mv, uint32_t epoch, uint8_t whoami, uint8_t status) {
        ImuHealthAlertRecord rec = {};
        if (readImuHealthAlert(rec)) {
            rec.count++;
        } else {
            rec.magic = IMU_HEALTH_MAGIC;
            rec.count = 1;
        }
        rec.batt_mv = batt_mv;
        rec.epoch = epoch;
        rec.whoami = whoami;
        rec.status = status;
        EEPROM.put(EEPROM_ADDR_IMU_HEALTH, rec);
    }

    void clearImuHealthAlert() {
        ImuHealthAlertRecord rec = {};
        EEPROM.put(EEPROM_ADDR_IMU_HEALTH, rec);
    }
}


void MotionService::timer_fnc(){
   inactivity_counter++;
   Serial.print("Inactivity counter: ");Serial.println(inactivity_counter);
    pinMode(A1, INPUT); // Accelerometer interrupt
}


Timer inactivity_timer(1000, MotionService::timer_fnc);

MotionService::MotionService() {
}
void scanI2Cwire() {
    Serial.println("\n----- I2C Scanner -----");

    if (!Wire.isEnabled()) {
        Serial.println("Wire not enabled! Initializing...");
        Wire.begin();
        Wire.setSpeed(CLOCK_SPEED_100KHZ);
        delay(50);
    }
    uint8_t count = 0;
    for (uint8_t addr = 1; addr < 127; addr++) {
        Wire.beginTransmission(addr);
        if (Wire.endTransmission() == 0) {
            Serial.printlnf("Found device at 0x%02X on wire", addr);
            count++;
        }
    }
    Serial.printlnf("Found %d device(s)\n", count);
}

// Direct register read function
uint8_t readRegisterDirect(uint8_t addr, uint8_t reg) {
    Wire.beginTransmission(addr);
    Wire.write(reg);
    Wire.endTransmission(false);  // Keep connection open
    Wire.requestFrom(addr, (uint8_t)1);
    if (Wire.available()) {
        return Wire.read();
    }
    return 0xFF;
}

// Direct register write function
void writeRegisterDirect(uint8_t addr, uint8_t reg, uint8_t value) {
    Wire.beginTransmission(addr);
    Wire.write(reg);
    Wire.write(value);
    Wire.endTransmission();
}

int MotionService::start()
{   

    // scanI2Cwire();
    // // Direct check for IMU at 0x0E
    // Serial.println("\nDirect IMU check at 0x0E:");
    // Wire.beginTransmission(0x0E);
    // uint8_t error = Wire.endTransmission();
    // Serial.printlnf("I2C transmission to 0x0E result: %d (0=success)", error);
    
    // // Read multiple registers directly to identify the chip
    // Serial.println("\n===== Direct Register Reads at 0x0E =====");
    
    // // Try reading WHO_AM_I at different register addresses
    // // KXTJ3-1057: WHO_AM_I at 0x0F, should return 0x35
    // // LIS2DH/LIS3DH: WHO_AM_I at 0x0F, returns 0x33
    // // MMA8452Q: WHO_AM_I at 0x0D, returns 0x2A
    // // ADXL345: DEVID at 0x00, returns 0xE5
    
    // uint8_t reg_00 = readRegisterDirect(0x0E, 0x00);
    // uint8_t reg_0D = readRegisterDirect(0x0E, 0x0D);
    // uint8_t reg_0F = readRegisterDirect(0x0E, 0x0F);
    // uint8_t reg_75 = readRegisterDirect(0x0E, 0x75);  // MPU WHO_AM_I
    
    // Serial.printlnf("Register 0x00: 0x%02X (ADXL345 DEVID=0xE5)", reg_00);
    // Serial.printlnf("Register 0x0D: 0x%02X (MMA8452 WHO_AM_I=0x2A)", reg_0D);
    // Serial.printlnf("Register 0x0F: 0x%02X (KXTJ3=0x35, LIS3DH=0x33)", reg_0F);
    // Serial.printlnf("Register 0x75: 0x%02X (MPU WHO_AM_I)", reg_75);
    
    // // Try software reset for KXTJ3
    // Serial.println("\nAttempting KXTJ3 software reset...");
    // writeRegisterDirect(0x0E, 0x1D, 0x80);  // CTRL_REG2 SRST bit
    // delay(100);
    
    // // Re-read WHO_AM_I after reset
    // uint8_t whoami_after = readRegisterDirect(0x0E, 0x0F);
    // Serial.printlnf("WHO_AM_I after reset: 0x%02X", whoami_after);
    
    // // Also check 0x69 and 0x6B - these might be the actual accelerometer
    // Serial.println("\n===== Checking other addresses =====");
    // if (Wire.endTransmission() == 0) {
    //     Wire.beginTransmission(0x69);
    //     if (Wire.endTransmission() == 0) {
    //         uint8_t who_69 = readRegisterDirect(0x69, 0x0F);
    //         uint8_t who_69_alt = readRegisterDirect(0x69, 0x75);
    //         Serial.printlnf("0x69 reg 0x0F: 0x%02X, reg 0x75: 0x%02X", who_69, who_69_alt);
    //     }
    // }
    
    // Wire.beginTransmission(0x6B);
    // if (Wire.endTransmission() == 0) {
    //     uint8_t who_6B = readRegisterDirect(0x6B, 0x0F);
    //     uint8_t who_6B_alt = readRegisterDirect(0x6B, 0x75);
    //     Serial.printlnf("0x6B reg 0x0F: 0x%02X, reg 0x75: 0x%02X", who_6B, who_6B_alt);
    // }
    
    // Serial.println("=====================================\n");

    // Wire.reset();
    // delay(50);
    //Particle.variable("OVERRIDE_AS", OVVERRIDE_AUTOSLEEP);
    uint8_t errorAccumulator = 0;
    if (HW_VERSION == V4)
    {

        bool imu_ok = false;
        const int MAX_RETRIES = 10;
                for (int attempt = 1; attempt <= MAX_RETRIES; attempt++)
        {

            Serial.printlnf("\n--- IMU initialization attempt %d/%d ---", attempt, MAX_RETRIES);

            if (!Wire.isEnabled()) {
                Serial.println("Wire not enabled, reinitializing...");
                Wire.begin();
                Wire.setSpeed(CLOCK_SPEED_100KHZ);
                delay(200);
                
                if (!Wire.isEnabled()) {
                    Serial.println("ERROR: Cannot enable Wire!");
                    continue;  // Skip this attempt
                }
            }
            
            if (myIMU.begin(sampleRate, accelRange) == 0)
            {
                Serial.println("IMU initialized successfully!");
                imu_ok = true;
                break;
            }
            
            Serial.printlnf("IMU init attempt %d failed", attempt);
            
            // Recovery actions between attempts
            if (attempt < MAX_RETRIES)
            {
                Serial.println("Attempting recovery...");
                
                // Reset I2C bus
                Wire.begin();
                Wire.setSpeed(CLOCK_SPEED_100KHZ);
                delay(100);
                
                // Longer delay on later attempts
                if (attempt >= 2)
                {
                    Serial.println("Extended delay before retry...");
                    delay(2000);
                }
            }
         }
        
        if (!imu_ok)
        {
            Serial.println("ERROR: IMU failed to initialize after all attempts!");
            errorAccumulator++;
        }
        else
        {
            // Configure interrupt only if IMU initialized successfully
            myIMU.intConf(50, 1, 10, LOW);

            uint8_t readData = 0;
            myIMU.readRegister(&readData, KXTJ3_WHO_AM_I);
            Serial.print("Who am I? 0x");
            Serial.println(readData, HEX);
                        if (readData == 0x35) {
                Serial.println("IMU identity confirmed: KXTJ3-1057");
            }
        }
    }
    if(errorAccumulator)
    {
        Serial.println("Problem configuring the device.");
    }
    else
    {
        Serial.println("Device O.K.");
    }
    
    attachInterrupt(INT_ACC, MotionService::resetInactivityCounter, FALLING);
    inactivity_timer.start();   
    Serial.println("timer started");
    motionservice_started = true;
    return true;
}

int MotionService::stop()
{
    inactivity_timer.dispose(); 
    motionservice_started = false;
    return 1;
}

int MotionService::waitOnEvent()
{
    return 1;
    // check accelerometer for events and add it to event queque
    // if no motion events for x minutes go to sleep
}

void MotionService::loop()
{
    static uint32_t last_imu_check_ms = 0;
    static uint32_t last_imu_fail_ms = 0;

    if (Particle.connected()) {
        ImuHealthAlertRecord rec = {};
        if (readImuHealthAlert(rec)) {
            float hv = rec.batt_mv / 1000.0f;
            String payload = String::format("status=fail,who=0x%02X,code=%u,batt_v=%.2f,ts=%lu,count=%lu",
                                            rec.whoami,
                                            rec.status,
                                            hv,
                                            rec.epoch,
                                            rec.count);
            Particle.publish("IMU_HEALTH", payload);
            clearImuHealthAlert();
        }
    }

    if (IMU_HEALTH_CHECK_INTERVAL_SEC > 0) {
        uint32_t now_ms = millis();
        if (now_ms - last_imu_check_ms >= (IMU_HEALTH_CHECK_INTERVAL_SEC * 1000UL)) {
            last_imu_check_ms = now_ms;
            uint8_t who = 0xFF;
            kxtj3_status_t st = myIMU.readRegister(&who, KXTJ3_WHO_AM_I);
            bool ok = (st == IMU_SUCCESS && who == 0x35);
            if (!ok) {
                if (now_ms - last_imu_fail_ms >= (IMU_HEALTH_FAIL_ALERT_INTERVAL_SEC * 1000UL)) {
                    last_imu_fail_ms = now_ms;
                    if (!CityVitals::instance().BATT_started) {
                        CityVitals::instance().startBattery();
                    }
                    float batt_v = CityVitals::instance().getBatteryVoltage();
                    uint16_t batt_mv = (uint16_t)(batt_v * 1000.0f);
                    uint32_t epoch = Time.isValid() ? Time.now() : 0;
                    writeImuHealthAlert(batt_mv, epoch, who, (uint8_t)st);
                    if (Particle.connected()) {
                        String payload = String::format("status=fail,who=0x%02X,code=%u,batt_v=%.2f,ts=%lu",
                                                        who, (uint8_t)st, batt_v, epoch);
                        Particle.publish("IMU_HEALTH", payload);
                        clearImuHealthAlert();
                    }
                }
            }
        }
    }

    if (STATIONARY_HIBERNATE && inactivity_counter > STATIONARY_HIBERNATE_TIME_SEC) {
        if (!CityVitals::instance().BATT_started) {
            CityVitals::instance().startBattery();
        }
        String charging_status = CityVitals::instance().getChargingStatus();
        if (charging_status.startsWith("1")) {
            return;
        }
        float batt_v = CityVitals::instance().getBatteryVoltage();
        uint16_t batt_mv = (uint16_t)(batt_v * 1000.0f);
        uint32_t epoch = Time.isValid() ? Time.now() : 0;
        writeStationaryAlert(batt_mv, epoch, (uint32_t)inactivity_counter);

        if (Particle.connected()) {
            String payload = String::format("inactive_s=%d,batt_v=%.2f,reason=stationary,ts=%lu",
                                            inactivity_counter, batt_v, epoch);
            Particle.publish("STATIONARY", payload);
            clearStationaryAlert();
        }

        Log.info("Stationary for %d seconds. Entering STOP for %d minutes.",
                 inactivity_counter, STATIONARY_HIBERNATE_DURATION_MIN);
        CitySleep::instance().stopFor(STATIONARY_HIBERNATE_DURATION_MIN, MINUTES, STATIONARY_KEEP_3V3_ON);
        return;
    }

    if(inactivity_counter > INACTIVITY_TIME){
        Serial.println("motion service loop inactivity timer");
        if(AUTOSLEEP && !OVVERRIDE_AUTOSLEEP){
        Serial.println("It's time to get some sleep");
        delay(100);
        resetInactivityCounter();
        CitySleep::instance().stop();
        }
    }
}

void MotionService::resetInactivityCounter(){
    inactivity_counter = 0;
}

int MotionService::getInactivityCounter()
{
    return inactivity_counter;
}

void MotionService::setOverrideAutosleep(bool override)
{
    if(override)
        OVVERRIDE_AUTOSLEEP = TRUE;
    else
        OVVERRIDE_AUTOSLEEP = FALSE;
}

void MotionService::testAccelerometer()
{  
  Serial.print("\nAccelerometer:\n");
  Serial.print(" X = ");
  Serial.println(myIMU.axisAccel( X ), 4);
  Serial.print(" Y = ");
  Serial.println(myIMU.axisAccel( Y ), 4);
  Serial.print(" Z = ");
  Serial.println(myIMU.axisAccel( Z ), 4);
  /*Serial.print("\nGyroscope:\n");
  Serial.print(" X = ");
  Serial.println(myIMU.readFloatGyroX(), 4);
  Serial.print(" Y = ");
  Serial.println(myIMU.readFloatGyroY(), 4);
  Serial.print(" Z = ");
  Serial.println(myIMU.readFloatGyroZ(), 4);
  Serial.print("\nThermometer:\n");
  Serial.print(" Degrees C = ");
  Serial.println(myIMU.readTempC(), 4);
  Serial.print(" Degrees F = ");
  Serial.println(myIMU.readTempF(), 4);*/
  delay(1000);
}
