#include <windows.h>

extern "C" __declspec(dllexport) void AbrirPrompt() {
    // Inicia o prompt de comando nativo na pasta de suporte a DLLs
    ShellExecuteA(NULL, "open", "cmd.exe", "/K cd plugins\\dllssuport", NULL, SW_SHOWNORMAL);
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    return TRUE;
}
