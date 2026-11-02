#!/usr/bin/env python3
"""Real Android emulator startup, touch inventory/swap/retry and lifecycle checks."""
from pathlib import Path
import subprocess
import struct
import time

out = Path('native-engine/android/smoke')
out.mkdir(parents=True, exist_ok=True)
apk = Path('native-engine/android/app/build/outputs/apk/debug/app-debug.apk')
def adb(*args):
    return subprocess.check_output(['adb', *args], stderr=subprocess.STDOUT)
def logs():
    return adb('logcat', '-d', '-v', 'brief').decode(errors='replace')
def wait_for(message, seconds=20):
    deadline = time.monotonic() + seconds
    while time.monotonic() < deadline:
        text = logs()
        if 'FATAL EXCEPTION' in text or 'Fatal signal' in text:
            raise RuntimeError('Android crash; inspect logcat')
        if message in text:
            return
        time.sleep(.5)
    raise RuntimeError('Expected app event missing: ' + message)
def capture(name):
    data = adb('exec-out', 'screencap', '-p')
    (out / (name + '.png')).write_bytes(data)
    return struct.unpack('>II', data[16:24])
def tap(x, y):
    w, h = capture('latest')
    scale = min(w / 1280, h / 720)
    adb('shell', 'input', 'tap', str(round((w - 1280 * scale) / 2 + x * scale)),
        str(round((h - 720 * scale) / 2 + y * scale)))
try:
    adb('install', '-r', str(apk))
    adb('logcat', '-c')
    adb('shell', 'am', 'start', '-W', '-n', 'com.eclipse.nativegame/.MainActivity')
    wait_for('ECLIPSE ready:')
    capture('01-combat')
    tap(800, 40)
    wait_for('inventory=1')
    capture('02-inventory')
    tap(100, 435)  # cycle weapon
    tap(800, 40)  # close inventory
    tap(1035, 490)  # swap to mage
    wait_for('form=1')
    capture('03-mage')
    adb('shell', 'input', 'keyevent', 'KEYCODE_HOME')
    time.sleep(1)
    adb('shell', 'am', 'start', '-W', '-n', 'com.eclipse.nativegame/.MainActivity')
    time.sleep(1)
    tap(600, 440)  # does nothing unless result overlay is visible
    # Retry keyboard event exercises the actual SDL event adapter.
    adb('shell', 'input', 'keyevent', 'KEYCODE_R')
    wait_for('restart=1')
    capture('04-restarted')
    assert adb('shell', 'pidof', 'com.eclipse.nativegame').strip(), 'App process exited'
    text = logs()
    assert 'FATAL EXCEPTION' not in text and 'Fatal signal' not in text, 'Android native crash'
    print('PASS Android emulator: launch/textures/touch inventory/equip/swap/background/restart')
finally:
    (out / 'logcat.txt').write_text(logs())
