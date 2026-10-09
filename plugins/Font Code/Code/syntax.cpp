#include <windows.h>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <richedit.h>

// Função auxiliar para aplicar cor em um intervalo específico de texto no Rich Edit
void PintarIntervalo(HWND hRichEdit, int inicio, int fim, COLORREF cor) {
    CHARFORMAT2A cf;
    ZeroMemory(&cf, sizeof(cf));
    cf.cbSize = sizeof(cf);
    cf.dwMask = CFM_COLOR;
    cf.crTextColor = cor;

    SendMessageA(hRichEdit, EM_SETSEL, inicio, fim);
    SendMessageA(hRichEdit, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
}

// Função para buscar palavras dentro de uma tag específica do seu arquivo XML
std::vector<std::string> extrairPalavrasDoXML(const std::string& conteudoXml, const std::string& grupo) {
    std::vector<std::string> palavras;
    std::string tagInicio = "<Keywords group=\"" + grupo + "\"";
    size_t posInicio = conteudoXml.find(tagInicio);

    if (posInicio == std::string::npos) return palavras;

    // Acha o fechamento da tag de abertura '>'
    posInicio = conteudoXml.find(">", posInicio);
    if (posInicio == std::string::npos) return palavras;
    posInicio++; // Pula o '>'

    size_t posFim = conteudoXml.find("</Keywords>", posInicio);
    if (posFim == std::string::npos) return palavras;

    // Pega o texto com as palavras separadas por espaço
    std::string blocoPalavras = conteudoXml.substr(posInicio, posFim - posInicio);
    std::stringstream ss(blocoPalavras);
    std::string palavra;

    while (ss >> palavra) {
        palavras.push_back(palavra);
    }
    return palavras;
}

bool PalavraExiste(const std::string& palavra, const std::vector<std::string>& lista) {
    for (const auto& item : lista) {
        if (palavra == item) return true;
    }
    return false;
}

extern "C" __declspec(dllexport) void ColorirSintaxe(HWND hRichEdit, const char* linguagem) {
    // 1. Descobre qual arquivo XML ler com base na seleção do Notemultiversal
    std::string arquivoXml = "";
    if (strcmp(linguagem, "C++") == 0) arquivoXml = "cpp_syntax.xml";
    else if (strcmp(linguagem, "Python") == 0) arquivoXml = "python_syntax.xml";
    else if (strcmp(linguagem, "PHP") == 0) arquivoXml = "PHP_syntax.xml";
    else if (strcmp(linguagem, "Lua") == 0) arquivoXml = "lua_syntax.xml";

    if (arquivoXml.empty()) return;

    // 2. Abre e lê o conteúdo do seu arquivo XML real na pasta
    std::string caminhoXml = "plugins\\XML\\syntax\\" + arquivoXml;
    std::ifstream arquivo(caminhoXml);
    if (!arquivo.is_open()) return;

    std::string conteudoXml((std::istreambuf_iterator<char>(arquivo)), std::istreambuf_iterator<char>());
    arquivo.close();

    // 3. Extrai as listas de palavras do XML dinamicamente
    std::vector<std::string> roxoKeywords = extrairPalavrasDoXML(conteudoXml, "keywords");
    std::vector<std::string> vermelhoKeywords = extrairPalavrasDoXML(conteudoXml, "errors");
    std::vector<std::string> amareloKeywords = extrairPalavrasDoXML(conteudoXml, "literals");

    // Evita efeito de piscar na tela enquanto redesenha
    SendMessageA(hRichEdit, WM_SETREDRAW, FALSE, 0);

    POINT scrollPos;
    SendMessageA(hRichEdit, EM_GETSCROLLPOS, 0, (LPARAM)&scrollPos);
    CHARRANGE crOriginal;
    SendMessageA(hRichEdit, EM_EXGETSEL, 0, (LPARAM)&crOriginal);

    int tamanhoTexto = GetWindowTextLengthA(hRichEdit);
    if (tamanhoTexto == 0) {
        SendMessageA(hRichEdit, WM_SETREDRAW, TRUE, 0);
        return;
    }

    std::string texto(tamanhoTexto + 1, '\0');
    GetWindowTextA(hRichEdit, &texto[0], tamanhoTexto + 1);

    // Reseta todo o texto para preto antes da nova varredura
    PintarIntervalo(hRichEdit, 0, tamanhoTexto, RGB(0, 0, 0));

    std::string palavraAtual = "";
    int inicioPalavra = -1;
    bool emString = false;
    char caractereAspas = '\0';
    int inicioString = -1;

    // 4. Analisador Léxico Geral de Varredura Blindado contra Recuos
    for (int i = 0; i <= tamanhoTexto; i++) {
        char c = (i < tamanhoTexto) ? texto[i] : '\0';

        // LÓGICA DE DETECÇÃO DE ASPAS UNIVERSAL (STRINGS) -> Ficam em AMARELO
        if ((c == '"' || c == '\'') && (i == 0 || texto[i - 1] != '\\')) {
            if (!emString) {
                emString = true;
                caractereAspas = c;
                inicioString = i;
            }
            else if (c == caractereAspas) {
                emString = false;
                PintarIntervalo(hRichEdit, inicioString, i + 1, RGB(180, 150, 0)); // Amarelo
                inicioString = -1;
            }
            palavraAtual = "";
            inicioPalavra = -1;
            continue;
        }

        if (emString) continue;

        // VERIFICADOR SEGURO MANUAL
        bool ehCaractereValido = ((c >= 'a' && c <= 'z') ||
            (c >= 'A' && c <= 'Z') ||
            (c >= '0' && c <= '9') ||
            c == '_' || c == '<' || c == '?' || c == '>' || c == '$' || c == '#');

        if (ehCaractereValido) {
            if (palavraAtual.empty()) {
                inicioPalavra = i;
            }
            palavraAtual += c;
        }
        else {
            if (!palavraAtual.empty()) {
                // Checa as listas que vieram direto do seu arquivo XML!
                if (PalavraExiste(palavraAtual, roxoKeywords)) {
                    PintarIntervalo(hRichEdit, inicioPalavra, i, RGB(128, 0, 128)); // Roxo
                }
                else if (PalavraExiste(palavraAtual, vermelhoKeywords)) {
                    PintarIntervalo(hRichEdit, inicioPalavra, i, RGB(255, 0, 0)); // Vermelho
                }
                else if (PalavraExiste(palavraAtual, amareloKeywords)) {
                    PintarIntervalo(hRichEdit, inicioPalavra, i, RGB(180, 150, 0)); // Amarelo
                }
                // Tratamento especial estável para PHP ($) e C++ (#)
                else if (palavraAtual[0] == '$' && strcmp(linguagem, "PHP") == 0) {
                    PintarIntervalo(hRichEdit, inicioPalavra, i, RGB(0, 110, 220)); // Azul
                }
                else if (palavraAtual == "#" && strcmp(linguagem, "C++") == 0) {
                    PintarIntervalo(hRichEdit, inicioPalavra, i, RGB(128, 0, 128)); // Roxo
                }

                palavraAtual = "";
                inicioPalavra = -1;
            }
        }
    }

    // Restaura o foco do cursor e reativa o desenho da tela
    SendMessageA(hRichEdit, EM_EXSETSEL, 0, (LPARAM)&crOriginal);
    SendMessageA(hRichEdit, EM_SETSCROLLPOS, 0, (LPARAM)&scrollPos);
    SendMessageA(hRichEdit, WM_SETREDRAW, TRUE, 0);
    InvalidateRect(hRichEdit, NULL, TRUE);
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    return TRUE;
}
