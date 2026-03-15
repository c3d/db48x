#!/usr/bin/env python3
# ****************************************************************************
#  tools/gen-menu-doc.py                                        DB48X project
# ****************************************************************************
#
#   Generate doc/menus-tree.md — a full description of the soft-menu
#   hierarchy, formatted to resemble the calculator's 3-row × 6-button
#   display.
#
#   Source of truth:   src/menu.cc   (MENU macros)
#   Name lookup:       src/ids.tbl   (CMD / NAMED / ALIAS entries)
#   Link targets:      doc/commands/*.md  (## headings)
#   Output:            doc/menus-tree.md
#
# ****************************************************************************
#   (C) 2026 Christophe de Dinechin <christophe@dinechin.org>
#   This software is licensed under the terms outlined in LICENSE.txt
# ****************************************************************************

import re
import sys
from pathlib import Path

ROOT      = Path(__file__).resolve().parent.parent
MENU_CC   = ROOT / 'src'  / 'menu.cc'
IDS_TBL   = ROOT / 'src'  / 'ids.tbl'
DOC_CMDS  = ROOT / 'doc'  / 'commands'
OUTPUT    = ROOT / 'doc'  / 'menus-tree.md'

SOFTKEYS  = 6   # buttons per row
PLANES    = 3   # rows per page


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
    text = strip_c_comments(path.read_text())
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
    text  = strip_c_comments(path.read_text())

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
        for m in re.finditer(r'^#{1,3} (.+)$', md.read_text(), re.MULTILINE):
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


def resolve(label, id_name, name_map, doc_index, menu_names):
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
        anchor = id_name.lower()
        kind   = 'external' if id_name in EXTERNAL_MENUS else 'menu'
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
            path, anchor = doc_index[k]
            return display, f'{path}#{anchor}', 'command'

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
        return '&nbsp;'

    if kind == 'selfinsert':
        return f'`{disp}`'
    if kind == 'unimplemented':
        return f'<font color="gray">{disp}</font>'
    if kind in ('menu', 'external'):
        inner = f'[{disp}]({url})' if url else disp
        return f'\\[{inner}\\]'
    # command
    return f'[{disp}]({url})' if url else disp


# ─────────────────────── table generation ───────────────────────────────────

def menu_table(menu_name, items, name_map, doc_index, menu_names):
    """Return a markdown table string for one menu."""
    if not items:
        return '*empty*\n'

    buttons = []
    for lbl, idn in items:
        disp, url, kind = resolve(lbl, idn, name_map, doc_index, menu_names)
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
        nav = ['&nbsp;', '&nbsp;', '&nbsp;']
        if multipage:
            nav[0] = '▶'
            nav[1] = '◀'

        # Rows stored bottom-first; display top-to-bottom (plane 2 → 1 → 0).
        for row in reversed(range(PLANES)):
            row_btns = page_btns[row * cols : (row + 1) * cols]
            while len(row_btns) < cols:
                row_btns.append('&nbsp;')
            if multipage:
                row_btns.append(nav[row])
            lines.append('| ' + ' | '.join(row_btns) + ' |')
        lines.append('')

    return '\n'.join(lines)


# ─────────────────────────── main ───────────────────────────────────────────

def main():
    print(f'Reading {IDS_TBL.relative_to(ROOT)} …',  file=sys.stderr)
    name_map = parse_ids_tbl(IDS_TBL)

    print(f'Reading {MENU_CC.relative_to(ROOT)} …',  file=sys.stderr)
    menus    = parse_menu_cc(MENU_CC)
    menu_names = set(menus) | EXTERNAL_MENUS

    print(f'Indexing {DOC_CMDS.relative_to(ROOT)}/ …', file=sys.stderr)
    doc_index = build_doc_index(DOC_CMDS)

    # ── file header ──────────────────────────────────────────────────────────
    out = []
    out.append('# DB48X Menu Tree\n')
    out.append(
        'Soft-menu hierarchy of the DB48X calculator.\n'
        'Buttons are shown as they appear on the calculator screen:\n'
        '**6 columns** (F1–F6) and **3 rows** per page.\n'
    )
    out.append('**Legend**\n')
    out.append(
        '| Style | Meaning |\n'
        '|:------|:--------|\n'
        '| \\[Menu\\] | Opens a sub-menu (defined in this document) |\n'
        '| [Command](commands/symbolic.md) | Executes a command — link leads to the reference doc |\n'
        '| Command | Command with no documentation entry found |\n'
        '| <font color="gray">Unimplemented</font> | Not yet implemented |\n'
        '| `text` | Inserts literal text in the command line |\n'
    )
    out.append('---\n')

    # Sort alphabetically for the document
    sorted_menus = dict(sorted(menus.items(), key=lambda x: x[0].lower()))

    # ── table of contents ────────────────────────────────────────────────────
    out.append('## Contents\n')
    for name in sorted_menus:
        out.append(f'- [{name}](#{name.lower()})')
    out.append('')
    out.append('---\n')

    # ── one section per menu ─────────────────────────────────────────────────
    for name, items in sorted_menus.items():
        out.append(f'## {name}\n')
        out.append(menu_table(name, items, name_map, doc_index, menu_names))

    OUTPUT.write_text('\n'.join(out) + '\n')
    print(f'→ {OUTPUT.relative_to(ROOT)}  ({len(menus)} menus)', file=sys.stderr)


if __name__ == '__main__':
    main()
