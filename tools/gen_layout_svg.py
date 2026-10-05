#!/usr/bin/env python3
"""Generate an annotated layout schematic for the Sofle rev1 / Miryoku keymap.

Geometry comes from keyboards/sofle/rev1/keyboard.json (so the thumb clusters and
outer columns sit where they do on the board), the function of every key is
decoded from the *compiled* keymap (tap-hold keycodes included, rendered for a
German host), and the small grey text is the cap legend actually on this board -
the caps are a Colemak-DH-ish set, so they disagree with the firmware in places
and both are worth seeing.

    python3 tools/gen_layout_svg.py docs/sofle-layout.svg
"""
import json
import re
import subprocess
import sys

KB = 'qmk_firmware/keyboards/sofle/rev1/keyboard.json'
ELF = 'qmk_firmware/.build/sofle_rev1_miryoku.elf'
ROWS, COLS = 10, 6
U = 74
MARGIN = 34

def index_of(r, c):
    """Key number: each half numbered from its inner edge outward, top to bottom.

    L1..L6 / R1..R6 the number row, L7..L12 the Q row, L13..L18 the A row,
    L19..L24 the Z row, L25..L29 the thumbs (inner wide one first).
    """
    half = 'L' if r < 5 else 'R'
    rr = r if r < 5 else r - 5
    n = rr * 6 + (5 - c) + 1 if rr < 4 else 25 + (4 - c)
    return '%s%d' % (half, n)


# hand-written: what the 24 cells beyond Miryoku's 36 do, and their colour group
EXTRA = {
    (0, 0): ('´ / `', 'german'), (0, 1): ('kitty', 'app'), (0, 2): ('wofi', 'app'),
    (0, 3): ('dolphin', 'app'), (0, 4): ('fullscreen', 'app'), (0, 5): ('close win', 'app'),
    (1, 0): ('# / \'', 'german'), (2, 0): ('ß / ?', 'german'), (3, 0): ('< / >', 'german'),
    (4, 0): ('RGB on/off', 'rgb'), (4, 1): ('prev effect', 'rgb'),
    (5, 0): ('hue', 'rgb'), (5, 1): ('mute', 'media'), (5, 2): ('next track', 'media'),
    (5, 3): ('play/pause', 'media'), (5, 4): ('prev track', 'media'), (5, 5): ('tmux ws', 'tmux'),
    (6, 0): ('ü', 'german'), (7, 0): ('ä', 'german'),
    (8, 0): ('next effect', 'rgb'), (9, 0): ('brighter', 'rgb'), (9, 1): ('dimmer', 'rgb'),
}

LAYER_NAMES = ['Base', 'Extra', 'Tap', 'Button', 'Nav', 'Mouse', 'Media', 'Num', 'Sym', 'Fun']
MOD_NAMES = [(0x01, 'CTL'), (0x02, 'SFT'), (0x04, 'ALT'), (0x08, 'GUI')]

# HID usage -> what it types on a German host (only where that differs from US)
DE = {0x1C: 'z', 0x1D: 'y', 0x2D: 'ß', 0x2E: '´', 0x2F: 'ü', 0x33: 'ö', 0x34: 'ä',
      0x32: '#', 0x64: '<', 0x36: ',', 0x37: '.', 0x38: '-'}
KC = {'28': 'Enter', '29': 'Esc', '2A': 'Backspace', '2B': 'Tab', '2C': 'Space',
      '4C': 'Del', '4A': 'Home', '4D': 'End', '4E': 'PgDn', '4B': 'PgUp',
      '49': 'Ins', '4F': 'Right', '50': 'Left', '51': 'Down', '52': 'Up',
      '35': '`', '2D': '-', '2E': '=', '2F': '[', '30': ']', '31': '\\', '33': ';', '34': '\'',
      '36': ',', '37': '.', '38': '/', '64': '<', '32': '#', 'A8': 'mute', 'AB': 'next',
      'AE': 'play', 'AC': 'prev', 'A9': 'vol+', 'AA': 'vol-'}
CATS = {
    'base':   ('#e9e9e9', 'base layer: the letter/character it types (German host)'),
    'mod':    ('#ccd7ec', 'mod-tap: hold for the modifier'),
    'layer':  ('#ffe8a3', 'layer access: hold for the layer, tap for the small function'),
    'german': ('#b9d9f2', 'German character, this keymap'),
    'app':    ('#d7c6ee', 'Hyprland window / app chord'),
    'media':  ('#c7e6c9', 'media key'),
    'tmux':   ('#bfe6d8', 'tmux special workspace'),
    'rgb':    ('#ffd39a', 'RGB Matrix control'),
}


def layer0():
    nm = subprocess.run(['avr-nm', '-S', '--defined-only', ELF], capture_output=True, text=True).stdout
    line = [l for l in nm.splitlines() if re.search(r'\bkeymaps\b', l)][0].split()
    addr, size = int(line[0], 16), int(line[1], 16)
    dump = subprocess.run(['avr-objdump', '-s', '--start-address=%d' % addr,
                           '--stop-address=%d' % (addr + size), ELF],
                          capture_output=True, text=True).stdout
    data = bytearray()
    for l in dump.splitlines():
        m = re.match(r'\s*([0-9a-f]{4,})\s((?:[0-9a-f]{2,8}\s)+)', l)
        if m and int(m.group(1), 16) >= addr:
            h = ''.join(m.group(2).split())
            data += bytes.fromhex(h if len(h) % 2 == 0 else '0' + h)
    return {(r, c): int.from_bytes(data[(r * COLS + c) * 2:(r * COLS + c) * 2 + 2], 'little')
            for r in range(ROWS) for c in range(COLS)}


def char_of(usage):
    """What an unmodified keycode types, for a German host."""
    if 0x04 <= usage <= 0x1D:                       # a..z
        return DE.get(usage, chr(ord('a') + usage - 0x04))
    if 0x1E <= usage <= 0x26:                       # 1..9
        return str(usage - 0x1D)
    if usage == 0x27:
        return '0'
    return DE.get(usage, KC.get('%02X' % usage, '0x%02X' % usage))


def mod_of(mod):
    """QMK mod bits (0x10 = right side) -> a readable name."""
    side = 'R' if mod & 0x10 else 'L'
    base = '+'.join(side + n for bit, n in MOD_NAMES if mod & bit)
    return base or 'mod'


def describe(code):
    """-> (tap function, hold label or None, category) from a compiled keycode."""
    if code == 0:
        return ('-', None, 'base')
    if 0x4000 <= code <= 0x4FFF:                    # layer-tap
        layer = (code >> 8) & 0x0F
        tap = code & 0xFF
        name = LAYER_NAMES[layer] if layer < len(LAYER_NAMES) else 'L%d' % layer
        return (char_of(tap) if tap else '-', 'hold ' + name, 'layer')
    if 0x2000 <= code <= 0x3FFF:                    # mod-tap
        return (char_of(code & 0xFF), 'hold ' + mod_of((code >> 8) & 0x1F), 'mod')
    if 0x0100 <= code <= 0x1FFF:                    # hsv/mod-chord: real mods + usage
        return ('%s+%s' % (mod_of((code >> 8) & 0x1F), char_of(code & 0xFF)), None, 'base')
    if 0x7800 <= code <= 0x78FF:                    # RGB Matrix keycode
        rgb = {0x7842: 'RGB on/off', 0x7843: 'next effect', 0x7844: 'prev effect',
               0x7845: 'hue', 0x7849: 'brighter', 0x784A: 'dimmer'}
        return (rgb.get(code, '0x%04X' % code), None, 'rgb')
    return (char_of(code), None, 'base')


def esc(s):
    return s.replace('&', '&amp;').replace('<', '&lt;').replace('>', '&gt;')


def fit(text, size, width):
    """Shrink the font until the text fits the cell."""
    while size > 7 and len(text) * size * 0.60 > width:
        size -= 1
    return size


def main():
    out_path = sys.argv[1] if len(sys.argv) > 1 else 'docs/sofle-layout.svg'
    lay = json.load(open(KB))['layouts']['LAYOUT']['layout']
    pos = {(k['matrix'][0], k['matrix'][1]): (k['x'], k.get('y', 0)) for k in lay}
    codes = layer0()

    xs = [x for x, y in pos.values()] + [6.0, 9.5]
    ys = [y for x, y in pos.values()]
    x0, y0 = min(xs), min(ys)
    W = (max(xs) - x0 + 1.7) * U + 2 * MARGIN
    keys_h = (max(ys) - y0 + 1.35) * U
    legend_h = 24 * (len(CATS) + 1) + 130
    H = MARGIN + keys_h + legend_h

    def px(x, y):
        return MARGIN + (x - x0) * U, MARGIN + 18 + (y - y0) * U

    s = [f'<svg xmlns="http://www.w3.org/2000/svg" width="{W:.0f}" height="{H:.0f}" '
         f'viewBox="0 0 {W:.0f} {H:.0f}" font-family="DejaVu Sans, Noto Sans, sans-serif">',
         '<rect width="100%" height="100%" fill="#2b2f33"/>',
         f'<text x="{MARGIN}" y="24" fill="#fff" font-size="16" font-weight="bold">'
         f'Sofle rev1 / Miryoku &#8212; base layer, with key numbers</text>']

    for (r, c), (x, y) in sorted(pos.items()):
        if (r, c) in ((4, 5), (9, 5)):      # the two knob push-buttons
            continue
        cap = index_of(r, c)
        if (r, c) in EXTRA:
            func, cat = EXTRA[(r, c)]
            hold = None
        else:
            func, hold, cat = describe(codes[(r, c)])
        fill = CATS[cat][0]
        cx, cy = px(x, y)
        wide = (r, c) in ((4, 4), (9, 4))
        w = U * 1.5 - 5 if wide else U - 5
        s.append(f'<rect x="{cx:.0f}" y="{cy:.0f}" width="{w:.0f}" height="{U - 5}" rx="7" '
                 f'fill="{fill}" stroke="#1b1e21" stroke-width="1.2"/>')
        inner = w - 6
        fs = fit(func, 15, inner)
        ytxt = cy + U / 2 + (2 if not hold else -2)
        s.append(f'<text x="{cx + w / 2:.0f}" y="{ytxt:.0f}" text-anchor="middle" font-size="{fs}" '
                 f'font-weight="bold" fill="#1b1e21">{esc(func)}</text>')
        if hold:
            s.append(f'<text x="{cx + w / 2:.0f}" y="{cy + U / 2 + 14:.0f}" text-anchor="middle" '
                     f'font-size="{fit(hold, 10, inner)}" fill="#55595d">{esc(hold)}</text>')
        s.append(f'<text x="{cx + w / 2:.0f}" y="{cy + U - 11:.0f}" text-anchor="middle" '
                 f'font-size="{fit(cap, 11, inner)}" fill="#70747a">{esc(cap)}</text>')

    for ((r, c), (x, y), name) in (((4, 5), (6.0, 2.75), 'enc 1'), ((9, 5), (9.5, 2.75), 'enc 2')):
        cx, cy = px(x, y)
        s.append(f'<circle cx="{cx + U / 2:.0f}" cy="{cy + U / 2:.0f}" r="{U / 2 - 8:.0f}" '
                 f'fill="#7b8085" stroke="#1b1e21" stroke-width="1.2"/>')
        s.append(f'<text x="{cx + U / 2:.0f}" y="{cy + U / 2 + 4:.0f}" text-anchor="middle" '
                 f'font-size="11" fill="#fff">{esc(name)}</text>')

    ly = MARGIN + keys_h + 26
    s.append(f'<text x="{MARGIN}" y="{ly}" fill="#fff" font-size="14" font-weight="bold">Legend</text>')
    ly += 24
    for cat, (fill, label) in CATS.items():
        s.append(f'<rect x="{MARGIN}" y="{ly - 13}" width="20" height="15" rx="3" fill="{fill}" '
                 f'stroke="#1b1e21" stroke-width="1"/>')
        s.append(f'<text x="{MARGIN + 30}" y="{ly}" fill="#d6d9dc" font-size="12.5">{esc(label)}</text>')
        ly += 24
    ly += 8
    for line in [
        'Bold = what the key sends on the base layer, decoded from the compiled keymap and rendered for a German host.',
        'Grey = the key number: each half counted from its inner edge outwards and top to bottom - L1..L6 / R1..R6 the number row, then six per row,',
        'L25..L29 / R25..R29 the thumbs (wide inner one first). The README refers to keys by these numbers.',
        'All 24 cells outside Miryoku\'s 36 behave the same on every layer, so a function row never disappears while a layer is held.',
        'Encoder 1 (left) = volume. Encoder 2 (right) = PgUp/PgDn, or Up/Down while Nav is held, or the mouse wheel on the other layers. Both knobs double as LEDs 0 and 36 (dim blue).',
        'The OLED shows TSNM / alphas / layer on the half holding the USB cable, the logo on the other. Miryoku\'s layer contents: users/manna-harbour_miryoku/readme.org.',
    ]:
        s.append(f'<text x="{MARGIN}" y="{ly}" fill="#9aa0a6" font-size="11.5">{esc(line)}</text>')
        ly += 19
    s.append('</svg>')

    open(out_path, 'w').write('\n'.join(s))
    print('wrote', out_path)


if __name__ == '__main__':
    main()
