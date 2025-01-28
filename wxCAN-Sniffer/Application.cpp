#include "Application.h"
#include "FormMain.h"
#include <wx/wx.h>
#include <windows.h>
#include <wx/colour.h>

wxColour GetSystemColor(int colorIndex) {
    COLORREF color = GetSysColor(colorIndex);
    return wxColour(GetRValue(color), GetGValue(color), GetBValue(color));
}

bool IsDarkThemeEnabled() {
    HKEY hKey;
    DWORD value = 0;
    DWORD valueSize = sizeof(value);

    if (RegOpenKeyEx(HKEY_CURRENT_USER,
        L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",
        0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        RegQueryValueEx(hKey, L"AppsUseLightTheme", nullptr, nullptr, (LPBYTE)&value, &valueSize);
        RegCloseKey(hKey);
    }
    return value == 0;  // 0 — тёмная тема
}

void SetThemeColors(wxWindow* window, const WindowColors& colors)
{
    window->SetBackgroundColour(colors.WindowBackground);
    window->SetForegroundColour(colors.GridFont);

    auto children = window->GetChildren();
    for (auto child : children) {
        child->SetBackgroundColour(colors.WindowBackground);
        child->SetForegroundColour(colors.GridFont);

        // Рекурсивно применить цвета дочерним элементам
        SetThemeColors(child, colors);
    }

    window->Refresh();
}


bool Application::OnInit()
{
    if (locale.Init()) {
        // locale.AddCatalog(GetAppDir() + wxT("\\."));
    }

    WindowColors colors;

    colors.WindowBackground = GetSystemColor(COLOR_WINDOW);
    colors.GridSelectedBackground = GetSystemColor(COLOR_HIGHLIGHT);

    if (IsDarkThemeEnabled()) {
        colors.IsDark = true;
        colors.GridFont = wxColour(0xFFF8D4ul);
        colors.GridLines = wxColour(0x404040ul);
        colors.GridBackground = wxColour(0x16120Eul);
        colors.GridNewBackground = wxColour(0x0abe3bul);
        colors.GridUpdateBackground = *wxRED;
        colors.GraphFrame = wxColour(0xFFF8D4ul);
        colors.GraphBackground = wxColour(0x16120Eul);
        colors.GraphDraw = *wxRED;
        colors.GraphText = wxColour(0xFFF8D4ul);
        colors.WindowBackground = wxColour(0x5E4c45ul);
    }
    else {
        colors.IsDark = false;
        colors.GridFont = *wxBLACK;
        colors.GridLines = wxColour(0xC0C0C0ul);
        colors.GridBackground = *wxWHITE;
        colors.GridNewBackground = *wxGREEN;
        colors.GridUpdateBackground = *wxRED;
        colors.GraphFrame = *wxBLACK;
        colors.GraphBackground = *wxWHITE;
        colors.GraphDraw = *wxRED;
        colors.GraphText = *wxBLACK;
    }

    auto form = new FormMain(colors);
    SetThemeColors(form, colors);
    form->Show(true);
    SetTopWindow(form);

    return true;
}