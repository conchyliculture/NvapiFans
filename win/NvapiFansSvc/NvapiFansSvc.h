#pragma once

#include <windows.h>
#include <iostream>
#include "json.hpp"
#include "NvapiFansLib.h" /* for NVAPI_MAX_PHYSICAL_GPUS */

#define NVAPIFANSSVC_VER 1

// These were copied from https://github.com/Distrotech/lm_sensors/blob/master/prog/pwm/fancontrol
struct gpu_config_t {
    int interval_s = 2;     // Which interval in seconds between checks, between 1 & 100
    int min_temp_c = 40;    // Below this, fan switches to its "minimum speed", max 100
    int max_temp_c = 80;    // Above this, fan switches to its "maximum speed", max 100
    int min_fan_start_speed = 35;    // This is the minimum speed at which the fan begins spinning, between 0 & 255 aka minsa
    int min_fan_stop_speed = 25;     // This is the minimum speed at which the fan keeps spinning once started, between 0 & 255 aka minso
    int min_fan_speed = 0;           // Speed to set when fan is below min_temp_c. 0 means "stopped", between 0 & 255
    int max_fan_speed = 200;         // Speed to set if temp is over max_temp_c. 255 is "max speed", between 0 & 255
    int average = 1;                 // How many last temp readings are used to average the temperature. Not implemented yet.
    std::vector<int> history_temp_c; // previous temperature readings, unused for now.
};

struct service_config_t {
    int version = NVAPIFANSSVC_VER;
    gpu_config_t gpu_config = {};
};
