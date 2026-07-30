#pragma once
#include <lvgl.h>

/* ============================================================
 * Lud OS — paleta e estilos compartilhados
 * Mesmos valores hex do mockup HTML (ver comentário no topo
 * do lud-os-mockup.html). Mantidos idênticos de propósito, pra
 * qualquer ajuste de cor ser feito num lugar só.
 * ============================================================ */

namespace theme {

constexpr uint32_t PINK_50  = 0xFFF6F9;
constexpr uint32_t PINK_100 = 0xFFE9F1;
constexpr uint32_t PINK_200 = 0xFFD3E3;
constexpr uint32_t PINK_300 = 0xFFB6D1;
constexpr uint32_t PINK_400 = 0xFF8FB3;
constexpr uint32_t PINK_600 = 0xE8628A;
constexpr uint32_t INK      = 0x5C3A45;
constexpr uint32_t INK_SOFT = 0x8C6270;
constexpr uint32_t WHITE    = 0xFFFFFF;

/* Nota de performance: o mockup HTML usa box-shadow suave nos
 * cartões. Em LVGL/MCU, sombra desfocada (lv_style_set_shadow_*)
 * é cara de renderizar em redraws frequentes. Por isso aqui usamos
 * "flat design": cartão branco/rosa + borda fina, sem sombra. */

/* Chama uma vez no boot, antes de criar qualquer tela. */
void init();

/* Estilos prontos, reaproveitados em várias telas: */
extern lv_style_t style_screen_bg;   /* fundo padrão das telas (PINK_100) */
extern lv_style_t style_card;        /* cartão branco arredondado */
extern lv_style_t style_app_icon;    /* ícone de app (quadrado arredondado) */
extern lv_style_t style_topbar;      /* barra superior das telas de app */
extern lv_style_t style_text_title;  /* título (bold, INK) */
extern lv_style_t style_text_soft;   /* texto secundário (INK_SOFT) */

} // namespace theme
