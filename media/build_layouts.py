#!/usr/bin/env python3
"""Generate SVG keymap images for the Corne Xiao v1 layout.

One SVG per layer in ./media/. 42-key physical layout.
Labels match the bindings in standard_layout.dtsi (incl. the recent edits:
tilde on Symbols, Windows NAV/Buttons, no Globe keys).

Run:  python3 media/build_layouts.py   (re-run after using the --svg flag below)
Output: media/*.svg
"""

import os
import html

MEDIA = os.path.dirname(os.path.abspath(__file__))

# ---- geometry ----
KW, KH = 96, 88
GX, GY = 10, 8
MX = 24
CENTRAL_GAP = 72
TH_DROP = 34
TITLE_Y = 20

def col_x(col): return col * (KW + GX)
def row_y(row): return 40 + row * (KH + GY)
def left_x(c):  return MX + col_x(c)
def right_x(c): return MX + 6 * col_x(1) + CENTRAL_GAP + col_x(c)

WIDTH  = MX + 6 * (KW + GX) + CENTRAL_GAP + 6 * (KW + GX) + MX
HEIGHT = row_y(2) + KH + GY + KH + TH_DROP + MX

def thumb_xy(i, side):
    x = (left_x(1) if side == 'L' else right_x(0)) + i * (KW + GX)
    y = row_y(3) + TH_DROP
    return x, y

C = dict(k='#f6f2ec', hr='#ffdfa8', ly='#d0e6ff', sp='#e4d9ff',
         sy='#ffd6cf', bl='#edede f'.replace(' ', ''), st='#3c3c46',
         tx='#20202b', ts='#7c7c8b', w='#ffffff')

def key(x, y, color, lines):
    out = [f'<rect x="{x}" y="{y}" width="{KW}" height="{KH}" rx="10" fill="{color}" '
           f'stroke="{C["st"]}" stroke-width="2"/>']
    if lines:
        cx = x + KW/2
        n = len(lines)
        start = y + KH/2 - (n-1)*11 + 4
        for i, (t, fs, col) in enumerate(lines):
            ty = start + i*22
            out.append(f'<text x="{cx}" y="{ty}" font-family="DejaVu Sans, Arial" '
                       f'font-size="{fs}" text-anchor="middle" fill="{col}" '
                       f'paint-order="stroke" stroke="{C["w"]}" stroke-width="2" '
                       f'stroke-linejoin="round">{html.escape(t)}</text>')
    return '\n'.join(out)

# line helpers
def L(t, s=17, c=None): return (t, s, c or C['tx'])
def LS(t, s=12):        return (t, s, C['ts'])

def blank(color=C['bl']):
    return lambda: (C['bl'], None)

def build_svg(title, grid):
    """grid: list of 42 key-defs in physical order.
       Each def = (side,row,col) or ('LT'/'RT', idx) mapped by position tens.
       We instead accept a dict keyed by position key."""
    # Build position -> (x,y) for all 42 keys
    pos = {}
    for r in range(3):
        for c in range(6):
            pos[('L', r, c)] = (left_x(c), row_y(r))
            pos[('R', r, c)] = (right_x(c), row_y(r))
    for i in range(3):
        pos[('LT', i)] = thumb_xy(i, 'L')
        pos[('RT', i)] = thumb_xy(i, 'R')

    parts = [f'<svg xmlns="http://www.w3.org/2000/svg" width="{WIDTH}" height="{HEIGHT}" '
             f'viewBox="0 0 {WIDTH} {HEIGHT}">',
             f'<rect width="{WIDTH}" height="{HEIGHT}" fill="{C["w"]}"/>',
             f'<text x="{WIDTH/2}" y="{TITLE_Y}" font-family="DejaVu Sans, Arial" '
             f'font-size="18" font-weight="bold" text-anchor="middle" fill="{C["tx"]}">{title}</text>']

    keys = grid.get('keys', {})
    for r in range(3):
        for c in range(6):
            for side in ('L', 'R'):
                x, y = pos[(side, r, c)]
                entry = keys.get((side, r, c))
                color, lines = entry if entry else (C['bl'], None)
                parts.append(key(x, y, color, lines))
    for i in range(3):
        for tag in ('LT', 'RT'):
            x, y = pos[(tag, i)]
            entry = keys.get((tag, i))
            color, lines = entry if entry else (C['bl'], None)
            parts.append(key(x, y, color, lines))
    parts.append('</svg>')
    return '\n'.join(parts)

def write_svg(filename, title, keys):
    path = os.path.join(MEDIA, filename)
    with open(path, 'w') as f:
        f.write(build_svg(title, {'keys': keys}))
    print('wrote', path)

# ===========================================================================
# Layer definitions
# ===========================================================================
P = {}  # shorthand: (side,row,col) or (tag,i) -> (color,[lines])

def Lk(x, color=C['k'], *lines): x[1] = (color, list(lines))

# ---- QWERTY ----
qw = {}
for i, t in enumerate(['[','!','@','#','$','%']): qw[('L',0,i)]=(C['k'],[L(t)])
for i, t in enumerate(['^','&','*','(',')',']']): qw[('R',0,i)]=(C['k'],[L(t)])
for i, t in enumerate(['TAB','Q','W','E','R','T']): qw[('L',1,i)]=(C['k'],[L(t)])
for i, t in enumerate(['Y','U','I','O','P','-']): qw[('R',1,i)]=(C['k'],[L(t)])
qw[('L',2,0)]=(C['k'],[L('`')])
qw[('L',2,1)]=(C['hr'],[L('A'),LS('Ctrl')])
qw[('L',2,2)]=(C['hr'],[L('S'),LS('Alt')])
qw[('L',2,3)]=(C['hr'],[L('D'),LS('Win')])
qw[('L',2,4)]=(C['hr'],[L('F'),LS('Shft')])
qw[('L',2,5)]=(C['k'],[L('G')])
qw[('R',2,0)]=(C['k'],[L('H')])
qw[('R',2,1)]=(C['hr'],[L('J'),LS('Shft')])
qw[('R',2,2)]=(C['hr'],[L('K'),LS('Win')])
qw[('R',2,3)]=(C['hr'],[L('L'),LS('Alt')])
qw[('R',2,4)]=(C['hr'],[L(';'),LS('Ctrl')])
qw[('R',2,5)]=(C['k'],[L("'")])
for i, t in enumerate(['\\','Z','X','C','V','B']): qw[('L',3,i)]=(C['k'],[L(t)])
for i, t in enumerate(['N','M',',','.','/','=']): qw[('R',3,i)]=(C['k'],[L(t)])
qw[('LT',0)]=(C['ly'],[L('SYS',15),LS('tap/toggle')])
qw[('LT',1)]=(C['ly'],[L('⌫ BACK',15),LS('hold NAV')])
qw[('LT',2)]=(C['ly'],[L('ESC',16),LS('hold SYM')])
qw[('RT',0)]=(C['ly'],[L('RET',16),LS('hold MED')])
qw[('RT',1)]=(C['ly'],[L('SPC',16),LS('hold NUM')])
qw[('RT',2)]=(C['ly'],[L('SYS',15),LS('tap/toggle')])
write_svg('layer-qwerty.svg', 'Corne Xiao v1 - Layer 1: QWERTY', qw)

# ---- NAVIGATION (Windows) ----
nav = {}
nav[('L',0,0)]=(C['sp'],[L('⌘⇥',15),LS('prev app')])   # Alt+Shift+Tab
for i, t in enumerate(['⌃1','⌃2','⌃3','⌃4','⌃5']): nav[('L',0,i+1)]=(C['k'],[L(t,15)])
nav[('R',0,0)]=(C['k'],[L('⇤',16)])
nav[('R',0,1)]=(C['k'],[L('⌃⇧G',13)])   # Ctrl+Shift+G find prev
nav[('R',0,2)]=(C['k'],[L('⌃G',14)])    # Ctrl+G find next
nav[('R',0,3)]=(C['k'],[L('TAB',14)])
nav[('L',1,0)]=(C['sp'],[L('Alt⇥',14),LS('next app')])  # Alt+Tab
for i, t in enumerate(['⌃Q','⌃W','⌃E','⌃R','⌃T']): nav[('L',1,i+1)]=(C['k'],[L(t,15)])
nav[('R',1,0)]=(C['k'],[L('HOME',13)])
nav[('R',1,1)]=(C['k'],[L('PGDN',13)])
nav[('R',1,2)]=(C['k'],[L('PGUP',13)])
nav[('R',1,3)]=(C['k'],[L('END',14)])
nav[('L',2,0)]=(C['sp'],[L('Alt⇥',14),LS('prev')])
for i, t in enumerate(['⌃A','⌃S','⌃D','⌃F']): nav[('L',2,i+1)]=(C['hr'],[L(t,15),LS('hold')])
nav[('L',2,5)]=(C['k'],[L('⌃G',13)])  # NAV_LG_G
for i, t in enumerate(['←','↓','↑','→']): nav[('R',2,1+i)]=(C['k'],[L(t,18)])
nav[('L',3,0)]=(C['sp'],[L('Alt',14),LS('prev win')])  # winPglobe prev window
for i, t in enumerate(['⌃Z','⌃X','⌃C','⌃V','⌃B']): nav[('L',3,i+1)]=(C['k'],[L(t,14)])
nav[('R',3,0)]=(C['k'],[L('⌃←',13)])  # prev word
nav[('R',3,1)]=(C['k'],[L('HOME',13)])
nav[('R',3,2)]=(C['k'],[L('END',14)])  # MAC_ENDL -> line end
nav[('R',3,3)]=(C['k'],[L('⌃→',13)])   # next word
nav[('LT',2)]=(C['ly'],[L('NAV',14),LS('toggle')])
nav[('RT',2)]=(C['ly'],[L('NAV',14),LS('toggle')])
write_svg('layer-nav.svg', 'Corne Xiao v1 - Layer 2: Navigation', nav)

# ---- NUMBERS ----
num = {}
num[('L',0,0)]=(C['k'],[L('[',16)])
num[('L',0,1)]=(C['k'],[L('*',16)])
num[('L',0,2)]=(C['k'],[L('/',16)])
num[('L',0,3)]=(C['k'],[L(']',16)])
num[('L',0,4)]=(C['k'],[L('^',16)])
num[('L',1,0)]=(C['k'],[L('TAB',14)])
for i, t in enumerate(['1','2','3','4','5']): num[('L',1,i+1)]=(C['k'],[L(t,17)])
num[('L',2,0)]=(C['k'],[L('⏎',16)])
for i, t in enumerate(['6','7','8','9','0']): num[('L',2,i+1)]=(C['k'],[L(t,17)])
num[('L',3,0)]=(C['k'],[L('=',15)])
for i, t in enumerate(['(','+','-',')','.']): num[('L',3,i+1)]=(C['k'],[L(t,16)])
num[('LT',0)]=(C['ly'],[L('NUM',14),LS('toggle')])
num[('LT',1)]=(C['k'],[L(',',16)])
num[('LT',2)]=(C['k'],[L('BS⌫',15),LS('⇧=DEL')])  # kp_bspcdel
num[('RT',0)]=(C['k'],[L('SPC',15)])
num[('RT',2)]=(C['ly'],[L('NUM',14),LS('toggle')])
write_svg('layer-num.svg', 'Corne Xiao v1 - Layer 3: Numbers', num)

# ---- SYMBOLS ----
sym = {}
for i, t in enumerate(['?','+','-','.','/','\\']): sym[('R',0,i)]=(C['k'],[L(t,16)])
for i, t in enumerate(['&','<','=','>','|','#']): sym[('R',1,i)]=(C['k'],[L(t,16)])
for i, t in enumerate(['[','(',':','·',')',']','%']):
    pass
sym[('R',2,0)]=(C['k'],[L('[',16)])
sym[('R',2,1)]=(C['k'],[L('(',16)])
sym[('R',2,2)]=(C['k'],[L(':',16)])
sym[('R',2,3)]=(C['k'],[L(')',16)])
sym[('R',2,4)]=(C['k'],[L(']',16)])
sym[('R',2,5)]=(C['k'],[L('%',16)])
for i, t in enumerate(['$','{','*','}','^','!']): sym[('R',3,i+1)]=(C['k'],[L(t,16)])
sym[('R',3,0)]=(C['k'],[L('$',16)])
for i in range(6): sym[('R',3,i)]=(C['k'],[L(['$','{','*','}','^','!'][i],16)])
sym[('LT',0)]=(C['k'],[L('~',18)])  # NEW tilde on left outer thumb
sym[('LT',2)]=(C['ly'],[L('SYM',14),LS('toggle')])
sym[('RT',0)]=(C['k'],[L('@',16)])
sym[('RT',1)]=(C['k'],[L('SPC',15)])
sym[('RT',2)]=(C['ly'],[L('SYM',14),LS('toggle')])
# left HRM layer (for completeness)
for i, t in enumerate(['⌃','Alt','Win','⇧']): sym[('L',2,i+1)]=(C['hr'],[L(t,14)])
write_svg('layer-sym.svg', 'Corne Xiao v1 - Layer 4: Symbols', sym)

# ---- MEDIA ----
med = {}
med[('L',0,0)]=(C['sp'],[L('VOL+',14)])
med[('L',0,1)]=(C['sp'],[L('BRI+',14)])
med[('L',1,0)]=(C['sp'],[L('VOL-',14)])
med[('L',1,1)]=(C['sp'],[L('BRI-',14)])
med[('L',1,2)]=(C['sp'],[L('⏮',16)])
med[('L',1,3)]=(C['sp'],[L('▶|❚❚',14)])
med[('L',1,4)]=(C['sp'],[L('⏭',16)])
med[('L',2,0)]=(C['sp'],[L('MUTE',13)])
med[('LT',0)]=(C['ly'],[L('MED',14),LS('toggle')])
med[('RT',0)]=(C['k'],[L('⏹',16),LS('stop')])
med[('RT',1)]=(C['ly'],[L('MED',14),LS('toggle')])
write_svg('layer-med.svg', 'Corne Xiao v1 - Layer 5: Media', med)

# ---- FUNCTIONS ----
fun = {}
for i, t in enumerate(['F16','F17','F18','F19','F20']): fun[('L',0,i+1)]=(C['k'],[L(t,15)])
for i, t in enumerate(['F11','F12','F13','F14','F15']): fun[('L',1,i+1)]=(C['k'],[L(t,15)])
for i, t in enumerate(['F6','F7','F8','F9','F10']): fun[('L',2,i+1)]=(C['k'],[L(t,15)])
for i, t in enumerate(['F1','F2','F3','F4','F5']): fun[('L',3,i+1)]=(C['k'],[L(t,15)])
fun[('LT',0)]=(C['ly'],[L('FUN',14),LS('toggle')])
write_svg('layer-fun.svg', 'Corne Xiao v1 - Layer 6: Functions', fun)

# ---- BUTTONS (Windows) ----
but = {}
but[('L',0,1)]=(C['k'],[L('⇤',16)])
but[('L',0,2)]=(C['k'],[L('⌃⇧G',13)])
but[('L',0,3)]=(C['k'],[L('⌃G',13)])
but[('L',0,4)]=(C['k'],[L('TAB',14)])
but[('L',0,5)]=(C['k'],[L('WinE',14)])
but[('R',0,0)]=(C['k'],[L('WinE',14)])
but[('R',0,1)]=(C['k'],[L('⇤',16)])
but[('R',0,2)]=(C['k'],[L('⌃⇧G',13)])
but[('R',0,3)]=(C['k'],[L('⌃G',13)])
but[('R',0,4)]=(C['k'],[L('TAB',14)])
for i, t in enumerate(['⌃Win←','⌃⇧F6','⌃F6','⌃Win→','⌃⇥']): but[('L',1,i+1)]=(C['k'],[L(t,12)])
for i, t in enumerate(['⌃⇥','⌃Win←','⌃⇧F6','⌃F6','⌃Win→']): but[('R',1,i+1)]=(C['k'],[L(t,12)])
but[('L',2,0)]=(C['k'],[L('⌃',13)])
but[('L',2,1)]=(C['k'],[L('Alt',13)])
but[('L',2,2)]=(C['k'],[L('Win',13)])
but[('L',2,3)]=(C['k'],[L('⇧',13)])
but[('L',2,5)]=(C['k'],[L('Win⇥',13)])  # Task View
but[('R',2,0)]=(C['k'],[L('Win⇥',13)])
but[('R',2,1)]=(C['k'],[L('⇧',13)])
but[('R',2,2)]=(C['k'],[L('Win',13)])
but[('R',2,3)]=(C['k'],[L('Alt',13)])
but[('R',2,4)]=(C['k'],[L('⌃',13)])
# row4 (bottom)
row4l = ['⌃Y','⌃Z','⌃X','⌃C','⌃V','WinS']
row4r = ['WinS','⌃V','⌃C','⌃X','⌃Z','⌃Y']
for i, t in enumerate(row4l): but[('L',3,i)]=(C['k'],[L(t,13)])
for i, t in enumerate(row4r): but[('R',3,i)]=(C['k'],[L(t,13)])
but[('LT',0)]=(C['ly'],[L('BUT',14),LS('toggle')])
but[('RT',2)]=(C['ly'],[L('BUT',14),LS('toggle')])
write_svg('layer-buttons.svg', 'Corne Xiao v1 - Layer 7: Buttons', but)

# ---- SYSTEM ----
sysd = {}
# row1
sysd[('L',1,0)]=(C['sy'],[L('BOOT',13),LS('left')])
sysd[('R',1,5)]=(C['sy'],[L('BOOT',13),LS('right')])
# row2
sysd[('L',2,0)]=(C['sy'],[L('RESET',13)])
sysd[('R',2,5)]=(C['sy'],[L('RESET',13)])
sysd[('L',1,5)]=(C['sp'],[L('PWR',14),LS('toggle')])  # ext_power
sysd[('R',2,0)]=(C['sp'],[L('USB/BLE',13)])           # out
# row4 bt
bd = ['BT0','BT1','BT2','BT3','BT4','CLR']
for i, t in enumerate(bd): sysd[('R',3,i)]=(C['sy'],[L(t,14)])
sysd[('L',3,5)]=(C['sy'],[L('PWR',13)])  # C_PWR
sysd[('LT',0)]=(C['ly'],[L('SYS',14),LS('toggle')])
sysd[('RT',2)]=(C['ly'],[L('SYS',14),LS('toggle')])
write_svg('layer-system.svg', 'Corne Xiao v1 - Layer 8: System', sysd)

print('done')
