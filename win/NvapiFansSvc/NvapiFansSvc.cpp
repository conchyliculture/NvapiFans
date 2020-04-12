// From https://www.codeproject.com/Articles/499465/Simple-Windows-Service-in-Cplusplus

#include <windows.h>
#include <codecvt>
#include <locale>
#include <shlobj.h> /* SHGetKnownFolderPath */
#include <tchar.h> /* for _T() macro */
#include <iostream>
#include <filesystem>
#include <fstream>
#include "NvapiFansLib.h"
#include "NvapiFansSvc.h"
#include "json.hpp"
#include "Logger.hpp"

SERVICE_STATUS        g_ServiceStatus = { 0 };
SERVICE_STATUS_HANDLE g_StatusHandle = NULL;
HANDLE                g_ServiceStopEvent = INVALID_HANDLE_VALUE;

VOID WINAPI ServiceMain(DWORD argc, LPTSTR* argv);
VOID WINAPI ServiceCtrlHandler(DWORD);
DWORD WINAPI ServiceWorkerThread(LPVOID lpParam);

VOID WINAPI ServiceMain(DWORD argc, LPTSTR* argv)
{
    DWORD Status = E_FAIL;

    // Register our service control handler with the SCM
    g_StatusHandle = RegisterServiceCtrlHandler(_T(NVAPIFANSSVC_SVC_NAME), ServiceCtrlHandler);

    if (g_StatusHandle == NULL)
    {
        return;
    }

    // Tell the service controller we are starting
    ZeroMemory(&g_ServiceStatus, sizeof(g_ServiceStatus));
    g_ServiceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    g_ServiceStatus.dwControlsAccepted = 0;
    g_ServiceStatus.dwCurrentState = SERVICE_START_PENDING;
    g_ServiceStatus.dwWin32ExitCode = 0;
    g_ServiceStatus.dwServiceSpecificExitCode = 0;
    g_ServiceStatus.dwCheckPoint = 0;

    if (SetServiceStatus(g_StatusHandle, &g_ServiceStatus) == FALSE)
    {
        OutputDebugString(_T(
            "NvapiFans Service: ServiceMain: SetServiceStatus returned error"));
    }

    /*
     * Perform tasks necessary to start the service here
     */

     // Create a service stop event to wait on later
    g_ServiceStopEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (g_ServiceStopEvent == NULL)
    {
        // Error creating event
        // Tell service controller we are stopped and exit
        g_ServiceStatus.dwControlsAccepted = 0;
        g_ServiceStatus.dwCurrentState = SERVICE_STOPPED;
        g_ServiceStatus.dwWin32ExitCode = GetLastError();
        g_ServiceStatus.dwCheckPoint = 1;

        if (SetServiceStatus(g_StatusHandle, &g_ServiceStatus) == FALSE)
        {
            OutputDebugString(_T(
                "NvapiFans Service: ServiceMain: SetServiceStatus returned error"));
        }
        return;
    }

    // Tell the service controller we are started
    g_ServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;
    g_ServiceStatus.dwCurrentState = SERVICE_RUNNING;
    g_ServiceStatus.dwWin32ExitCode = 0;
    g_ServiceStatus.dwCheckPoint = 0;

    if (SetServiceStatus(g_StatusHandle, &g_ServiceStatus) == FALSE)
    {
        OutputDebugString(_T(
            "NvapiFans Service: ServiceMain: SetServiceStatus returned error"));
    }

    // Start a thread that will perform the main task of the service
    HANDLE hThread = CreateThread(NULL, 0, ServiceWorkerThread, NULL, 0, NULL);

    if (hThread == nullptr) {
        OutputDebugString(NVAPIFANSSVC_SVC_NAME _T(" ServiceMain: Couldn't CreateThread"));
    }

    // Wait until our worker thread exits signaling that the service needs to stop
    WaitForSingleObject(hThread, INFINITE);


    /*
     * Perform any cleanup tasks
     */

    CloseHandle(g_ServiceStopEvent);

    // Tell the service controller we are stopped
    g_ServiceStatus.dwControlsAccepted = 0;
    g_ServiceStatus.dwCurrentState = SERVICE_STOPPED;
    g_ServiceStatus.dwWin32ExitCode = 0;
    g_ServiceStatus.dwCheckPoint = 3;

    if (SetServiceStatus(g_StatusHandle, &g_ServiceStatus) == FALSE)
    {
        OutputDebugString(_T(
            "NvapiFans Service: ServiceMain: SetServiceStatus returned error"));
    }

    return;
}

VOID WINAPI ServiceCtrlHandler(DWORD CtrlCode)
{
    switch (CtrlCode)
    {
    case SERVICE_CONTROL_STOP:

        if (g_ServiceStatus.dwCurrentState != SERVICE_RUNNING)
            break;

        /*
         * Perform tasks necessary to stop the service here
         */

        g_ServiceStatus.dwControlsAccepted = 0;
        g_ServiceStatus.dwCurrentState = SERVICE_STOP_PENDING;
        g_ServiceStatus.dwWin32ExitCode = 0;
        g_ServiceStatus.dwCheckPoint = 4;

        if (SetServiceStatus(g_StatusHandle, &g_ServiceStatus) == FALSE)
        {
            OutputDebugString(_T(
                "NvapiFans Service: ServiceCtrlHandler: SetServiceStatus returned error"));
        }

        // This will signal the worker thread to start shutting down
        SetEvent(g_ServiceStopEvent);

        break;

    default:
        break;
    }
}

void EventLogError(HANDLE event_log, const std::wstring &message) {
    const wchar_t* buffer = message.c_str();
    ReportEvent(event_log,        // event log handle
        EVENTLOG_ERROR_TYPE, // event type
        0,                   // event category
        1,                   // event identifier
        NULL,                // no security identifier
        1,                   // size of lpszStrings array
        0,                   // no binary data
        &buffer,         // array of strings
        NULL);               // no binary data
}

void EventLogInfo(HANDLE event_log, const std::wstring &message) {
    const wchar_t* buffer = message.c_str();
    ReportEvent(event_log,        // event log handle
        EVENTLOG_INFORMATION_TYPE, // event type
        0,                   // event category
        1,                   // event identifier
        NULL,                // no security identifier
        1,                   // size of lpszStrings array
        0,                   // no binary data
        &buffer,         // array of strings
        NULL);               // no binary data
}

//Returns the last Win32 error, in string format. Returns an empty string if there is no error.
static std::wstring GetErrorAsString(DWORD error_value) {
    LPVOID  lpMsgBuf;
    DWORD  size = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL, error_value, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&lpMsgBuf, 0, NULL);
    if (size <=0 )
        return std::wstring();

    LPCWSTR lpMsgStr = (LPCWSTR)lpMsgBuf;
    std::wstring message(lpMsgStr, lpMsgStr + size);
    LocalFree(lpMsgBuf);
    return message;
}

std::wstring utf8_decode(const std::string& str)
{
    if (str.empty()) return std::wstring();
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
    std::wstring wstrTo(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
    return wstrTo;
}

static void pushTemp(service_config_t& service_config, int temp) {
    int max_history_size = service_config.gpu_config.average;
    if (service_config.gpu_config.history_temp_c.size() < max_history_size) {
        service_config.gpu_config.history_temp_c.insert(service_config.gpu_config.history_temp_c.begin(), temp);
    }
    else {
        for (int i = max_history_size - 1; i >= 1; i--) {
            service_config.gpu_config.history_temp_c.at(i) = service_config.gpu_config.history_temp_c.at(i - 1);
        }
        service_config.gpu_config.history_temp_c.at(0) = temp;
    }
}

bool parseConfig(HANDLE event_log, const std::wstring& config_path, service_config_t& service_config) {

    service_config_t draft_config = {};
    try {
        nlohmann::json j;
        std::ifstream ifs(config_path);
        ifs >> j;

        // Parse json
        if (!j.count("version")) {
            EventLogError(event_log, L"Config " + config_path + L" doesn't contain a key 'version'");
            return false;
        }
        int j_version = j["version"].get<int>();
        if (j_version != NVAPIFANSSVC_VER) {
            EventLogError(event_log, L"Config " + config_path + L" has wrong version: " + std::to_wstring(j_version) + L" != " + std::to_wstring(NVAPIFANSSVC_VER));
            return false;
        }

        if (j.count("gpu_config")) {
            if (j["gpu_config"].count("interval_s")) {
                draft_config.gpu_config.interval_s = j["gpu_config"]["interval_s"].get<int>();
            }
            if (j["gpu_config"].count("min_temp_c")) {
                draft_config.gpu_config.min_temp_c = j["gpu_config"]["min_temp_c"].get<int>();
            }
            if (j["gpu_config"].count("max_temp_c")) {
                draft_config.gpu_config.max_temp_c = j["gpu_config"]["max_temp_c"].get<int>();
            }
            if (j["gpu_config"].count("min_fan_start_speed")) {
                draft_config.gpu_config.min_fan_start_speed = j["gpu_config"]["min_fan_start_speed"].get<int>();
            }
            if (j["gpu_config"].count("min_fan_stop_speed")) {
                draft_config.gpu_config.min_fan_stop_speed = j["gpu_config"]["min_fan_stop_speed"].get<int>();
            }
            if (j["gpu_config"].count("min_fan_speed")) {
                draft_config.gpu_config.min_fan_speed = j["gpu_config"]["min_fan_speed"].get<int>();
            }
            if (j["gpu_config"].count("max_fan_speed")) {
                draft_config.gpu_config.max_fan_speed = j["gpu_config"]["max_fan_speed"].get<int>();
            }
            if (j["gpu_config"].count("average")) {
                draft_config.gpu_config.average = j["gpu_config"]["average"].get<int>();
            }
        }
    }
    catch (nlohmann::json::parse_error& e)
    {
        std::string errmsg = "Error parsing json:";
        errmsg += "message: " + (std::string)(e.what()) + "\n";
        errmsg += "exception id: " + std::to_string(e.id) + "\n";
        errmsg += "byte position of error: " + std::to_string(e.byte) + "\n";
        return false;
    }
    catch (nlohmann::json::type_error& e) {
        std::string errmsg = "Error parsing json:";
        errmsg += "message: " + (std::string)(e.what()) + "\n";
        errmsg += "exception id: " + std::to_string(e.id) + "\n";
        EventLogError(event_log, utf8_decode(errmsg));
        return false;
    }
    std::string errmsg = "";
    // Do some sanity check on values
    if ((draft_config.gpu_config.interval_s < 1) || (draft_config.gpu_config.interval_s > 30) ) {
        errmsg += "bad value for interval_s: " + std::to_string(draft_config.gpu_config.interval_s) + "\n";
    }
    if ((draft_config.gpu_config.min_temp_c < 0) || (draft_config.gpu_config.min_temp_c > 100)) {
        errmsg += "bad value for min_temp_c: " + std::to_string(draft_config.gpu_config.min_temp_c) + "\n";
    }
    if ((draft_config.gpu_config.max_temp_c < 0) || (draft_config.gpu_config.max_temp_c > 100)) {
        errmsg += "bad value for max_temp_c: " + std::to_string(draft_config.gpu_config.max_temp_c) + "\n";
    }
    if ((draft_config.gpu_config.min_fan_start_speed < 0) || (draft_config.gpu_config.min_fan_start_speed > 255)) {
        errmsg += "bad value for min_fan_start_speed: " + std::to_string(draft_config.gpu_config.min_fan_start_speed) + "\n";
    }
    if ((draft_config.gpu_config.min_fan_stop_speed < 0) || (draft_config.gpu_config.min_fan_stop_speed > 255)) {
        errmsg += "bad value for min_fan_stop_speed: " + std::to_string(draft_config.gpu_config.min_fan_stop_speed) + "\n";
    }
    if ((draft_config.gpu_config.min_fan_speed < 0) || (draft_config.gpu_config.min_fan_speed > 255)) {
        errmsg += "bad value for min_fan_speed: " + std::to_string(draft_config.gpu_config.min_fan_speed) + "\n";
    }
    if ((draft_config.gpu_config.max_fan_speed < 0) || (draft_config.gpu_config.max_fan_speed > 255)) {
        errmsg += "bad value for max_fan_speed: " + std::to_string(draft_config.gpu_config.max_fan_speed) + "\n";
    }
    if ((draft_config.gpu_config.average < 1) || (draft_config.gpu_config.average > 100)) {
        errmsg += "bad value for average: " + std::to_string(draft_config.gpu_config.average) + "\n";
    }
    if (draft_config.gpu_config.min_temp_c > draft_config.gpu_config.max_temp_c) {
        errmsg += "min_temp_c can't be superior to max_temp_c\n";
    }
    if (draft_config.gpu_config.min_fan_speed > draft_config.gpu_config.max_fan_speed) {
        errmsg += "min_fan_speed can't be superior to max_fan_speed\n";
    }
    if ((draft_config.gpu_config.min_fan_stop_speed < draft_config.gpu_config.min_fan_speed) || (draft_config.gpu_config.min_fan_stop_speed > draft_config.gpu_config.max_fan_speed)) {
        errmsg += "min_fan_stop_speed should be between min_fan_speed and max_fan_speed\n";
    }
    if (!errmsg.empty()) {
        EventLogError(event_log, utf8_decode("Error in config file:\n" + errmsg + "custom config ignored\n"));
        return false;
    }

    service_config = draft_config;
    return true;
}

bool loadConfig(HANDLE event_log, service_config_t &service_config) {

    LPWSTR szPath;

    // Get path for each computer, non-user specific and non-roaming data.
    if (SHGetKnownFolderPath(FOLDERID_ProgramData, NULL, 0, &szPath) == S_OK) {

        std::wstring appdata = szPath;
        std::filesystem::path appdata_directory = appdata;

        std::filesystem::path config_directory_path = appdata_directory / NVAPIFANSSVC_CONFIG_DIR_NAME;

        if (!std::filesystem::exists(config_directory_path)) {
            EventLogError(event_log, L"Can't find config file parent: " + config_directory_path.wstring() + L". Creating it");
            bool res = std::filesystem::create_directory(config_directory_path);
            if (!res) {
                EventLogError(event_log, L"Could not create directory: " + config_directory_path.wstring());
                CoTaskMemFree(szPath);
                return false;
            }
        }
        std::filesystem::path config_path = config_directory_path / NVAPIFANSSVC_CONFIG_FILE_NAME;

        if (!std::filesystem::exists(config_path)) {
            EventLogError(event_log, L"Can't find config file: " + config_path.wstring());
        }
        else {
            const std::wstring config_path_w = config_path.wstring();
            parseConfig(event_log, config_path_w, service_config);
            service_config.log_filepath = config_directory_path / NVAPIFANSSVC_LOGFILE_NAME;
        }

        CoTaskMemFree(szPath);
    }
    else {
        EventLogError(event_log, L"SHGetKnownFolderPath failed");
        return false;
    }
    return true;
}

// stealing from https://github.com/lm-sensors/lm-sensors/blob/master/prog/pwm/fancontrol#L623
static int getNewSpeed(const service_config_t& service_config, const int current_temp) {
    if (current_temp <= service_config.gpu_config.min_temp_c) {
        // below min temp, use defined min pwm
        return service_config.gpu_config.min_fan_speed;
    }
    if (current_temp >= service_config.gpu_config.max_temp_c) {
        // over max temp, use defined max pwm
        return service_config.gpu_config.max_fan_speed;
    }

    int temp_over_min = current_temp - service_config.gpu_config.min_temp_c;
    float speed_to_temp_ratio = (float)(service_config.gpu_config.max_fan_speed - service_config.gpu_config.min_fan_stop_speed) / (service_config.gpu_config.max_temp_c - service_config.gpu_config.min_temp_c);
    return (int)(temp_over_min * speed_to_temp_ratio) + service_config.gpu_config.min_fan_stop_speed;
}

DWORD WINAPI ServiceWorkerThread(LPVOID lpParam)
{
    NvApiClient api;
    HANDLE event_log = RegisterEventSource(NULL, _T(NVAPIFANSSVC_SVC_NAME));
    service_config_t service_config{};

    EventLogInfo(event_log, L"NvapiFansSvc is starting");

    bool res = loadConfig(event_log, service_config);
    if (!res) {
        EventLogInfo(event_log, L"Could not load configuration file, using defaults");
    }

    // From here the logfile path is hopefully set
    Logger logger(service_config.log_filepath.string());
    logger.Info("Service Started");

    std::vector<NV_PHYSICAL_GPU_HANDLE> list_gpu;
    res = api.getGPUHandles(list_gpu);
    if (!res) {
        logger.Error("Error calling getGPUHandles");
        return ERROR_BAD_UNIT;
    }

    if (list_gpu.size() == 0) {
        logger.Error("Could not find any Nvidia GPU");
        return ERROR_BAD_UNIT; // Maybe find a better error but eh
    }

    bool detected = true;
    for (NV_PHYSICAL_GPU_HANDLE gpu : list_gpu) {
        detected &= api.detectI2CDevice(gpu);
    }
    if (!detected) {
        logger.Error("Failed to detect all GPU I2C devices");
        return ERROR_BAD_COMMAND; // Maybe find a better error but eh
    }

    //  Periodically check if the service has been requested to stop
    while (WaitForSingleObject(g_ServiceStopEvent, 0) != WAIT_OBJECT_0)
    {
        bool res;

        for (NV_PHYSICAL_GPU_HANDLE gpu : list_gpu) {
            int current_temp = api.getGPUTemperature(gpu);
            if (current_temp == -1) {
                logger.Info("Error calling getGPUTemperature");
                continue;
            }
            int current_speed = api.getExternalFanSpeedPercent(gpu);
            if (current_speed == -1) {
                EventLogError(event_log, L"Error calling getGPUTemperature");
                continue;
            }

            int new_speed = getNewSpeed(service_config, current_temp);

            if (
                (current_speed == 0) // fan is not spinning
                && (new_speed >= service_config.gpu_config.min_fan_stop_speed) // And if the fan *could* be spinning
                && (new_speed < service_config.gpu_config.min_fan_start_speed)) {
                    new_speed = service_config.gpu_config.min_fan_start_speed;
            }

            pushTemp(service_config, current_temp);
            res = api.setExternalFanSpeedPWM(gpu, new_speed);
            if (!res) {
                EventLogError(event_log, L"Error calling setExternalFanSpeedPercent");
                continue;
            }
            logger.Info("Set new speed: "+std::to_string(hexToPercent(new_speed))+"%");
        }
        logger.Flush();
        Sleep(service_config.gpu_config.interval_s * 1000);
    }
    logger.Info("Service Stopped");
    return ERROR_SUCCESS;
}

int _tmain(int argc, TCHAR* argv[])
{
    TCHAR serviceName[100] = _T(NVAPIFANSSVC_SVC_NAME);

    SERVICE_TABLE_ENTRY dispatchTable[] = {
        { serviceName, (LPSERVICE_MAIN_FUNCTION)ServiceMain },
        { NULL, NULL }
    };

    if (StartServiceCtrlDispatcher(dispatchTable) == FALSE)
    {
        int err = GetLastError();
        std::cout << err << std::endl;
        std::wcout << GetErrorAsString(err) << std::endl;

        return err;
    }

    return 0;
}
