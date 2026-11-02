#!/usr/bin/env python3
"""Fetch the same pinned SDL release used by CMake, for its official Java bridge."""
from pathlib import Path
import subprocess

root = Path(__file__).resolve().parents[1]
source = root / 'android/vendor/SDL'
release = 'release-3.2.22'
if not source.exists():
    source.parent.mkdir(parents=True, exist_ok=True)
    subprocess.run(['git', 'clone', '--depth', '1', '--branch', release,
                    'https://github.com/libsdl-org/SDL.git', str(source)], check=True)
actual = subprocess.check_output(['git', '-C', str(source), 'describe', '--tags', '--exact-match'], text=True).strip()
if actual != release:
    raise SystemExit('Expected SDL ' + release + ', got ' + actual)
if not (source / 'android-project/app/src/main/java/org/libsdl/app/SDLActivity.java').is_file():
    raise SystemExit('SDL Java bridge missing')
print('Ready: official SDL Android Java/C source ' + release)
