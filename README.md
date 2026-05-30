# MTG Token Counter

A minimal physical device to track token counts during Magic: The Gathering games. Smaller than a Magic card, table-flat, button-operated.

---

## Concept

Tokens have two states: **untapped** and **tapped**. The device tracks both simultaneously on a single 4-digit display — left two digits for untapped, right two for tapped.

```
[ 0 3 | 0 5 ]
  ^       ^
  untapped tapped
```

---

## Physical Design

- **Form factor:** Flat, sits on the table face-up. Smaller than a Magic card (target: under 63mm × 88mm)
- **Display:** Single 4-digit 7-segment display (e.g. TM1637-based, ~50mm × 14mm)
- **Buttons:** 4 tactile switches, one per side of the device
- **Power:** CR2032 coin cell or small LiPo

### Button Layout

```
         [ TOP ]
            +1 untapped
            (hold) +2 untapped

[ LEFT ]  [display]  [ RIGHT ]
  tap 1                untap 1
  (hold)               (hold)
  tap all              untap all

         [ BOTTOM ]
            -1 untapped
            (hold) reset
```

---

## Button Actions

| Button | Short Press | Long Press (hold ~500ms) |
|--------|-------------|--------------------------|
| **Top** | +1 untapped | +2 untapped |
| **Bottom** | -1 untapped | Reset (zero both values) |
| **Left** | Tap 1 (untapped → tapped) | Tap all |
| **Right** | Untap 1 (tapped → untapped) | Untap all |

**8 actions, 4 buttons.**

---

## Core Gameplay Loop

1. **Add** tokens entering the battlefield → Top button
2. **Tap** them to attack → Left button
3. **Untap** survivors → Right button
4. **Remove** the dead → Bottom button (untap first if needed)
5. **Reset** when the token type leaves play → Long-press Bottom

---

## Design Decisions & Notes

### Why no "add tapped" button?
Tokens almost always enter untapped. Adding then tapping costs one extra press but keeps button count minimal. The button to add tapped directly was intentionally cut.

### Why no "remove tapped" button?
Untap then remove. Follows natural gameplay flow and avoids an extra button.

### Why 4-digit single display instead of two 2-digit displays?
Saves significant space. Token counts rarely exceed 99 per side, so two 2-digit numbers fit cleanly on a single 4-digit module with the middle used as a separator.

### Reset safety
Reset is a long-press on Bottom to avoid accidental triggers. A future hardware revision could require a two-button combo for extra safety.

---

## Maybe / Future

- **Undo** — no button real estate for it currently; could be a two-button combo in firmware
- **+2 tapped / -2 tapped** — not needed if tap/untap buttons handle movement between states
- **Bulk increment** — no clear use case identified yet

---

## Hardware Candidates

| Component | Option |
|-----------|--------|
| Microcontroller | ATtiny85, Seeed XIAO, or similar |
| Display | TM1637 4-digit 7-segment or 0.96" OLED |
| Buttons | 4× tactile switches |
| Power | CR2032 or small LiPo |

---

## Status

- [x] Interaction model defined
- [x] Button layout defined
- [ ] Hardware selected
- [ ] Schematic
- [ ] PCB layout
- [ ] Firmware
- [ ] Enclosure / 3D print
