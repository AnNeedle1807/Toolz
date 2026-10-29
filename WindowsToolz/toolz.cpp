#include <SFML\Graphics.hpp>
#include <iostream>
#include <windows.h>
#include <string>
#include <vector>
#include <dwmapi.h>
#include <conio.h>
#include <winhttp.h>
#pragma comment(lib, "winhttp.lib")

WNDPROC originalProc = nullptr;

const std::string CURRENT_VERSION = "v0.3.0-beta";
bool checkedForUpdates = false;

enum class Sidebar {
    Settings,
    About,
    None
};

std::string HttpGet(const std::wstring& host, const std::wstring& path) {
    std::string result;
    HINTERNET session = WinHttpOpen(
        L"Windows Toolz Updater/1.0",
        WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
        WINHTTP_NO_PROXY_NAME,
        WINHTTP_NO_PROXY_BYPASS,
        0
    );
    if(!session) return result;
    HINTERNET connect = WinHttpConnect(
        session,
        host.c_str(),
        INTERNET_DEFAULT_HTTPS_PORT,
        0
    );
    if(!connect) {
        WinHttpCloseHandle(session);
        return result;
    }
    HINTERNET request = WinHttpOpenRequest(
        connect,
        L"GET",
        path.c_str(),
        NULL,
        WINHTTP_NO_REFERER,
        WINHTTP_DEFAULT_ACCEPT_TYPES,
        WINHTTP_FLAG_SECURE
    );
    if(request && WinHttpSendRequest(request, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0) && WinHttpReceiveResponse(request, NULL)) {
        DWORD size = 0;
        do {
            WinHttpQueryDataAvailable(
                request,
                &size
            );
            if(size == 0) break;
            std::vector<char> buffer(size + 1);
            DWORD downloaded = 0;
            WinHttpReadData(
                request,
                buffer.data(),
                size,
                &downloaded
            );
            result.append(
                buffer.data(),
                downloaded
            );
        } while(size > 0);
    }
    if(request) WinHttpCloseHandle(request);
    if(connect) WinHttpCloseHandle(connect);
    if(session) WinHttpCloseHandle(session);
    return result;
}

void CheckForUpdates() {
    std::string json = HttpGet(L"api.github.com", L"/repos/AnNeedle1807/Toolz/releases/latest");
    if(json.empty()) return;
    size_t pos = json.find("\"tag_name\":\"");
    if(pos == std::string::npos) return;
    pos += 12;
    size_t end = json.find('"', pos);
    if(end == std::string::npos) return;
    std::string latestVersion = json.substr(pos, end - pos);
    if(latestVersion == CURRENT_VERSION) return;
    int result = MessageBoxA(
        NULL,
        ("Update available!\n\nCurrent version: " + CURRENT_VERSION + "\nLatest version: " + latestVersion + "\n\nWould you like to open Releases?").c_str(), "Windows Toolz Update", MB_ICONINFORMATION | MB_YESNO);
    if(result == IDYES) {
        ShellExecuteA(
            NULL,
            "open",
            "https://github.com/AnNeedle1807/Toolz/releases/latest",
            NULL,
            NULL,
            SW_SHOW
        );
    }
}

bool GetSystemColorMode() {
    HKEY hKey;
    DWORD value = 1;
    DWORD size = sizeof(value);
    if(RegOpenKeyExA(
        HKEY_CURRENT_USER,
        "Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",
        0,
        KEY_READ,
        &hKey
    ) == ERROR_SUCCESS) {
        RegGetValueA(
            hKey,
            NULL,
            "AppsUseLightTheme",
            RRF_RT_REG_DWORD,
            NULL,
            &value,
            &size
        );
        RegCloseKey(hKey);
    }
    return value == 0;
}

void SaveColorMode(const std::string& mode) {
    HKEY hKey;
    RegCreateKeyExA(
        HKEY_CURRENT_USER,
        "Software\\WindowsToolz",
        0, NULL, 0, KEY_WRITE, NULL, &hKey, NULL
    );
    RegSetValueExA(
        hKey,
        "ColorMode",
        0,
        REG_SZ,
        (const BYTE*)mode.c_str(),
        mode.size() + 1
    );
    RegCloseKey(hKey);
}

std::string LoadColorMode() {
    HKEY hKey;
    char buffer[32] = {0};
    DWORD size = sizeof(buffer);
    if(RegOpenKeyExA(
        HKEY_CURRENT_USER,
        "Software\\WindowsToolz",
        0,
        KEY_READ,
        &hKey
    ) == ERROR_SUCCESS)
    {
        RegGetValueA(
            hKey,
            NULL,
            "ColorMode",
            RRF_RT_REG_SZ,
            NULL,
            buffer,
            &size
        );
        RegCloseKey(hKey);
        return buffer;
    }
    SaveColorMode("SystemDefault");
    return "SystemDefault";
}

void drawMenu(const std::vector<std::string>& MainMenuOptions, int selected) {
    system("cls");
    std::cout << " __          ___           _                     _______          _ \n";
    std::cout << " \\ \\        / (_)         | |                   |__   __|        | |\n";
    std::cout << "  \\ \\  /\\  / / _ _ __   __| | _____      _____     | | ___   ___ | |___\n";
    std::cout << "   \\ \\/  \\/ / | | '_ \\ / _` |/ _ \\ \\ /\\ / / __|    | |/ _ \\ / _ \\| |_  /\n";
    std::cout << "    \\  /\\  /  | | | | | (_| | (_) \\ V  V /\\__ \\    | | (_) | (_) | |/ /\n";
    std::cout << "     \\/  \\/   |_|_| |_|\\__,_|\\___/ \\_/\\_/ |___/    |_|\\___/ \\___/|_/___|\n\n";
    std::cout << "----------\n";
    for(int i = 0; i < MainMenuOptions.size(); i++) {
        if(i == selected) {
            std::cout << " > " << MainMenuOptions[i] << " <\n";
        } else {
            std::cout << "   " << MainMenuOptions[i] << "\n";
        }
    }
    std::cout << "----------\n";
    std::cout << "↓/↑ Go Up/Down | ENTER - Confirm";
}

void drawSettings(const std::vector<std::string>& SettingsOptions, int selected) {
    system("cls");
    std::cout << "SETTINGS\n\n";
    std::cout << "----------\n";
    for(int i = 0; i < SettingsOptions.size(); i++) {
        if(i == selected) {
            std::cout << " > " << SettingsOptions[i] << " <\n";
        } else {
            std::cout << "   " << SettingsOptions[i] << "\n";
        }
    }
    std::cout << "----------\n";
    std::cout << "↓/↑ Go Up/Down | ENTER - Confirm";
}

void drawBatchSettings(const std::vector<std::string>& BatchOptions, int selected) {
    system("cls");
    std::cout << "EDIT BATCH FILES\n\n";
    std::cout << "----------\n";
    for(int i = 0; i < BatchOptions.size(); i++) {
        if(i == selected) {
            std::cout << " > " << BatchOptions[i] << " <\n";
        } else {
            std::cout << "   " << BatchOptions[i] << "\n";
        }
    }
    std::cout << "----------\n";
    std::cout << "↓/↑ Go Up/Down | ENTER - Confirm";
}

void UpdateCliTheme() {
    HWND hwnd = GetConsoleWindow();
    std::string mode = LoadColorMode();
    BOOL useDark;
    if(mode == "Dark") {
        system("color 0F");
        useDark = TRUE;
    } else if(mode == "Light") {
        system("color F0");
        useDark = FALSE;
    } else {
        bool dark = GetSystemColorMode();
        if(dark) {
            system("color 0F");
        } else {
            system("color F0");
        }
        useDark = dark ? TRUE : FALSE;
    }
    DwmSetWindowAttribute(
        hwnd,
        20,
        &useDark,
        sizeof(useDark)
    );
}

LRESULT CALLBACK CustomWndProc(
    HWND hwnd,
    UINT msg,
    WPARAM wParam,
    LPARAM lParam) {
    if(msg == WM_GETMINMAXINFO) {
        MINMAXINFO* mmi = reinterpret_cast<MINMAXINFO*>(lParam);
        mmi->ptMinTrackSize.x = 1280;
        mmi->ptMinTrackSize.y = 720;
        return 0;
    }
    return CallWindowProc(
        originalProc,
        hwnd,
        msg,
        wParam,
        lParam
    );
}

int RunGUI() {
    char path[MAX_PATH];
    GetModuleFileNameA(NULL, path, MAX_PATH);
    std::string exePath(path);
    std::string dir = exePath.substr(0, exePath.find_last_of("\\/"));
    SetCurrentDirectoryA(dir.c_str());
    bool darkMode = false;
    std::string mode = LoadColorMode();
    if(mode == "Dark") {
        darkMode = true;
    } else if(mode == "Light") {
        darkMode = false;
    } else {
        darkMode = GetSystemColorMode();
    }
    sf::Color currentBg = darkMode ? sf::Color::Black : sf::Color::White;
    sf::Color targetBg = currentBg;
    bool animating = false;
    const float sidebarMargin = 10.f;
    sf::Clock animClock;
    sf::RenderWindow window(sf::VideoMode({1280, 720}), "Windows Toolz", sf::Style::Titlebar | sf::Style::Resize | sf::Style::Close); 
    HWND hwnd = window.getSystemHandle();
    originalProc = reinterpret_cast<WNDPROC>(
        SetWindowLongPtr(
            hwnd,
            GWLP_WNDPROC,
            reinterpret_cast<LONG_PTR>(CustomWndProc)
        )
    );
    BOOL useDark = darkMode ? TRUE : FALSE;
    DwmSetWindowAttribute(
        hwnd,
        20,
        &useDark,
        sizeof(useDark)
    );
    window.setView(sf::View(sf::FloatRect({0.f, 0.f}, {1280.f, 720.f})));
    window.setVerticalSyncEnabled(true);
    sf::Vector2u windowSize = window.getSize();
    float centerX = windowSize.x / 2.f;
    float centerY = windowSize.y / 2.f;
    const float sidebarWidth = 500.f;
    float sidebarX = windowSize.x - sidebarMargin;
    float targetSidebarX = sidebarX;
    sf::Image icon;
    if(icon.loadFromFile("assets\\images\\icon.png")) {
        window.setIcon(
            icon.getSize().x,
            icon.getSize().y,
            icon.getPixelsPtr()
        );
    }
    HCURSOR hand = LoadCursor(NULL, IDC_HAND);
    HCURSOR arrow = LoadCursor(NULL, IDC_ARROW);
    Sidebar currentSidebar = Sidebar::None;
    int selectedMain = 0;
    int selectedSettings = 0;
    int selectedAbout = 0;
    int selected = 0;
    //SIDEBAR BACKGROUND
    sf::RectangleShape sidebarBg;
    //SELECTOR
    sf::RectangleShape selector;
    selector.setSize(sf::Vector2f(400.f, 25.f));
    selector.setFillColor(darkMode ? sf::Color::White : sf::Color::Black);
    //FONTS
    sf::Font font; if(!font.loadFromFile("assets\\fonts\\Roboto.ttf")) return -1;
    //TEXTURES
    sf::Texture logoT; if(!logoT.loadFromFile("assets\\images\\icon.png")) return -1;
    sf::Texture UnknownT; if(!UnknownT.loadFromFile("assets\\images\\Unknown.png")) return -1;
    sf::Texture IconCacheT; if(!IconCacheT.loadFromFile("assets\\images\\IconCache.png")) return -1;
    sf::Texture Sun; if(!Sun.loadFromFile("assets\\images\\Light.png")) return -1;
    sf::Texture Moon; if(!Moon.loadFromFile("assets\\images\\Dark.png")) return -1;
    sf::Texture SystemFixT; if(!SystemFixT.loadFromFile("assets\\images\\SystemFix.png")) return -1;
    sf::Texture SettingsT; if(!SettingsT.loadFromFile("assets\\images\\Settings.png")) return -1;
    sf::Texture cliT; if(!cliT.loadFromFile("assets\\images\\cli.png")) return -1;
    sf::Texture AboutT; if(!AboutT.loadFromFile("assets\\images\\About.png")) return -1;
    sf::Texture GitHubT; if(!GitHubT.loadFromFile("assets\\images\\Github.png")) return -1;
    sf::Texture BatchT; if(!BatchT.loadFromFile("assets\\images\\Batch.png")) return -1;
    //HORIZONTAL LINE
    sf::RectangleShape hl;
    hl.setFillColor(darkMode ? sf::Color::White : sf::Color::Black);
    //MAIN-MENU SPRITES
    sf::Sprite logoS(logoT);
    sf::Sprite IconCacheS(IconCacheT);
    sf::Sprite IconCacheEdit(BatchT);
    sf::Sprite SystemFixS(SystemFixT);
    sf::Sprite SystemFixEdit(BatchT);
    sf::Sprite BatchS(BatchT);
    sf::Sprite GithubS(GitHubT);
    //SETTINGS SPRITES
    sf::Sprite ModeIcon(UnknownT);
    sf::Sprite cliS(cliT);
    //MAIN-MENU TEXTS
    sf::Text btnIconCache("Clear Icon Cache", font);
    sf::Text btnSystemFix("Find missing Windows Files", font);
    //SIDEBAR BUTTONS
    sf::RectangleShape SettingsBox;
    sf::RectangleShape AboutBox;
    sf::Sprite SettingsS(SettingsT); SettingsS.setScale({2.f, 2.f}); SettingsBox.setSize({40.f, 40.f}); SettingsBox.setFillColor(sf::Color::Black); SettingsBox.setOutlineThickness(3.f); SettingsBox.setOutlineColor(sf::Color(80, 80, 80)); 
    sf::Sprite AboutS(AboutT); AboutS.setScale({2.f, 2.f});AboutBox.setSize({40.f, 40.f}); AboutBox.setFillColor(sf::Color::Black); AboutBox.setOutlineThickness(3.f); AboutBox.setOutlineColor(sf::Color(80, 80, 80)); 
    //SETTINGS TEXTS
    sf::Text SettingsTitle("Settings", font);
    sf::Text btnColor("Mode: Unknown", font);
    sf::Text btnCliMode("Enter CLI Mode", font);
    //ABOUT TEXTS
    sf::Text AboutTitle("About", font);
    sf::Text AboutGithub("Check this program's GitHub Page", font);
    sf::Text AboutAuthor("Author: AnNeedle", font);
    sf::Text AboutVersion("Version: " + CURRENT_VERSION, font);
    std::vector<sf::Text*> texts = {
        &btnColor,
        &SettingsTitle,
        &btnIconCache,
        &btnSystemFix,
        &btnCliMode,
        &AboutTitle,
        &AboutGithub,
        &AboutAuthor,
        &AboutVersion
    };
    while(window.isOpen()) {
        bool hovering = false;
        auto mouse = sf::Mouse::getPosition(window);
        sf::Vector2f m((float)mouse.x, (float)mouse.y);
        if(currentSidebar == Sidebar::None) {
            if(IconCacheS.getGlobalBounds().contains(m) || btnIconCache.getGlobalBounds().contains(m)) {
                hovering = true;
                selectedMain = 0;
            }
            if(IconCacheEdit.getGlobalBounds().contains(m)) {
                hovering = true;
            }
            if(SystemFixS.getGlobalBounds().contains(m) || btnSystemFix.getGlobalBounds().contains(m)) {
                hovering = true;
                selectedMain = 1;
            }
            if(SystemFixEdit.getGlobalBounds().contains(m)) {
                hovering = true;
            }
            if(SettingsS.getGlobalBounds().contains(m)) {
                hovering = true;
            }   
            if(AboutS.getGlobalBounds().contains(m)) {
                hovering = true;
            }
        } else if(currentSidebar == Sidebar::Settings) {
            if(SettingsS.getGlobalBounds().contains(m)) {
                hovering = true;
            }
            if(AboutS.getGlobalBounds().contains(m)) {
                hovering = true;
            }
            if(ModeIcon.getGlobalBounds().contains(m) || btnColor.getGlobalBounds().contains(m)) {
                hovering = true;
                selectedSettings = 0;
            }
            if(cliS.getGlobalBounds().contains(m) || btnCliMode.getGlobalBounds().contains(m)) {
                hovering = true;
                selectedSettings = 1;
            }
        } else if(currentSidebar == Sidebar::About) {
            if(SettingsS.getGlobalBounds().contains(m)) {
                hovering = true;
            }
            if(AboutS.getGlobalBounds().contains(m)) {
                hovering = true;
            }
            if(GithubS.getGlobalBounds().contains(m) || AboutGithub.getGlobalBounds().contains(m)) {
                hovering = true;
                selectedAbout = 0;
            }
        }
        if(hovering) {
            SetClassLongPtr(hwnd, GCLP_HCURSOR, (LONG_PTR)hand);
        } else {
            SetClassLongPtr(hwnd, GCLP_HCURSOR, (LONG_PTR)arrow);
        }
        sidebarBg.setFillColor(darkMode ? sf::Color(30, 30, 30) : sf::Color(235, 235, 235));
        SettingsBox.setOutlineColor(darkMode ? sf::Color(30, 30, 30) : sf::Color(235, 235, 235));
        AboutBox.setOutlineColor(darkMode ? sf::Color(30, 30, 30) : sf::Color(235, 235, 235));
        sf::Event event;
        while(window.pollEvent(event)) {
            if(event.type == sf::Event::Closed) window.close();
            if(event.type == sf::Event::MouseButtonPressed) {
                auto mouse = sf::Mouse::getPosition(window);
                if(currentSidebar == Sidebar::None) {
                    if(IconCacheS.getGlobalBounds().contains({(float)mouse.x, (float)mouse.y}) || btnIconCache.getGlobalBounds().contains({(float)mouse.x, (float)mouse.y})) ShellExecuteA(NULL, "open", "batch\\IconCache.bat", NULL, NULL, SW_SHOW);
                    if(IconCacheEdit.getGlobalBounds().contains({(float)mouse.x, (float)mouse.y})) ShellExecuteA(NULL, "open", "notepad", "batch\\IconCache.bat", NULL, SW_SHOW);
                    if(SystemFixS.getGlobalBounds().contains({(float)mouse.x, (float)mouse.y}) || btnSystemFix.getGlobalBounds().contains({(float)mouse.x, (float)mouse.y})) ShellExecuteA(NULL, "runas", "batch\\SystemFix.bat", NULL, NULL, SW_SHOW);
                    if(SystemFixEdit.getGlobalBounds().contains({(float)mouse.x, (float)mouse.y})) ShellExecuteA(NULL, "open", "notepad", "batch\\SystemFix.bat", NULL, SW_SHOW);
                    if(SettingsS.getGlobalBounds().contains({(float)mouse.x, (float)mouse.y})) {
                        currentSidebar = Sidebar::Settings;
                        targetSidebarX = windowSize.x - 500.f;
                    }
                    if(AboutS.getGlobalBounds().contains({(float)mouse.x, (float)mouse.y})) {
                        currentSidebar = Sidebar::About;
                        targetSidebarX = windowSize.x - 500.f;
                    }
                }
                else if(currentSidebar == Sidebar::Settings) {
                    if(SettingsS.getGlobalBounds().contains({(float)mouse.x, (float)mouse.y})) {
                        targetSidebarX = windowSize.x - sidebarMargin;
                        currentSidebar = Sidebar::None;
                    }
                    if(AboutS.getGlobalBounds().contains({(float)mouse.x, (float)mouse.y})) currentSidebar = Sidebar::About;
                    if(ModeIcon.getGlobalBounds().contains({(float)mouse.x, (float)mouse.y}) || btnColor.getGlobalBounds().contains({(float)mouse.x, (float)mouse.y})) {
                        if(mode == "Light") {
                            mode = "Dark";
                            SaveColorMode("Dark");
                            btnColor.setString("Mode: Dark");
                            darkMode = true;
                            selector.setFillColor(sf::Color::White);
                            hl.setFillColor(sf::Color::White);
                            targetBg = sf::Color::Black;
                            animClock.restart();
                            animating = true;
                        } else if (mode == "Dark") {
                            mode = "SystemDefault";
                            SaveColorMode("SystemDefault");
                            btnColor.setString("Mode: System");
                            darkMode = GetSystemColorMode();
                            targetBg = darkMode ? sf::Color::Black : sf::Color::White;
                            selector.setFillColor(darkMode ? sf::Color::White : sf::Color::Black);
                            hl.setFillColor(darkMode ? sf::Color::White : sf::Color::Black);
                            animClock.restart();
                            animating = true;
                        } else {
                            mode = "Light";
                            SaveColorMode("Light");
                            btnColor.setString("Mode: Light");
                            darkMode = false;
                            targetBg = sf::Color::White;
                            selector.setFillColor(sf::Color::Black);
                            hl.setFillColor(sf::Color::Black);
                            animClock.restart();
                            animating = true;
                        }
                        BOOL useDark = darkMode ? TRUE : FALSE;
                        DwmSetWindowAttribute(window.getSystemHandle(), 20, &useDark, sizeof(useDark));
                    }
                    if(logoS.getGlobalBounds().contains({(float)mouse.x, (float)mouse.y}) || btnCliMode.getGlobalBounds().contains({(float)mouse.x, (float)mouse.y})) {
                        ShellExecuteA(NULL, "open", "toolz.exe", "cli", NULL, SW_SHOW);
                        return 0;
                    }
                } else if(currentSidebar == Sidebar::About) {
                    if(AboutS.getGlobalBounds().contains({(float)mouse.x, (float)mouse.y})) {
                        targetSidebarX = windowSize.x - sidebarMargin;
                        currentSidebar = Sidebar::None;
                    }
                    if(SettingsS.getGlobalBounds().contains({(float)mouse.x, (float)mouse.y})) {
                        currentSidebar = Sidebar::Settings;
                        targetSidebarX = windowSize.x - 500.f;
                    }
                    if(GithubS.getGlobalBounds().contains({(float)mouse.x, (float)mouse.y}) || AboutGithub.getGlobalBounds().contains({(float)mouse.x, (float)mouse.y})) ShellExecuteA(NULL, "open", "https://github.com/AnNeedle1807/Toolz", NULL, NULL, SW_SHOW);
                }
            }
            if(event.type == sf::Event::KeyPressed) {
                if(currentSidebar == Sidebar::None) {
                    if(event.key.code == sf::Keyboard::Down) {
                        selectedMain++;
                        if(selectedMain > 1) {
                            selectedMain = 0;
                        }
                    }
                    if(event.key.code == sf::Keyboard::Up) {
                        selectedMain--;
                        if(selectedMain < 0) {
                            selectedMain = 1;
                        }
                    }
                    if(event.key.code == sf::Keyboard::Enter) {
                        switch(selectedMain) {
                            case 0:
                                ShellExecuteA(NULL, "open", "batch\\IconCache.bat", NULL, NULL, SW_SHOW);
                                break;
                            case 1:
                                ShellExecuteA(NULL, "runas", "batch\\SystemFix.bat", NULL, NULL, SW_SHOW);
                                break;
                        }
                    }
                } else if(currentSidebar == Sidebar::Settings) {
                    if(event.key.code == sf::Keyboard::Down) {
                        selectedSettings++;
                        if(selectedSettings > 1) {
                            selectedSettings = 0;
                        }
                    }
                    if(event.key.code == sf::Keyboard::Up) {
                        selectedSettings--;
                        if(selectedSettings < 0) {
                            selectedSettings = 1;
                        }
                    }
                    if(event.key.code == sf::Keyboard::Enter) {
                        switch(selectedSettings) {
                            case 0: {
                                if(mode == "Light") {
                                    mode = "Dark";
                                    SaveColorMode("Dark");
                                    btnColor.setString("Mode: Dark");
                                    darkMode = true;
                                    selector.setFillColor(sf::Color::White);
                                    hl.setFillColor(sf::Color::White);
                                    targetBg = sf::Color::Black;
                                    animClock.restart();
                                    animating = true;
                                } else if (mode == "Dark") {
                                    mode = "SystemDefault";
                                    SaveColorMode("SystemDefault");
                                    btnColor.setString("Mode: System");
                                    darkMode = GetSystemColorMode();
                                    targetBg = darkMode ? sf::Color::Black : sf::Color::White;
                                    selector.setFillColor(darkMode ? sf::Color::White : sf::Color::Black);
                                    hl.setFillColor(darkMode ? sf::Color::White : sf::Color::Black);
                                    animClock.restart();
                                    animating = true;
                                } else {
                                    mode = "Light";
                                    SaveColorMode("Light");
                                    btnColor.setString("Mode: Light");
                                    darkMode = false;
                                    targetBg = sf::Color::White;
                                    selector.setFillColor(sf::Color::Black);
                                    hl.setFillColor(sf::Color::Black);
                                    animClock.restart();
                                    animating = true;
                                }
                                BOOL useDark = darkMode ? TRUE : FALSE;
                                DwmSetWindowAttribute(window.getSystemHandle(), 20, &useDark, sizeof(useDark));
                                break;
                            }
                            case 1:
                                ShellExecuteA(NULL, "open", "toolz", "cli", NULL, SW_SHOW);
                                return 0;
                                break;
                        }
                    }
                } else if(currentSidebar == Sidebar::About) {
                    if(event.key.code == sf::Keyboard::Down) {
                        selectedAbout++;
                        if(selectedAbout > 0) {
                            selectedAbout = 0;
                        }
                    }
                    if(event.key.code == sf::Keyboard::Up) {
                        selectedAbout--;
                        if(selectedAbout < 0) {
                            selectedAbout = 0;
                        }
                    }
                    if(event.key.code == sf::Keyboard::Enter) {
                        switch(selectedAbout) {
                            case 0:
                                ShellExecuteA(NULL, "open", "https://github.com/AnNeedle1807/Toolz", NULL, NULL, SW_SHOW);
                                break;
                        }
                    }
                }
            }
        }
        if(event.type == sf::Event::Resized) {
            sf::FloatRect visibleArea(
                0.f,
                0.f,
                static_cast<float>(event.size.width),
                static_cast<float>(event.size.height)
            );
            window.setView(sf::View(visibleArea));
            windowSize = window.getSize();
            centerX = windowSize.x / 2.f;
            centerY = windowSize.y / 2.f;
            if(currentSidebar == Sidebar::None) {
                targetSidebarX = windowSize.x - sidebarMargin;
            } else {
                targetSidebarX = windowSize.x - sidebarWidth;
            }
        }
        //COLOR SWITCHING ANIMATION
        if(animating) {
            float t = animClock.getElapsedTime().asSeconds() / 1.f;
            if(t >= 1.f) {
                t = 1.f;
                animating = false;
            }
            auto lerp = [](sf::Color a, sf::Color b, float t) {
                return sf::Color(
                    a.r + (b.r - a.r) * t,
                    a.g + (b.g - a.g) * t,
                    a.b + (b.b - a.b) * t
                );
            };
            currentBg = lerp(currentBg, targetBg, t);
        }
        sidebarX += (targetSidebarX - sidebarX) * 0.1f;
        window.clear(currentBg);
        sf::Color textColor = darkMode ? sf::Color::White : sf::Color::Black;
        for(auto t : texts) t->setFillColor(textColor);
        sf::Color selectedTextColor = darkMode ? sf::Color::Black : sf::Color::White;
        if(currentSidebar == Sidebar::None) {
            switch(selectedMain) {
                case 0:
                    btnIconCache.setFillColor(selectedTextColor);
                    break;
                case 1:
                    btnSystemFix.setFillColor(selectedTextColor);
                    break;
            }
        } else if(currentSidebar == Sidebar::Settings) {
            switch(selectedSettings) {
                case 0:
                    btnColor.setFillColor(selectedTextColor);
                    break;
                case 1:
                    btnCliMode.setFillColor(selectedTextColor);
                    break;
            }
        } else if(currentSidebar == Sidebar::About) {
            switch(selectedAbout) {
                case 0:
                    AboutGithub.setFillColor(selectedTextColor);
                    break;
            }
        }
        //WINDOW CONTENTS RENDERING
        if(currentSidebar == Sidebar::None) {
            selector.setPosition({0.f, 137.5f + selectedMain * 30.f});
            window.draw(selector);
            SettingsBox.setFillColor(darkMode ? sf::Color::Black : sf::Color::White);
            AboutBox.setFillColor(darkMode ? sf::Color::Black : sf::Color::White);
        }
        logoS.setScale({0.25f, 0.25f}); logoS.setOrigin({logoS.getLocalBounds().width / 2.f, logoS.getLocalBounds().height / 2.f}); logoS.setPosition({centerX, logoS.getGlobalBounds().height / 2.f + 5.f}); window.draw(logoS);
        hl.setOrigin({hl.getLocalBounds().width / 2.f, hl.getLocalBounds().height / 2.f}); hl.setSize(sf::Vector2f(windowSize.x - 20.f, 2.f)); hl.setPosition({centerX, logoS.getGlobalBounds().height + 10.f}); window.draw(hl);
        IconCacheS.setPosition({0.f, 140.f}); window.draw(IconCacheS);
        btnIconCache.setCharacterSize(24); btnIconCache.setPosition({30.f, 135.f}); window.draw(btnIconCache);
        IconCacheEdit.setPosition({400.f, 140.f}); window.draw(IconCacheEdit);
        SystemFixS.setPosition({0.f, 170.f}); window.draw(SystemFixS);
        btnSystemFix.setCharacterSize(24); btnSystemFix.setPosition({30.f, 165.f}); window.draw(btnSystemFix);
        SystemFixEdit.setPosition({400.f, 170.f}); window.draw(SystemFixEdit);
        sidebarBg.setPosition({sidebarX - 20.f, 0.f}); sidebarBg.setSize({520.f, static_cast<float>(windowSize.y)}); window.draw(sidebarBg);
        SettingsBox.setPosition({sidebarX - 50.f, 10.f}); window.draw(SettingsBox);
        AboutBox.setPosition({sidebarX - 50.f, 60.f}); window.draw(AboutBox);
        SettingsS.setOrigin({SettingsS.getLocalBounds().width, 0.f}); SettingsS.setPosition({sidebarX - 10.f, 10.f}); window.draw(SettingsS);
        AboutS.setOrigin({AboutS.getLocalBounds().width, 0.f}); AboutS.setPosition({sidebarX - 10.f, 60.f}); window.draw(AboutS);
        if(currentSidebar == Sidebar::Settings) {
            SettingsBox.setFillColor(darkMode ? sf::Color(30, 30, 30) : sf::Color(235, 235, 235));
            AboutBox.setFillColor(darkMode ? sf::Color::Black : sf::Color::White);
            SettingsS.setPosition({sidebarX - 10.f, 10.f});
            AboutS.setPosition({sidebarX - 10.f, 60.f});
            selector.setPosition({sidebarX, 77.5f + selectedSettings * 30.f}); window.draw(selector);
            SettingsTitle.setCharacterSize(50); SettingsTitle.setPosition({sidebarX, 0.f}); window.draw(SettingsTitle);
            ModeIcon.setPosition({sidebarX, 80.f}); window.draw(ModeIcon);
            if(mode == "Light") ModeIcon.setTexture(Sun);
                else if(mode == "Dark") ModeIcon.setTexture(Moon);
                else {
                    if(darkMode) {
                        ModeIcon.setTexture(Moon);
                    } else {
                        ModeIcon.setTexture(Sun);
                    }
                }
            if(mode == "Dark") btnColor.setString("Mode: Dark");
            else if(mode == "Light") btnColor.setString("Mode: Light");
            else btnColor.setString("Mode: System");
            btnColor.setCharacterSize(24); btnColor.setPosition({sidebarX + 30.f, 75.f}); window.draw(btnColor);
            cliS.setPosition({sidebarX, 107.5f}); window.draw(cliS);
            btnCliMode.setCharacterSize(24); btnCliMode.setPosition({sidebarX + 30.f, 105.f}); window.draw(btnCliMode);
        } else if(currentSidebar == Sidebar::About) {
            SettingsBox.setFillColor(darkMode ? sf::Color::Black : sf::Color::White);
            AboutBox.setFillColor(darkMode ? sf::Color(30, 30, 30) : sf::Color(235, 235, 235));
            selector.setPosition({sidebarX, 77.5f + selectedAbout * 30.f}); window.draw(selector);
            SettingsS.setPosition({sidebarX, 10.f});
            AboutS.setPosition({sidebarX, 60.f});
            AboutTitle.setCharacterSize(50); AboutTitle.setPosition({sidebarX, 0.f}); window.draw(AboutTitle);
            GithubS.setPosition({sidebarX, 80.f}); window.draw(GithubS);
            AboutGithub.setCharacterSize(24); AboutGithub.setPosition({sidebarX + 30.f, 75.f}); window.draw(AboutGithub);
            AboutAuthor.setCharacterSize(24); AboutAuthor.setPosition({sidebarX, 105.f}); window.draw(AboutAuthor);
            AboutVersion.setCharacterSize(24); AboutVersion.setPosition({sidebarX, 135.f}); window.draw(AboutVersion);
        }
        window.display();
        if(!checkedForUpdates) {
            CheckForUpdates();
            checkedForUpdates = true;
        }
    }
    return 0;
}

int RunCLI(int argc, char** argv) {
    std::string mode = LoadColorMode();
    if(mode == "Dark") {
        system("color 0F");
    }
    else if(mode == "Light") {
        system("color F0");
    }
    else {
        if(GetSystemColorMode()) {
            system("color 0F");
        } else {
            system("color F0");
        }
    }
    UpdateCliTheme();
    //MENU OPTIONS
    std::vector<std::string> MainMenuOptions = {
        "Clear Icon Cache",
        "Find missing system files",
        "Settings",
        "About"
    };
    std::vector<std::string> SettingsOptions = {
        "Back",
        "Mode = Unknown",
        "Edit Batch Files",
        "Switch to GUI"
    };
    std::vector<std::string> BatchOptions = {
        "Back",
        "Clear Icon Cache",
        "Find missing system files"
    };
    int selected = 0;
    int key;
    bool settingsOpen = false;
    system("title Windows Toolz CLI"); 
    while(true) {
        while(!settingsOpen) {
            drawMenu(MainMenuOptions, selected);
            if(!checkedForUpdates) {
                CheckForUpdates();
                checkedForUpdates = true;
            } 
            key = _getch();
            if(key == 72) {
                selected--;
                if (selected < 0) selected = MainMenuOptions.size() - 1;
            }
            if(key == 80) {
                selected++;
                if (selected >= MainMenuOptions.size()) selected = 0;
            }
            if(key == 13) {
                break;
            }
        }
        if(!settingsOpen) {
            switch(selected) {
                case 0:
                    ShellExecuteA(NULL, "open", "batch\\IconCache.bat", NULL, NULL, SW_SHOW);
                    break;
                case 1:
                    ShellExecuteA(NULL, "runas", "batch\\SystemFix.bat", NULL, NULL, SW_SHOW);
                    break;
                case 2:
                    settingsOpen = true;
                    selected = 0; 
                    break;
                case 3:
                    MessageBoxA(NULL, (std::string("Windows Toolz\nVersion: ") + CURRENT_VERSION).c_str(), "About", MB_ICONINFORMATION);
                    break;
            }
        }
        while(settingsOpen) {
            if(mode == "Dark") {
                SettingsOptions[1] = "Mode = Dark";
            } else if(mode == "Light") {
                    SettingsOptions[1] = "Mode = Light";
            } else {
                SettingsOptions[1] = "Mode = System";
            }
            drawSettings(SettingsOptions, selected);
            key = _getch();
            if(key == 72) {
                selected --;
                if(selected < 0) selected = SettingsOptions.size() - 1;
            }
            if(key == 80) {
                selected++;
                if(selected >= SettingsOptions.size()) selected = 0;
            }
            if(key == 13) {
                break;
            }
        }
        if(settingsOpen) {
            switch(selected) {
                case 0:
                    settingsOpen = false; 
                    break;
                case 1: 
                    if(mode == "Light") {
                        mode = "Dark";
                        SaveColorMode("Dark");
                        system("color 0F");
                    } else if(mode == "Dark") {
                        mode = "SystemDefault";
                        SaveColorMode("SystemDefault");
                        if(GetSystemColorMode()) {
                            system("color 0F");
                        } else {
                            system("color F0");
                        }
                    } else {
                        mode = "Light";
                        SaveColorMode("Light");
                        system("color F0");
                    }
                    UpdateCliTheme();
                    break;
                case 2: {
                    int batchSelected = 0;
                    bool batchOpen = true;
                    while(batchOpen) {
                        drawBatchSettings(BatchOptions, batchSelected);
                        key = _getch();
                        if(key == 72) {
                            batchSelected--;
                            if(batchSelected < 0) batchSelected = BatchOptions.size() - 1;
                        }
                        if(key == 80) {
                            batchSelected++;
                            if(batchSelected >= BatchOptions.size()) batchSelected = 0;
                        }
                        if(key == 13) {
                            switch(batchSelected) {
                                case 0:
                                    batchOpen = false;
                                    break;
                                case 1:
                                    ShellExecuteA(NULL, "open", "notepad", "batch\\IconCache.bat", NULL, SW_SHOW);
                                    break;
                                case 2:
                                    ShellExecuteA(NULL, "open", "notepad", "batch\\SystemFix.bat", NULL, SW_SHOW);
                                    break;
                            }
                        }
                    }
                    selected = 2;
                    break;
                }
                case 3:
                    ShellExecuteA(NULL, "open", "toolz.exe", NULL, NULL, SW_SHOW);
                    FreeConsole();
                    ExitProcess(0);
            }
        }
    }
    return 0;
}

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR lpCmdLine, int) {
    if(!lpCmdLine || strlen(lpCmdLine) == 0) {
        return RunGUI();
    }
    int argc = 0;
    char* argv[16];
    char* token = strtok(lpCmdLine, " ");
    while(token && argc < 16) {
        argv[argc++] = token;
        token = strtok(NULL, " ");
    }
    std::string cmd = argv[0];
    if(cmd == "cli") {
        AllocConsole();
        freopen("CONOUT$", "w", stdout);
        freopen("CONIN$", "r", stdin);
        SetConsoleOutputCP(CP_UTF8);
        SetConsoleCP(CP_UTF8);
        return RunCLI(argc, argv);
    }
    if(cmd == "clear-icon-cache") {
        ShellExecuteA(NULL, "open", "batch\\IconCache.bat", NULL, NULL, SW_SHOW);
        return 0;
    }
    if(cmd == "fix-system") {
        ShellExecuteA(NULL, "runas", "batch\\SystemFix.bat", NULL, NULL, SW_SHOW);
        return 0;
    }
    std::string msg = "Unknown argument: " + cmd;
    MessageBoxA(NULL, msg.c_str(), "Windows Toolz", MB_ICONERROR);
    return -1;
}