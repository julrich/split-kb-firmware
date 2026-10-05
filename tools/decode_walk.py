#!/usr/bin/env python3
"""Decode the LED walk (auto-advance walker build) from the hidraw captures.

  usage: decode_walk.py <skip-list> [key-file]

Order-based: in that build every press on a half's own keys advances the index,
so the capture holds exactly one report per index the user could press for - the
k-th press belongs to the k-th index that is not in <skip-list>.  Also maps the
probe's F13-F24 substitutions back to their matrix cells.
"""
import re
import subprocess
import sys

MATRIX_ROWS, MATRIX_COLS = 10, 6

# probe rewrite table: substitute keycode -> matrix cell
SUBSTITUTE_CELLS = {
    0x68: (0, 1),   # F13 <- LGUI(Q)       kitty
    0x69: (0, 2),   # F14 <- LGUI(R)       wofi
    0x6A: (0, 3),   # F15 <- LGUI(E)       dolphin
    0x6B: (0, 4),   # F16 <- LGUI(F)       fullscreen
    0x6C: (0, 5),   # F17 <- LGUI(Shift+Q) close window
    0x6D: (5, 5),   # F18 <- LAlt(Space)   tmux special workspace
    0x6E: (4, 0),   # F19 <- RM_TOGG       outermost left thumb
    0x6F: (8, 0),   # F20 <- RM_NEXT       bottom-right outer key
    0x70: (5, 1),   # F21 <- KC_MUTE       right number row
    0x71: (5, 2),   # F22 <- KC_MNXT
    0x72: (5, 3),   # F23 <- KC_MPLY
    0x73: (5, 4),   # F24 <- KC_MPRV
}


def parse_skips(spec):
    out = set()
    for part in spec.split(','):
        part = part.strip()
        if not part:
            continue
        if '-' in part:
            a, b = part.split('-')
            out.update(range(int(a), int(b) + 1))
        else:
            out.add(int(part))
    return out


def read_events(path):
    events = []
    try:
        with open(path) as fh:
            for line in fh:
                m = re.match(r'\s*([\d.]+)\s+((?:[0-9a-f]{2} )+[0-9a-f]{2})\s*$', line)
                if m:
                    events.append((float(m.group(1)), bytes.fromhex(m.group(2).replace(' ', ''))))
    except FileNotFoundError:
        pass
    return events


def presses(path):
    out = []
    for t, d in read_events(path):
        if len(d) < 8:
            continue
        keys = [k for k in d[2:8] if k]
        if len(keys) == 1:
            out.append((t, d[0], keys[0]))
    return out


def layer0_keycodes(elf):
    nm = subprocess.run(['avr-nm', '-S', '--defined-only', elf], capture_output=True, text=True).stdout
    line = [l for l in nm.splitlines() if re.search(r'\bkeymaps\b', l)][0].split()
    addr, size = int(line[0], 16), int(line[1], 16)
    dump = subprocess.run(['avr-objdump', '-s', '--start-address=%d' % addr,
                           '--stop-address=%d' % (addr + size), elf],
                          capture_output=True, text=True).stdout
    data = bytearray()
    for l in dump.splitlines():
        m = re.match(r'\s*([0-9a-f]{4,})\s((?:[0-9a-f]{2,8}\s)+)', l)
        if m and int(m.group(1), 16) >= addr:
            h = ''.join(m.group(2).split())
            data += bytes.fromhex(h if len(h) % 2 == 0 else '0' + h)
    layer = data[0:MATRIX_ROWS * MATRIX_COLS * 2]
    return {(r, c): int.from_bytes(layer[(r * MATRIX_COLS + c) * 2:(r * MATRIX_COLS + c) * 2 + 2], 'little')
            for r in range(MATRIX_ROWS) for c in range(MATRIX_COLS)}


def json_claim(gen_path):
    gen = open(gen_path).read()
    block = re.search(r'g_led_config = \{\s*\{(.*?)\},\s*\{', gen, re.S).group(1)
    rows = [[int(x) for x in re.findall(r'\d+', line)] for line in block.strip().splitlines()]
    claim = {}
    for r, row in enumerate(rows):
        for c, led in enumerate(row):
            if led != 255:
                claim[led] = (r, c)
    return claim


def match_cell(table, mods, keycode):
    if keycode in SUBSTITUTE_CELLS:
        return [SUBSTITUTE_CELLS[keycode]]
    hits = []
    for cell, v in table.items():
        if v == 0:
            continue
        if v >= 0x2000:                                # mod-tap / layer-tap: tap emits bare kc
            if (v & 0xFF) == keycode and mods == 0:
                hits.append(cell)
        elif v >= 0x100:                               # mod-chord
            if (v & 0xFF) == keycode and (v >> 8) == mods:
                hits.append(cell)
        else:
            if v == keycode and mods == 0:
                hits.append(cell)
    return hits


def main():
    skips = parse_skips(sys.argv[1] if len(sys.argv) > 1 else '')
    key_file = sys.argv[2] if len(sys.argv) > 2 else '/tmp/walk-key.txt'

    table = layer0_keycodes('qmk_firmware/.build/sofle_rev1_miryoku.elf')
    claim = json_claim('qmk_firmware/.build/obj_sofle_rev1_miryoku/src/default_keyboard.c')

    presses_list = presses(key_file)
    print('presses captured: %d   indices skipped (per your notes): %d' % (len(presses_list), len(skips)))

    print('\nindex  report    cell   keyboard-definition says   keycode')
    idx = 0
    for (t, mods, kc) in presses_list:
        while idx in skips:
            print('%5d  (skipped)' % idx)
            idx += 1
        hits = match_cell(table, mods, kc)
        cell = hits[0] if len(hits) == 1 else '?%d' % len(hits)
        want = claim.get(idx, '?')
        kc16 = table[cell] if isinstance(cell, tuple) else 0
        print('%5d  %02x %02x     %-6s %-10s %04x%s' % (
            idx, mods, kc, cell, want, kc16, '' if cell == want else '   <-- definition is wrong here'))
        idx += 1


if __name__ == '__main__':
    main()
