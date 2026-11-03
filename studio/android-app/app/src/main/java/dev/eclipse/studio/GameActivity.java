package dev.eclipse.studio;

import org.libsdl.app.SDLActivity;

/** Runs the exact native-engine C++ SDL3 application inside the Studio package. */
public final class GameActivity extends SDLActivity {
    @Override
    protected String[] getLibraries() {
        return new String[] { "SDL3", "SDL3_image", "main" };
    }
}
