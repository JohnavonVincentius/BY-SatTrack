# sat_test: live satellite azimuth / elevation

A test app for the BeagleY-AI tracker. It prints where to point the antenna (azimuth, elevation) every second. You can use it in two ways:

| Mode | Who does the orbit math | Command |
|---|---|---|
| `tle` | This app (SGP4 in C, `sat/` library) | `sat_test tle --tle tle/amateur.txt --sat ISS` |
| `gpredict` | GPredict on your laptop; the board acts as a Hamlib `rotctld` rotator | `sat_test gpredict --port 4533` |

## Build

```sh
mkdir -p build && cd build && cmake .. && make sat_test
./sat_test/sat_test tle --tle ../tle/sample.txt --sat ISS
```

The `sat/` library needs only libc, libm and sockets, so it builds on the host PC as well as on the board, with no HAL needed. When you cross-compile, the post-build step copies `sat_test` and the TLE files to `beagley@192.168.7.2:~`.

## Mode 1: TLE (standalone)

```
sat_test tle --tle FILE [--sat NAME|NORAD_ID] [--lat 49.2781 --lon -122.9199 --alt 370]
             [--interval 1] [--count N] [--time 2024-11-01T17:10:00Z] [--passes 3]
```

* **Get fresh TLEs first:** run `tle/fetch_tle.sh` (CelesTrak amateur, weather and stations groups). `tle/sample.txt` only holds old sample elements for offline testing. The app warns you when a TLE is more than 7 days old.
* `--sat` takes a name substring (`ISS`, `"NOAA 19"`) or a NORAD number (`25544`).
* `--time` runs a simulated clock from that instant, which helps for testing a known pass indoors.
* On startup the app prints the next passes (AOS/TCA/LOS times and azimuths). It then prints a line every `--interval` seconds:
  `UTC  Az  El  Range(km)  RangeRate(km/s)  VISIBLE|below horizon`.
* **The clock must be right.** Each second of error moves a LEO satellite about 7 km along its track. Check `timedatectl` / NTP on the board.

## Mode 2: GPredict

1. On the board, run `./sat_test gpredict` (it listens on TCP 4533).
2. In GPredict, go to *Edit → Preferences → Interfaces → Rotators → Add*. Set the host to the board IP, the port to 4533, the Az type to 0°→360°, and the El range to 0–90.
3. Open a module, then go to *menu ▸ Antenna Control*. Pick the satellite and the rotator, then press **Track** and **Engage**.
4. The board prints each `P az el` target that GPredict sends. The position replies currently echo the target back. Replace this with encoder/ADC feedback (`rot.cur_az/cur_el`).

## Accuracy (checked in `tests/verify.py`)

* SGP4 position matches python-sgp4 (the Vallado reference) to within less than 1 µm, including the Vanguard-1 verification case.
* Az/El matches Skyfield to within 0.006° over a full ISS pass.
* Pass rise, peak and set times match Skyfield `find_events` to within 1 s.

Real-world error comes from TLE age (about 1–2 km per day for LEO), not from this code.

## Limits

* Near-Earth SGP4 only. Orbits with a period of 225 minutes or more (GEO, Molniya) are rejected with a clear error. That's fine for LEO trackers.
* Polar motion and UT1−UTC are ignored. Each contributes less than 0.01°, far below any antenna beamwidth.

## Hardware hook (next step)

Look for `TODO hardware` in `src/main.c`. That is where to call a `rotator_goto(az, el)` that uses `hal/pwm` and `hal/encoder` / `hal/eqep` / `hal/mcp300x` for feedback.
