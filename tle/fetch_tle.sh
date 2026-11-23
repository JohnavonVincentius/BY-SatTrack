#!/bin/sh
# Download fresh TLEs from CelesTrak (run on the BeagleY-AI or host).
# TLEs go stale: refresh at least every few days for LEO satellites.
#   ./fetch_tle.sh            -> amateur.txt, weather.txt, stations.txt
set -e
cd "$(dirname "$0")"
for g in amateur weather stations; do
  curl -fsS -o "$g.txt" "https://celestrak.org/NORAD/elements/gp.php?GROUP=$g&FORMAT=tle"
  echo "$g.txt: $(($(wc -l < "$g.txt") / 3)) satellites"
done
