import re
from pathlib import Path

DATE = '2026-01-18'
AUTHOR = 'YALCIN Yalcin'

# Matches common C++ method definitions like:
# return_type ClassName::MethodName(args) [const] {
FUNC_RE = re.compile(
    r'^(?P<indent>\s*)'
    r'(?P<rettype>(?:[\w:\<\>\~\*&]+\s+)+)'
    r'(?P<class>[A-Za-z_][\w:]*)::'
    r'(?P<func>[A-Za-z_][\w:]*)\s*'
    r'\((?P<args>[^;]*?)\)\s*'
    r'(?P<const>const\s*)?'
    r'(?P<trailing>(?:noexcept\s*)?)'
    r'(?P<brace>\{)\s*$'
)

def has_file_header(text: str) -> bool:
    head = text.lstrip()[:400]
    return 'Author:' in head and 'Date:' in head

def make_file_header(relpath: str) -> str:
    return (
        '/*\n'
        f'  File: {relpath}\n'
        '  Project: RealityWorld (Win32++)\n'
        f'  Author: {AUTHOR}\n'
        f'  Date: {DATE}\n'
        '\n'
        '  Notes:\n'
        '  - Bu dosya okunabilirlik icin duzenlenmistir (yorum basliklari, bolum ayiraclari).\n'
        '  - Davranis degistirecek refaktorlerden kacinilmistir.\n'
        '*/\n\n'
    )

def make_func_header(cls: str, func: str) -> str:
    return (
        '    // --------------------------------------------------------------------\n'
        f'    // {AUTHOR} | {DATE}\n'
        f'    // Function: {cls}::{func}\n'
        '    // Purpose : Bu fonksiyonun amaci asagidaki implementasyonda gorulur.\n'
        '    //           (Okunabilirlik icin standart yorum blogu eklendi.)\n'
        '    // --------------------------------------------------------------------\n'
    )

def process_cpp(text: str) -> str:
    out_lines = []
    lines = text.splitlines(True)
    i = 0
    while i < len(lines):
        line = lines[i]
        m = FUNC_RE.match(line)
        if m:
            # Avoid inserting inside already-commented blocks (simple heuristic: previous non-empty line is a header separator)
            prev_nonempty = None
            j = len(out_lines) - 1
            while j >= 0:
                if out_lines[j].strip() != '':
                    prev_nonempty = out_lines[j]
                    break
                j -= 1
            if prev_nonempty and ('Function:' in prev_nonempty or prev_nonempty.strip().startswith('// --------------------------------------------------------------------')):
                out_lines.append(line)
            else:
                indent = m.group('indent')
                cls = m.group('class')
                func = m.group('func')
                header = make_func_header(cls, func)
                header = ''.join(indent + h if h.strip() else h for h in header.splitlines(True))
                out_lines.append(header)
                out_lines.append(line)
        else:
            out_lines.append(line)
        i += 1
    return ''.join(out_lines)

def process_file(path: Path):
    rel = path.relative_to(root)
    text = path.read_text(encoding='utf-8', errors='ignore')

    changed = False

    if not has_file_header(text):
        text = make_file_header(str(rel).replace('\\', '/')) + text
        changed = True

    if path.suffix.lower() == '.cpp':
        new_text = process_cpp(text)
        if new_text != text:
            text = new_text
            changed = True

    if changed:
        path.write_text(text, encoding='utf-8')

root = Path(__file__).resolve().parent

# Target all .cpp/.h in the project root (recursive)
files = [p for p in root.rglob('*') if p.suffix.lower() in ('.cpp', '.h')]

for p in files:
    # skip third-party / generated / backup if any
    name = p.name.lower()
    if name.startswith('wxx_'):
        # Win32++ headers: do not modify vendor headers
        continue
    process_file(p)

print(f'Updated {len(files)} files (vendor headers skipped).')
