# IC7610SDR - histórico

## 1.1.2 (10/11/2026)
- Squelch mais sensível em FM, AM e nos outros modos: agora ele mede quanto o sinal passa do ruído de um
  canal vazio da largura do filtro. Antes o nível tinha um teto em 0 dB e, com o pré-amplificador ou o AGC do
  dongle, o próprio ruído já batia nesse teto: o squelch que fechava o ruído não abria com sinal nenhum (ex.:
  DMR em 165 MHz no VFO-B). Na escala nova o ruído fica em -60: SQL -55 = 5 dB acima do ruído, -40 = 20 dB.
  Vale também para o squelch do outro VFO com dois aparelhos.
- Quem usava o squelch ligado fica com -50 (10 dB acima do ruído) na primeira vez que abre esta versão.
- BUSY do outro VFO (com dois aparelhos) agora acende só com o squelch dele aberto (antes acendia com o
  S-meter acima do ruído, mesmo com o squelch fechado).

## 1.1.1 (10/11/2026)
- Dígitos das duas frequências da tela menores.
- No canto de cada VFO (à esquerda do MAIN e à direita do SUB), dois sliders em pé: AF (volume) e SQL (squelch)
  daquele receptor - arrastar, roda do mouse; duplo clique no SQL abre. Valem com um aparelho ou com dois e em
  qualquer modo; são os mesmos valores dos botões MAIN:AF-RF/SQL e SUB:AF-RF/SQL.

## 1.1.0 (10/11/2026)
- Dois aparelhos: o VFO-B pode ter o seu próprio dongle (RTL-SDR, RTL-TCP ou SDRplay), recebendo ao mesmo tempo
  que o VFO-A - por exemplo um RTL-TCP em HF e outro em VHF. CONFIGURAÇÃO > VFO-B (segundo aparelho): tipo,
  IP e porta (ou o dongle USB), Conectar. O áudio dos dois toca junto (padrão) ou só o do VFO selecionado.
- Volume, RF e squelch de cada VFO nos botões dele: o MAIN:AF-RF/SQL controla o VFO-A e o SUB:AF-RF/SQL controla o VFO-B (o anel é RF ou SQL, conforme o MENU > Áudio > Anel RF/SQL); o MUTE de cada um cala só aquele VFO.
- XFC segurado: só o outro VFO toca.
- Com DUAL, o escopo do VFO-B mostra a janela inteira do aparelho dele. S-meter, squelch e AUTO da cachoeira de cada VFO no aparelho dele.
- Desligando a caixa "VFO-B com aparelho próprio" tudo volta a ser como antes (um aparelho só).

## 1.0.0 (07/11/2026)
- Primeira versão. Mesmo motor do FTdxSDR 1.0.0 / SDR990S 1.2.8 (RTL-SDR, RTL-TCP, SDRplay, todos os
  decodificadores, Radar Aéreo, Satélites), com o painel novo no estilo do IC-7610, desenhado a partir da foto:
  - tela com as células ANT / P.AMP / ATT / IP+ / AGC / VOX / COMP, os dois S-meters iguais aos do rádio
    (S, Id, Po, SWR, COMP, ALC, Vd), BW / SFT / desenho do filtro / MET, relógio local e UTC, TX e DUAL-W;
  - VFO, etiqueta azul do modo (clique escolhe), FIL1/2/3, frequência grande e a linha da memória;
  - escopo DUAL: o do receptor ouvido e, ao lado, o do outro receptor (recortado do mesmo dongle);
  - teclas de toque <MENU1> (EDGE, HOLD, CENT/FIX, MAIN/SUB, DUAL, EXPD/SET) e <MENU2> (NOTCH, A-NOTCH,
    CONT, BC, DECODE, IF SCOPE);
  - MENU, M.SCOPE, MP-W, MP-R, AUTO TUNE, QUICK, EXIT, REC e PLAY embaixo da tela;
  - POWER, TRANSMIT, TIMER, TUNER, PHONES, ELEC-KEY, os AF-RF/SQL do MAIN e do SUB com MUTE e TX/RX, NB e NR;
  - MULTI, teclas de banda 1.8 ... 50, GENE, SPLIT, DUAL-W, MAIN/SUB, XFC, CHANGE e o dial grande;
  - TWIN-PBT (com CLR), FILTER, APF/TPF, KEY SPEED-PITCH, RIT, delta-TX, CLEAR, o botão RIT/delta-TX e SPEECH.
- Botão CACHOEIRA no lugar do cartão SD (embaixo, à esquerda, ao lado do MIC): liga o espectro e a cachoeira
  grandes em cima do rádio, ocupando a tela.
- Sem barra de título do Windows: tampa do rádio com o nome, a versão e minimizar / maximizar / fechar;
  fundo transparente, o rádio em pé sobre os dois pés prata.
- Cores da cachoeira "IC-7610 (azul)" (padrão) e o espectro em linha dourada (preenchido é opção no SCOPE SET), com o ruído embaixo.
- S-meter: o ponteiro mede o sinal acima do ruído (o ruído fica no S1-S2 e o ponteiro sobe com o sinal).
- DUAL: escopo do MAIN à esquerda e do SUB à direita; a moldura laranja marca o receptor em uso.
- Ícone próprio (dial prata com a tela do escopo).
- Pés cromados escovados; nome IC7610SDR dourado no lugar da marca; LED verde na tecla NR com o NR espectral.
