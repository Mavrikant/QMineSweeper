#!/usr/bin/env bash
# Fetch the 10 flag icons used by the Settings → Language menu.
# Flags come from https://flagcdn.com (MIT, backed by lipis/flag-icons),
# rendered at 40px wide which is plenty for a menu row at @2x retina.
# Run this script whenever the locale set changes.

set -euo pipefail

cd "$(dirname "$0")"

# locale:country pairs — parallel arrays for portability.
pairs=(
    "en:us"
    "tr_TR:tr"
    "zh_CN:cn"
    "hi_IN:in"
    "es_ES:es"
    "ar_SA:sa"
    "fr_FR:fr"
    "ru_RU:ru"
    "pt_BR:br"
    "de_DE:de"
)

for pair in "${pairs[@]}"; do
    locale="${pair%%:*}"
    country="${pair##*:}"
    url="https://flagcdn.com/w40/${country}.png"
    out="${locale}.png"
    printf 'fetching %-8s  %s\n' "$out" "$url"
    curl -fsSL "$url" -o "$out"
done

echo "done — ${#pairs[@]} flag PNGs written to $(pwd)"
