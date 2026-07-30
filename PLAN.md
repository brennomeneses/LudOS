# Lud OS — Implementation Plan

This document goes along with the code skeleton already generated
(`platformio.ini`, `include/lv_conf.h`, `src/*`). It explains **why**
each thing was done that way and **what's left to do**, in order.
Whenever a decision depends on your specific hardware, it's flagged
with ⚠️.

---

## 0. Before anything: confirm your board

The name "CYD" (Cheap Yellow Display) covers several different board
revisions. Before flashing anything:

- [ ] Check whether the chip has "ESP32-2432S028" printed on it, and
      if possible the suffix (R/C vary the touch wiring).
- [ ] Confirm the touch is **resistive** (4 wires + XPT2046, which you
      already confirmed) — good, this project's defaults already
      assume that.
- [ ] Confirm whether your board has **PSRAM**. This directly changes
      the game implementation (section 8). To check: flash the
      skeleton, open the serial monitor, and temporarily add this to
      `setup()`:
      ```cpp
      Serial.printf("PSRAM: %d bytes\n", ESP.getPsramSize());
      ```
      If it prints `0`, your board doesn't have PSRAM enabled/present.
- [ ] If you can, find the repo/schematic link from the seller of your
      specific CYD — the pins in `platformio.ini` are the most common
      ones published for the ESP32-2432S028, but they vary between
      batches.

This step avoids 90% of "blank screen" or "touch doesn't match the
finger" problems.

---

## 1. Environment

- **PlatformIO** in VS Code (you already have this).
- Libraries managed automatically via `lib_deps` in `platformio.ini`:
  `TFT_eSPI`, `lvgl` (pinned to 8.3.x — **don't** move to LVGL 9, the
  `lv_disp_drv_t`/`lv_indev_drv_t` API changed a lot and most CYD
  examples/tutorials out there are still for v8), `XPT2046_Touchscreen`.
- **Important:** `TFT_eSPI` configuration was done entirely via
  `build_flags` in `platformio.ini` (instead of editing `User_Setup.h`
  inside the library itself). This is intentional: editing the lib
  directly gets lost on every `pio pkg update` and doesn't go into your
  version control. If you ever need a `TFT_eSPI` option that doesn't
  exist as a `-D`, that's when we'd create a dedicated `User_Setup.h`.

Steps to open it:
1. Open the `LudOS/` folder in VS Code with the PlatformIO extension.
2. Let it download the libraries (takes a bit the first time).
3. Build (✔) before flashing, just to catch compile errors without
   depending on the hardware.
4. Upload + Monitor.

---

## 2. Folder structure

```
LudOS/
├── platformio.ini
├── include/
│   └── lv_conf.h            # LVGL config
├── src/
│   ├── main.cpp              # setup()/loop(), just orchestrates
│   ├── theme.h / theme.cpp   # palette + reusable LVGL styles
│   ├── display_driver.h/.cpp # TFT_eSPI <-> LVGL, backlight PWM
│   ├── touch_driver.h/.cpp   # XPT2046 <-> LVGL
│   └── screens/
│       ├── screen_boot.*     # animated logo, ~1.7s
│       ├── screen_lock.*     # tap to unlock
│       ├── screen_home.*     # app grid + dock
│       └── app_placeholder.* # generic "under construction" screen
└── data/                      # will become the LittleFS content
```

Each screen is a `create()` function that builds a new `lv_obj_t*` and
calls `lv_scr_load()`. This is exactly the same model as the HTML
mockup (one "screen" at a time, swapped via `goTo()`), just that here
each one is a C++ function instead of a `<div>`.

---

## 3. Display (TFT_eSPI)

Points of attention in `src/display_driver.cpp`:

- **Rotation:** `ROTATION = 0` is the starting point. If the image
  comes out flipped, mirrored, or in landscape when powered on, try
  values `1`, `2`, `3` in that constant (nowhere else) until it matches
  portrait 240×320 without mirroring. ⚠️ This is specific to your unit.
- **Partial buffer:** we use two `240×40` pixel buffers (not the whole
  screen) for drawing — this is the pattern LVGL itself recommends for
  an ESP32 without PSRAM. Less RAM used, at the cost of more flush
  calls (imperceptible for our use case).
- **Backlight via PWM (LEDC)**, not on/off `digitalWrite` — this
  already sets up the brightness slider for the Settings app (call
  `display::set_backlight(value_0_to_100)`).

Quick sanity check before moving on: if the boot screen shows the "L"
centered and with no swapped colors (pink shouldn't show up as
blue/green), the color config (`LV_COLOR_16_SWAP`) is correct. If the
colors come out swapped, change `LV_COLOR_16_SWAP` to `1` in
`lv_conf.h`.

---

## 4. Touch (XPT2046) — pinout and calibration

⚠️ **Important correction:** unlike what I initially assumed, the
touch does **not** share the display's SPI bus. It uses its own bus,
with completely different GPIOs:

| Signal | GPIO |
|--------|------|
| T_CLK  | 25   |
| T_CS   | 33   |
| T_DIN (MOSI) | 32 |
| T_DO (MISO)  | 39 |
| T_IRQ  | 36   |

Because of this, in `touch_driver.cpp` we use a dedicated `SPIClass`
(`HSPI`) for the touch, separate from the global `SPI` (`VSPI`) that
`TFT_eSPI` uses for the display. If you ever switch boards and the
touch stops responding again, the first thing to check is whether this
pinout matches the actual wiring of the new unit.

The resistive touch also **doesn't come calibrated** out of the box
for your finger/stylus. `src/touch_driver.cpp` already has example
calibration constants, but you need to find your own:

1. Flash the firmware normally. `touch_driver.cpp` already ships with
   `TOUCH_DEBUG` on, so the serial monitor shows a line like
   `[touch] raw x=1234 y=2345 z=567` every time you touch the screen.
2. Touch the **4 corners** of the physical screen and note the raw x/y
   values that show up.
3. Find the min/max X and min/max Y observed → those are
   `RAW_X_MIN/MAX` and `RAW_Y_MIN/MAX`.
4. Update the constants at the top of `touch_driver.cpp`.
5. Once you're done calibrating, change `#define TOUCH_DEBUG 1` to `0`
   (avoids serial spam and a bit of overhead).
6. If X and Y come out swapped (touching the top moves the logical
   cursor sideways), turn on `SWAP_XY`. If one axis comes out inverted
   (left becomes right), turn on `INVERT_X`/`INVERT_Y`.

This is tedious but only needs to be done **once per physical unit**.

---

## 5. Memory — rough budget (ESP32 without PSRAM)

### ⚠️ Important note: `build_flags` instead of `lv_conf.h`

In this environment, LVGL isn't managing to find our
`include/lv_conf.h` while compiling the library itself (this shows up
as the `Possible failure to include lv_conf.h` warning repeated across
practically every LVGL file in the build log). It's not worth chasing
down the exact cause (it's a quirk of how PlatformIO resolves include
paths between the project and the libs in this setup) — the solution
adopted was to **force the settings that actually matter via `-D`
directly in `platformio.ini`**, because a `-D` flag applies to the
entire build (project + libraries) regardless of any file lookup.

Two practical consequences of this:

1. **Fonts and memory** (`LV_FONT_MONTSERRAT_12/16/20/28`,
   `LV_MEM_SIZE`) are forced via `build_flags`, no longer relying only
   on `lv_conf.h`.
2. **LVGL's tick no longer uses `LV_TICK_CUSTOM`.** Instead,
   `main.cpp` calls `lv_tick_inc()` manually on every pass of `loop()`,
   based on `millis()`. This is LVGL's "default" mode and works
   regardless of whether `lv_conf.h` is found or not — which is why
   it's the more robust approach for our case.

**If you ever need to add a new LVGL option** (e.g. enabling one more
font, or changing `LV_DPI_DEF`), the more reliable path is to add
another `-DXXXX=value` line to `build_flags`, rather than trusting
that editing `lv_conf.h` alone will take effect — always confirm with
a clean build (`pio run -t clean && pio run`) that the value actually
took (for example, an "undefined reference" error to a font symbol is
a sign the flag didn't take).

### RAM budget

| Consumer                                   | ~RAM        |
|---------------------------------------------|-------------|
| Wi-Fi/BT stack from the Arduino framework    | 40–70 KB    |
| Display drawing buffers (2× 240×40×2B)       | ~38 KB   |
| LVGL heap (`LV_MEM_SIZE`)                    | 48 KB       |
| General Arduino core overhead                | ~20–30 KB   |
| **Left over for widgets, apps, and the game** | **~100–130 KB** |

This is an estimate, not a promise — it varies with what you enable.
If at some point the board resets on its own (usually stack overflow
or a full heap), the first thing to look at is not just reducing
`LV_MEM_SIZE` (that helps you *detect* the problem, not solve it — the
right move is finding what's consuming too much, e.g. an oversized
canvas, see section 8) or turning on `LV_USE_MEM_MONITOR` in
`lv_conf.h` to see usage in real time on screen.

---

## 6. Current boot flow

```
setup()
 └─ lv_init()
 └─ display::init()   → tft.init(), registers disp driver
 └─ touch::init()     → ts.begin(), registers indev
 └─ theme::init()     → creates the shared lv_style_t's
 └─ screen_boot::create()
       └─ (1.7s timer) → screen_lock::create()
             └─ (tap) → screen_home::create()
                   └─ (tap on an icon) → app_placeholder::show(name, symbol)
                         └─ (back button) → screen_home::create()
```

Notice that **no screen keeps a pointer to the previous screen** —
each `create()` builds everything from scratch and calls
`lv_scr_load()`. LVGL destroys the old screen on its own (default
behavior — `lv_scr_load` without the `_anim` variant already frees the
previous one). This greatly simplifies navigation, at the cost of not
having "state" between visits to the same app yet — once we implement
Notes/Calculator for real, this starts to matter (see section 7).

---

## 7. How to add a real app (moving on from the placeholder)

Always use this pattern, copying the structure of
`src/screens/app_placeholder.cpp`:

1. Create `src/apps/app_notes.h` and `.cpp` (create the `apps/` folder).
2. The `create()` function builds: a `topbar` (with a back button
   identical to the placeholder's — reuse `theme::style_topbar`) + the
   app's real content below it.
3. In `screen_home.cpp`, inside `app_icon_cb`, replace the generic call
   with an index check:
   ```cpp
   if (idx == 2) { app_notes::create(); return; } // Notes
   app_placeholder::show(app.name, app.symbol);    // fallback
   ```
4. If the app needs to persist data (Notes, Settings), initialize
   LittleFS once in `main.cpp`:
   ```cpp
   #include <LittleFS.h>
   LittleFS.begin(true); // true = format if it doesn't mount
   ```
   and read/write simple files (`/notes.txt`, `/settings.json`) from
   inside the app.

Suggested order for implementing the apps (from simplest to most
work):
1. **Settings** — already has most of what it needs
   (`display::set_backlight` already exists; just need the slider
   calling that function and a Wi-Fi toggle turning `WiFi.mode()`
   on/off).
2. **Calculator** — just string logic + `lv_btnmatrix` or a grid of
   `lv_btn`, no external I/O.
3. **Notes** — `lv_textarea` + LittleFS.
4. **Wi-Fi** — `WiFi.scanNetworks()` (asynchronous, don't block the
   UI — fire the scan, show "searching...", and an `lv_timer` checks
   `WiFi.scanComplete()` periodically).
5. **Messages / Phone / Gallery** — these stay mocks even in v1 (no
   cellular radio, no connected camera), so they're last and just use
   sample data, same as the HTML.

---

## 8. Space Invaders (canvas) — plan before coding

You already decided on canvas instead of individual LVGL objects — the
right call for ~28 enemies + moving shots. But there's a memory
gotcha that needs to be solved **before** writing the game:

### The calculation that matters

An `lv_canvas` in `LV_IMG_CF_TRUE_COLOR` (RGB565, 2 bytes/pixel) at the
size of the mockup's game area (240×248) costs:

```
240 × 248 × 2 bytes = 119,040 bytes (~116 KB)
```

Looking at the table in section 5, this **doesn't fit** comfortably if
your board doesn't have PSRAM (only ~100–130 KB is left for
everything). So before implementing the game:

- [ ] Run the test from section 0 (`ESP.getPsramSize()`).

**If you have PSRAM:** allocate the canvas buffer in it
(`heap_caps_malloc(size, MALLOC_CAP_SPIRAM)`) and go with
`LV_IMG_CF_TRUE_COLOR` at full resolution — it ends up identical to
the mockup.

**If you don't have PSRAM**, two options (I'd recommend the first):
1. **4-bit indexed canvas** (`LV_IMG_CF_INDEXED_4BIT`, 16-color
   palette — more than enough for our game, which only uses ~6
   colors). Cost: `240 × 248 × 0.5 byte = 29,760 bytes (~29 KB)` + a
   small palette. Fits comfortably.
2. Shrink the play area (e.g. a logical 120×124, redrawn at 2× when
   pushing to the display) — more work to write, avoid this unless
   option 1 doesn't solve it.

### Game loop (non-blocking)

No `delay()` inside the game. Pattern:

```cpp
// inside game/space_invaders.cpp
static lv_timer_t *game_timer;

static void game_tick(lv_timer_t *t) {
    update_positions();
    check_collisions();
    redraw_canvas(); // only the dirty area, see below
}

void game::start() {
    // ... create canvas, initial state ...
    game_timer = lv_timer_create(game_tick, 16, NULL); // ~60 FPS target
}

void game::stop() {
    lv_timer_del(game_timer);
    screen_home::create();
}
```

### Redrawing: don't clear the whole screen every frame

Clearing and redrawing all 240×248 pixels every tick is wasteful —
most of the background doesn't change. A strategy simple enough for
our case (without needing sophisticated dirty-rects):
- Keep each entity's previous-frame position (player, shots, enemies).
- On each tick, **only erase the old rectangle** of each entity that
  moved (`lv_canvas_draw_rect` with the background color) before
  drawing it at the new position.
- This alone avoids 90% of unnecessary redrawing without much
  complexity.

### Controls

Reuse exactly the mockup's scheme: three fixed `lv_btn`s
(◀ / FIRE / ▶) in a bottom bar, using `LV_EVENT_PRESSED` /
`LV_EVENT_RELEASED` to hold continuous movement (equivalent to the
HTML's `pointerdown`/`pointerup`) and `LV_EVENT_CLICKED` on the fire
button.

### Sprites (later, as agreed)

Each entity should already be born as a `struct` with `x, y, w, h,
alive` (same as the mockup) and a `draw_entity()` function that today
draws a colored rectangle. Once the images are ready, that function
turns into a call to `lv_canvas_draw_img()`/`lv_img_set_src()` — the
game logic (movement, collision, scoring) doesn't change at all.

---

## 9. Milestone checklist

- [ ] **M1** — Skeleton compiles and flashes without error.
- [ ] **M2** — Boot → Lock → Home shows up in the right orientation,
      no mirroring, correct colors.
- [ ] **M3** — Touch calibrated: can reliably tap each icon in the
      grid and dock, and the back button works.
- [ ] **M4** — Real Settings app (brightness working via PWM).
- [ ] **M5** — Functional calculator.
- [ ] **M6** — Notes with LittleFS persistence.
- [ ] **M7** — Wi-Fi with real scanning.
- [ ] **M8** — PSRAM confirmed present or not → game canvas sized
      correctly (section 8).
- [ ] **M9** — Space Invaders playable (collision, scoring, lives,
      restart).
- [ ] **M10** — Sprites/images replacing the colored rectangles.

---

## 10. Quick debug reference

- General logging: `Serial.begin(115200)` is already active, use
  `Serial.printf` freely during development, remove it afterward.
- Real-time FPS/heap: turn on `LV_USE_PERF_MONITOR` and/or
  `LV_USE_MEM_MONITOR` in `include/lv_conf.h` (change the `0` to `1`)
  — an overlay shows up on screen. **Turn it off before considering
  the build "final"**, it consumes resources.
- White/black screen on boot: usually a wrong `TFT_RST` pin or
  `SPI_FREQUENCY` too high for your cable/board — try lowering it to
  `27000000` in `platformio.ini` as a test.
- Touch never responds: check whether `XPT2046_IRQ` is correct —
  without the right IRQ, `ts.tirqTouched()` is never `true`. (Note:
  we've since stopped relying on `tirqTouched()` in `read_cb()` for
  exactly this reason — see section 4.)