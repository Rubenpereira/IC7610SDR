// ---------------------------------------------------------------------------
//  AcarsLegivel - ver AcarsLegivel.h
//
//  Multidecoder SDR / IC7610SDR - PU1XTB - GPL-3.0
// ---------------------------------------------------------------------------
#include "AcarsLegivel.h"

#include "../app/PosicaoAcars.h"
#include "../decoders/AdsbEmpresas.h"

#include <algorithm>
#include <cctype>
#include <climits>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace masdr {

const char* const kMarcaOriginal = "  |  --- mensagem original ---  |  ";

namespace {

struct Aeroporto { const char* icao; const char* nome; };
const Aeroporto kAeroportos[] = {
    // Brasil
    {"SBSP", "São Paulo/Congonhas"}, {"SBGR", "São Paulo/Guarulhos"}, {"SBKP", "Campinas/Viracopos"},
    {"SBMT", "São Paulo/Campo de Marte"}, {"SBJD", "Jundiaí"}, {"SBSJ", "São José dos Campos"}, {"SBTA", "Taubaté"},
    {"SBGL", "Rio de Janeiro/Galeão"}, {"SBRJ", "Rio de Janeiro/Santos Dumont"}, {"SBJR", "Rio de Janeiro/Jacarepaguá"},
    {"SBAF", "Rio de Janeiro/Afonsos"}, {"SBSC", "Rio de Janeiro/Santa Cruz"}, {"SBME", "Macaé"}, {"SBCB", "Cabo Frio"},
    {"SBBR", "Brasília"}, {"SBCF", "Belo Horizonte/Confins"}, {"SBBH", "Belo Horizonte/Pampulha"}, {"SBVT", "Vitória"},
    {"SBSV", "Salvador"}, {"SBRF", "Recife"}, {"SBFZ", "Fortaleza"}, {"SBPA", "Porto Alegre"}, {"SBCT", "Curitiba"},
    {"SBFL", "Florianópolis"}, {"SBSG", "Natal/São Gonçalo do Amarante"}, {"SBMO", "Maceió"}, {"SBJP", "João Pessoa"},
    {"SBAR", "Aracaju"}, {"SBTE", "Teresina"}, {"SBSL", "São Luís"}, {"SBBE", "Belém"}, {"SBEG", "Manaus"},
    {"SBPV", "Porto Velho"}, {"SBRB", "Rio Branco"}, {"SBBV", "Boa Vista"}, {"SBMQ", "Macapá"}, {"SBPJ", "Palmas"},
    {"SBGO", "Goiânia"}, {"SBCY", "Cuiabá"}, {"SBCG", "Campo Grande"}, {"SBLO", "Londrina"}, {"SBMG", "Maringá"},
    {"SBFI", "Foz do Iguaçu"}, {"SBNF", "Navegantes"}, {"SBJV", "Joinville"}, {"SBCH", "Chapecó"}, {"SBUL", "Uberlândia"},
    {"SBUR", "Uberaba"}, {"SBRP", "Ribeirão Preto"}, {"SBSR", "São José do Rio Preto"}, {"SBPS", "Porto Seguro"},
    {"SBIL", "Ilhéus"}, {"SBJU", "Juazeiro do Norte"}, {"SBPL", "Petrolina"}, {"SBMK", "Montes Claros"},
    {"SBIZ", "Imperatriz"}, {"SBMA", "Marabá"}, {"SBSN", "Santarém"}, {"SBCZ", "Cruzeiro do Sul"}, {"SBTT", "Tabatinga"},
    {"SBKG", "Campina Grande"}, {"SBCA", "Cascavel"}, {"SBPF", "Passo Fundo"}, {"SBCX", "Caxias do Sul"},
    {"SBPK", "Pelotas"}, {"SBSM", "Santa Maria"}, {"SBUG", "Uruguaiana"}, {"SBDN", "Presidente Prudente"},
    {"SBBU", "Bauru"}, {"SBAU", "Araçatuba"}, {"SBML", "Marília"}, {"SBGV", "Governador Valadares"},
    {"SBIP", "Ipatinga"}, {"SBZM", "Juiz de Fora/Zona da Mata"}, {"SBQV", "Vitória da Conquista"},
    {"SBFN", "Fernando de Noronha"}, {"SBHT", "Altamira"}, {"SBSI", "Sinop"}, {"SBAT", "Alta Floresta"},
    {"SBRD", "Rondonópolis"}, {"SBVH", "Vilhena"}, {"SBJI", "Ji-Paraná"}, {"SBCR", "Corumbá"}, {"SBDO", "Dourados"},
    {"SBTG", "Três Lagoas"}, {"SBAN", "Anápolis"}, {"SBCN", "Caldas Novas"}, {"SBPB", "Parnaíba"},
    {"SBJE", "Jericoacoara"}, {"SBLE", "Lençóis"}, {"SBAX", "Araxá"}, {"SBGP", "Gavião Peixoto"}, {"SBTF", "Tefé"},
    {"SBUA", "São Gabriel da Cachoeira"}, {"SBCI", "Carolina"}, {"SBBW", "Barra do Garças"}, {"SBST", "Santos"},
    {"SBYS", "Pirassununga"}, {"SBNT", "Natal (Augusto Severo)"}, {"SBCO", "Canoas"}, {"SBSN", "Santarém"},
    // America do Sul, Central e Caribe
    {"SAEZ", "Buenos Aires/Ezeiza"}, {"SABE", "Buenos Aires/Aeroparque"}, {"SACO", "Córdoba"}, {"SAME", "Mendoza"},
    {"SAAR", "Rosário"}, {"SAZS", "Bariloche"}, {"SAWH", "Ushuaia"}, {"SCEL", "Santiago"}, {"SCFA", "Antofagasta"},
    {"SCIE", "Concepción"}, {"SPJC", "Lima"}, {"SPIM", "Lima"}, {"SKBO", "Bogotá"}, {"SKRG", "Medellín"},
    {"SKCL", "Cali"}, {"SKCG", "Cartagena"}, {"SEQM", "Quito"}, {"SEGU", "Guayaquil"}, {"SVMI", "Caracas"},
    {"SUMU", "Montevidéu"}, {"SGAS", "Assunção"}, {"SLVR", "Santa Cruz de la Sierra"}, {"SLLP", "La Paz"},
    {"SMJP", "Paramaribo"}, {"SYCJ", "Georgetown"}, {"SOCA", "Caiena"}, {"MPTO", "Cidade do Panamá"},
    {"MMMX", "Cidade do México"}, {"MMUN", "Cancún"}, {"MDPC", "Punta Cana"}, {"TNCC", "Curaçao"},
    {"MKJP", "Kingston"}, {"MUHA", "Havana"}, {"TJSJ", "San Juan"},
    // America do Norte
    {"KJFK", "Nova York/JFK"}, {"KEWR", "Newark"}, {"KLGA", "Nova York/LaGuardia"}, {"KMIA", "Miami"},
    {"KFLL", "Fort Lauderdale"}, {"KMCO", "Orlando"}, {"KATL", "Atlanta"}, {"KIAH", "Houston"}, {"KDFW", "Dallas"},
    {"KORD", "Chicago"}, {"KLAX", "Los Angeles"}, {"KSFO", "São Francisco"}, {"KIAD", "Washington/Dulles"},
    {"KBOS", "Boston"}, {"KDTW", "Detroit"}, {"KMSP", "Minneapolis"}, {"KSEA", "Seattle"}, {"KDEN", "Denver"},
    {"KLAS", "Las Vegas"}, {"KPHX", "Phoenix"}, {"KCLT", "Charlotte"}, {"KPHL", "Filadélfia"}, {"KTPA", "Tampa"},
    {"KMEM", "Memphis"}, {"KSDF", "Louisville"}, {"CYYZ", "Toronto"}, {"CYUL", "Montreal"}, {"CYVR", "Vancouver"},
    {"PANC", "Anchorage"}, {"PHNL", "Honolulu"}, {"CYQX", "Gander"},
    // Europa
    {"LPPT", "Lisboa"}, {"LPPR", "Porto"}, {"LPFR", "Faro"}, {"LPMA", "Funchal (Madeira)"}, {"LPLA", "Lajes (Açores)"},
    {"LPPD", "Ponta Delgada"}, {"LEMD", "Madri"}, {"LEBL", "Barcelona"}, {"LEMG", "Málaga"}, {"LFPG", "Paris/Charles de Gaulle"},
    {"LFPO", "Paris/Orly"}, {"EGLL", "Londres/Heathrow"}, {"EGKK", "Londres/Gatwick"}, {"EHAM", "Amsterdã"},
    {"EDDF", "Frankfurt"}, {"EDDM", "Munique"}, {"LIRF", "Roma/Fiumicino"}, {"LIMC", "Milão/Malpensa"},
    {"LSZH", "Zurique"}, {"LOWW", "Viena"}, {"EBBR", "Bruxelas"}, {"EKCH", "Copenhague"}, {"ENGM", "Oslo"},
    {"ESSA", "Estocolmo"}, {"EFHK", "Helsinque"}, {"LTFM", "Istambul"}, {"UUEE", "Moscou/Sheremetyevo"},
    {"EINN", "Shannon"}, {"BIKF", "Reykjavík"}, {"GCLP", "Gran Canaria"}, {"GCTS", "Tenerife Sul"},
    // Africa, Oriente Medio, Asia, Oceania
    {"GVAC", "Ilha do Sal (Cabo Verde)"}, {"GVNP", "Praia (Cabo Verde)"}, {"GOBD", "Dakar"}, {"DNMM", "Lagos"},
    {"FAOR", "Joanesburgo"}, {"FACT", "Cidade do Cabo"}, {"FNLU", "Luanda"}, {"HAAB", "Adis Abeba"}, {"HECA", "Cairo"},
    {"GMMN", "Casablanca"}, {"OMDB", "Dubai"}, {"OTHH", "Doha"}, {"OERK", "Riad"}, {"LLBG", "Tel Aviv"},
    {"VHHH", "Hong Kong"}, {"RJAA", "Tóquio/Narita"}, {"RJTT", "Tóquio/Haneda"}, {"ZBAA", "Pequim"},
    {"WSSS", "Singapura"}, {"YSSY", "Sydney"}, {"NZAA", "Auckland"},
};

std::string trim(const std::string& s)
{
    size_t a = 0, b = s.size();
    while (a < b && (s[a] == ' ' || s[a] == '\t' || s[a] == '\r')) ++a;
    while (b > a && (s[b - 1] == ' ' || s[b - 1] == '\t' || s[b - 1] == '\r')) --b;
    return s.substr(a, b - a);
}

std::string maiusc(std::string s) { for (auto& c : s) c = (char)std::toupper((unsigned char)c); return s; }

bool ehAlnum(char c) { return std::isalnum((unsigned char)c) != 0; }

// valor depois de um rotulo ("Label : H1", "Reg: .PR-MYI"): o rotulo precisa comecar palavra
std::string valorDe(const std::string& s, const char* rot, size_t maxLen = 12)
{
    const size_t n = std::strlen(rot);
    for (size_t p = s.find(rot); p != std::string::npos; p = s.find(rot, p + 1)) {
        if (p > 0 && ehAlnum(s[p - 1])) continue;
        size_t i = p + n;
        // o rotulo tem de acabar ali ("No" nao casa com "Not")
        if (i < s.size() && ehAlnum(s[i]) && rot[n - 1] != ':') continue;
        while (i < s.size() && (s[i] == ' ' || s[i] == ':')) ++i;
        size_t f = i;
        while (f < s.size() && s[f] != ' ' && s[f] != '\n' && s[f] != '\r' && f - i < maxLen) ++f;
        std::string v = s.substr(i, f - i);
        if (!v.empty()) return v;
    }
    return "";
}

std::vector<std::string> linhasDe(const std::string& bloco)
{
    std::string s = bloco;
    for (size_t p = s.find("  |  "); p != std::string::npos; p = s.find("  |  ", p)) s.replace(p, 5, "\n");
    std::vector<std::string> v;
    size_t a = 0;
    while (a <= s.size()) {
        size_t b = s.find('\n', a);
        if (b == std::string::npos) b = s.size();
        v.push_back(s.substr(a, b - a));
        a = b + 1;
    }
    return v;
}

bool comeca(const std::string& s, const char* p) { return s.compare(0, std::strlen(p), p) == 0; }

bool contem(const std::string& s, const char* p) { return s.find(p) != std::string::npos; }

// palavra inteira (cercada de nao-alfanumericos)
size_t achaPalavra(const std::string& s, const std::string& w, size_t de = 0)
{
    for (size_t p = s.find(w, de); p != std::string::npos; p = s.find(w, p + 1)) {
        const bool a = p == 0 || !ehAlnum(s[p - 1]);
        const bool b = p + w.size() >= s.size() || !ehAlnum(s[p + w.size()]);
        if (a && b) return p;
    }
    return std::string::npos;
}

// o primeiro "token" depois da palavra w (pulando espacos, '/', ':', '=', '-')
std::string tokenDepois(const std::string& s, const char* w, size_t maxLen = 10)
{
    const size_t p = achaPalavra(s, w);
    if (p == std::string::npos) return "";
    size_t i = p + std::strlen(w);
    while (i < s.size() && (s[i] == ' ' || s[i] == '/' || s[i] == ':' || s[i] == '=' || s[i] == '-' || s[i] == '.')) ++i;
    size_t f = i;
    while (f < s.size() && ehAlnum(s[f]) && f - i < maxLen) ++f;
    return s.substr(i, f - i);
}

std::string grausMin(double v, bool lat)
{
    const char h = lat ? (v < 0 ? 'S' : 'N') : (v < 0 ? 'W' : 'E');
    v = std::fabs(v);
    int g = (int)v;
    double m = (v - g) * 60.0;
    if (m >= 59.95) { m = 0; ++g; }
    char b[40];
    std::snprintf(b, sizeof b, lat ? "%02d°%04.1f'%c" : "%03d°%04.1f'%c", g, m, h);
    std::string r = b;
    for (auto& c : r) if (c == '.') c = ',';
    return r;
}

std::string hhmm(const std::string& t)   // "1035" / "103452" -> "10:35" / "10:34:52"
{
    if (t.size() == 4) return t.substr(0, 2) + ":" + t.substr(2, 2);
    if (t.size() == 6) return t.substr(0, 2) + ":" + t.substr(2, 2) + ":" + t.substr(4, 2);
    return t;
}

bool soDigitos(const std::string& s) { return !s.empty() && std::all_of(s.begin(), s.end(), [](char c) { return std::isdigit((unsigned char)c) != 0; }); }

bool horaValida(const std::string& t)
{
    if (!soDigitos(t) || (t.size() != 4 && t.size() != 6)) return false;
    const int h = std::atoi(t.substr(0, 2).c_str()), m = std::atoi(t.substr(2, 2).c_str());
    return h < 24 && m < 60 && (t.size() == 4 || std::atoi(t.substr(4, 2).c_str()) < 60);
}

std::string comAeroporto(const std::string& icao)
{
    const char* n = nomeAeroporto(icao);
    return n ? icao + " (" + n + ")" : icao;
}

// codigos ICAO de aeroporto citados no texto (so os conhecidos: senao qualquer palavra de 4 letras entraria)
std::vector<std::string> aeroportosCitados(const std::string& t)
{
    std::vector<std::string> v;
    for (size_t i = 0; i + 4 <= t.size(); ++i) {
        if (i > 0 && ehAlnum(t[i - 1])) continue;
        if (i + 4 < t.size() && std::isalpha((unsigned char)t[i + 4])) continue;
        bool ok = true;
        for (int k = 0; k < 4; ++k) if (!std::isupper((unsigned char)t[i + k])) ok = false;
        if (!ok) continue;
        const std::string c = t.substr(i, 4);
        if (nomeAeroporto(c) && std::find(v.begin(), v.end(), c) == v.end()) v.push_back(c);
    }
    return v;
}

struct Rotulo { const char* label; const char* titulo; const char* significado; };
// Titulos pelo "label" (ARINC 618 / 620 / 623). Os de companhia (numeros) variam de empresa para
// empresa: o conteudo refina depois.
const Rotulo kRotulos[] = {
    {"_d", "Confirmação técnica (sem conteúdo)", "Confirmação de recebimento — não carrega informação"},
    {"_j", "Confirmação técnica (sem conteúdo)", "Confirmação de recebimento — não carrega informação"},
    {"Q0", "Teste do enlace", "O avião confere se a ligação de dados está funcionando — sem conteúdo"},
    {"SQ", "Anúncio da estação de terra (squitter)", "A estação de terra avisa que está no ar"},
    {"SA", "Aviso de mídia (enlace de dados)", "O avião avisa qual enlace está usando (VHF, VDL2, satélite, HF)"},
    {"S1", "Mensagem de rede", nullptr},
    {"H1", "Dados do FMS / sistemas de bordo", nullptr},
    {"H2", "Dados de meteorologia (ventos)", nullptr},
    {"5U", "Pedido de meteorologia", nullptr},
    {"5Z", "Mensagem da companhia", nullptr},
    {"54", "Pedido de contato por voz", "A tripulação pede para a companhia chamá-la pelo rádio"},
    {"57", "Relatório de posição (companhia)", nullptr},
    {"80", "Mensagem da tripulação/companhia", nullptr},
    {"81", "Mensagem da tripulação/companhia", nullptr},
    {"83", "Mensagem da tripulação/companhia", nullptr},
    {"8E", "Relatório de ETA", nullptr},
    {"10", "Mensagem da companhia (rota/ETA)", nullptr},
    {"15", "Relatório de posição", nullptr},
    {"16", "Relatório de posição", nullptr},
    {"20", "Relatório de posição/combustível", nullptr},
    {"21", "Relatório de posição/combustível", nullptr},
    {"22", "Relatório de posição/meteorologia", nullptr},
    {"44", "Relatório de posição/combustível", nullptr},
    {"4N", "Relatório de posição", nullptr},
    {"RA", "Mensagem para o cockpit (impressora)", nullptr},
    {"RB", "Mensagem do cockpit para a companhia", nullptr},
    {"C1", "Mensagem para o cockpit", nullptr},
    {"A1", "Autorização oceânica", nullptr},
    {"A3", "Autorização de saída (PDC)", nullptr},
    {"A6", "Pedido de relatório ADS", nullptr},
    {"A9", "Informação de ATIS", nullptr},
    {"AA", "Mensagem de controle de tráfego (CPDLC)", nullptr},
    {"B1", "Pedido de autorização oceânica", nullptr},
    {"B2", "Confirmação da autorização oceânica", nullptr},
    {"B3", "Pedido de autorização de saída", nullptr},
    {"B4", "Confirmação da autorização pela tripulação", nullptr},
    {"B6", "Relatório ADS-C (posição automática)", nullptr},
    {"B9", "Pedido de ATIS", nullptr},
    {"BA", "Mensagem de controle de tráfego (CPDLC)", nullptr},
    {"MA", "Mensagem comprimida (MIAM)", nullptr},
    {"QA", "Saída do portão (OUT) / combustível", nullptr},
    {"QB", "Decolagem (OFF)", nullptr},
    {"QC", "Pouso (ON)", nullptr},
    {"QD", "Chegada ao portão (IN)", nullptr},
    {"QE", "Saída do portão (OUT) / combustível / destino", nullptr},
    {"QF", "Decolagem (OFF) / destino", nullptr},
    {"QG", "Volta ao portão", nullptr},
    {"QH", "Saída do portão (OUT)", nullptr},
    {"QK", "Pouso", nullptr},
    {"QL", "Chegada", nullptr},
    {"QM", "Chegada (informação)", nullptr},
    {"QN", "Desvio de rota/destino", nullptr},
    {"QP", "Saída do portão (OUT)", nullptr},
    {"QQ", "Decolagem (OFF)", nullptr},
    {"QR", "Pouso (ON)", nullptr},
    {"QS", "Chegada ao portão (IN)", nullptr},
    {"QT", "Saída do portão / volta (OUT/RETURN)", nullptr},
};

const Rotulo* rotulo(const std::string& l)
{
    for (const auto& r : kRotulos) if (l == r.label) return &r;
    return nullptr;
}

void add(MsgLegivel& m, const std::string& r, const std::string& v)
{
    if (v.empty()) return;
    for (auto& c : m.campos) if (c.first == r) return;
    m.campos.push_back({r, v});
}

// texto livre: so os caracteres que se leem, os espacos juntos
std::string limpo(const std::string& t, size_t maxc = 400)
{
    std::string o;
    for (char c : t) {
        const unsigned char u = (unsigned char)c;
        if (c == '\n' || c == '\r' || c == '\t') c = ' ';
        else if (u < 32 || u == 127) continue;
        if (c == ' ' && (o.empty() || o.back() == ' ')) continue;
        o += c;
    }
    while (!o.empty() && o.back() == ' ') o.pop_back();
    if (o.size() > maxc) o = o.substr(0, maxc) + "...";
    return o;
}

// "FL182" / "ALT 30522" / "ALT:350" -> texto com pes
std::string altitudeDe(const std::string& t)
{
    for (size_t p = t.find("FL"); p != std::string::npos; p = t.find("FL", p + 1)) {
        if (p > 0 && ehAlnum(t[p - 1])) continue;
        size_t i = p + 2;
        while (i < t.size() && t[i] == ' ') ++i;
        size_t f = i;
        while (f < t.size() && std::isdigit((unsigned char)t[f])) ++f;
        if (f - i == 3 || f - i == 2) {
            const int fl = std::atoi(t.substr(i, f - i).c_str());
            if (fl >= 10 && fl <= 510) return "FL" + t.substr(i, f - i) + " (~" + milhar(fl * 100LL) + " ft)";
        }
    }
    const std::string a = tokenDepois(t, "ALT", 6);
    if (soDigitos(a)) {
        long v = std::atol(a.c_str());
        if (a.size() <= 3) v *= 100;   // em centenas de pes
        if (v > 0 && v < 60000) return milhar(v) + " ft";
    }
    return "";
}

// ---- conteudo das mensagens de FMS (label H1 e as que comecam com "#M1B", "#DF"...) ----
void lerFms(MsgLegivel& m, const std::string& t)
{
    // "- #M1BPOS..." ou "#M1BPOS...": o tipo vem logo depois do prefixo do equipamento
    size_t p = t.find('#');
    std::string corpo = p == std::string::npos ? t : t.substr(p + 1);
    std::string equip;
    if (corpo.size() >= 3) {
        if (comeca(corpo, "M1B") || comeca(corpo, "M2B") || comeca(corpo, "M3B")) { equip = "FMS"; corpo = corpo.substr(3); }
        else if (comeca(corpo, "DF") || comeca(corpo, "D1") || comeca(corpo, "D2")) equip = "ACMS";
        else if (comeca(corpo, "CF") || comeca(corpo, "C1")) equip = "CMC";
        else if (comeca(corpo, "EI") || comeca(corpo, "E1")) equip = "EICAS";
        else if (comeca(corpo, "T1") || comeca(corpo, "T2")) equip = "IMPRESSORA";
    }
    if (equip == "ACMS") {
        m.titulo = "Dados de monitoramento (ACMS/DFDAU)";
        add(m, "Conteúdo", "Parâmetros de motores e sistemas em formato próprio da companhia — não legível diretamente");
        const auto ap = aeroportosCitados(t);
        if (ap.size() >= 2) { const size_t a = t.find(ap[0]), b = t.find(ap[1]); if (b > a && b - a <= 6) add(m, "Rota", comAeroporto(ap[0]) + " → " + comAeroporto(ap[1])); }
        return;
    }
    if (equip == "CMC") { m.titulo = "Relatório de manutenção / falhas (CMC)"; return; }
    if (equip == "EICAS") { m.titulo = "Alertas do sistema do avião (EICAS)"; return; }
    if (equip != "FMS") return;
    struct T { const char* p; const char* tit; const char* sig; };
    static const T kT[] = {
        {"POS", "Relatório de posição (FMS)", nullptr},
        {"PRG", "Relatório de progresso do voo (FMS)", nullptr},
        {"REQPWI", "Pedido de dados do FMS", "FMS pedindo ao solo os ventos previstos da rota"},
        {"REQPER", "Pedido de dados do FMS", "FMS pedindo ao solo os dados de desempenho"},
        {"REQ", "Pedido de dados do FMS", "FMS solicitando reenvio/atualização de dados ao solo"},
        {"PWI", "Ventos previstos para a rota (FMS)", nullptr},
        {"PER", "Dados de desempenho (FMS)", nullptr},
        {"FPN", "Plano de voo (FMS)", nullptr},
        {"RTE", "Rota (FMS)", nullptr},
        {"INI", "Inicialização do FMS", nullptr},
        {"ETA", "Relatório de ETA (FMS)", nullptr},
        {"WXR", "Pedido de meteorologia (FMS)", nullptr},
        {"RES", "Resposta do FMS", nullptr},
    };
    m.titulo = "Dados do FMS";
    for (const auto& k : kT) if (comeca(corpo, k.p)) {
        m.titulo = k.tit;
        if (k.sig) add(m, "Significado", k.sig);
        corpo = corpo.substr(std::strlen(k.p));
        if (std::strcmp(k.p, "POS") == 0 || std::strcmp(k.p, "PRG") == 0) {
            double la = 91, lo = 181;
            if (posicaoAcars(corpo, la, lo)) add(m, "Posição", grausMin(la, true) + " " + grausMin(lo, false));
        }
        if (std::strcmp(k.p, "PRG") == 0) {
            // "/DTSBRJ,20L,104,133825,116": destino, pista, (combustivel), ETA
            const size_t d = corpo.find("/DT");
            if (d != std::string::npos) {
                std::vector<std::string> f;
                size_t a = d + 3;
                while (a <= corpo.size()) { size_t b = corpo.find_first_of(",/", a); if (b == std::string::npos) b = corpo.size(); f.push_back(corpo.substr(a, b - a)); if (b >= corpo.size() || corpo[b] == '/') break; a = b + 1; }
                if (!f.empty() && nomeAeroporto(f[0])) add(m, "Destino", comAeroporto(f[0]));
                if (f.size() > 1 && !f[1].empty() && std::isdigit((unsigned char)f[1][0]) && f[1].size() <= 3) add(m, "Pista prevista", f[1]);
                for (size_t i = 2; i < f.size(); ++i) if (f[i].size() == 6 && horaValida(f[i])) { add(m, "Chegada estimada (ETA)", hhmm(f[i]) + " UTC"); break; }
            }
        }
        if (std::strcmp(k.p, "POS") == 0) {
            // POS<lat><lon>,<ultimo fixo>,<hora>,<FL>,<proximo>,<hora>,...
            std::vector<std::string> f;
            size_t a = 0;
            while (a <= corpo.size()) { size_t b = corpo.find(',', a); if (b == std::string::npos) b = corpo.size(); f.push_back(trim(corpo.substr(a, b - a))); a = b + 1; }
            std::string fixo, hora, nivel, prox;
            for (size_t i = 1; i < f.size(); ++i) {
                if (fixo.empty() && !f[i].empty() && std::isalpha((unsigned char)f[i][0]) && f[i].size() <= 13) { fixo = f[i]; continue; }
                if (hora.empty() && horaValida(f[i])) { hora = f[i]; continue; }
                if (!hora.empty() && nivel.empty() && soDigitos(f[i]) && f[i].size() <= 3) { nivel = f[i]; continue; }
                if (!nivel.empty() && prox.empty() && !f[i].empty() && std::isalpha((unsigned char)f[i][0]) && f[i].size() <= 6) { prox = f[i]; break; }
            }
            if (!fixo.empty()) add(m, "Último fixo", fixo + (hora.empty() ? "" : " às " + hhmm(hora) + " UTC"));
            if (!nivel.empty()) { const int fl = std::atoi(nivel.c_str()); if (fl > 0 && fl < 520) add(m, "Altitude", "FL" + nivel + " (~" + milhar(fl * 100LL) + " ft)"); }
            if (!prox.empty()) add(m, "Próximo fixo", prox);
        }
        if (std::strcmp(k.p, "PER") == 0) {
            std::string nums;
            int n = 0;
            for (size_t i = 0; i < corpo.size() && n < 6;) {
                if (std::isdigit((unsigned char)corpo[i])) {
                    size_t f = i; while (f < corpo.size() && std::isdigit((unsigned char)corpo[f])) ++f;
                    nums += (nums.empty() ? "" : ", ") + corpo.substr(i, f - i); ++n; i = f;
                } else ++i;
            }
            if (!nums.empty()) add(m, "Parâmetros", nums + " (peso / índices de custo)");
        }
        if (std::strcmp(k.p, "FPN") == 0 || std::strcmp(k.p, "RTE") == 0) {
            // "/FP:...:DA:SBSP:AA:SBVT:..." (origem / destino)
            const std::string da = tokenDepois(corpo, "DA", 4), aa = tokenDepois(corpo, "AA", 4);
            if (nomeAeroporto(da) || nomeAeroporto(aa)) add(m, "Rota", (da.empty() ? "?" : comAeroporto(da)) + " → " + (aa.empty() ? "?" : comAeroporto(aa)));
        }
        break;
    }
}

// autorizacao de saida (PDC) e mensagens do controle no texto livre
void lerAutorizacao(MsgLegivel& m, const std::string& t)
{
    std::string nr = !tokenDepois(t, "CLRNC NR", 6).empty() ? tokenDepois(t, "CLRNC NR", 6)
                         : !tokenDepois(t, "CLX NR", 6).empty() ? tokenDepois(t, "CLX NR", 6) : tokenDepois(t, "CLR NR", 6);
    if (nr.empty()) { const std::string p = tokenDepois(t, "PDC", 6); if (soDigitos(p)) nr = p; }
    if (!nr.empty()) add(m, "Autorização nº", nr);
    std::string dest = tokenDepois(t, "CLEARED TO", 4);
    if (dest.empty()) dest = tokenDepois(t, "CLRD TO", 4);
    if (dest.empty()) dest = tokenDepois(t, "CLR TO", 4);
    if (nomeAeroporto(dest)) add(m, "Autorizado para", comAeroporto(dest));
    std::string rwy = tokenDepois(t, "RWY", 4);
    if (rwy.empty()) rwy = tokenDepois(t, "RUNWAY", 4);
    if (rwy.empty()) rwy = tokenDepois(t, "OFF", 3);
    if (!rwy.empty() && std::isdigit((unsigned char)rwy[0])) add(m, "Pista", rwy);
    std::string sid = tokenDepois(t, "SID", 9);
    if (sid.empty() || !std::isalpha((unsigned char)sid[0])) sid = tokenDepois(t, "DEP", 9);
    if (sid.empty() || !std::isalpha((unsigned char)sid[0])) sid = tokenDepois(t, "VIA", 9);
    if (!sid.empty() && std::isalpha((unsigned char)sid[0]) && sid.size() >= 4 && std::isdigit((unsigned char)sid[sid.size() - 2])) {
        // "BAIAN3B UREMI": a SID e a transicao (o ponto logo depois)
        const size_t p = t.find(sid);
        std::string tr;
        if (p != std::string::npos) { size_t i = p + sid.size(); while (i < t.size() && (t[i] == ' ' || t[i] == '/')) ++i; size_t f = i; while (f < t.size() && std::isalpha((unsigned char)t[f])) ++f; if (f - i == 5) tr = t.substr(i, 5); }
        add(m, "Saída (SID)", tr.empty() ? sid : sid + " / " + tr);
    }
    std::string sq = tokenDepois(t, "SQUAWK", 4);
    if (sq.empty()) sq = tokenDepois(t, "SQK", 4);
    if (sq.empty()) sq = tokenDepois(t, "SSR", 4);
    if (sq.size() == 4 && soDigitos(sq)) add(m, "Código transponder", sq);
    const std::string alt = altitudeDe(t);
    if (!alt.empty()) add(m, "Nível autorizado", alt);
    const std::string fr = tokenDepois(t, "FREQ", 7);
    if (!fr.empty() && std::isdigit((unsigned char)fr[0])) add(m, "Frequência", fr);
    const std::string atis = tokenDepois(t, "ATIS", 1);
    if (atis.size() == 1 && std::isalpha((unsigned char)atis[0])) add(m, "ATIS", atis);
}

// o que da para tirar de qualquer texto: rota, ETA, combustivel, altitude, posicao
void lerComum(MsgLegivel& m, const std::string& t, bool comAltitude)
{
    // rota "SBKP/SBVT", "SBKP-SBVT", "SBKP SBVT"
    const auto ap = aeroportosCitados(t);
    std::string origem = tokenDepois(t, "ORIG", 4), destino = tokenDepois(t, "DEST", 4);
    if (!nomeAeroporto(origem)) origem.clear();
    if (!nomeAeroporto(destino)) destino.clear();
    if (origem.empty() && destino.empty() && ap.size() >= 2) {
        const size_t a = t.find(ap[0]), b = t.find(ap[1]);
        if (a != std::string::npos && b != std::string::npos && b > a && b - a <= 6) { origem = ap[0]; destino = ap[1]; }
    }
    if (!origem.empty() && !destino.empty()) add(m, "Rota", comAeroporto(origem) + " → " + comAeroporto(destino));
    else if (!destino.empty()) add(m, "Destino", comAeroporto(destino));
    std::string eta = tokenDepois(t, "ETA", 6);
    if (horaValida(eta)) add(m, "Chegada estimada (ETA)", hhmm(eta.substr(0, 4)) + " UTC");
    std::string fob = tokenDepois(t, "FOB", 7);
    if (fob.empty()) fob = tokenDepois(t, "FUEL", 7);
    if (soDigitos(fob) && fob.size() >= 2) add(m, "Combustível a bordo", milhar(std::atoll(fob.c_str())));
    if (comAltitude) { const std::string a = altitudeDe(t); if (!a.empty()) add(m, "Altitude", a); }
    // "/WP MOLSU" ponto de rota, "/MCH 0783" numero de Mach
    const std::string wp = tokenDepois(t, "WP", 6);
    if (wp.size() >= 3 && std::isalpha((unsigned char)wp[0])) add(m, "Ponto de rota", wp);
    const std::string mch = tokenDepois(t, "MCH", 4);
    if (soDigitos(mch) && mch.size() >= 3) { std::string v = "0," + mch.substr(mch.size() == 4 ? 1 : 0); add(m, "Velocidade", "Mach " + v); }
    double la = 91, lo = 181; int alt = INT_MIN;
    if (posicaoAcars(t, la, lo, &alt)) {
        add(m, "Posição", grausMin(la, true) + " " + grausMin(lo, false));
        if (alt != INT_MIN) add(m, "Altitude", milhar(alt) + " ft");
    }
    // aeroportos citados que ainda nao apareceram
    std::string cit;
    for (const auto& a : ap) {
        bool ja = false;
        for (const auto& c : m.campos) if (c.second.find(a) != std::string::npos) ja = true;
        if (!ja) cit += (cit.empty() ? "" : "; ") + comAeroporto(a);
    }
    if (!cit.empty()) add(m, "Aeroportos citados", cit);
}

} // namespace

const char* nomeAeroporto(const std::string& icao)
{
    if (icao.size() != 4) return nullptr;
    for (const auto& a : kAeroportos) if (icao == a.icao) return a.nome;
    return nullptr;
}

std::string milhar(long long v)
{
    const bool neg = v < 0;
    std::string s = std::to_string(neg ? -v : v), o;
    int k = 0;
    for (size_t i = s.size(); i > 0; --i) { o.insert(o.begin(), s[i - 1]); if (++k % 3 == 0 && i > 1) o.insert(o.begin(), '.'); }
    return neg ? "-" + o : o;
}

MsgLegivel lerMensagemAviao(const std::string& bloco)
{
    MsgLegivel m;
    const auto ls = linhasDe(bloco);
    std::string tudo;
    for (const auto& l : ls) { tudo += trim(l); tudo += '\n'; }

    // ---- cabecalho ACARS (acarsdec: "Label : H1", "Aircraft reg: .PR-MYI Flight id: JJ3640", "No: M10A";
    //      dumpvdl2 / dumphfdl: "Reg: .PR-MYI Flight: JJ3640", "Label: H1 ... Msg no: M10A") ----
    m.label = valorDe(tudo, "Label", 3);
    if (m.label.size() == 2 && m.label[0] == '_' && (unsigned char)m.label[1] >= 127) m.label = "_d";
    if (m.label.size() > 2) m.label = m.label.substr(0, 2);
    std::string reg = valorDe(tudo, "Aircraft reg:", 10);
    if (reg.empty()) reg = valorDe(tudo, "Reg:", 10);
    while (!reg.empty() && reg[0] == '.') reg.erase(0, 1);
    m.prefixo = reg;
    std::string voo = valorDe(tudo, "Flight id:", 9);
    if (voo.empty()) voo = valorDe(tudo, "Flight ID:", 9);
    if (voo.empty()) voo = valorDe(tudo, "Flight:", 9);
    while (!voo.empty() && voo[0] == '.') voo.erase(0, 1);
    m.voo = voo;
    m.numero = valorDe(tudo, "Msg no:", 6);
    if (m.numero.empty()) m.numero = valorDe(tudo, "Msg num:", 6);
    if (m.numero.empty()) m.numero = valorDe(tudo, "No:", 6);
    if (!m.voo.empty()) {
        const std::string icao = vooParaIcao(m.voo, m.prefixo);
        const std::string cod = icao.size() > 3 ? icao.substr(0, 3) : m.voo.size() > 3 && std::isalpha((unsigned char)m.voo[2]) ? m.voo.substr(0, 3) : "";
        if (const char* n = nomeCompanhia(cod)) m.companhia = n;
    }

    // ---- texto da mensagem: as linhas depois de "Message:" (ate um trecho decodificado pelo libacars) ----
    std::string texto;
    bool dentro = false, naSecao = false;
    std::string decodificado;   // o que o libacars ja explicou (ADS-C, CPDLC, Media Advisory...)
    for (const auto& l0 : ls) {
        const std::string l = trim(l0);
        if (l == "Message:" || l == "Message :") { dentro = true; continue; }
        if (comeca(l, "Message :") || comeca(l, "Message:")) { dentro = true; texto += trim(l.substr(l.find(':') + 1)) + "\n"; continue; }
        if (naSecao) { if (!l.empty()) decodificado += l + "\n"; continue; }
        if (!dentro) continue;
        if (comeca(l, "ARINC") || comeca(l, "ADS-C") || comeca(l, "CPDLC") || comeca(l, "FANS") || comeca(l, "MIAM") ||
            comeca(l, "Media Advisory") || comeca(l, "Reassembly") || comeca(l, "Unparseable") || comeca(l, "[#")) {
            dentro = false; naSecao = true; decodificado += l + "\n"; continue;
        }
        texto += l + "\n";
    }
    // acarsdec: nao tem a linha "Message :" - o texto vem logo depois do cabecalho
    bool temMessage = false;
    for (const auto& l0 : ls) { const std::string l = trim(l0); if (comeca(l, "Message:") || comeca(l, "Message :")) temMessage = true; }
    if (!temMessage) {
        for (const auto& l0 : ls) {
            const std::string l = trim(l0);
            if (l.empty() || comeca(l, "[#") || comeca(l, "[20") || comeca(l, "Mode") || comeca(l, "Aircraft reg") || comeca(l, "Reg:") ||
                comeca(l, "No:") || comeca(l, "Msg no") || comeca(l, "Sublabel") || comeca(l, "MFI") || comeca(l, "ACARS") ||
                comeca(l, "AVLC") || contem(l, " -> ") || l == "ETB" || l == "ETX" || comeca(l, "Uplink") || comeca(l, "Downlink"))
                continue;
            texto += l + "\n";
        }
    }
    {   // "ETB" / "ETX" no fim: marcas de fim de bloco
        auto ls2 = linhasDe(texto);
        texto.clear();
        for (const auto& l : ls2) { const std::string x = trim(l); if (x == "ETB" || x == "ETX" || x.empty()) continue; texto += x + "\n"; }
    }
    while (!texto.empty() && (texto.back() == '\n' || texto.back() == ' ')) texto.pop_back();
    const std::string T = maiusc(texto);

    const bool temAcars = !m.label.empty() && (contem(tudo, "Mode") || contem(tudo, "Label"));
    if (temAcars) {
        m.reconhecida = true;
        const Rotulo* r = rotulo(m.label);
        if (!r && m.label.size() == 2 && m.label[0] == 'C') r = rotulo("C1");
        m.titulo = r ? r->titulo : "Mensagem da companhia (tipo " + m.label + ")";
        if (r && r->significado) add(m, "Significado", r->significado);
        // conteudo
        const bool fms = T.find("#M1B") != std::string::npos || T.find("#M2B") != std::string::npos || T.find("#DF") != std::string::npos ||
                         T.find("#CF") != std::string::npos || T.find("#EI") != std::string::npos || T.find("#M3B") != std::string::npos;
        if (fms) lerFms(m, T);
        const bool pdc = contem(T, "CLRNC") || contem(T, "CLEARED TO") || contem(T, "CLRD TO") || achaPalavra(T, "PDC") != std::string::npos ||
                         contem(T, "DEP CLX") || contem(T, "DEPARTURE CLEARANCE");
        if (pdc) {
            // B2 / B3 / B4: a tripulacao pedindo ou confirmando; o resto e a autorizacao em si
            if (m.label != "B2" && m.label != "B3" && m.label != "B4") m.titulo = "Autorização de saída (PDC)";
            lerAutorizacao(m, T);
        }
        if (!fms && !pdc) {
            if (m.label == "B3") {
                const std::string gate = tokenDepois(T, "GATE", 5), stand = tokenDepois(T, "STAND", 5);
                if (!gate.empty()) add(m, "Portão", gate); else if (!stand.empty()) add(m, "Posição de estacionamento", stand);
            }
            const bool atis = m.label == "B9" || (achaPalavra(T, "ATIS") != std::string::npos && T.size() < 80);
            const bool wx = m.label == "5U" || contem(T, "WXRQ") || achaPalavra(T, "METAR") != std::string::npos || achaPalavra(T, "TAF") != std::string::npos ||
                            achaPalavra(T, "WX") != std::string::npos || achaPalavra(T, "WXR") != std::string::npos;
            const bool eta = achaPalavra(T, "ETA") != std::string::npos;
            if (atis) {
                m.titulo = "Pedido de ATIS";
                const auto ap = aeroportosCitados(T);
                if (!ap.empty()) add(m, "Pedido", "ATIS de " + comAeroporto(ap[0]));
            } else if (wx && (m.label == "5U" || T.size() < 200)) {
                m.titulo = "Pedido de meteorologia";
                const auto ap = aeroportosCitados(T);
                std::string l;
                for (const auto& a : ap) l += (l.empty() ? "" : "; ") + comAeroporto(a);
                if (!l.empty()) add(m, "Meteorologia pedida para", l);
            } else if (contem(T, "POSRPT") || contem(T, "POS RPT") || contem(T, "POSREP")) {
                m.titulo = "Relatório de posição (companhia)";
            } else if (eta && (m.label == "80" || m.label == "8E" || m.label == "10" || m.label == "5Z" || m.label == "81")) {
                m.titulo = "Relatório de ETA";
            }
        }
        if (!fms) lerComum(m, T, !pdc);
        // OOOI: horas "OUT 1025", "OFF 1030", "ON 1132", "IN 1140"
        for (const char* k : {"OUT", "OFF", "ON", "IN"}) {
            const std::string h = tokenDepois(T, k, 4);
            if (horaValida(h) && (m.label.size() == 2 && m.label[0] == 'Q')) {
                const char* n = !std::strcmp(k, "OUT") ? "Saiu do portão" : !std::strcmp(k, "OFF") ? "Decolou" : !std::strcmp(k, "ON") ? "Pousou" : "Chegou ao portão";
                add(m, n, hhmm(h) + " UTC");
            }
        }
        // texto livre da tripulacao / companhia: vale mostrar inteiro
        const bool livre = !fms && (m.label == "80" || m.label == "81" || m.label == "83" || m.label == "5Z" || m.label == "RA" ||
                                    m.label == "RB" || (m.label.size() == 2 && m.label[0] == 'C') || m.label == "A9" || pdc ||
                                    m.campos.empty());
        if (livre && !texto.empty() && m.label != "_d" && m.label != "Q0" && m.label != "SQ") add(m, "Texto", limpo(texto));
        if (!decodificado.empty()) {
            std::string d = decodificado;
            const bool midia = comeca(d, "Media Advisory");
            if (midia) { m.titulo = "Aviso de mídia (enlace de dados)"; d = d.substr(d.find('\n') + 1); }
            static const std::pair<const char*, const char*> kTr[] = {
                {"Default SATCOM", "satélite (padrão)"}, {" established", " estabelecido"}, {" lost", " perdido"},
                {" at ", " às "}, {"Available links:", "enlaces disponíveis:"}, {"Link ", ""},
            };
            for (const auto& tr : kTr)
                for (size_t p = d.find(tr.first); p != std::string::npos; p = d.find(tr.first, p + std::strlen(tr.second)))
                    d.replace(p, std::strlen(tr.first), tr.second);
            std::string o;
            for (const auto& l : linhasDe(d)) { const std::string x = trim(l); if (!x.empty()) o += (o.empty() ? "" : " · ") + x; }
            o = limpo(o, 300);
            if (!o.empty()) add(m, midia ? "Enlace" : "Decodificado", o);
        }
        if (!m.numero.empty()) add(m, "Nº da mensagem", m.numero);
    }

    // ---- HFDL sem ACARS dentro: squitter, logon, dados de desempenho ----
    if (!temAcars) {
        auto depoisDe = [&](const char* k) { const auto p = tudo.find(k); if (p == std::string::npos) return std::string(); const auto f = tudo.find('\n', p); return trim(tudo.substr(p + std::strlen(k), f == std::string::npos ? std::string::npos : f - p - std::strlen(k))); };
        const std::string src = depoisDe("Src GS:"), dst = depoisDe("Dst GS:");
        if (contem(tudo, "Squitter") || contem(tudo, "SPDU")) {
            m.reconhecida = true;
            m.titulo = "Anúncio da estação de terra HFDL (squitter)";
            add(m, "Significado", "A estação de terra avisa as frequências que estão no ar");
            add(m, "Estação", src);
        } else if (contem(tudo, "Logon request") || contem(tudo, "Logon resume")) {
            m.reconhecida = true; m.titulo = "Avião pedindo conexão (logon HFDL)"; add(m, "Estação", dst);
        } else if (contem(tudo, "Logon confirm") || contem(tudo, "Logon resume confirm")) {
            m.reconhecida = true; m.titulo = "Conexão do avião confirmada (logon HFDL)"; add(m, "Estação", src);
        } else if (contem(tudo, "Logoff")) {
            m.reconhecida = true; m.titulo = "Avião desconectando (logoff HFDL)"; add(m, "Estação", !dst.empty() ? dst : src);
        } else if (contem(tudo, "Performance data")) {
            m.reconhecida = true; m.titulo = "Dados do enlace HFDL (posição do avião)";
            add(m, "Estação", dst);
        } else if (contem(tudo, "Frequency data")) {
            m.reconhecida = true; m.titulo = "Estações HFDL que o avião está ouvindo";
            add(m, "Estação", dst);
        } else if (contem(tudo, "XID")) {
            m.reconhecida = true; m.titulo = "Troca de estação de terra (VDL2)";
        } else if (contem(tudo, "AVLC type: S") || contem(tudo, "Receive Ready")) {
            m.reconhecida = true; m.titulo = "Controle do enlace (sem conteúdo)";
            add(m, "Significado", "Confirmação de recebimento — não carrega informação");
        }
        if (m.reconhecida) {
            if (m.voo.empty()) { m.voo = valorDe(tudo, "Flight ID:", 9); }
            double la = 91, lo = 181; int alt = INT_MIN;
            if (posicaoAcars(tudo, la, lo, &alt)) {
                add(m, "Posição", grausMin(la, true) + " " + grausMin(lo, false));
                if (alt != INT_MIN) add(m, "Altitude", milhar(alt) + " ft");
            }
        }
    }
    // VDL2: "E4988D (Aircraft, Airborne) -> F042B8 (Ground station): Command"
    for (const auto& l0 : ls) {
        const std::string l = trim(l0);
        const size_t seta = l.find(" -> ");
        if (seta == std::string::npos || l.size() < 8 || l[6] != ' ') continue;
        const std::string a = l.substr(0, 6), b = l.substr(seta + 4, 6);
        const bool doAviao = l.substr(0, seta).find("Air") != std::string::npos;
        const bool paraAviao = l.substr(seta).find("Air") != std::string::npos;
        if (doAviao) { m.hex = a; add(m, "Sentido", "avião → estação de terra"); }
        else if (paraAviao) { m.hex = b; add(m, "Sentido", "estação de terra → avião"); }
        break;
    }
    if (m.reconhecida && !m.hex.empty() && m.voo.empty() && m.prefixo.empty()) add(m, "Avião (código ICAO)", m.hex);
    if (m.reconhecida && (!m.voo.empty() || !m.prefixo.empty())) {
        std::string v = m.voo;
        if (!m.companhia.empty()) v += (v.empty() ? "" : " · ") + m.companhia;
        if (!m.prefixo.empty()) v += (v.empty() ? "" : " · ") + m.prefixo;
        m.campos.insert(m.campos.begin(), {"Voo", v});
    }
    return m;
}

std::string textoLegivel(const MsgLegivel& m)
{
    std::string s = "» " + m.titulo;
    for (const auto& c : m.campos) s += "  |  " + c.first + ": " + c.second;
    return s;
}

} // namespace masdr
