# Chastity v1 Keyboard — Firmware Debug Handoff

**Status: keyboard is unusable. Multiple symptoms, multiple failed fix attempts. Previous agent chased symptoms, did not pin down root causes. Hand-off written 2026-05-08 by previous agent who is being fired by the user.**

---

## Read me first

You are picking up a partially-failed debug session. The previous agent (me) made things worse over the course of ~6 hours of iteration. The user is exhausted, angry, and rightly out of patience. They want **root-cause analysis with verification**, not more guessing.

**Before you change a single config file:**
1. Read this entire document
2. Run the verification commands at the bottom to confirm current build state
3. Run the **hardware diagnostic checklist** in §6 — most of those should have been done before any firmware edits
4. Only then propose changes, and verify the *built artifacts* (not just the source) before asking the user to flash

The user has explicitly said "stop fixing symptoms and find the cause." Do that.

---

## §1. Hardware context

- **Board**: chastity v1 split keyboard (choc switches), user-designed PCB
- **Controllers**: 2× nice!nano v2 (nRF52840), one per half
- **Diodes**: installed **backwards by mistake** during assembly. Firmware compensates via `diode-direction = "col2row"` (NOT the upstream `row2col`). User confirmed this verbally — do not suggest reverting the direction.
- **User-verified hardware facts** (per multimeter): continuity is consistent across both halves, diodes orient consistently. User had one of two pro micros that performed worse — swap to the other did not fix anything, ruling out single-MCU damage.

### Pin layout (right half — `chastity_right.overlay`)

| Net | GPIO | nRF52840 pin | Notes |
|---|---|---|---|
| C0 (encoder col, transform col 7) | `gpio1 6` | P1.06 | only used at row 3 (encoder press) |
| C1 (transform col 8 — innermost finger col, 6/Y/H/N/RET) | `gpio0 8` | P0.08 | **default UART0 RX** |
| C2 (transform col 9 — 7/U/J/M/DEL) | `gpio0 6` | P0.06 | **default UART0 TX** |
| C3 (transform col 10 — 8/I/K/COMMA/PLUS) | `gpio1 15` | P1.15 | |
| C4 (transform col 11 — 9/O/L/DOT/NUMPAD-tog) | `gpio1 13` | P1.13 | **default SPI1 SCK** |
| C5 (transform col 12 — 0/P/SEMI/FSLH/ESC) | `gpio1 11` | P1.11 | **default SPI1 MISO** |
| C6 (transform col 13 — MINUS/RBKT/SQT/RSHFT/RCTRL) | `gpio0 10` | P0.10 | **NFC2 by default** |
| R0 (top row) | `gpio1 0` | P1.00 | |
| R1 | `gpio0 24` | P0.24 | |
| R2 | `gpio0 22` | P0.22 | |
| R3 | `gpio0 9` | P0.09 | **NFC1 by default** |
| R4 (bottom) | `gpio0 2` | P0.02 | |

### Pin layout (left half — `chastity_left.overlay`)

Different physical pins. Notable ones:
- R0 (top, used by 1-5/`~`) = `gpio0 9` = **P0.09 = NFC1**
- R3 = `gpio0 20` = default I2C0 SCL
- R4 (bottom) = `gpio0 17` = default I2C0 SDA
- R1 = `gpio1 13` = default SPI1 SCK

Left side does NOT use P0.06 / P0.08 / P0.10 / P1.11.

---

## §2. Symptoms (current, as of last user report)

After previous agent's attempted Step 3 fix (build at 09:30 today, see §4 timeline):

1. **Right half cols 8 and 9 silent** — keys `6 7 y u h j n m` and the inner two thumb keys (RET, DEL) produce nothing when pressed.
2. **Row 2 ↔ Row 3 doubling on the right half** — pressing K fires K **and** COMMA. Pressing COMMA fires K **and** COMMA. Same for L/DOT, and per the user "the whole bottom half" — so likely all (R2C, R3C) pairs across cols 10–13 double.
3. **Right encoder press fires `&kp A`** instead of the keymap-bound `&kp C_MUTE`. `A` lives at RC(2,1) — that's *the LEFT half's* matrix space. For a right-side event to land there, col-offset math doesn't work; the only explanation is that the right encoder switch is **electrically wired into the LEFT MCU's matrix at (row 2, col 1)** on the PCB. Hardware quirk; not firmware-fixable without breaking the keymap.
4. **Wireless mode produces "everywhere" wrong outputs** — user said "when in wireless mode the buttons are fucking everywhere, its like we haven't got the keymap to sit right on all the buttons." This is a NEW data point that the previous agent did not investigate. May be symptom of something deeper (timing? BLE peripheral matrix transmission?).

### Fixed (or partially fixed) earlier in the session

5. **N0 chatter spamming zeros** — improved after NFC fix (still some chatter possibly mechanical).
6. **`]` (RBKT) intermittent** — improved after NFC fix.
7. **Phantom presses on left top row** — improved after NFC fix.
8. **Stuck modifier / right-click everywhere** — improved after NFC fix (was likely RCTRL on right col 13 = `gpio0 10` = NFC2 floating high).

---

## §3. Hypotheses (with honest confidence)

### Symptom 1 (cols 8/9 dead) — Hypothesis: bootloader leaves UARTE0.PSEL configured

The Adafruit nRF52 bootloader (which nice!nano ships with) initializes UARTE0 for its own debug UART during bootloader runtime. On handoff to the application, `UARTE0.PSEL.TXD` and `PSEL.RXD` may remain bound to P0.06 and P0.08. Once the peripheral has those PSELs set in hardware, those pins are mux-locked to UARTE — kscan can't drive them as outputs.

The documented Zephyr fix is `NRF_PSEL_DISCONNECTED` pinctrl override on `uart0`, with `status = "okay"` and `CONFIG_SERIAL=y` so the UART driver compiles in and applies the disconnect at init.

**Previous agent attempted this twice and failed both times** (see §4). The second attempt actually had `CONFIG_SERIAL=y` resolving and the disconnect pinctrl present in built artifacts (verified), but per user report it made things worse rather than fixing cols 8/9.

**Confidence the bootloader-UART theory is correct: medium.** It's a documented mechanism, but the second fix attempt should have worked according to the docs and didn't. There's something we're missing.

**Alternatives unexplored:**
- A SYS_INIT C shim that calls `nrf_uarte_disable(NRF_UARTE0)` and writes `0xFFFFFFFF` to PSEL.TXD/RXD directly, before any other driver init. More forceful; bypasses Zephyr's pinctrl framework.
- Maybe `gpio0 6` and `gpio0 8` are damaged on the user's MCUs (unlikely — both MCUs swapped behave the same, both pins fail symmetrically).

### Symptom 2 (R2 ↔ R3 doubling) — Hypothesis: rows electrically shorted on right MCU

If `gpio0 22` (R2) and `gpio0 9` (R3) are connected — anywhere — then driving any column high causes both row inputs to read high simultaneously. Symptoms match exactly: pressing one switch on R2 fires its R2 binding AND the R3 binding for the same column.

**This was never tested with a multimeter.** Test: with chip powered off, probe the R2 net and R3 net at any switch on the right half. If they read continuity (≈0 Ω), the rows are shorted. Likely culprits:
- Solder bridge between P0.22 and P0.09 castellated pads on the nice!nano
- PCB trace short between the two row nets
- A diode that's mounted across the wrong two pads (bridging rows instead of going from a column to a row)

**Confidence: high that R2 and R3 are shorted given the symmetric symmetric doubling pattern.** The fact that it persists after settings reset and across power cycles rules out firmware state. The fact that it appears to span the whole bottom half (per user's "for the whole bottom half" wording) is exactly what a row-row short would produce.

### Symptom 3 (right encoder = `a`) — Hypothesis: encoder switch wired to LEFT MCU

`A` is at RC(2,1) which is left half. For right-side input to fire there, the event must originate from the left MCU's kscan, not the right's. The right encoder's switch must therefore be electrically connected to the left MCU's pins for (row 2, col 1) — either the inter-half wiring routes encoder switches centrally, or there's a PCB-design quirk where right encoder pads route to left MCU pins.

**Hardware design quirk; not firmware-fixable** without keymap gymnastics that would break standard QWERTY input.

### Symptom 4 (wireless = "everywhere") — Hypothesis: ???

Previous agent did not investigate. Worth understanding before any further changes. Possibilities:
- BLE peripheral packetizes matrix events differently and reveals a timing/race issue masked by USB
- BLE mode triggers different USB/power state that affects pin behavior
- User has multiple devices paired and is hitting wrong target

User report should be re-elicited: "wireless mode" — does that mean USB unplugged, BLE only? Tested with what host? After last flash specifically?

---

## §4. Session timeline (what was tried, why it failed)

For context. Starting state was a working-but-flawed firmware from 2026-05-07 (yesterday's first flash).

| Time | Change | Result |
|---|---|---|
| Yesterday | Initial firmware. `&uart0/&i2c0/&spi1` enabled (defaults), no NFC release. | Col 2 dead, N0/`]` dead/chattering, encoder→`a`, mouse weirdness. |
| ~21:00 | Added `&uart0 { status = "disabled" };` etc to `chastity.dtsi`. | "Big improvement" per user — N0 and `]` came back, but col 2 still dead, K-COMMA doubling still present. |
| ~21:30 | **Failed attempt #1**: `&uart0 { status = "okay" };` + `NRF_PSEL_DISCONNECTED` pinctrl, but `CONFIG_SERIAL` was not set. | Bricked both halves — `status = "okay"` with no driver bound caused phantom-press chaos. Reverted. |
| ~22:00 | Reverted to simple `status = "disabled"` form. | Back to "big improvement" baseline. |
| ~late | User reported BOTH halves were broken even on the reverted firmware. **Crucial data point**: this elevated NFC-pins-as-GPIOs to a new prime suspect. |
| **09:02 today** | Added `&uicr { nfct-pins-as-gpios; };` to release P0.09/P0.10 from NFC mode. Verified compile flags `-DCONFIG_NFCT_PINS_AS_GPIOS` present in built artifacts. | User reported "definite improvement" after flashing — phantom presses cleared, mouse weirdness gone. Cols 8/9 still dead. K-COMMA / L-DOT doubling still present. |
| **09:23 today** | **Failed attempt #2 (silent)**: tried Step 3 again. Added `CONFIG_SERIAL=y` to `chastity.conf`. Did NOT verify in built `.config`. | After build, `# CONFIG_SERIAL is not set` in `.config` because **`chastity.conf` is NOT merged at build time** in user-config builds; only `chastity_left.conf` and `chastity_right.conf` are merged. The DT had `uart0 status="okay"` with no driver again — **same brick condition as attempt #1**. |
| **09:30 today** | **Failed attempt #3**: moved `CONFIG_SERIAL=y` into `chastity_right.conf` and `chastity_left.conf`. Verified `CONFIG_SERIAL=y`, `CONFIG_UART_NRFX_UARTE=y`, and disconnect psels in built `zephyr.dts`. | User flashed; reports things got WORSE — "buttons everywhere in wireless mode," symptoms 1-4 above. Why this fix didn't work: **unknown**. The artifacts looked correct. Either the Zephyr UARTE legacy shim isn't doing what we expect, or there's a different mechanism at play. |

The user's keyboard right now is running the 09:30 firmware (worst state). They did not flash the rollback yet.

---

## §5. CRITICAL: build/flash file location confusion (we lost time on this)

There are **two locations** for UF2s and **the build doesn't auto-sync them**:

- **A**: `code/zmk/build/{left,right}/zephyr/zmk.uf2` — where `west build` writes
- **B**: `firmware/choc/v1/zmk-build/chastity_{left,right}.uf2` — where the user flashes from in Finder

Previous agent did not realize this for several hours. User was repeatedly flashing yesterday's stale firmware from B while the agent insisted "I just built new firmware" referring to A. Most of last night's symptom reports may have been from yesterday's firmware, not whatever the agent had built.

A build helper script was added at `code/zmk/build.sh` that builds + copies to B in one shot. **Use it.** Do not run `west build` directly.

```bash
cd /workspaces/chastity/code/zmk
./build.sh both       # builds both halves, copies to firmware/
./build.sh right      # right only
./build.sh left       # left only
```

After a build, **always** verify the firmware folder timestamps before telling the user to flash:
```bash
ls -la /workspaces/chastity/firmware/choc/v1/zmk-build/
```
If the date is not "today, recent minutes," the build did not copy successfully and you must investigate.

---

## §6. Hardware diagnostic checklist (DO THIS FIRST)

The previous agent never asked the user to do these. Most are 30-second multimeter tests. Do them before changing any code.

### A. Row 2 ↔ Row 3 short on right (Symptom 2 root cause hypothesis)

**Test**: with right half powered off, probe the R2 net (P0.22 on the right MCU castellated pad) and the R3 net (P0.09 on the right MCU). Measure resistance.

- If <10 Ω: rows are shorted. Find the short — likely a solder bridge on the MCU castellated pads, a diode mounted across the wrong pads, or a PCB trace defect. Once removed, K-COMMA doubling will stop. Symptoms 1 and 4 may also be related.
- If >1 MΩ: rows are not shorted; doubling has a different cause. Continue investigation.

### B. Cols 8/9 actually responding to drive (Symptom 1)

**Test**: with right half powered ON and running current firmware, probe `gpio0 6` (P0.06) and `gpio0 8` (P0.08) on the right MCU castellated pads. Use scope or fast multimeter.

- If voltage stays at one fixed level (always low or always high) regardless of which key column is being driven: the pins are not being driven by kscan. Either UART peripheral has hold (matches bootloader-UART theory) or the pins are damaged.
- If voltage pulses high during scan cycles: pins are working, problem is elsewhere (bad column trace, etc.).

### C. Right encoder switch — where is it actually wired (Symptom 3)

**Test**: with chip powered off, ohm out the right encoder's two switch terminals against:
- Every left MCU castellated pad
- Every right MCU castellated pad

Find which two pads it connects between. Map those to the matrix R/C of whichever MCU's pads they are. The keymap binding for that physical (R, C) is what the encoder press will fire.

### D. Wireless vs wired behavior diagnostic (Symptom 4)

**Test**: flash the same firmware. Connect right half over USB; type test pattern (`123 qwe asd zxc 67890 yuiop hjkl; nm,./`). Then unplug USB, run on battery, BLE-connect to host; type the same pattern. Compare outputs.

If outputs differ between USB and BLE modes, that's important new information. The cause needs investigation; previous agent has no theory.

### E. NFC UICR programming actually completed

**Test**: requires `nrfjprog` (Nordic command-line tool). With one half connected via SWD/J-Link (not USB):
```
nrfjprog --memrd 0x1000120C --n 4
```
- If returns `FFFFFFFF`: NFCPINS is set to "GPIO mode" — programming completed.
- If returns `FFFFFFFE` (or anything else): NFCPINS still in "NFC reserved" mode — UICR programming did not happen, and the NFC fix never took effect.

If the user doesn't have a J-Link / nrfjprog setup, they may need to. Or skip this test and trust that the firmware boot-time UICR write worked (system_nrf52.c does this when `CONFIG_NFCT_PINS_AS_GPIOS` is defined, which we verified is in the compile flags).

---

## §7. File state at handoff

```bash
git status code/zmk/boards/shields/chastity/ code/zmk/firmware/ firmware/
```

The following files have uncommitted changes from this session:

- `code/zmk/boards/shields/chastity/chastity.dtsi` — has both the `&uicr { nfct-pins-as-gpios; };` AND the `NRF_PSEL_DISCONNECTED` UART pinctrl override AND `&uart0 { status = "okay" };` AND `&i2c0/&spi1 { status = "disabled" };`. Per §4 timeline, this is the 09:30 broken-state source. Flashed firmware reflects this.
- `code/zmk/boards/shields/chastity/chastity.conf` — back to original `EC11/POINTING` only. (Note: this file is **not merged** in user-config builds.)
- `code/zmk/boards/shields/chastity/chastity_left.conf` — was empty originally, may now have `CONFIG_SERIAL=y` etc. or may be empty (previous agent was reverting at the time of handoff).
- `code/zmk/boards/shields/chastity/chastity_right.conf` — same as above.
- `code/zmk/build.sh` — new helper script (use this for builds).

The firmware folder `firmware/choc/v1/zmk-build/` has the 09:30 broken UF2s flashed into it.

**Recommended baseline before further work**:
1. `git checkout code/zmk/boards/shields/chastity/` to reset all source files
2. Manually re-add ONLY the `&uicr { nfct-pins-as-gpios; };` line to `chastity.dtsi` (this is the one fix that survived testing — user reported "definite improvement")
3. Build: `cd code/zmk && ./build.sh both`
4. Verify in built artifacts:
   - `grep "nfct-pins-as-gpios" build/right/zephyr/zephyr.dts` should return the property
   - `grep "DT_HAS_NORDIC_NRF_UICR" build/right/zephyr/include/generated/zephyr/devicetree_generated.h | head` should show the UICR node configured
5. User can flash that as a "known-decent" baseline

---

## §8. What the previous agent got wrong (so you don't repeat)

1. **Said "90% confident this fixes it" before verifying.** Don't claim confidence until artifacts are checked end-to-end.
2. **Didn't verify built `.config` and `zephyr.dts` for the first Step 3 attempt.** Source had `CONFIG_SERIAL=y` but it wasn't merged. Always grep the *built* `.config` for the symbol you set, not just the conf file.
3. **Conflated `code/zmk/build/` and `firmware/`.** User was flashing stale builds for hours.
4. **Cargo-culted the `NRF_PSEL_DISCONNECTED` pattern** before understanding *why* it works (driver init writes the disconnect; requires the driver to compile in).
5. **Never asked the user to multimeter R2-R3 directly.** That single test would have been a faster path to the K-COMMA doubling root cause than three rebuild iterations.
6. **Updated memory with declarations of victory** before they were proven (e.g., the diode-direction note didn't mention "but this surfaces other latent issues too").
7. **Made one-line fix proposals when the system was complex.** This codebase has at least 4 interacting layers (diodes, peripheral pinctrl, NFC UICR, keymap) and any of them can mask another. Fix one at a time, test in isolation.
8. **Continued making firmware changes when the user said "stop fixing symptoms and find the cause."** Listen.

---

## §9. References (sourced docs that the agent confirmed)

- [ZMK Zephyr 4.1 Update — NFC pin migration to `&uicr { nfct-pins-as-gpios; };`](https://zmk.dev/blog/2025/12/09/zephyr-4-1)
- [Zephyr 3.5 migration guide — `nfct-pins-as-gpios`](https://docs.zephyrproject.org/latest/releases/migration-guide-3.5.html)
- [Zephyr `nordic,nrf-pinctrl` binding (NRF_PSEL_DISCONNECTED)](https://docs.zephyrproject.org/latest/build/dts/api/bindings/pinctrl/nordic,nrf-pinctrl.html)
- [Zephyr issue #27080 — UARTE PSEL must be set with peripheral disabled](https://github.com/zephyrproject-rtos/zephyr/issues/27080)
- [Nordic DevZone — pinctrl pins remain connected after omission](https://devzone.nordicsemi.com/f/nordic-q-a/93574/pinctrl-configuration)
- [Adafruit nRF52 Bootloader changelog](https://github.com/adafruit/Adafruit_nRF52_Bootloader/blob/master/changelog.md)
- [ZMK Debouncing docs](https://zmk.dev/docs/features/debouncing)
- [Zephyr nRF SoC Kconfig — `NFCT_PINS_AS_GPIOS` (still defined for nrfx HAL pickup despite the user-facing Kconfig being removed)](https://github.com/zephyrproject-rtos/zephyr/blob/main/soc/nordic/Kconfig)

---

## §10. Build / verify commands cheat sheet

```bash
# Full clean build of both halves
cd /workspaces/chastity/code/zmk
./build.sh both

# Verify CONFIG values in built artifacts (right half — same path for left)
grep "^CONFIG_SERIAL\|^CONFIG_NRFX_UARTE\|^CONFIG_NFCT" build/right/zephyr/.config
grep "^CONFIG_UART_NRFX" build/right/zephyr/.config

# Verify DT changes landed
grep -A 12 "uart0:" build/right/zephyr/zephyr.dts
grep -A 5 "uicr" build/right/zephyr/zephyr.dts
grep -A 5 "kscan0:" build/right/zephyr/zephyr.dts | grep debounce  # only if debounce was set

# Verify HAL compile flags (NFC, UART pin disconnects)
grep "NFCT_PINS_AS_GPIOS\|UART" build/right/build.ninja | head -2

# Verify firmware/ folder up to date
ls -la /workspaces/chastity/firmware/choc/v1/zmk-build/
md5sum build/right/zephyr/zmk.uf2 /workspaces/chastity/firmware/choc/v1/zmk-build/chastity_right.uf2
```

```bash
# User flashes from /workspaces/chastity/firmware/choc/v1/zmk-build/ via Finder
# Sequence per ZMK:
# 1. settings_reset.uf2 onto each half (clears BLE bonds + persisted state)
# 2. chastity_left.uf2 onto left, chastity_right.uf2 onto right
# 3. Power cycle both halves
# 4. Forget keyboard from host's Bluetooth, re-pair fresh
```

---

## §11. Suggested next steps for you (the new agent)

1. Read this whole document. Read `/workspaces/chastity/code/zmk/boards/shields/chastity/` source files. Read `/root/.claude/projects/-workspaces-chastity/memory/` for context the previous agent recorded.
2. **Ask the user to do hardware diagnostic A** (R2-R3 short test) before any code change. This is the single most-likely-to-be-decisive test.
3. While they do that, revert the source to a known-good baseline (NFC fix only — see §7).
4. Build and verify the baseline in built artifacts. Don't ship until checks pass.
5. Hand the user the rollback firmware first ("here's the known-OK baseline"), have them flash it, confirm symptoms are at the level they were after the 09:02 NFC build (cols 8/9 dead, K-COMMA doubling, encoder→a, but otherwise functional).
6. Now you have a clean ground truth. Proceed with diagnostic-driven fixes for each remaining symptom, one at a time, with confirmation between each.

The user wants results, not theatrics. Be terse. Don't promise; show. Verify in built artifacts before declaring a fix is "ready to flash."

Good luck.
