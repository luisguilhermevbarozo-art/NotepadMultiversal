#include <windows.h>
#include <commctrl.h>
#include <iostream>
#include <fstream>
#include <string>
#include <richedit.h> // Biblioteca nativa para controle de texto rico

#define IDC_COMBO_LANG 101
#define IDC_BTN_CMD 102
#define IDC_BTN_EXPORT 103
#define IDC_BTN_SITE_PY 104
#define IDC_BTN_SITE_PHP 105
#define IDC_BTN_SITE_LUA 106
#define IDC_BTN_SITE_CPP 107
#define IDC_MAIN_EDIT 108

HWND hCombo, hBtnCmd, hBtnExport, hEditCodigo;
HINSTANCE hRichEditLib = NULL;

void AbrirAtalhoInternet(const char* nomeAtalho) {
    std::string caminho = "plugins\\atalhos de internet\\" + std::string(nomeAtalho);
    ShellExecuteA(NULL, "open", caminho.c_str(), NULL, NULL, SW_SHOWNORMAL);
}

// Função para disparar a coloração do código
void ColorirTextoAgora() {
    int index = SendMessageA(hCombo, CB_GETCURSEL, 0, 0);
    char langSelecionada[32] = { 0 };
    SendMessageA(hCombo, CB_GETLBTEXT, index, (LPARAM)langSelecionada);

    // Só dispara se houver uma linguagem válida selecionada
    if (strcmp(langSelecionada, "Texto Comum") != 0) {
        HINSTANCE hSyntaxDll = LoadLibraryA("syntax.dll");
        if (hSyntaxDll) {
            // Nova assinatura de função: passamos o HWND do RichEdit e a linguagem escolhida
            typedef void (*FuncExecutarColoracao)(HWND, const char*);
            FuncExecutarColoracao ColorirSintaxe = (FuncExecutarColoracao)GetProcAddress(hSyntaxDll, "ColorirSintaxe");
            if (ColorirSintaxe) {
                ColorirSintaxe(hEditCodigo, langSelecionada);
            }
            FreeLibrary(hSyntaxDll);
        }
    }
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_CREATE: {
        // Carrega o motor avançado do Rich Edit do Windows
        hRichEditLib = LoadLibraryA("Msftedit.dll");

        CreateWindowA("STATIC", "Language:", WS_VISIBLE | WS_CHILD, 20, 20, 80, 20, hwnd, NULL, NULL, NULL);
        hCombo = CreateWindowA("COMBOBOX", "", WS_VISIBLE | WS_CHILD | CBS_DROPDOWNLIST, 110, 18, 150, 200, hwnd, (HMENU)IDC_COMBO_LANG, NULL, NULL);

        SendMessageA(hCombo, CB_ADDSTRING, 0, (LPARAM)"Texto Comum");
        SendMessageA(hCombo, CB_ADDSTRING, 0, (LPARAM)"C++");
        SendMessageA(hCombo, CB_ADDSTRING, 0, (LPARAM)"Python");
        SendMessageA(hCombo, CB_ADDSTRING, 0, (LPARAM)"PHP");
        SendMessageA(hCombo, CB_ADDSTRING, 0, (LPARAM)"Lua");
        SendMessageA(hCombo, CB_SETCURSEL, 0, 0);

        hBtnCmd = CreateWindowA("BUTTON", "CMD", WS_VISIBLE | WS_CHILD, 280, 15, 80, 28, hwnd, (HMENU)IDC_BTN_CMD, NULL, NULL);
        hBtnExport = CreateWindowA("BUTTON", "Exportar Trabalho", WS_VISIBLE | WS_CHILD, 380, 15, 150, 28, hwnd, (HMENU)IDC_BTN_EXPORT, NULL, NULL);

        CreateWindowA("STATIC", "Sites Oficiais:", WS_VISIBLE | WS_CHILD, 560, 20, 100, 20, hwnd, NULL, NULL, NULL);
        CreateWindowA("BUTTON", "Python", WS_VISIBLE | WS_CHILD, 660, 15, 70, 25, hwnd, (HMENU)IDC_BTN_SITE_PY, NULL, NULL);
        CreateWindowA("BUTTON", "PHP", WS_VISIBLE | WS_CHILD, 740, 15, 70, 25, hwnd, (HMENU)IDC_BTN_SITE_PHP, NULL, NULL);
        CreateWindowA("BUTTON", "Lua", WS_VISIBLE | WS_CHILD, 820, 15, 70, 25, hwnd, (HMENU)IDC_BTN_SITE_LUA, NULL, NULL);
        CreateWindowA("BUTTON", "C++", WS_VISIBLE | WS_CHILD, 900, 15, 70, 25, hwnd, (HMENU)IDC_BTN_SITE_CPP, NULL, NULL);

        // Trocamos "EDIT" por MSFTEDIT_CLASS (Rich Edit 4.1+) para aceitar formatação colorida por palavra
        hEditCodigo = CreateWindowExA(
            WS_EX_CLIENTEDGE,
            "RICHEDIT50W",
            "",
            WS_VISIBLE | WS_CHILD | WS_BORDER | ES_MULTILINE | ES_AUTOVSCROLL | ES_AUTOHSCROLL | WS_VSCROLL | WS_HSCROLL | ES_NOHIDESEL,
            20, 60, 950, 480,
            hwnd,
            (HMENU)IDC_MAIN_EDIT,
            NULL,
            NULL
        );

        HFONT hFont = CreateFontA(18, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FIXED_PITCH | FF_MODERN, "Courier New");
        SendMessageA(hEditCodigo, WM_SETFONT, (WPARAM)hFont, TRUE);

        // Avisa o RichEdit para notificar quando o texto mudar
        SendMessageA(hEditCodigo, EM_SETEVENTMASK, 0, ENM_CHANGE);
        break;
    }
    case WM_COMMAND: {
        int wmId = LOWORD(wParam);
        int wmEvent = HIWORD(wParam);

        // Se o usuário mudar a linguagem na caixinha, recolore o texto na hora
        if (wmId == IDC_COMBO_LANG && wmEvent == CBN_SELCHANGE) {
            ColorirTextoAgora();
        }

        // Captura o evento de alteração de texto digitado (Dispara a sintaxe em tempo real)
        if (wmId == IDC_MAIN_EDIT && wmEvent == EN_CHANGE) {
            // Para não dar loop infinito enquanto a DLL pinta, tiramos a máscara temporariamente
            SendMessageA(hEditCodigo, EM_SETEVENTMASK, 0, 0);
            ColorirTextoAgora();
            SendMessageA(hEditCodigo, EM_SETEVENTMASK, 0, ENM_CHANGE);
        }

        if (wmId == IDC_BTN_SITE_PY)  AbrirAtalhoInternet("Baixe o Python - Python.org.url");
        if (wmId == IDC_BTN_SITE_PHP) AbrirAtalhoInternet("PHP- Downloads.url");
        if (wmId == IDC_BTN_SITE_LUA) AbrirAtalhoInternet("Lua- baixar.url");
        if (wmId == IDC_BTN_SITE_CPP) AbrirAtalhoInternet("Compilador e a IDE do Visual Studio C-C++ para Windows.url");

        if (wmId == IDC_BTN_CMD) {
            HINSTANCE hCmdDll = LoadLibraryA("plugins\\dllssuport\\cmd.dll");
            if (hCmdDll) {
                typedef void (*FuncAbrirCmd)();
                FuncAbrirCmd AbrirPrompt = (FuncAbrirCmd)GetProcAddress(hCmdDll, "AbrirPrompt");
                if (AbrirPrompt) AbrirPrompt();
                FreeLibrary(hCmdDll);
            }
        }

        if (wmId == IDC_BTN_EXPORT) {
            char langSelecionada[32] = { 0 };
            int index = SendMessageA(hCombo, CB_GETCURSEL, 0, 0);
            SendMessageA(hCombo, CB_GETLBTEXT, index, (LPARAM)langSelecionada);

            std::string extensao = ".txt";
            if (strcmp(langSelecionada, "Python") == 0) extensao = ".py";
            if (strcmp(langSelecionada, "PHP") == 0) extensao = ".php";
            if (strcmp(langSelecionada, "Lua") == 0) extensao = ".lua";
            if (strcmp(langSelecionada, "C++") == 0) extensao = ".cpp";

            if (strcmp(langSelecionada, "C++") == 0) {
                HINSTANCE hCppDll = LoadLibraryA("plugins\\dllssuport\\cppfunction.dll");
                if (hCppDll) {
                    typedef void (*FuncVerificarCpp)();
                    FuncVerificarCpp VerificarCompilador = (FuncVerificarCpp)GetProcAddress(hCppDll, "VerificarCompilador");
                    if (VerificarCompilador) VerificarCompilador();
                    FreeLibrary(hCppDll);
                }
            }

            OPENFILENAMEA ofn;
            char szFile[MAX_PATH] = "trabalho";
            ZeroMemory(&ofn, sizeof(ofn));
            ofn.lStructSize = sizeof(ofn);
            ofn.hwndOwner = hwnd;
            ofn.lpstrFile = szFile;
            ofn.nMaxFile = sizeof(szFile);
            ofn.lpstrFilter = "Arquivos de Codigo\0*.*\0";
            ofn.nFilterIndex = 1;
            ofn.lpstrDefExt = extensao.c_str() + 1;
            ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;

            if (GetSaveFileNameA(&ofn) == TRUE) {
                int tamanhoTexto = GetWindowTextLengthA(hEditCodigo);
                std::string conteudoTexto(tamanhoTexto, '\0');
                GetWindowTextA(hEditCodigo, &conteudoTexto[0], tamanhoTexto + 1);

                std::ofstream arquivoSaida(ofn.lpstrFile);
                if (arquivoSaida.is_open()) {
                    arquivoSaida << conteudoTexto;
                    arquivoSaida.close();
                    MessageBoxA(hwnd, "Arquivo exportado com sucesso!", "Notemultiversal", MB_ICONINFORMATION);
                }
            }
        }
        break;
    }
    case WM_DESTROY:
        if (hRichEditLib) FreeLibrary(hRichEditLib);
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcA(hwnd, uMsg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    const char CLASS_NAME[] = "NotemultiversalWindowClass";
    WNDCLASSA wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

    RegisterClassA(&wc);
    HWND hwnd = CreateWindowExA(0, CLASS_NAME, "Notemultiversal", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 1000, 600, NULL, NULL, hInstance, NULL);

    if (hwnd == NULL) return 0;
    ShowWindow(hwnd, nCmdShow);

    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return 0;
}
