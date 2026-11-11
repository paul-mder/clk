import re
from pathlib import Path

html_path = Path(r"C:\esp\clk\main\web_ui.html")
cpp_path  = Path(r"C:\esp\clk\main\clock_machine.cpp")

html = html_path.read_text(encoding="utf-8-sig")

# clock_machine.cpp ist wahrscheinlich ANSI / Windows-1252, deswegen fallback
try:
    cpp = cpp_path.read_text(encoding="utf-8-sig")
except UnicodeDecodeError:
    cpp = cpp_path.read_text(encoding="cp1252")

pattern = r'static const char\*\s+HTML_PAGE\s*=\s*R"rawhtml\d*\(.*?\)rawhtml\d*";'

replacement = 'static const char* HTML_PAGE = R"rawhtml2(\n' + html + '\n)rawhtml2";'

new_cpp, count = re.subn(
    pattern,
    lambda m: replacement,
    cpp,
    flags=re.DOTALL
)

if count != 1:
    print("FEHLER: HTML_PAGE wurde nicht eindeutig gefunden. Gefunden:", count)
else:
    cpp_path.write_text(new_cpp, encoding="utf-8")
    print("Done, HTML ersetzt.")
