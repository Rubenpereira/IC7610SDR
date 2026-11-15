// ---------------------------------------------------------------------------
//  Ui - mensagens de avioes em portugues facil de ler (ver app/AcarsLegivel.h)
//
//  ACARS, VDL2 e HFDL: o texto dos decodificadores externos chega em blocos
//  (uma mensagem por bloco, separados por linha em branco). Cada bloco vira:
//
//      [13:18:59 UTC]  VDL2  » Relatorio de ETA
//          Voo: G31833 · GOL Linhas Aereas · PS-GPK
//          Chegada estimada (ETA): 13:33 UTC
//
//  Mensagem longa em varios blocos ("ETB") vira uma so; as repetidas somem.
//  "Mensagem original" poe embaixo o texto cru, como o decodificador entregou.
//
//  ADS-B: um aviso por aviao (voo, companhia, altitude, velocidade, rumo...),
//  outro quando decola ou pousa e um ALERTA nos codigos 7500 / 7600 / 7700.
//
//  IC7610SDR - PU1XTB - GPL-3.0
// ---------------------------------------------------------------------------
#include "Ui.h"

#include "../app/AcarsLegivel.h"
#include "../app/Config.h"
#include "Idioma.h"

#include "imgui.h"

#include <windows.h>

#include <cmath>
#include <cstdio>
#include <cstdlib>

namespace masdr {

namespace {
std::string horaUtcAgora()
{
    SYSTEMTIME st; GetSystemTime(&st);
    char b[24];
    std::snprintf(b, sizeof b, "%02d:%02d:%02d UTC", st.wHour, st.wMinute, st.wSecond);
    return b;
}
double segAgora() { return (double)GetTickCount64() / 1000.0; }
const char* nomeDec(int t)
{
    return t == Decoders::ACARS ? "ACARS" : t == Decoders::VDL2 ? "VDL2" : t == Decoders::HFDL ? "HFDL" : t == Decoders::ADSB ? "ADS-B" : "";
}
} // namespace

bool Ui::decAviao() const
{
    return decTipo_ == Decoders::ACARS || decTipo_ == Decoders::VDL2 || decTipo_ == Decoders::HFDL;
}

// as duas caixinhas, nas janelas do ACARS, VDL2, HFDL e ADS-B
void Ui::opcoesLegivel()
{
    if (ImGui::Checkbox(T("Português fácil de ler"), &decLegivel_)) {
        Config::instance().set("dec_legivel", decLegivel_);
        if (!decLegivel_) fecharBlocoAviao(true, true);
    }
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("%s", T("Cada mensagem ganha um título (\"Relatório de posição (FMS)\", \"Autorização de saída (PDC)\"...)\n"
                                  "e os campos explicados: voo e companhia, posição, altitude, rota, ETA, pista, transponder.\n"
                                  "Desmarcado: o texto aparece como o decodificador entrega."));
    if (decTipo_ == Decoders::ADSB || !decLegivel_) return;
    ImGui::SameLine();
    if (ImGui::Checkbox(T("Mensagem original"), &decOriginal_)) Config::instance().set("dec_original", decOriginal_);
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", T("Mostra também, embaixo de cada mensagem, o texto cru do decodificador."));
}

// texto novo dos decodificadores: ACARS / VDL2 / HFDL juntam o bloco; o resto vai direto
void Ui::textoDecoder(const std::string& novo)
{
    const bool legAviao = decLegivel_ && decAviao();
    const bool legAdsb = decLegivel_ && decTipo_ == Decoders::ADSB;
    if (!legAviao && !legAdsb) { anexarDecTexto(novo); return; }
    decPedaco_ += novo;
    size_t a = 0;
    std::string direto;
    for (;;) {
        const size_t b = decPedaco_.find('\n', a);
        if (b == std::string::npos) break;
        std::string l = decPedaco_.substr(a, b - a);
        a = b + 1;
        while (!l.empty() && (l.back() == '\r' || l.back() == ' ')) l.pop_back();
        if (legAdsb) {
            // "[ADSB] 13:31:26 UTC  novo  E4998E" / "... voo PSTHG": viram os avisos legiveis (avisosAdsb)
            if (l.rfind("[ADSB] ", 0) == 0 && (l.find(" novo ") != std::string::npos || l.find(" voo ") != std::string::npos)) continue;
            direto += l + "\n";
            continue;
        }
        // avisos do proprio programa ("[ACARS] acarsdec iniciado...") e a apresentacao dos externos: direto
        const bool aviso = l.size() > 3 && l[0] == '[' && l.find("] ") != std::string::npos && l.find("] ") < 12 &&
                           l.compare(0, 2, "[#") != 0 && l.compare(0, 3, "[20") != 0;
        if (aviso || l.rfind("dumpvdl2 ", 0) == 0 || l.rfind("dumphfdl ", 0) == 0 || l.rfind("acarsdec", 0) == 0 ||
            l.rfind("Sampling rate", 0) == 0 || l.rfind("Starting, decoding", 0) == 0 || l.rfind("System table loaded", 0) == 0 ||
            l.rfind("Aircraft cache TTL", 0) == 0 || l.rfind("Initialized ", 0) == 0 || l.rfind("Using ", 0) == 0) {
            if (!direto.empty()) { anexarDecTexto(direto); direto.clear(); }
            anexarDecTexto(l + "\n");
            continue;
        }
        if (l.empty()) { fecharBlocoAviao(true); continue; }
        // cabecalho de outra mensagem: a anterior acabou
        if ((l.compare(0, 2, "[#") == 0 || l.compare(0, 3, "[20") == 0) && !decBloco_.empty()) fecharBlocoAviao(true);
        if (!decBloco_.empty()) decBloco_ += "  |  ";
        decBloco_ += l;
        decBlocoT_ = segAgora();
        if (decBloco_.size() > 4000) fecharBlocoAviao(true);
    }
    decPedaco_.erase(0, a);
    if (!direto.empty()) anexarDecTexto(direto);
}

// fecha o bloco que esta juntando (agora, ou se ficou 1,2 s sem linha nova) e a mensagem
// pendente de varios blocos (15 s sem o proximo)
void Ui::fecharBlocoAviao(bool agora, bool tudo)
{
    const double t = segAgora();
    if (!decBloco_.empty() && (agora || t - decBlocoT_ > 1.2)) {
        std::string b;
        b.swap(decBloco_);
        mensagemAviao(b);
    }
    if (!decPend_.empty() && (tudo || t - decPendT_ > 15.0)) {
        std::string p;
        p.swap(decPend_);
        mensagemAviao(p + "  |  .");
    }
}

void Ui::mensagemAviao(std::string bloco)
{
    const double t = segAgora();
    auto corpo = [](const std::string& b) { const size_t p = b.find("  |  "); return p == std::string::npos ? b : b.substr(p + 5); };
    auto terminaEtb = [](const std::string& b) { return b.size() >= 3 && b.compare(b.size() - 3, 3, "ETB") == 0; };
    MsgLegivel m = lerMensagemAviao(bloco);
    const std::string chave = m.voo + "|" + m.prefixo + "|" + (m.numero.size() == 4 ? m.numero.substr(0, 3) : m.numero);
    if (!decPend_.empty() && decPendChave_ != chave) { std::string p; p.swap(decPend_); mensagemAviao(p + "  |  ."); }
    // repetida (dois canais ouviram a mesma; o aviao repete quando nao recebe a confirmacao)
    const std::string assinatura = chave + "#" + corpo(bloco);
    while (!decRecentes_.empty() && t - decRecentes_.front().second > 90.0) decRecentes_.pop_front();
    for (const auto& r : decRecentes_) if (r.first == assinatura) return;
    decRecentes_.push_back({assinatura, t});
    if (decRecentes_.size() > 200) decRecentes_.pop_front();
    if (m.reconhecida && m.numero.size() == 4 && terminaEtb(bloco)) {
        if (decPend_.empty()) { decPend_ = bloco; decPendChave_ = chave; }
        else decPend_ += "  |  " + corpo(bloco);
        decPendT_ = t;
        return;
    }
    if (!decPend_.empty() && decPendChave_ == chave) {
        bloco = decPend_ + "  |  " + corpo(bloco);
        decPend_.clear();
        m = lerMensagemAviao(bloco);
    }
    auto linhas = [](const std::string& s, const char* recuo) {
        std::string o;
        size_t a = 0;
        for (;;) {
            const size_t b = s.find("  |  ", a);
            o += recuo + s.substr(a, b == std::string::npos ? std::string::npos : b - a) + "\n";
            if (b == std::string::npos) break;
            a = b + 5;
        }
        return o;
    };
    std::string o = "\n[" + horaUtcAgora() + "]  " + nomeDec(decTipo_) + "  ";
    if (!m.reconhecida) { o += "\n" + linhas(bloco, "    "); anexarDecTexto(o); return; }
    o += "» " + m.titulo + "\n";
    for (const auto& c : m.campos) o += "    " + c.first + ": " + c.second + "\n";
    if (decOriginal_) o += "    --- mensagem original ---\n" + linhas(bloco, "      ");
    anexarDecTexto(o);
}

// ADS-B: um aviso legivel por aviao, decolou / pousou e os codigos de emergencia
void Ui::avisosAdsb(const std::vector<AdsbCore::Aviao>& avs)
{
    if (!decLegivel_) return;
    const double agora = segAgora();
    if (agora - adsbAvisosT_ < 2.0) return;
    adsbAvisosT_ = agora;
    for (const auto& av : avs) {
        if (!av.ativo || av.idade > 15) continue;
        AvisoAdsb& a = adsbAvisos_[av.hex];
        if (a.t == 0) a.t = agora;
        auto linha = [&](const std::string& titulo) {
            std::string s = "\n[" + horaUtcAgora() + "]  ADS-B  » " + titulo + "\n";
            std::string v = av.voo;
            if (!av.empresa.empty()) v += (v.empty() ? "" : " · ") + av.empresa;
            if (!av.prefixo.empty()) v += (v.empty() ? "" : " · ") + av.prefixo;
            if (!v.empty()) s += "    Voo: " + v + "\n";
            if (!av.tipo.empty()) s += "    Modelo: " + av.tipo + "\n";
            if (!av.pais.empty()) s += "    País do registro: " + av.pais + "\n";
            if (av.noChao) s += "    No solo\n";
            else if (av.altitude != AdsbCore::kSemValor) {
                s += "    Altitude: " + milhar(av.altitude) + " ft";
                if (av.vertical != AdsbCore::kSemValor && std::abs(av.vertical) >= 300)
                    s += av.vertical > 0 ? " (subindo " + milhar(av.vertical) + " ft/min)" : " (descendo " + milhar(-av.vertical) + " ft/min)";
                s += "\n";
            }
            if (av.velocidade >= 0)
                s += "    Velocidade: " + milhar(std::lround(av.velocidade)) + " nós (~" + milhar(std::lround(av.velocidade * 1.852)) + " km/h)\n";
            char b[48];
            if (av.rumo >= 0) { std::snprintf(b, sizeof b, "    Rumo: %03.0f°\n", av.rumo); s += b; }
            if (av.squawk >= 0) { std::snprintf(b, sizeof b, "    Transponder: %04d\n", av.squawk); s += b; }
            if (av.lat <= 90 && av.lon <= 180) { std::snprintf(b, sizeof b, "    Posição: %.4f  %.4f\n", av.lat, av.lon); s += b; }
            s += "    Código ICAO: " + av.hex + "\n";
            anexarDecTexto(s);
        };
        if (!a.anunciado && (!av.voo.empty() || agora - a.t > 45.0)) {
            a.anunciado = true; a.semVoo = av.voo.empty();
            linha(av.noChao ? "Avião detectado (no solo)" : "Avião detectado");
        } else if (a.anunciado && a.semVoo && !av.voo.empty()) { a.semVoo = false; linha("Voo identificado"); }
        const int emg = av.squawk == 7500 || av.squawk == 7600 || av.squawk == 7700 ? av.squawk : -1;
        if (emg > 0 && a.squawk != emg)
            linha(emg == 7500 ? "ALERTA: transponder 7500 (interferência ilícita / sequestro)"
                  : emg == 7600 ? "ALERTA: transponder 7600 (falha de comunicação por rádio)"
                                : "ALERTA: transponder 7700 (emergência geral)");
        a.squawk = av.squawk;
        const int chao = av.noChao ? 1 : 0;
        if (a.anunciado && a.chao >= 0 && chao != a.chao) linha(chao ? "Pousou" : "Decolou");
        a.chao = chao;
    }
}

} // namespace masdr
