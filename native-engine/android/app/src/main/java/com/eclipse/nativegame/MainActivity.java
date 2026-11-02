package com.eclipse.nativegame;

import android.view.KeyEvent;
import org.libsdl.app.SDLActivity;

public final class MainActivity extends SDLActivity {
    @Override
    protected String[] getLibraries() {
        return new String[] { "SDL3", "SDL3_image", "main" };
    }

    // The game has no Android text fields. Route its physical-key controls to
    // SDL independently of which child view owns focus after fullscreen changes.
    @Override
    public boolean dispatchKeyEvent(KeyEvent event) {
        if ((event.getAction() == KeyEvent.ACTION_DOWN || event.getAction() == KeyEvent.ACTION_UP)
                && isGameKey(event.getKeyCode())) {
            return SDLActivity.handleKeyEvent(getWindow().getDecorView(), event.getKeyCode(), event, null);
        }
        return super.dispatchKeyEvent(event);
    }

    private static boolean isGameKey(int key) {
        switch (key) {
            case KeyEvent.KEYCODE_A: case KeyEvent.KEYCODE_D: case KeyEvent.KEYCODE_W:
            case KeyEvent.KEYCODE_J: case KeyEvent.KEYCODE_Z: case KeyEvent.KEYCODE_K:
            case KeyEvent.KEYCODE_X: case KeyEvent.KEYCODE_L: case KeyEvent.KEYCODE_C:
            case KeyEvent.KEYCODE_I: case KeyEvent.KEYCODE_R: case KeyEvent.KEYCODE_1:
            case KeyEvent.KEYCODE_2: case KeyEvent.KEYCODE_3: case KeyEvent.KEYCODE_SPACE:
            case KeyEvent.KEYCODE_TAB: case KeyEvent.KEYCODE_SHIFT_LEFT: case KeyEvent.KEYCODE_SHIFT_RIGHT:
            case KeyEvent.KEYCODE_DPAD_LEFT: case KeyEvent.KEYCODE_DPAD_RIGHT: case KeyEvent.KEYCODE_DPAD_UP:
                return true;
            default:
                return false;
        }
    }
}
