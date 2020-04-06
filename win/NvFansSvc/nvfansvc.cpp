// From https://www.codeproject.com/Articles/499465/Simple-Windows-Service-in-Cplusplus

#include <windows.h>
#include "nv.h"
#include <shlobj.h> /* SHGetKnownFolderPath */ 
#include <Shlwapi.h> /* PathAppend */
#include <tchar.h> /* for _T() macro */
#include <iostream>
#include <filesystem>
#include <fstream>
#include "json.hpp"

SERVICE_STATUS        g_ServiceStatus = { 0 };
SERVICE_STATUS_HANDLE g_StatusHandle = NULL;
HANDLE                g_ServiceStopEvent = INVALID_HANDLE_VALUE;

VOID WINAPI ServiceMain(DWORD argc, LPTSTR* argv);
VOID WINAPI ServiceCtrlHandler(DWORD);
DWORD WINAPI ServiceWorkerThread(LPVOID lpParam);

#define SERVICE_NAME _T("NvapiFans Service")  
#define CONFIG_PATH _T("\\NvapiSvc\\config.json")

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

bool loadConfig(HANDLE event_log, nlohmann::json& config) {

    LPWSTR szPath[MAX_PATH];
    // Get path for each computer, non-user specific and non-roaming data.
    if (SHGetKnownFolderPath(FOLDERID_ProgramData, NULL, 0, szPath) > 0) {
        PathAppend(*szPath, CONFIG_PATH);

        std::wstring config_path = *szPath;

        if (!std::filesystem::exists(config_path)) {
            LogError(event_log, L"Can't find config file: " + config_path);
        }
        // TODO: maybe not read file every second

        std::ifstream ifs(config_path);
        ifs >> config;
    }
    else {
        DWORD err = GetLastError();
        return false;
    }
    return true;
}


DWORD WINAPI ServiceWorkerThread(LPVOID lpParam)
{
    NvApiClient api;
    HANDLE event_log = RegisterEventSource(NULL, L"NvapiFansSvc");
    nlohmann::json config;

    loadConfig(event_log, config);

    //  Periodically check if the service has been requested to stop
    while (WaitForSingleObject(g_ServiceStopEvent, 0) != WAIT_OBJECT_0)
    {
        
        bool res;
        std::string gpuName;
        std::vector<NV_PHYSICAL_GPU_HANDLE> list_gpu;
        res = api.getGPUHandles(list_gpu);

        int index = 0;
        for (NV_PHYSICAL_GPU_HANDLE gpu : list_gpu) {
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
        return GetLastError();
    }

    return 0;
}