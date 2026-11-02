package com.eclipse.nativegame;

import android.view.KeyEvent;
import org.libsdl.app.SDLActivity;

public final class MainActivity extends SDLActivity {
    private static native boolean nativeOnGameKey(int keyCode, boolean down);

    @Override
    protected String[] getLibraries() {
        return new String[] { "SDL3", "SDL3_image", "main" };
    }

    // The game owns scancode state and press edges. Queue its controls directly
    // rather than depending on SDL's virtual-keyboard/native-focus translation.
    // Unmapped Android system keys retain SDL/Activity's normal handling.
    @Override
    public boolean dispatchKeyEvent(KeyEvent event) {
        if (!mBrokenLibraries
                && (event.getAction() == KeyEvent.ACTION_DOWN || event.getAction() == KeyEvent.ACTION_UP)
                && nativeOnGameKey(event.getKeyCode(), event.getAction() == KeyEvent.ACTION_DOWN)) {
            return true;
        }
        return super.dispatchKeyEvent(event);
    }
}
