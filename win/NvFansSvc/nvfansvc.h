#pragma once

#include <windows.h>
#include <iostream>
#include "json.hpp"

VOID WINAPI ServiceMain(DWORD argc, LPTSTR* argv);
VOID WINAPI ServiceCtrlHandler(DWORD);
DWORD WINAPI ServiceWorkerThread(LPVOID lpParam);

VOID WINAPI ServiceMain(DWORD argc, LPTSTR* argv);

VOID WINAPI ServiceCtrlHandler(DWORD CtrlCode);

void LogError(HANDLE event_log, std::wstring message);
void LogSuccess(HANDLE event_log, std::wstring message);

bool loadConfig(HANDLE event_log, nlohmann::json& config);

DWORD WINAPI ServiceWorkerThread(LPVOID lpParam);