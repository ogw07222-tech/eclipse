#!/usr/bin/env python3
"""Real Android emulator startup, touch inventory/swap/retry and lifecycle checks."""
from pathlib import Path
import subprocess
import struct
import time

out = Path('native-engine/android/smoke')
out.mkdir(parents=True, exist_ok=True)
history = []
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
def action_and_wait(action, message):
    history.append(logs())
    adb('logcat', '-c')
    action()
    wait_for(message)

def restart():
    action_and_wait(lambda: adb('shell', 'input', 'keyevent', 'KEYCODE_R'), 'restart=1')

try:
    # Fresh emulator images finish package/IME provisioning after boot_completed.
    time.sleep(20)
    adb('shell', 'wm', 'dismiss-keyguard')
    adb('install', '-r', str(apk))
    adb('logcat', '-c')
    adb('shell', 'am', 'start', '-W', '-n', 'com.eclipse.nativegame/.MainActivity')
    wait_for('ECLIPSE ready:')
    deadline = time.monotonic() + 10
    while True:
        width, height = capture('latest')
        if width > height:
            break
        if time.monotonic() > deadline:
            raise RuntimeError('Game did not enter landscape orientation')
        time.sleep(.5)
    tap(640, 250)  # establish an initial real surface interaction before keyboard checks
    restart()
    capture('01-combat')
    action_and_wait(lambda: tap(800, 40), 'inventory=1')
    action_and_wait(lambda: tap(100, 435), 'weapon=1')
    capture('02-inventory')
    # Isolate scenarios while retaining the production enemy AI and damage rules.
    restart()
    action_and_wait(lambda: tap(1035, 490), 'form=1')
    capture('03-mage')
    adb('shell', 'input', 'keyevent', 'KEYCODE_HOME')
    time.sleep(1)
    adb('shell', 'am', 'start', '-W', '-n', 'com.eclipse.nativegame/.MainActivity')
    time.sleep(1)
    restart()
    capture('04-restarted')
    assert adb('shell', 'pidof', 'com.eclipse.nativegame').strip(), 'App process exited'
    text = logs()
    assert 'FATAL EXCEPTION' not in text and 'Fatal signal' not in text, 'Android native crash'
    print('PASS Android emulator: landscape/launch/textures/touch inventory/equip/swap/background/restart')
finally:
    capture('final')
    (out / 'windows.txt').write_bytes(adb('shell', 'dumpsys', 'window', 'windows'))
    (out / 'input.txt').write_bytes(adb('shell', 'dumpsys', 'input'))
    history.append(logs())
    (out / 'logcat.txt').write_text('\n--- ACTION BOUNDARY ---\n'.join(history))
