#pragma once

#include <windows.h>
#include <iostream>
#include "json.hpp"
#include "NvapiFansLib.h" /* for NVAPI_MAX_PHYSICAL_GPUS */

#define NVAPIFANSSVC_VER 1
#define NVAPIFANSVC_FANSPEED_CHANGE_INCREMENTS_DEFAULT 5
#define NVAPIFANSSVC_TARGET_TEMP_DEFAULT 70;
#define NVAPIFANSSVC_MIN_FANSPEED_DEFAULT 25;
#define NVAPIFANSSVC_START_FAN_TEMP_DEFAULT 40;

struct gpu_config_t {
    int target_temp_max_C = NVAPIFANSSVC_TARGET_TEMP_DEFAULT;     // Try to keep this as the maximum temperature for GPU
    int start_fan_temp_C = NVAPIFANSSVC_START_FAN_TEMP_DEFAULT;   // We're comfortable with letting the GPU go this hot before turning the fan on
    int min_fanspeed_percent = NVAPIFANSSVC_MIN_FANSPEED_DEFAULT; // Minimum speed at which we hope the fan is actually rotating.
                                                                  // Below this value, it might be that the fan is actually not moving any air
                                                                  // ie. Noctua A12x25 min. rotation speed is 450 RPM (+/- 20%) which is 22% of 
                                                                  // its maximum rotational speed (2000RPM). Check with your fan manufacturer.
    int speed_change_increments = NVAPIFANSVC_FANSPEED_CHANGE_INCREMENTS_DEFAULT; // how many percent do we add or substract to the fan speed
};

struct service_config_t {
    int version = NVAPIFANSSVC_VER;
    gpu_config_t gpu_config;
};
