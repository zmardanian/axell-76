#!/usr/bin/env bash
# Linux release archive from the portable build (scripts/build-portable.sh):
#   ~/axell-76-release/Axell76-<version>-linux-x86_64{/,.tar.gz} + SHA256SUMS.txt
set -euo pipefail
cd "$(dirname "$0")/.."
VERSION=$(sed -n 's/^project(Axell76 VERSION \([0-9.]*\)).*/\1/p' CMakeLists.txt)
ART="build-portable/Axell76_artefacts/Release"
PKG="Axell76-$VERSION-linux-x86_64"; OUTROOT="$HOME/axell-76-release"; OUT="$OUTROOT/$PKG"
[ -d "$ART/VST3/Axell 76.vst3" ] && [ -d "$ART/LV2/Axell 76.lv2" ] && [ -x "$ART/Standalone/Axell 76" ] || { echo "Run scripts/build-portable.sh first."; exit 1; }
grep -q "\"Version\": \"$VERSION\"" "$ART/VST3/Axell 76.vst3/Contents/Resources/moduleinfo.json" || { echo "bundle version is not $VERSION"; exit 1; }
rm -rf "$OUT" "$OUTROOT/$PKG.tar.gz"; mkdir -p "$OUT/fonts"
cp -r "$ART/VST3/Axell 76.vst3" "$OUT/"
cp -r "$ART/LV2/Axell 76.lv2" "$OUT/"
cp "$ART/Standalone/Axell 76" "$OUT/Axell 76"
strip --strip-unneeded "$OUT/Axell 76.vst3/Contents/x86_64-linux/Axell 76.so" "$OUT/Axell 76.lv2/libAxell 76.so" "$OUT/Axell 76"
cp LICENSE "$OUT/"; cp Resources/fonts/OFL-*.txt "$OUT/fonts/"
cp docs/img/axell76.png "$OUT/axell76.png"

cat > "$OUT/install.sh" <<'EOS'
#!/usr/bin/env bash
# Installs Axell 76 for this user: the VST3 plugin into ~/.vst3 (every Linux DAW scans it) and the standalone app into
# ~/.local/bin with a menu entry. Nothing outside your home folder is touched; no root needed.
set -euo pipefail
cd "$(dirname "$0")"
[ "$(uname -m)" = "x86_64" ] || { echo "This build is for x86_64 Linux; this machine is $(uname -m)."; exit 1; }
mkdir -p "$HOME/.vst3" "$HOME/.lv2" "$HOME/.local/bin" "$HOME/.local/share/applications" "$HOME/.local/share/icons/hicolor/256x256/apps"
TMP=$(mktemp -d "$HOME/.vst3/.axell-76-install.XXXXXX"); trap 'rm -rf "$TMP"' EXIT
cp -r "Axell 76.vst3" "$TMP/"
rm -rf "$HOME/.vst3/Axell 76.vst3"; mv "$TMP/Axell 76.vst3" "$HOME/.vst3/"
rm -rf "$HOME/.lv2/Axell 76.lv2"; cp -r "Axell 76.lv2" "$HOME/.lv2/"
install -m 755 "Axell 76" "$HOME/.local/bin/axell-76"
cp axell76.png "$HOME/.local/share/icons/hicolor/256x256/apps/axell-76.png"
cat > "$HOME/.local/share/applications/axell-76.desktop" <<EOD
[Desktop Entry]
Type=Application
Name=Axell 76
Comment=FET limiting amplifier (standalone)
Exec=$HOME/.local/bin/axell-76
Icon=axell-76
Categories=AudioVideo;Audio;
EOD
missing=$(ldd "$HOME/.vst3/Axell 76.vst3/Contents/x86_64-linux/Axell 76.so" "$HOME/.lv2/Axell 76.lv2/libAxell 76.so" "$HOME/.local/bin/axell-76" | grep "not found" | sort -u || true)
if [ -n "$missing" ]; then
  echo "Installed, but these system libraries are missing (install them with your package manager):"; echo "$missing"; exit 1
fi
echo "Installed:"
echo "  VST3 plugin    ~/.vst3/Axell 76.vst3   (restart your DAW so it rescans)"
echo "  LV2 plugin     ~/.lv2/Axell 76.lv2"
echo "  Standalone     ~/.local/bin/axell-76    (also in your application menu as 'Axell 76')"
EOS
cat > "$OUT/uninstall.sh" <<'EOS'
#!/usr/bin/env bash
rm -rf "$HOME/.vst3/Axell 76.vst3" "$HOME/.lv2/Axell 76.lv2" "$HOME/.local/bin/axell-76" "$HOME/.local/share/applications/axell-76.desktop" "$HOME/.local/share/icons/hicolor/256x256/apps/axell-76.png"
echo "Axell 76 removed."
EOS
chmod +x "$OUT/install.sh" "$OUT/uninstall.sh" "$OUT/Axell 76"
cat > "$OUT/README.txt" <<EOR
Axell 76 $VERSION - FET limiting amplifier for Linux (x86_64)

Install:    ./install.sh        (no root; VST3 into ~/.vst3, LV2 into ~/.lv2, the standalone app into your menu)
Run now:    ./"Axell 76"        (standalone: pick your audio device under Options)
Uninstall:  ./uninstall.sh

Runs on any x86_64 Linux with glibc 2.30 or newer (Ubuntu 20.04+, Debian 11+, Fedora 32+, Arch, Mint 20+, ...).
Needs freetype and fontconfig (present on every desktop), and ALSA for the standalone app.
Panel: SILVER / BLACK switch at the bottom left; click the meter for GR / VU +4 / VU +8; presets in the name band.

License: GNU AGPLv3 (LICENSE). Nameplate font: Great Vibes, SIL Open Font License (fonts/).
1176 and UADx are trademarks of Universal Audio, Inc.; Axell 76 is independent and not affiliated.
EOR
(cd "$OUTROOT" && tar czf "$PKG.tar.gz" "$PKG" && sha256sum "$PKG.tar.gz" > "$PKG.SHA256SUMS.txt")
echo "Release: $OUTROOT/$PKG.tar.gz"; cat "$OUTROOT/$PKG.SHA256SUMS.txt"; du -h "$OUTROOT/$PKG.tar.gz" | cut -f1
