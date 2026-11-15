#pragma once
// ---------------------------------------------------------------------------
//  AcarsLegivel - as mensagens de avioes (ACARS, VDL2, HFDL) em portugues
//  facil de ler
//
//  O acarsdec, o dumpvdl2 e o dumphfdl entregam a mensagem crua: cabecalho
//  tecnico, "Label: H1", "#M1BPOSS23349W046028,UREMI,103452,182..." etc.
//  Aqui vira:
//
//      Relatorio de posicao (FMS)
//      Posicao: 23°34,9'S 046°02,8'W  ·  Ultimo fixo: UREMI as 10:34:52 UTC
//      Altitude: FL182 (~18.200 ft)
//
//  - o "label" (o tipo da mensagem no padrao ARINC 618/620/623) da o titulo;
//  - o conteudo refina (posicao, autorizacao de saida, ETA, meteorologia...);
//  - os aeroportos citados (codigo ICAO) ganham o nome da cidade;
//  - o voo ganha o nome da companhia (JJ3640 -> LATAM Brasil).
//  Nada disto usa a internet.
//
//  Multidecoder SDR / IC7610SDR - PU1XTB - GPL-3.0
// ---------------------------------------------------------------------------
#include <string>
#include <utility>
#include <vector>

namespace masdr {

struct MsgLegivel {
    std::string titulo;                                      // "Relatorio de posicao (FMS)"
    std::vector<std::pair<std::string, std::string>> campos; // ("Posicao", "23°34,9'S 046°02,8'W")
    std::string voo, prefixo, label, numero, companhia, hex; // do cabecalho (hex: endereco ICAO, VDL2)
    bool reconhecida = false;                                // achou um formato de mensagem de aviao
};

// bloco cru (linhas separadas por '\n' ou por "  |  ")
MsgLegivel lerMensagemAviao(const std::string& bloco);

// "Titulo  |  Campo: valor  |  Campo: valor" (uma parte por linha na lista do programa)
std::string textoLegivel(const MsgLegivel& m);

// separa a parte legivel da mensagem original dentro do texto gravado
extern const char* const kMarcaOriginal;                     // "  |  --- mensagem original ---  |  "

// "SBSP" -> "São Paulo/Congonhas" ("" se nao conhece)
const char* nomeAeroporto(const std::string& icao);

// numero com ponto de milhar: 18200 -> "18.200"
std::string milhar(long long v);

} // namespace masdr
