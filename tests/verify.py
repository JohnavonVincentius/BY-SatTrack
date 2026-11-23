"""Cross-check the C implementation against python-sgp4 and Skyfield.
usage: python3 tests/verify.py <teme_dump> <sat_test>   (pip install sgp4 skyfield)"""
import subprocess, sys, numpy as np
from sgp4.api import Satrec, WGS72
from skyfield.api import EarthSatellite, load, wgs84
teme, app = sys.argv[1], sys.argv[2]
lines = open("tle/sample.txt").read().splitlines()
tles = {lines[i]: (lines[i+1], lines[i+2]) for i in range(0, len(lines), 3)}
worst = 0
for name, sel in [("VANGUARD 1 (SGP4 verification case)", "00005"), ("ISS (ZARYA)", "25544"), ("NOAA 19", "33591")]:
    l1, l2 = tles[name]
    sat = Satrec.twoline2rv(l1, l2, WGS72)
    ts = [0, 360, 720, 1440, 2880, 4320] if sel == "00005" else [0, 30, 360, 1440, 4320, 10080]
    out = subprocess.run([teme, "tle/sample.txt", sel] + [str(t) for t in ts], capture_output=True, text=True).stdout.split("\n")
    for t, row in zip(ts, out):
        f = row.split()
        e, r, v = sat.sgp4_tsince(t)
        dr = np.linalg.norm(np.array(r) - np.array(list(map(float, f[2:5])))) * 1000
        worst = max(worst, dr)
        print(f"{name[:12]:12s} t={t:6g} min  |dr| = {dr:.6f} m")
print(f"TEME max position diff vs python-sgp4: {worst:.6f} m\n")

# az/el vs Skyfield
ts = load.timescale()
obs = wgs84.latlon(49.2781, -122.9199, elevation_m=370)
l1, l2 = tles["ISS (ZARYA)"]
es = EarthSatellite(l1, l2, "ISS", ts)
start = "2024-11-01T17:10:00Z"
rows = subprocess.run([app, "tle", "--tle", "tle/sample.txt", "--sat", "ISS", "--time", start,
                       "--count", "720", "--passes", "0"], capture_output=True, text=True).stdout.splitlines()
rows = [r for r in rows if r[:4] == "2024"]
daz = del_ = 0
for k, r in enumerate(rows):
    f = r.split()
    az, el = float(f[2]), float(f[3])
    t = ts.utc(2024, 11, 1, 17, 10, k)
    alt, azs, _ = (es - obs).at(t).altaz()
    if alt.degrees > 0:
        d = abs((az - azs.degrees + 180) % 360 - 180)
        daz = max(daz, d * np.cos(np.radians(el))); del_ = max(del_, abs(el - alt.degrees))
print(f"Az/El vs Skyfield over 12 min ISS pass: max |dEl| = {del_:.4f} deg, max |dAz|*cos(el) = {daz:.4f} deg")
