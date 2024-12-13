#include <windows.h>
#include <tlhelp32.h>
#include <thread>
#include <vector>
#include <string>
#include <shellapi.h>

const std::vector<std::string> MYDOCKFINDER_PROCESS_NAMES = {
    "Mydock.exe", "dockmod64.exe", "dock.exe", "dockmod.exe", "Dock_64.exe", "Dock_32.exe"
};

const std::string MYDOCKFINDER_PATH = "C:\\Program Files\\MyDockFinder\\Dock_64.exe"; // Percorso principale di MyDockFinder

NOTIFYICONDATA nid = {};

bool is_process_running(const std::string& process_name) {
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        return false;
    }

    PROCESSENTRY32 pe;
    pe.dwSize = sizeof(PROCESSENTRY32);

    if (Process32First(hSnapshot, &pe)) {
        do {
            if (process_name == pe.szExeFile) {
                CloseHandle(hSnapshot);
                return true;
            }
        } while (Process32Next(hSnapshot, &pe));
    }

    CloseHandle(hSnapshot);
    return false;
}

void stop_process(const std::vector<std::string>& process_names) {
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        return;
    }

    PROCESSENTRY32 pe;
    pe.dwSize = sizeof(PROCESSENTRY32);

    if (Process32First(hSnapshot, &pe)) {
        do {
            for (const auto& process_name : process_names) {
                if (process_name == pe.szExeFile) {
                    HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pe.th32ProcessID);
                    if (hProcess) {
                        TerminateProcess(hProcess, 0);
                        CloseHandle(hProcess);
                    }
                }
            }
        } while (Process32Next(hSnapshot, &pe));
    }

    CloseHandle(hSnapshot);
}

void start_process(const std::string& process_path) {
    ShellExecuteA(nullptr, "open", process_path.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
}

bool is_fullscreen() {
    RECT desktopRect;
    HWND hwnd = GetForegroundWindow();

    if (hwnd) {
        HMONITOR monitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTOPRIMARY);
        MONITORINFO mi = { sizeof(mi) };
        if (GetMonitorInfo(monitor, &mi)) {
            GetWindowRect(hwnd, &desktopRect);
            return (desktopRect.left == mi.rcMonitor.left &&
                    desktopRect.top == mi.rcMonitor.top &&
                    desktopRect.right == mi.rcMonitor.right &&
                    desktopRect.bottom == mi.rcMonitor.bottom &&
                    !(GetWindowLong(hwnd, GWL_STYLE) & WS_BORDER));
        }
    }

    return false;
}

void wait_for_fullscreen_change(bool& dockfinder_running, const std::vector<std::string>& process_names, const std::string& process_path) {
    while (true) {
        if (is_fullscreen() && dockfinder_running) {
            stop_process(process_names);
            dockfinder_running = false;
        } else if (!is_fullscreen() && !dockfinder_running) {
            start_process(process_path);
            dockfinder_running = true;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_COMMAND:
            if (LOWORD(wParam) == 1) {
                PostQuitMessage(0);
            }
            break;

        case WM_DESTROY:
            Shell_NotifyIcon(NIM_DELETE, &nid);
            PostQuitMessage(0);
            break;

        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    const char* className = "MyDockFinderToggleClass";

    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = className;

    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(
        0, className, "MyDockFinderToggle", WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        nullptr, nullptr, hInstance, nullptr);

    nid.cbSize = sizeof(NOTIFYICONDATA);
    nid.hWnd = hwnd;
    nid.uID = 1;
    nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    nid.uCallbackMessage = WM_APP + 1;
    nid.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
    strcpy_s(nid.szTip, "MyDockFinder Toggle");

    Shell_NotifyIcon(NIM_ADD, &nid);

    bool dockfinder_running = true;
    std::thread monitor_thread(wait_for_fullscreen_change, std::ref(dockfinder_running), MYDOCKFINDER_PROCESS_NAMES, MYDOCKFINDER_PATH);
    monitor_thread.detach();

    MSG msg = {};
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}