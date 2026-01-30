from pathlib import Path
path = Path(r"c:\Personnel\TextualGameEngine\src\Histoire.h")
lines = path.read_text(encoding='utf-8').splitlines()
if '#include <iosfwd>' not in lines:
    for idx, line in enumerate(lines):
        if line.strip() == '#include <filesystem>':
            lines.insert(idx + 1, '#include <iosfwd>')
            break
    else:
        raise SystemExit('Failed to find <filesystem> include for insertion')
    path.write_text('\n'.join(lines) + '\n', encoding='utf-8')
