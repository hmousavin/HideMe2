#include "HideMe2.h"

std::atomic<bool> running(true);

void signalHandler(int signal) {
    if (signal == SIGINT) {
        std::wcout << L"\nTermination requested by user (Ctrl+C)...\n";
        running = false;
    }
}

std::vector<std::wstring> GetAllDrives() {
    DWORD bufferSize = GetLogicalDriveStringsW(0, nullptr);
    std::vector<wchar_t> buffer(bufferSize + 1);
    GetLogicalDriveStringsW(bufferSize, buffer.data());

    std::vector<std::wstring> drives;
    wchar_t* drive = buffer.data();

    while (*drive) {
        drives.emplace_back(drive);
        drive += wcslen(drive) + 1;
    }

    return drives;
}

void HideFilesWithExtension(const std::wstring& folder, const std::wstring& extension) {
    
    for (const auto& entry : std::filesystem::recursive_directory_iterator(folder, 
        std::filesystem::directory_options::skip_permission_denied)) {
        try
        {
            if (!entry.is_regular_file()) continue;

            const auto& path = entry.path();
            const std::wstring ext = path.extension().wstring();

            if (_wcsicmp(ext.c_str(), extension.c_str()) == 0) {
                DWORD attributes = GetFileAttributesW(path.c_str());

                if (attributes == INVALID_FILE_ATTRIBUTES) {
                    std::wcerr << L"Failed to get attributes for: " << path << '\n';
                    continue;
                }

                // Remove read-only flag if set
                bool wasReadOnly = (attributes & FILE_ATTRIBUTE_READONLY) != 0;
                if (wasReadOnly) {
                    attributes &= ~FILE_ATTRIBUTE_READONLY;
                }

                attributes |= (FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM);

                if (!SetFileAttributesW(path.c_str(), attributes)) {
                    std::wcerr << L"Failed to set attributes for: " << path << '\n';
                }
                else {
                    std::wcout << L"Hidden: " << path << '\n';
                }
            }
        }
        catch (const std::filesystem::filesystem_error& e) {
            std::wcerr << L"Filesystem error in " << folder << L": " << e.what() << L"\n";
        }
    }
}

void RefreshExplorerSettings() {
    // Notify Explorer to update its settings
    SendMessageTimeoutW(HWND_BROADCAST, WM_SETTINGCHANGE, 0, (LPARAM)L"ShellState", SMTO_ABORTIFHUNG, 100, nullptr);
}

bool HideFilesUsingRegistry(HKEY hRoot, const std::wstring& subKey, const std::wstring& valueName, DWORD data) {
    HKEY hKey;
    if (RegOpenKeyExW(hRoot, subKey.c_str(), 0, KEY_SET_VALUE | KEY_QUERY_VALUE, &hKey) != ERROR_SUCCESS) {
        std::wcerr << L"Failed to open registry key: " << subKey << '\n';
        return false;
    }

    DWORD type = 0;
    DWORD value = 0;
    DWORD size = sizeof(value);

    if (RegQueryValueExW(hKey, valueName.c_str(), nullptr, &type, reinterpret_cast<LPBYTE>(&value), &size) != ERROR_SUCCESS) {
        std::wcerr << L"Failed to query registry value: " << valueName << '\n';
        RegCloseKey(hKey);
        return false;
    }

    if (type == REG_DWORD && value != data) {
        if (RegSetValueExW(hKey, valueName.c_str(), 0, REG_DWORD, reinterpret_cast<const BYTE*>(&data), sizeof(data)) != ERROR_SUCCESS) {
            std::wcerr << L"Failed to set registry value: " << valueName << '\n';
            RegCloseKey(hKey);
            return false;
        }
        else
            RefreshExplorerSettings();
    }

    RegCloseKey(hKey);
    return true;
}

void UnHideFilesWithExtension(const std::wstring& folder, const std::wstring& extension) {
    for (const auto& entry : std::filesystem::recursive_directory_iterator(folder,
        std::filesystem::directory_options::skip_permission_denied)) {
        try {
            if (!entry.is_regular_file()) continue;

            const auto& path = entry.path();
            const std::wstring ext = path.extension().wstring();

            if (_wcsicmp(ext.c_str(), extension.c_str()) == 0) {
                DWORD attributes = GetFileAttributesW(path.c_str());

                if (attributes == INVALID_FILE_ATTRIBUTES) {
                    std::wcerr << L"Failed to get attributes for: " << path << '\n';
                    continue;
                }

                // Remove hidden and system attributes
                if (attributes & FILE_ATTRIBUTE_HIDDEN) {
                    attributes &= ~FILE_ATTRIBUTE_HIDDEN;
                }
                if (attributes & FILE_ATTRIBUTE_SYSTEM) {
                    attributes &= ~FILE_ATTRIBUTE_SYSTEM;
                }

                if (!SetFileAttributesW(path.c_str(), attributes)) {
                    std::wcerr << L"Failed to set attributes for: " << path << '\n';
                }
                else {
                    std::wcout << L"Unhidden: " << path << '\n';
                }
            }
        }
        catch (const std::filesystem::filesystem_error& e) {
            std::wcerr << L"Filesystem error in " << folder << L": " << e.what() << L"\n";
        }
    }
}


int main() {
    std::signal(SIGINT, signalHandler); // Handle Ctrl+C

    const std::wstring extension = L".hideme";

    auto drives = GetAllDrives();
    for (const auto& drive : drives) {
        std::wcout << L"Scanning drive: " << drive << L"\n";
        try {
            HideFilesWithExtension(drive, extension);
            RefreshExplorerSettings();
        }
        catch (const std::exception& e) {
            std::wcerr << L"Error scanning " << drive << L": " << e.what() << L"\n";
        }
    }

    const std::wstring regPath = L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced";
    std::thread enforcer([&]() {
        const std::wstring valueName = L"Hidden";

        while (running) {
            HideFilesUsingRegistry(HKEY_CURRENT_USER, regPath, L"ShowSuperHidden", REG_HIDE_SUPERHIDDEN_FILES);
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
    });

    // Wait until user interrupts (Ctrl+C)
    while (running) {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    // Restore the visibility of hidden files
    if (HideFilesUsingRegistry(HKEY_CURRENT_USER, regPath, L"Hidden", REG_SHOW_SUPERHIDDEN_FILES)) {
        for (const auto& drive : drives) {
            std::wcout << L"Scanning drive: " << drive << L"\n";
            try {
                UnHideFilesWithExtension(drive, extension);
                RefreshExplorerSettings();
            }
            catch (const std::exception& e) {
                std::wcerr << L"Error scanning " << drive << L": " << e.what() << L"\n";
            }
        }
        
        std::wcout << L"Restored hidden file visibility.\n";
    }

    enforcer.join();

    return 0;
}
