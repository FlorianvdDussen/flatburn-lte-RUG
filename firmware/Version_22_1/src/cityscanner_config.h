#define MODE PWRSAVE

#define AUTOSLEEP FALSE             //Autosleep when inactive
#define INACTIVITY_TIME 420         //Seconds, triggers light sleep
#define STATIONARY_HIBERNATE TRUE   //Hibernate after long inactivity
#define STATIONARY_HIBERNATE_TIME_SEC 300  // 5 min stationary before sleep
#define STATIONARY_HIBERNATE_DURATION_MIN 60 // Sleep duration in minutes 
#define STATIONARY_KEEP_3V3_ON TRUE // Keep sensor 3V3 on so IMU can wake device
#define IMU_HEALTH_CHECK_INTERVAL_SEC 3600  // Check IMU health every hour
#define IMU_HEALTH_FAIL_ALERT_INTERVAL_SEC (6 * 60 * 60) // Rate-limit IMU alerts
#define DEBUG TRUE
 
#define HW_VERSION V4
#define CELLULAR_ON_STARTUP FALSE    // TRUE or FALSE
#define DTIME 100  
#define BATT_ENABLED TRUE

#define OPC_DATA_VERSION EXTENDED       // BASE or EXTENDED for full BIN data
#define TCP_GHOSTWRITE FALSE         //For testing purpose, doesn't dump data over TCP but prints it over serial
#define SD_FORMAT_ONSTARTUP FALSE   //Erase SD Card on startup

// Data sampling
#define SAMPLE_RATE 5 //Seconds 
#define VITALS_RATE 300 //Seconds
#define ROUTINE_RATE 60 //seconds

// BME280 watchdog
#define BME_FAIL_REINIT_MS (60UL * 1000UL)      // Try re-init every 60s on failure
#define BME_FAIL_RESET_MS (10UL * 60UL * 1000UL) // Reset after 10 minutes of continuous failure

// Data Storage and Broadcasting
#define RECORDS_PER_FILE 200 //standard is 200
#define LOW_BATTERY_THRESHOLD 3.70 //volt
#define LOW_BATT_ALERT_INTERVAL_SEC (4 * 60 * 60) // 4 hours

#define TCP_ENDPOINT "127.0.0.1" //change the IP address to dump data over TCP (not implemented yet)

// Battery voltage sanity checks
#define BATT_VOLT_STUCK_DELTA 0.01f
#define BATT_VOLT_STUCK_MS (15UL * 60UL * 1000UL)
#define BATT_VOLT_REINIT_COOLDOWN_MS (180UL * 1000UL)
