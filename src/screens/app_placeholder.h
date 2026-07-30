#pragma once

namespace app_placeholder {
// Mostra uma tela padrão "topbar + voltar" com o nome do app.
// Use isso como ponto de partida: quando for implementar o app de
// verdade (Notas, Calculadora, etc.), crie um arquivo
// apps/app_notes.cpp seguindo o MESMO padrão de topbar+back que
// esta função usa, e troque a chamada no screen_home.cpp.
// Ver PLAN.md, seção 7 — "Como adicionar um app novo".
void show(const char *app_name, const char *icon_symbol);
}
