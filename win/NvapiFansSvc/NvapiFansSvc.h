#pragma once

#include <filesystem>

#define NVAPIFANSSVC_VER 1
#define NVAPIFANSSVC_LOGFILE_NAME "NvapiFansSvc.log"
#define NVAPIFANSSVC_SVC_NAME "NvapiFans Service"
#define NVAPIFANSSVC_CONFIG_DIR_NAME "NvapiFansSvc"
#define NVAPIFANSSVC_CONFIG_FILE_NAME "config.json"
#define NVAPIFANSSVC_TARGET_TEMP_DEFAULT 70
#define NVAPIFANSSVC_MIN_FANSPEED_DEFAULT 25
#define NVAPIFANSSVC_START_FAN_TEMP_DEFAULT 40


// These were copied from https://github.com/Distrotech/lm_sensors/blob/master/prog/pwm/fancontrol
struct gpu_config_t {
    int interval_s = 2;     // The frequency at which the service checks the GPU temperature and adjusts the fan speed.
                            // Seconds, range 1 - 30
    int min_temp_c = 40;    // At this temperature or lower, the service will run the fan at min_fan_speed.
                            // Celsius, range 0 - 100
    int max_temp_c = 80;    // At this temperature or higher, the service will run the fan at max_fan_speed.
                            // Celsius, range 0 - 100
    int min_fan_start_speed = 35;    // When the fan is stopped, this is the minimum speed at which the fan will start spinning.
                                     // Range 0 - 255
    int min_fan_stop_speed = 25;     // When the fan is already spinning, this is the lowest speed at which the fan will continue spinning.
                                     // Range 0 - 255
    int min_fan_speed = 0;           // The speed at which the service will run the fan when the GPU temperature is less than or equal to min_temp_c.
                                     // Range 0 - 255. A value of 0 indicates that the fan will not spin
    int max_fan_speed = 200;         // The speed at which the service will run the fan when the GPU temperature is greater than or equal to max_temp_c.
                                     // Range 0 - 255.
    int average = 1;                 // How many of the recent temperature readings are averaged to calculate the "current" temperature, used in fan speed calculations.
                                     // Not implemented
    std::vector<int> history_temp_c; // previous temperature readings, unused for now.
};

struct service_config_t {
    int version = NVAPIFANSSVC_VER;
    std::filesystem::path log_filepath = "";
    gpu_config_t gpu_config = {};
};
