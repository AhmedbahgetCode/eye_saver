#include <windows.h>
#include <string>
#include <chrono>
#include <thread>

// متغير عالمي لتحديث الوقت في النافذة
int remainingSeconds = 20;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            
            RECT rect;
            GetClientRect(hwnd, &rect);
            
            // خلفية سوداء ونص أبيض
            SetTextColor(hdc, RGB(255, 255, 255));
            SetBkMode(hdc, TRANSPARENT);
            
            HFONT hFont = CreateFontW(60, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, 
                                     OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, 
                                     VARIABLE_PITCH, L"Arial");
            SelectObject(hdc, hFont);

            // نص التنبيه مع العداد
            std::wstring text = L"أرح عينيك الآن.. المتبقي: " + std::to_wstring(remainingSeconds) + L" ثانية";
            DrawTextW(hdc, text.c_str(), -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            
            DeleteObject(hFont);
            EndPaint(hwnd, &ps);
            return 0;
        }
        case WM_CLOSE: return 0; // منع الإغلاق اليدوي
        case WM_DESTROY: PostQuitMessage(0); return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void showReminder() {
    HINSTANCE hInstance = GetModuleHandle(NULL);
    WNDCLASSW wc = {0};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wc.lpszClassName = L"EyeCareClass";
    wc.hCursor = LoadCursor(NULL, IDC_WAIT);

    RegisterClassW(&wc);

    HWND hwnd = CreateWindowExW(
        WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
        L"EyeCareClass", L"Reminder",
        WS_POPUP | WS_VISIBLE,
        0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN),
        NULL, NULL, hInstance, NULL
    );

    // حلقة معالجة الرسائل مع تحديث العداد كل ثانية
    remainingSeconds = 20;
    auto lastTick = std::chrono::steady_clock::now();

    MSG msg = {0};
    while (remainingSeconds > 0) {
        // إذا كان هناك رسائل (مثل طلب إعادة رسم الشاشة)
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        // تحديث العداد كل ثانية واحدة
        auto now = std::chrono::steady_clock::now();
        if (std::chrono::duration_cast<std::chrono::seconds>(now - lastTick).count() >= 1) {
            remainingSeconds--;
            lastTick = now;
            InvalidateRect(hwnd, NULL, TRUE); // أمر بإعادة رسم النافذة لتحديث الرقم
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    DestroyWindow(hwnd);
    UnregisterClassW(L"EyeCareClass", hInstance);
}

int main() {
    // إخفاء الكونسول
    ShowWindow(GetConsoleWindow(), SW_HIDE);

    while (true) {
        std::this_thread::sleep_for(std::chrono::minutes(20));
        showReminder();
    }
    return 0;
}