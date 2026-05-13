#!/usr/bin/env python3
# ****************************************************************************
#  tools/gen-menu-doc.py                                        DB48X project
# ****************************************************************************
#
#   Generate doc/8-menus-tree-<model>.md — a full description of the soft-menu
#   hierarchy, formatted to resemble the calculator's 3-row × 6-button
#   display.
#
#   Source of truth:   src/menu.cc   (MENU macros)
#   Name lookup:       src/ids.tbl   (CMD / NAMED / ALIAS entries)
#   Link targets:      doc/commands/*.md  (## headings)
#   Output:            doc/8-menus-tree-{dm42,dm32}.md
#
# ****************************************************************************
#   (C) 2026 Christophe de Dinechin <christophe@dinechin.org>
#   This software is licensed under the terms outlined in LICENSE.txt
# ****************************************************************************

import argparse
import re
import sys
from pathlib import Path

ROOT      = Path(__file__).resolve().parent.parent
# Explicit UTF-8: Windows defaults read_text/write_text to the locale (e.g. cp1252).
TEXT_ENCODING = 'utf-8'
MENU_CC   = ROOT / 'src'  / 'menu.cc'
UI_CC     = ROOT / 'src'  / 'user_interface.cc'
IDS_TBL   = ROOT / 'src'  / 'ids.tbl'
DOC_CMDS  = ROOT / 'doc'  / 'commands'

SOFTKEYS  = 6   # buttons per row
PLANES    = 3   # rows per page

# Physical key labels per calculator model
KEY_LABELS = {
    'dm42': {
        'KEY_SIGMA': 'Σ+',  'KEY_INV':   '1/x', 'KEY_SQRT':  '√x',
        'KEY_LOG':   'LOG',  'KEY_LN':    'LN',   'KEY_XEQ':   'XEQ',
        'KEY_STO':   'STO',  'KEY_RCL':   'RCL',  'KEY_RDN':   'R↓',
        'KEY_SIN':   'SIN',  'KEY_COS':   'COS',  'KEY_TAN':   'TAN',
        'KEY_ENTER': 'ENTER','KEY_SWAP':  'x⇆y', 'KEY_CHS':   '+/-',
        'KEY_E':     'EEX',  'KEY_BSP':   '⌫',   'KEY_UP':    '▲',
        'KEY_7':     '7',    'KEY_8':     '8',    'KEY_9':     '9',
        'KEY_DIV':   '÷',   'KEY_DOWN':  '▼',   'KEY_4':     '4',
        'KEY_5':     '5',    'KEY_6':     '6',    'KEY_MUL':   '×',
        'KEY_SHIFT': '🟨',  'KEY_1':     '1',    'KEY_2':     '2',
        'KEY_3':     '3',    'KEY_SUB':   '-',    'KEY_EXIT':  'EXIT',
        'KEY_0':     '0',    'KEY_DOT':   '.',    'KEY_RUN':   'R/S',
        'KEY_ADD':   '+',
    },
    'dm32': {
        # Row 1 is reordered vs DM42: √x eˣ 10ˣ yˣ 1/x Σ+
        'KEY_SIGMA': '√x',  'KEY_INV':   'eˣ',  'KEY_SQRT':  '10ˣ',
        'KEY_LOG':   'yˣ',  'KEY_LN':    '1/x',  'KEY_XEQ':   'Σ+',
        'KEY_STO':   'STO',  'KEY_RCL':   'RCL',  'KEY_RDN':   'R↓',
        'KEY_SIN':   'SIN',  'KEY_COS':   'COS',  'KEY_TAN':   'TAN',
        'KEY_ENTER': 'ENTER','KEY_SWAP':  'x⇆y', 'KEY_CHS':   '+/-',
        'KEY_E':     'EEX',  'KEY_BSP':   '⌫',   'KEY_UP':    '▲',
        'KEY_7':     '7',    'KEY_8':     '8',    'KEY_9':     '9',
        'KEY_DIV':   '÷',   'KEY_DOWN':  '▼',   'KEY_4':     '4',
        'KEY_5':     '5',    'KEY_6':     '6',    'KEY_MUL':   '×',
        'KEY_SHIFT': '🟦',  'KEY_1':     '1',    'KEY_2':     '2',
        'KEY_3':     '3',    'KEY_SUB':   '-',    'KEY_EXIT':  'ON',
        'KEY_0':     '0',    'KEY_DOT':   '.',    'KEY_RUN':   'R/S',
        'KEY_ADD':   '+',
    },
}
KEY_LABELS['dm42n'] = KEY_LABELS['dm42']   # DM42N shares DM42 key labels

# Alpha letter assigned to each physical key (from user_interface.cc key_label)
# Same for all models — letters are logical, not physical
KEY_LETTERS = {
    'KEY_SIGMA': 'A', 'KEY_INV':  'B', 'KEY_SQRT': 'C',
    'KEY_LOG':   'D', 'KEY_LN':   'E', 'KEY_XEQ':  'F',
    'KEY_STO':   'G', 'KEY_RCL':  'H', 'KEY_RDN':  'I',
    'KEY_SIN':   'J', 'KEY_COS':  'K', 'KEY_TAN':  'L',
    'KEY_SWAP':  'M', 'KEY_CHS':  'N', 'KEY_E':    'O',
    'KEY_7':     'P', 'KEY_8':    'Q', 'KEY_9':    'R',
    'KEY_DIV':   'S', 'KEY_4':    'T', 'KEY_5':    'U',
    'KEY_6':     'V', 'KEY_MUL':  'W', 'KEY_1':    'X',
    'KEY_2':     'Y', 'KEY_3':    'Z',
}

SHIFT_SYMBOLS = ['', '🟨 ', '🟦 ']   # plane 0/1/2


# ─────────────────────────── helpers ────────────────────────────────────────

def strip_c_comments(text):
    """Remove // and /* */ C/C++ comments."""
    text = re.sub(r'/\*.*?\*/', ' ', text, flags=re.DOTALL)
    text = re.sub(r'//[^\n]*',  ' ', text)
    return text


def split_top_level(text):
    """Split *text* on top-level commas (not inside parens/strings)."""
    tokens, depth, in_str, esc, start = [], 0, False, False, 0
    for i, c in enumerate(text):
        if esc:             esc = False; continue
        if c == '\\' and in_str: esc = True; continue
        if c == '"':        in_str = not in_str; continue
        if in_str:          continue
        if c in '([{':      depth += 1
        elif c in ')]}':    depth -= 1
        elif c == ',' and depth == 0:
            tokens.append(text[start:i]); start = i + 1
    tokens.append(text[start:])
    return tokens


def extract_c_string(tok):
    """Return the value of the first C string literal found in *tok*."""
    m = re.search(r'"((?:[^"\\]|\\.)*)"', tok)
    if not m:
        return None
    s = m.group(1)
    s = s.replace('\\t', '')   # \t = cursor-position hint in SelfInsert labels
    s = s.replace('\\n', ' ')
    return s


# ───────────────────────── ids.tbl parsing ──────────────────────────────────

def parse_ids_tbl(path):
    """
    Return dict  cpp_name → canonical_display_name.
    CMD(X)           → X → 'X'
    NAMED(X, "y")    → X → 'y'
    """
    text = strip_c_comments(path.read_text(encoding=TEXT_ENCODING))
    names = {}
    for m in re.finditer(r'\bCMD\s*\(\s*(\w+)\s*\)', text):
        names.setdefault(m.group(1), m.group(1))
    for m in re.finditer(r'\bNAMED\s*\(\s*(\w+)\s*,\s*"([^"]+)"\s*\)', text):
        names[m.group(1)] = m.group(2)
    return names


# ───────────────────────── menu.cc parsing ──────────────────────────────────

def _find_balanced(text, start):
    """Return index of the closing ')' matching the '(' just before *start*."""
    depth = 1
    i     = start
    in_str = esc = False
    while i < len(text) and depth:
        c = text[i]
        if esc:             esc = False
        elif c == '\\' and in_str: esc = True
        elif c == '"':      in_str = not in_str
        elif not in_str:
            if   c == '(':  depth += 1
            elif c == ')':  depth -= 1
        i += 1
    return i - 1   # index of the matching ')'


def parse_menu_items(body):
    """
    Parse the argument list of a MENU() body (after the menu name and comma).
    Returns  list[tuple[str|None, str]]  = [(label_or_None, id_name), …]
    """
    items = []
    tokens = split_top_level(body)
    i = 0
    while i < len(tokens):
        raw = tokens[i].strip()
        i  += 1
        if not raw:
            continue

        # ── string literal → explicit label, next token must be ID_xxx ──
        if raw.startswith('"') or ('?' in raw and '"' in raw):
            label = extract_c_string(raw)
            if label is None:
                continue
            if i < len(tokens):
                nxt = tokens[i].strip()
                i  += 1
                if nxt.startswith('ID_'):
                    items.append((label, nxt[3:]))
            continue

        # ── bare ID_xxx → no explicit label ──
        if raw.startswith('ID_'):
            items.append((None, raw[3:]))
            continue

        # ── SomeClass::label,  ID_xxx → use class name as placeholder ──
        if '::label' in raw:
            cls   = raw.split('::')[0].strip()
            label = f'[{cls}]'
            if i < len(tokens):
                nxt = tokens[i].strip()
                i  += 1
                if nxt.startswith('ID_'):
                    items.append((label, nxt[3:]))
            continue

        # ── anything else: skip ──

    return items


def parse_menu_cc(path):
    """
    Return  OrderedDict  menu_name → [(label|None, id_name), …]
    """
    from collections import OrderedDict
    menus = OrderedDict()
    text  = strip_c_comments(path.read_text(encoding=TEXT_ENCODING))

    macro_re = re.compile(r'\bMENU\s*\(')
    pos = 0
    while True:
        m = macro_re.search(text, pos)
        if not m:
            break

        # Skip #define lines
        line_start = text.rfind('\n', 0, m.start()) + 1
        prefix     = text[line_start : m.start()].strip()
        if '#' in prefix:
            pos = m.end(); continue

        close = _find_balanced(text, m.end())
        body  = text[m.end() : close].strip()

        # First identifier is the menu name
        nm = re.match(r'(\w+)\s*,?\s*', body)
        if not nm:
            pos = close + 1; continue

        menu_name = nm.group(1)
        rest      = body[nm.end():]
        items     = parse_menu_items(rest) or []
        menus[menu_name] = items
        pos = close + 1

    return menus


# ──────────────────── doc anchor index ──────────────────────────────────────

def _gh_anchor(heading):
    """
    Approximate GitHub Markdown anchor from a heading string.
    Rules: lowercase; keep ASCII letters, digits, spaces, hyphens;
    strip everything else; replace spaces with '-'.
    """
    s = heading.lower()
    # keep alphanumeric, space, hyphen; strip rest
    s = re.sub(r'[^\w\s-]', '', s, flags=re.ASCII)
    s = re.sub(r'\s+', '-', s.strip())
    s = re.sub(r'-+', '-', s)
    return s


def build_conflicts(doc_root, menu_names):
    """
    Scan all doc/**/*.md (except 8-menus-tree-*.md) for headings that match
    a menu name.  Returns dict: menu_name → existing_anchor.
    """
    conflicts = {}
    for md in sorted(doc_root.glob('**/*.md')):
        if md.name.startswith('8-menus-tree-'):
            continue
        for m in re.finditer(r'^#{1,3} (.+)$',
                             md.read_text(encoding=TEXT_ENCODING),
                             re.MULTILINE):
            h = m.group(1).strip()
            if h in menu_names and h not in conflicts:
                conflicts[h] = _gh_anchor(h)
    return conflicts


def build_doc_index(doc_dir):
    """
    Scan doc/commands/*.md; return dict  key → (rel_path, anchor).
    Keys: heading text (as-is and lowercased), plus common aliases.
    Handles duplicate anchors per file the same way GitHub does (append -N).
    """
    idx = {}

    def add(key, path, anchor):
        k = key.lower().strip()
        if k and k not in idx:
            idx[k] = (path, anchor)

    for md in sorted(doc_dir.glob('*.md')):
        rel    = f'commands/{md.name}'
        seen   = {}   # base_anchor → count, for deduplication within this file
        for m in re.finditer(r'^#{1,3} (.+)$',
                             md.read_text(encoding=TEXT_ENCODING),
                             re.MULTILINE):
            h        = m.group(1).strip()
            base_anc = _gh_anchor(h)
            # GitHub dedup: first occurrence is plain, then -1, -2, ...
            cnt = seen.get(base_anc, 0)
            seen[base_anc] = cnt + 1
            anc = base_anc if cnt == 0 else f'{base_anc}-{cnt}'

            add(h,  rel, anc)
            # Also index without leading arrow characters (→ ← ↑ ↓ ▶ ◀)
            add(re.sub(r'^[→←↑↓▶◀]+\s*', '', h), rel, anc)

    return idx


# ─────────────────────── link resolution ────────────────────────────────────

# IDs whose `do_menu()` body lives in another .cc file (external menus)
EXTERNAL_MENUS = {
    'ConstantsMenu', 'EquationsMenu', 'Library',
    'UnitsMenu', 'CharactersMenu',
}


def resolve(label, id_name, name_map, doc_index, menu_names, conflicts=None):
    """
    Return  (display_text, url_or_None, kind)
    kind ∈ {'menu', 'command', 'unimplemented', 'selfinsert', 'external'}
    """
    is_menu = (id_name in menu_names or
               id_name in EXTERNAL_MENUS or
               id_name.endswith('Menu'))

    # Display text
    if label is not None:
        display = label
    elif id_name in name_map:
        display = name_map[id_name]
    else:
        display = id_name

    if not display:
        display = id_name

    # Special IDs
    if id_name == 'Unimplemented':
        return display, None, 'unimplemented'
    if id_name == 'SelfInsert':
        return display, None, 'selfinsert'

    # Menu reference
    if is_menu:
        if conflicts and id_name in conflicts:
            anchor = id_name.lower() + '-reference'
        else:
            anchor = id_name.lower()
        kind = 'external' if id_name in EXTERNAL_MENUS else 'menu'
        return display, f'#{anchor}', kind

    # Command: try several lookup keys
    keys = []
    if label:
        # class::label placeholder like [Base] → try without brackets
        clean_label = re.sub(r'^\[(.+)\]$', r'\1', label)
        keys.append(clean_label.lower())
        if clean_label != label:
            keys.append(label.lower())
    keys.append(id_name.lower())
    if id_name in name_map:
        keys.append(name_map[id_name].lower())
    # Strip leading arrow-like prefixes
    for k in list(keys):
        stripped = re.sub(r'^[→←↑↓▶◀]+\s*', '', k)
        if stripped != k:
            keys.append(stripped)

    for k in keys:
        if k in doc_index:
            _path, anchor = doc_index[k]
            return display, f'#{anchor}', 'command'

    return display, None, 'command'


# ─────────────────────── button rendering ───────────────────────────────────

def render_button(display, url, kind):
    """
    Return a markdown cell string for one soft-key button.

    Visual conventions (closest achievable in plain Markdown/GitHub):

      Menu button   →  visible brackets     [Label](#anchor) with escaped brackets
      Ext. menu     →  visible brackets     same
      Command       →  link                 [Label](path#anchor)
      No link cmd   →  plain                Label
      Unimplemented →  italic               *Label*
      SelfInsert    →  code                 `Label`
    """
    # Strip [ClassName] placeholder brackets from class::label references
    disp = re.sub(r'^\[(.+)\]$', r'\1', display)
    disp = disp.replace('|', '\\|').strip()
    if not disp:
        return ' '

    if kind == 'selfinsert':
        return f'`{disp}`'
    if kind == 'unimplemented':
        return f'_{disp}_'
    if kind in ('menu', 'external'):
        inner = f'[{disp}]({url})' if url else disp
        return f'\\[{inner}\\]'
    # command
    return f'[{disp}]({url})' if url else disp


# ─────────────────────── table generation ───────────────────────────────────

def menu_table(menu_name, items, name_map, doc_index, menu_names, conflicts=None):
    """Return a markdown table string for one menu."""
    if not items:
        return '*empty*\n'

    buttons = []
    for lbl, idn in items:
        disp, url, kind = resolve(lbl, idn, name_map, doc_index, menu_names, conflicts)
        buttons.append(render_button(disp, url, kind))

    n = len(buttons)

    # Single-page: ≤ 18 items use all 6 columns (no navigation).
    # Multi-page : > 18 items → 15 items/page (5 cols × 3 rows);
    #   F6 is reserved: bottom row = ▶ next, middle row = ◀ prev, top row = empty.
    single_page_max = SOFTKEYS * PLANES          # 18
    multi_page_size = (SOFTKEYS - 1) * PLANES    # 15

    multipage = n > single_page_max
    page_size = multi_page_size if multipage else single_page_max
    cols      = SOFTKEYS - 1   if multipage else SOFTKEYS
    pages     = (n + page_size - 1) // page_size

    lines = []
    if multipage:
        lines.append(f'*{pages} pages · {n} items total*\n')

    hdr = '| ' + ' | '.join(f'F{k+1}' for k in range(SOFTKEYS)) + ' |'
    sep = '|' + '|'.join(':--:' for _ in range(SOFTKEYS)) + '|'

    for page in range(pages):
        if multipage:
            lines.append(f'**Page {page+1}**\n')
        lines.append(hdr)
        lines.append(sep)

        base      = page * page_size
        page_btns = buttons[base : base + page_size]

        # Navigation buttons for F6 (only for multi-page menus).
        # Navigation is cyclic: every page shows both ▶ (next) and ◀ (prev).
        # Plane 0 (bottom): ▶ next, plane 1 (middle): ◀ prev, plane 2 (top): empty.
        nav = [' ', ' ', ' ']
        if multipage:
            nav[0] = '▶'
            nav[1] = '◀'

        # Rows stored bottom-first; display top-to-bottom (plane 2 → 1 → 0).
        for row in reversed(range(PLANES)):
            row_btns = page_btns[row * cols : (row + 1) * cols]
            while len(row_btns) < cols:
                row_btns.append(' ')
            if multipage:
                row_btns.append(nav[row])
            lines.append('| ' + ' | '.join(row_btns) + ' |')
        lines.append('')

    return '\n'.join(lines)


# ────────────────────── keymap parsing ──────────────────────────────────────

def parse_keymap(path):
    """
    Parse the three default keymap tables in user_interface.cc.
    Returns dict: id_name → list[(plane, key_const)]
    plane: 0=unshifted, 1=🟨, 2=🟦
    """
    text = path.read_text(encoding=TEXT_ENCODING)
    table_names = [
        'defaultUnshiftedCommand',
        'defaultShiftedCommand',
        'defaultSecondShiftedCommand',
    ]
    result = {}
    for plane, tname in enumerate(table_names):
        start = text.find(tname + '[')
        if start < 0:
            continue
        brace = text.find('{', start)
        end   = text.find('};', brace)
        if brace < 0 or end < 0:
            continue
        block = text[brace:end]
        for m in re.finditer(
            r'OP2BYTES\s*\(\s*(KEY_\w+)\s*,\s*(?:\w+::)?ID_(\w+)\s*\)',
            block
        ):
            result.setdefault(m.group(2), []).append((plane, m.group(1)))
    return result


# ────────────────────── parent map ──────────────────────────────────────────

def build_parents(menus):
    """
    Return dict: child_menu_name → list[(parent_menu_name, item_index)]
    item_index is the 0-based position in the parent's items list.
    """
    parents = {}
    for parent_name, items in menus.items():
        for idx, (_lbl, id_name) in enumerate(items):
            if id_name in menus or id_name in EXTERNAL_MENUS:
                parents.setdefault(id_name, []).append((parent_name, idx))
    return parents


# ────────────────────── access path computation ─────────────────────────────

def item_key_str(idx, total):
    """
    Return the key press string to select item at position *idx* in a
    menu with *total* items: e.g. 'F3', '🟨 F2', '▶ 🟦 F1'.
    """
    multipage = total > SOFTKEYS * PLANES
    cols      = SOFTKEYS - 1 if multipage else SOFTKEYS
    page_size = cols * PLANES
    page   = idx // page_size
    within = idx % page_size
    row    = within // cols   # 0=bottom(unshifted), 1=middle(🟨), 2=top(🟦)
    col    = within % cols
    prefix = '' if page == 0 else ('▶ ' if page == 1 else f'▶×{page} ')
    return f'{prefix}{SHIFT_SYMBOLS[row]}F{col + 1}'


def format_access_line(menu_name, direct_keys, parents, menus,
                       key_labels, conflicts=None):
    """
    Return the 'Access: …\\n' markdown line, or '' if none found.

    Shows direct keyboard shortcuts first, then immediate parent menus
    as '[ParentMenu] Fk' (no full chain).
    """
    parts = []

    # Direct keyboard shortcuts (shortest first)
    def fmt_key(p, k):
        label  = key_labels.get(k, k)
        letter = KEY_LETTERS.get(k)
        key    = f'{letter} ({label})' if letter else f'({label})'
        return f'{SHIFT_SYMBOLS[p]}{key}'

    direct = sorted(
        {fmt_key(p, k) for p, k in direct_keys.get(menu_name, [])},
        key=lambda s: (s.count(' '), s)
    )
    parts.extend(direct)

    # Immediate parent menus
    for parent_name, idx in sorted(parents.get(menu_name, []),
                                   key=lambda t: t[0].lower()):
        total  = len(menus.get(parent_name, []))
        step   = item_key_str(idx, total)
        anchor = parent_name.lower() + ('-reference' if conflicts and parent_name in conflicts else '')
        parts.append(f'[{parent_name}](#{anchor}) {step}')

    if not parts:
        return ''
    return 'Access: ' + '; '.join(parts) + '\n'


# ─────────────────────────── main ───────────────────────────────────────────

def main():
    parser = argparse.ArgumentParser(
        description='Generate DB48X menu tree documentation')
    parser.add_argument('--model', choices=list(KEY_LABELS), default='dm42',
                        help='Calculator model (default: dm42)')
    args = parser.parse_args()

    model      = args.model
    key_labels = KEY_LABELS[model]
    output     = ROOT / 'doc' / f'8-menus-tree-{model}.md'

    print(f'Reading {IDS_TBL.relative_to(ROOT)} …',  file=sys.stderr)
    name_map = parse_ids_tbl(IDS_TBL)

    print(f'Reading {MENU_CC.relative_to(ROOT)} …',  file=sys.stderr)
    menus    = parse_menu_cc(MENU_CC)
    menu_names = set(menus) | EXTERNAL_MENUS

    print(f'Reading {UI_CC.relative_to(ROOT)} …',    file=sys.stderr)
    direct_keys = parse_keymap(UI_CC)
    parents     = build_parents(menus)

    print(f'Indexing {DOC_CMDS.relative_to(ROOT)}/ …', file=sys.stderr)
    doc_index  = build_doc_index(DOC_CMDS)
    conflicts  = build_conflicts(ROOT / 'doc', menu_names)

    # ── file header ──────────────────────────────────────────────────────────
    out = []
    out.append('# Soft Menus Tree\n')
    out.append('<!--- DM.WARNING: Auto-generated by tools/gen-menu-doc.py — do not edit manually. --->\n')
    out.append('<!--- !DM.WARNING --->\n')
    out.append(
        f'Soft-menu hierarchy of the DB48X calculator for {model} keyboard\n'
        '\n'
        'Buttons are shown as they appear on the calculator screen:\n'
        '**6 columns** (F1–F6) and **3 rows** per page.\n'
    )
    out.append('**Legend**\n')
    out.append(
        '| Style | Meaning |\n'
        '|:------|:--------|\n'
        '| \\[Menu\\] | Opens a sub-menu (defined in this document) |\n'
        '| [Command](#add) | Executes a command — link leads to the reference doc |\n'
        '| Command | Command with no documentation entry found |\n'
        '| _Unimplemented_ | Not yet implemented |\n'
        '| `text` | Inserts literal text in the command line |\n'
        '| 🟨 KEY | Press the yellow shift key once, then KEY |\n'
        '| 🟦 KEY | Press the yellow shift key twice, then KEY |\n'
        '| ▶ Fn | Navigate to next page, then press Fn |\n'
    )
    out.append('---\n')

    # Sort alphabetically for the document, including external menus
    all_menu_names = set(menus) | EXTERNAL_MENUS
    sorted_menus = dict(sorted(menus.items(), key=lambda x: x[0].lower()))
    sorted_external = sorted(EXTERNAL_MENUS - set(menus), key=str.lower)

    # ── table of contents ────────────────────────────────────────────────────
    out.append('## Contents\n')
    for name in sorted(all_menu_names, key=str.lower):
        anchor = name.lower() + ('-reference' if name in conflicts else '')
        out.append(f'- [{name}](#{anchor})')
    out.append('')
    out.append('---\n')

    # ── one section per menu ─────────────────────────────────────────────────
    for name, items in sorted_menus.items():
        if name in conflicts:
            out.append(f'### {name} Reference\n')
            out.append(f'See also: [{name}](#{conflicts[name]}) user documentation\n')
        else:
            out.append(f'### {name}\n')
        access = format_access_line(name, direct_keys, parents, menus,
                                    key_labels, conflicts)
        if access:
            out.append(access)
        out.append(menu_table(name, items, name_map, doc_index, menu_names, conflicts))

    # ── placeholder sections for external (dynamically populated) menus ──────
    for name in sorted_external:
        if name in conflicts:
            out.append(f'### {name} Reference\n')
            out.append(f'See also: [{name}](#{conflicts[name]}) user documentation\n')
        else:
            out.append(f'### {name}\n')
        out.append('_Content loaded dynamically at runtime._\n')

    output.write_text('\n'.join(out) + '\n', encoding=TEXT_ENCODING)
    print(f'→ {output.relative_to(ROOT)}  ({len(menus)} menus)', file=sys.stderr)


if __name__ == '__main__':
    main()
