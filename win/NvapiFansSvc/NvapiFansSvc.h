#pragma once

#include <windows.h>
#include <iostream>
#include "json.hpp"
#include "NvapiFansLib.h" /* for NVAPI_MAX_PHYSICAL_GPUS */

#define NVAPIFANSSVC_VER 1
#define NVAPIFANSVC_FANSPEED_CHANGE_INCREMENTS_DEFAULT 5
#define NVAPIFANSSVC_TARGET_TEMP_DEFAULT 70;
#define NVAPIFANSSVC_MIN_FANSPEED_DEFAULT 15;
#define NVAPIFANSSVC_START_FAN_TEMP_DEFAULT 40;

struct gpu_config_t {
    int target_temp_max_C = NVAPIFANSSVC_TARGET_TEMP_DEFAULT; // Try to not keep below this temperature
    int min_fanspeed_percent = NVAPIFANSSVC_MIN_FANSPEED_DEFAULT; // Minimum speed at which we hope the fan is actually rotating
    int start_fan_temp_C = NVAPIFANSSVC_START_FAN_TEMP_DEFAULT; // We're confortable with letting the GPU go this hot before turning the fan on
    int speed_change_increments = NVAPIFANSVC_FANSPEED_CHANGE_INCREMENTS_DEFAULT; // how many percent do we add or substract to the fan speed
};

struct service_config_t {
    int version = NVAPIFANSSVC_VER;
    gpu_config_t gpu_config;
};

VOID WINAPI ServiceMain(DWORD argc, LPTSTR* argv);
VOID WINAPI ServiceCtrlHandler(DWORD);
DWORD WINAPI ServiceWorkerThread(LPVOID lpParam);

VOID WINAPI ServiceMain(DWORD argc, LPTSTR* argv);

VOID WINAPI ServiceCtrlHandler(DWORD CtrlCode);

void LogError(HANDLE event_log, std::wstring message);
void LogSuccess(HANDLE event_log, std::wstring message);
bool parseConfig(HANDLE event_log, std::wstring config_path, service_config_t& service_config);
bool loadConfig(HANDLE event_log, nlohmann::json& config);

DWORD WINAPI ServiceWorkerThread(LPVOID lpParam);