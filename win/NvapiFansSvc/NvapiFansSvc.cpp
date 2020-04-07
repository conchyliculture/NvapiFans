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

SERVICE_STATUS        g_ServiceStatus = { 0 };
SERVICE_STATUS_HANDLE g_StatusHandle = NULL;
HANDLE                g_ServiceStopEvent = INVALID_HANDLE_VALUE;

VOID WINAPI ServiceMain(DWORD argc, LPTSTR* argv);
VOID WINAPI ServiceCtrlHandler(DWORD);
DWORD WINAPI ServiceWorkerThread(LPVOID lpParam);

#define SERVICE_NAME _T("NvapiFans Service")  
#define CONFIG_DIR_NAME L"NvapiFansSvc"
#define CONFIG_FILE_NAME L"config.json"

FILETIME last_config_write_time;

VOID WINAPI ServiceMain(DWORD argc, LPTSTR* argv)
{
    DWORD Status = E_FAIL;

    // Register our service control handler with the SCM
    g_StatusHandle = RegisterServiceCtrlHandler(SERVICE_NAME, ServiceCtrlHandler);

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
        OutputDebugString(SERVICE_NAME _T(" ServiceMain: Couldn't CreateThread"));
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

void LogError(HANDLE event_log, std::wstring message) {
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

void LogSuccess(HANDLE event_log, std::wstring message) {
    const wchar_t* buffer = message.c_str();
    ReportEvent(event_log,        // event log handle
        EVENTLOG_SUCCESS, // event type
        0,                   // event category
        1,                   // event identifier
        NULL,                // no security identifier
        1,                   // size of lpszStrings array
        0,                   // no binary data
        &buffer,         // array of strings
        NULL);               // no binary data
}

//Returns the last Win32 error, in string format. Returns an empty string if there is no error.
void GetLastErrorAsString(std::wstring &message)
{
    //Get the error message, if any.
    DWORD errorMessageID = GetLastError();
    if (errorMessageID == 0)
        return; //No error message has been recorded

    LPVOID  lpMsgBuf;
    DWORD  size = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&lpMsgBuf, 0, NULL);
    if (size <=0 ) {
        return;
    }
    LPCWSTR lpMsgStr = (LPCWSTR)lpMsgBuf;
    std::wstring result(lpMsgStr, lpMsgStr + size);
    //Free the buffer.
    LocalFree(lpMsgBuf);
    message = result;
    return;
}


std::wstring utf8_decode(const std::string& str)
{
    if (str.empty()) return std::wstring();
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
    std::wstring wstrTo(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
    return wstrTo;
}

bool parseConfig(HANDLE event_log, std::wstring config_path, service_config_t& service_config) {
    try {
        nlohmann::json j;
        std::ifstream ifs(config_path);
        ifs >> j;

        // Parse json
        if (!j.count("version")) {
            LogError(event_log, L"Config " + config_path + L" doesn't contain a key 'version'");
            return false;
        }
        int j_version = j["version"].get<int>();
        if (j_version != NVAPIFANSSVC_VER) {
            LogError(event_log, L"Config " + config_path + L" has wrong version: " + std::to_wstring(j_version) + L" != " + std::to_wstring(NVAPIFANSSVC_VER));
            return false;
        }

        if (j.count("gpu_config")) {
            if (j["gpu_config"].count("target_temp_max_C")) {
                service_config.gpu_config.target_temp_max_C = j["gpu_config"]["target_temp_max_C"].get<int>();
            }
            if (j["gpu_config"].count("min_fanspeed_percent")) {
                service_config.gpu_config.min_fanspeed_percent = j["gpu_config"]["min_fanspeed_percent"].get<int>();
            }
            if (j["gpu_config"].count("start_fan_temp_C")) {
                service_config.gpu_config.start_fan_temp_C = j["gpu_config"]["start_fan_temp_C"].get<int>();
            }
        }
        return true;
    }
    catch (nlohmann::json::parse_error& e)
    {
        std::string errmsg = "Error parsing json:";
        errmsg += "message: " + (std::string)(e.what()) + "\n";
        errmsg += "exception id: " + std::to_string(e.id) + "\n";
        errmsg += "byte position of error: " + std::to_string(e.byte) + "\n";
       LogError(event_log, utf8_decode(errmsg));
    //   LogError(event_log, std::wstring_convert(errmsg));
    }
    catch (nlohmann::json::type_error& e) {
        std::string errmsg = "Error parsing json:";
        errmsg += "message: " + (std::string)(e.what()) + "\n";
        errmsg += "exception id: " + std::to_string(e.id) + "\n";
        LogError(event_log, utf8_decode(errmsg));
    }
    return false;
}

bool loadConfig(HANDLE event_log, service_config_t &service_config) {

    LPWSTR szPath;

    // Get path for each computer, non-user specific and non-roaming data.
    if (SHGetKnownFolderPath(FOLDERID_ProgramData, NULL, 0, &szPath) >= 0) {

        std::wstring appdata = szPath;
        std::filesystem::path appdata_directory = appdata;

        std::filesystem::path config_directory_path = appdata_directory / CONFIG_DIR_NAME;
 
        if (!std::filesystem::exists(config_directory_path)) {
            LogError(event_log, L"Can't find config file parent: " + config_directory_path.wstring() + L". Creating it");
            bool res = std::filesystem::create_directory(config_directory_path);
            if (!res) {
                LogError(event_log, L"Could not create directory: " + config_directory_path.wstring());
                return false;
            }
        }
        std::filesystem::path config_path = config_directory_path / CONFIG_FILE_NAME;

        if (!std::filesystem::exists(config_path)) {
            LogError(event_log, L"Can't find config file: " + config_path.wstring());
        }
        else {
            LogSuccess(event_log, L"I found: " + config_path.wstring());
            const std::wstring config_path_w = config_path.wstring();
            parseConfig(event_log, config_path_w, service_config);
        }

        std::wstring message =
            L"Applying config:\n "
            "\t- Target GPU temp: " + std::to_wstring(service_config.gpu_config.target_temp_max_C) + L"C" +
            L"\t- Min Speed: " + std::to_wstring(service_config.gpu_config.min_fanspeed_percent) + L"%" +
            L"\t- Start Fan: " + std::to_wstring(service_config.gpu_config.start_fan_temp_C) + L"C"
            ;
        LogSuccess(event_log, message);


        CoTaskMemFree(szPath);
    }
    else {
        std::wstring error_message;
        GetLastErrorAsString(error_message);
        LogError(event_log, L"SHGetKnownFolderPath failed: " + error_message);
        return false;
    }
    return true;
}

DWORD WINAPI ServiceWorkerThread(LPVOID lpParam)
{
    NvApiClient api;
    HANDLE event_log = RegisterEventSource(NULL, L"NvapiFansSvc");
    service_config_t service_config{};

    LogSuccess(event_log, L"Worker created");

    bool res = loadConfig(event_log, service_config);
    if (!res) {
        LogError(event_log, L"Could not load config, using defaults");
    }
    LogSuccess(event_log, L"Config loaded");

    //  Periodically check if the service has been requested to stop
    while (WaitForSingleObject(g_ServiceStopEvent, 0) != WAIT_OBJECT_0)
    {
        
        bool res;
        std::string gpuName;
        std::vector<NV_PHYSICAL_GPU_HANDLE> list_gpu;
        res = api.getGPUHandles(list_gpu);

        int index = 0;
        for (NV_PHYSICAL_GPU_HANDLE &gpu : list_gpu) {
            int currentTemp = api.getGPUTemperature(gpu);
            if (currentTemp == -1) {
                LogError(event_log, L"Error calling getGPUTemperature for GPU id " + index);
            }
            index += 1;
        }
        Sleep(1000);
    }

    return ERROR_SUCCESS;
}

int _tmain(int argc, TCHAR* argv[])
{
    TCHAR serviceName[100] = SERVICE_NAME;

    SERVICE_TABLE_ENTRY dispatchTable[] = {
        { serviceName, (LPSERVICE_MAIN_FUNCTION)ServiceMain },
        { NULL, NULL }
    };

    if (StartServiceCtrlDispatcher(dispatchTable) == FALSE)
    {
        int err = GetLastError();
        std::wstring errormsg;
        GetLastErrorAsString(errormsg);
        std::cout << err << std::endl;
        std::wcout << errormsg << std::endl;
    }

    return 0;
}

int main(int argc, TCHAR* argv[]) {
    HANDLE event_log = RegisterEventSource(NULL, L"NvapiFansSvc");
    service_config_t service_config{};

    bool res = loadConfig(event_log, service_config);
    if (!res) {
        LogError(event_log, L"Could not load config, using defaults");
    }

    return 0;
}