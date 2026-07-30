/* ============================================================
 * Lud OS — lv_conf.h
 * Config enxuta do LVGL 8.3.x para ESP32 (CYD, 240x320, RGB565)
 *
 * Isto NÃO é o lv_conf_template.h completo da lib (esse tem ~900
 * linhas). Aqui só definimos o que realmente muda pro nosso caso;
 * qualquer macro não definida aqui recebe o valor padrão do LVGL
 * via lv_conf_internal.h. Se o compilador reclamar de alguma opção
 * ausente, copie o bloco correspondente do lv_conf_template.h
 * (fica em .pio/libdeps/.../lvgl/lv_conf_template.h) pra cá.
 * ============================================================ */

#if 1 /* mantenha em 1 pra habilitar este arquivo */
#ifndef LV_CONF_H
#define LV_CONF_H

/* ---------------- cor ---------------- */
#define LV_COLOR_DEPTH 16          /* RGB565, igual ao ILI9341 */
#define LV_COLOR_16_SWAP 0         /* mude pra 1 se as cores saírem trocadas na tela real */

/* ---------------- memória ---------------- */
/* ESP32 clássico tem ~320KB de RAM interna. Reservamos uma fatia
 * pro heap do LVGL (widgets, estilos, animações). Ajuste se notar
 * crash por falta de memória (ver PLAN.md, seção de memória). */
#define LV_MEM_CUSTOM 0
/* LV_MEM_SIZE é aplicado via -DLV_MEM_SIZE=49152 em platformio.ini (força
 * a flag em todos os TUs, inclusive os da LVGL). Ver PLAN.md §5. */

/* ---------------- tick ---------------- */
/* NÃO usamos LV_TICK_CUSTOM aqui de propósito. O tick é avançado
 * manualmente via lv_tick_inc() dentro de loop() em main.cpp — isso
 * funciona mesmo se este arquivo lv_conf.h não estiver sendo
 * encontrado pelo compilador (ver PLAN.md, seção sobre build_flags
 * vs lv_conf.h). Deixe LV_TICK_CUSTOM em 0 (ou nem defina). */
#define LV_TICK_CUSTOM 0

/* ---------------- display ---------------- */
#define LV_DPI_DEF 130             /* densidade aproximada de um TFT 2.8" */

/* ---------------- fontes ---------------- */
#define LV_FONT_MONTSERRAT_12 1
#define LV_FONT_MONTSERRAT_14 1
#define LV_FONT_MONTSERRAT_16 1
#define LV_FONT_MONTSERRAT_20 1
#define LV_FONT_MONTSERRAT_28 1
#define LV_FONT_DEFAULT &lv_font_montserrat_14

/* ---------------- comportamento / debug ---------------- */
#define LV_USE_LOG 1
#if LV_USE_LOG
  #define LV_LOG_LEVEL LV_LOG_LEVEL_WARN
  #define LV_LOG_PRINTF 1
#endif

#define LV_USE_ASSERT_NULL 1
#define LV_USE_ASSERT_MALLOC 1

/* Monitor de performance/memória na tela — ligue durante o
 * desenvolvimento pra ver FPS e uso de heap em tempo real. */
#define LV_USE_PERF_MONITOR 0
#define LV_USE_MEM_MONITOR 0

/* ---------------- widgets que usamos ---------------- */
/* Deixamos os widgets padrão do template habilitados (btn, label,
 * img, bar, slider, switch, textarea, canvas, etc.) — não há custo
 * de RAM em habilitar, só de flash, e sobra bastante no ESP32. */

#endif /* LV_CONF_H */
#endif /* 1 */
