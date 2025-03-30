#include <windows.h>
#include <shellapi.h>

#define WM_TRAYICON (WM_USER + 1)
#define ID_TRAY_EXIT 1001

typedef void (*HookFunc)();
HINSTANCE hInst;
NOTIFYICONDATA nid = { 0 };
HookFunc InstallHook = nullptr;
HookFunc UninstallHook = nullptr;
HMODULE hHookDLL = NULL;

void AddToStartup(const char* exePath, const char* appName) {
    HKEY hKey;
    if (RegOpenKey(HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\Run", &hKey) == ERROR_SUCCESS) {
        RegSetValueEx(hKey, appName, 0, REG_SZ, (BYTE*)exePath, (DWORD)(strlen(exePath) + 1));
        RegCloseKey(hKey);
    }
}

void AddTrayIcon(HWND hWnd) {
    nid.cbSize = sizeof(NOTIFYICONDATA);
    nid.hWnd = hWnd;
    nid.uID = 1;
    nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    nid.uCallbackMessage = WM_TRAYICON;
    nid.hIcon = LoadIcon(NULL, IDI_INFORMATION);
    lstrcpy(nid.szTip, TEXT("Keyboard Hook Running"));
    Shell_NotifyIcon(NIM_ADD, &nid);
}

void RemoveTrayIcon() {
    Shell_NotifyIcon(NIM_DELETE, &nid);
}

void ShowContextMenu(HWND hWnd) {
    POINT pt;
    GetCursorPos(&pt);
    HMENU hMenu = CreatePopupMenu();
    InsertMenu(hMenu, -1, MF_BYPOSITION, ID_TRAY_EXIT, TEXT("退出"));
    SetForegroundWindow(hWnd);
    TrackPopupMenu(hMenu, TPM_BOTTOMALIGN | TPM_LEFTALIGN, pt.x, pt.y, 0, hWnd, NULL);
    DestroyMenu(hMenu);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: AddTrayIcon(hWnd); break;
        case WM_TRAYICON:
            if (lParam == WM_RBUTTONUP) ShowContextMenu(hWnd);
            break;
        case WM_COMMAND:
            if (LOWORD(wParam) == ID_TRAY_EXIT) PostQuitMessage(0);
            break;
        case WM_DESTROY:
            RemoveTrayIcon();
            PostQuitMessage(0);
            break;
    }
    return DefWindowProc(hWnd, msg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int) {
    hInst = hInstance;

    char path[MAX_PATH];
    GetModuleFileName(NULL, path, MAX_PATH);
    AddToStartup(path, "KeyboardHookMonitor");

    hHookDLL = LoadLibrary(TEXT("myhook.dll"));
    if (!hHookDLL) {
        MessageBox(NULL, TEXT("无法加载 myhook.dll"), TEXT("错误"), MB_ICONERROR);
        return 1;
    }

    InstallHook = (HookFunc)GetProcAddress(hHookDLL, "InstallHook");
    UninstallHook = (HookFunc)GetProcAddress(hHookDLL, "UninstallHook");

    if (!InstallHook || !UninstallHook) {
        MessageBox(NULL, TEXT("找不到 Hook 函数"), TEXT("错误"), MB_ICONERROR);
        return 1;
    }

    InstallHook();

    const TCHAR CLASS_NAME[] = TEXT("TrayHookWindow");
    WNDCLASS wc = { };
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInst;
    wc.lpszClassName = CLASS_NAME;
    RegisterClass(&wc);

    HWND hWnd = CreateWindow(CLASS_NAME, TEXT("Keyboard Hook"), WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 300, 200,
        NULL, NULL, hInst, NULL);

    ShowWindow(hWnd, SW_HIDE);  // 隐藏窗口

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    UninstallHook();
    FreeLibrary(hHookDLL);
    return 0;
}