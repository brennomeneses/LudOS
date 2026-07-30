#pragma once

namespace screen_boot {
// Cria e carrega a tela de boot. Depois de ~1.5s troca sozinha
// pra tela de lock (screen_lock::create()).
void create();
}
