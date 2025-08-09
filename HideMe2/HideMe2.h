// HideMe2.h : Include file for standard system include files,
// or project specific include files.

#pragma once

#include <atomic>
#include <csignal>
#include <filesystem>
#include <iostream>
#include <string>
#include <tchar.h>
#include <thread>
#include <windows.h>

#define REG_SHOW_HIDDEN_FILES 1
#define REG_HIDE_HIDDEN_FILES 2
#define REG_SHOW_SUPERHIDDEN_FILES 1
#define REG_HIDE_SUPERHIDDEN_FILES 0

void signalHandler(int signal);

void HideFilesWithExtension(const std::wstring& folder, const std::wstring& extension);

void RefreshExplorerSettings();

bool HideFilesUsingRegistry(HKEY hRoot, const std::wstring& subKey, const std::wstring& valueName, DWORD data);