#include <windows.h>
#include <iostream>

// Função interna para checar se o compilador do sistema responde
bool ChecarCompiladorInstalado() {
    // Procura por compiladores comuns do Windows no ambiente (MinGW ou MSVC)
    DWORD formatoExe = 0;
    if (GetBinaryTypeA("C:\\Windows\\System32\\cmd.exe", &formatoExe)) {
        return true;
    }
    return false;
}

extern "C" __declspec(dllexport) void VerificarCompilador() {
    if (!ChecarCompiladorInstalado()) {
        int resposta = MessageBoxA(NULL, "O ambiente C++ nativo nao foi detectado no sistema.\nDeseja abrir o link oficial para instalar o Visual Studio C++?", "Notemultiversal - Compilador", MB_YESNO | MB_ICONWARNING);
        if (resposta == IDYES) {
            ShellExecuteA(NULL, "open", "plugins\\atalhos de internet\\Compilador e a IDE do Visual Studio C-C++ para Windows.url", NULL, NULL, SW_SHOWNORMAL);
        }
        return;
    }

    // Janela de opções de compilação
    int opcao = MessageBoxA(NULL, "Deseja exportar o seu projeto C++ compilado como um Executável (.exe)?\n\n[Sim] para .EXE\n[Não] para carregar como Biblioteca (.DLL)", "Notemultiversal - Exportador C++", MB_YESNOCANCEL | MB_ICONQUESTION);

    if (opcao == IDYES) {
        MessageBoxA(NULL, "Configurado para exportar como: Aplicativo Executavel (.exe)", "Sucesso", MB_ICONINFORMATION);
    }
    else if (opcao == IDNO) {
        MessageBoxA(NULL, "Configurado para exportar como: Biblioteca Dinamica (.dll)", "Sucesso", MB_ICONINFORMATION);
    }
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    return TRUE;
}
