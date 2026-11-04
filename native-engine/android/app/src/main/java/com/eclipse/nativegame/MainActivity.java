package com.eclipse.nativegame;

import android.graphics.Color;
import android.os.Bundle;
import android.os.CancellationSignal;
import android.view.Gravity;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.FrameLayout;
import android.widget.Toast;

import androidx.credentials.CredentialManager;
import androidx.credentials.CredentialManagerCallback;
import androidx.credentials.CustomCredential;
import androidx.credentials.GetCredentialRequest;
import androidx.credentials.GetCredentialResponse;
import androidx.credentials.exceptions.GetCredentialException;

import com.google.android.libraries.identity.googleid.GetSignInWithGoogleOption;
import com.google.android.libraries.identity.googleid.GoogleIdTokenCredential;

import org.json.JSONObject;
import org.libsdl.app.SDLActivity;

import java.io.ByteArrayOutputStream;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.HttpURLConnection;
import java.net.URL;
import java.nio.charset.StandardCharsets;
import java.security.MessageDigest;
import java.security.SecureRandom;
import java.util.UUID;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

/**
 * Native VACODE sign-in overlay for the SDL3 game.
 *
 * Google credentials are verified by VACODE's existing Supabase Auth; a Google account hint
 * or an unverified ID token never counts as an authenticated ECLIPSE user.
 * This initial bridge keeps the resulting session in memory only. Cloud save is NOT enabled.
 */
public final class MainActivity extends SDLActivity {
    private final ExecutorService authExecutor = Executors.newSingleThreadExecutor();
    private Button accountButton;
    private volatile boolean authenticating;
    private volatile String signedInUserId;
    private volatile String accessToken;
    private volatile String refreshToken;

    @Override
    protected String[] getLibraries() {
        return new String[] { "SDL3", "SDL3_image", "main" };
    }

    @Override
    protected void onCreate(Bundle state) {
        super.onCreate(state);
        // SDL continues to own the game view. This small native control is temporary until
        // the title-screen UI can dispatch an explicit authentication request.
        accountButton = new Button(this);
        accountButton.setText("VACODE 로그인");
        accountButton.setAllCaps(false);
        accountButton.setTextColor(Color.WHITE);
        accountButton.setBackgroundTintList(android.content.res.ColorStateList.valueOf(0xBB252637));
        FrameLayout.LayoutParams params = new FrameLayout.LayoutParams(
            ViewGroup.LayoutParams.WRAP_CONTENT, ViewGroup.LayoutParams.WRAP_CONTENT,
            Gravity.TOP | Gravity.END);
        params.topMargin = Math.round(6 * getResources().getDisplayMetrics().density);
        addContentView(accountButton, params);
        accountButton.setOnClickListener(v -> {
            if (authenticating) return;
            if (signedInUserId != null) {
                // Explicit sign-out: no credential or token is retained by this bridge.
                signedInUserId = null;
                accessToken = null;
                refreshToken = null;
                updateButton();
                Toast.makeText(this, "VACODE 로그아웃", Toast.LENGTH_SHORT).show();
            } else {
                signInWithVacode();
            }
        });
    }

    private void updateButton() {
        runOnUiThread(() -> {
            if (accountButton == null) return;
            accountButton.setEnabled(!authenticating);
            accountButton.setText(authenticating ? "VACODE 인증 중…" :
                signedInUserId == null ? "VACODE 로그인" : "VACODE 연결됨 · 로그아웃");
        });
    }

    private void notifyUser(String message) {
        runOnUiThread(() -> Toast.makeText(this, message, Toast.LENGTH_LONG).show());
    }

    private void signInWithVacode() {
        if (authenticating) return;
        authenticating = true;
        updateButton();
        try {
            byte[] nonceBytes = new byte[32];
            new SecureRandom().nextBytes(nonceBytes);
            String rawNonce = hex(nonceBytes);
            String hashedNonce = hex(MessageDigest.getInstance("SHA-256")
                .digest(rawNonce.getBytes(StandardCharsets.UTF_8)));
            GetSignInWithGoogleOption option = new GetSignInWithGoogleOption.Builder(
                BuildConfig.VACODE_GOOGLE_WEB_CLIENT_ID).setNonce(hashedNonce).build();
            GetCredentialRequest request = new GetCredentialRequest.Builder()
                .addCredentialOption(option).build();

            CredentialManager.create(this).getCredentialAsync(
                this, request, new CancellationSignal(), getMainExecutor(),
                new CredentialManagerCallback<GetCredentialResponse, GetCredentialException>() {
                    @Override
                    public void onResult(GetCredentialResponse result) {
                        try {
                            if (!(result.getCredential() instanceof CustomCredential)) {
                                throw new IllegalStateException("Unsupported credential");
                            }
                            CustomCredential credential = (CustomCredential) result.getCredential();
                            if (!GoogleIdTokenCredential.TYPE_GOOGLE_ID_TOKEN_CREDENTIAL
                                    .equals(credential.getType())) {
                                throw new IllegalStateException("Unexpected credential type");
                            }
                            String idToken = GoogleIdTokenCredential.createFrom(
                                credential.getData()).getIdToken();
                            authExecutor.execute(() -> exchangeWithVacode(idToken, rawNonce));
                        } catch (Exception ignored) {
                            authFailed("Google 계정 인증 실패");
                        }
                    }

                    @Override
                    public void onError(GetCredentialException error) {
                        authFailed("Google 로그인 취소 또는 사용할 수 없음");
                    }
                });
        } catch (Exception ignored) {
            authFailed("VACODE 로그인 시작 실패");
        }
    }

    private void exchangeWithVacode(String idToken, String rawNonce) {
        HttpURLConnection connection = null;
        try {
            // Same public Supabase Auth instance as VACODE/NOCT. Never bundle service_role keys.
            URL url = new URL(BuildConfig.VACODE_SUPABASE_URL
                + "/auth/v1/token?grant_type=id_token");
            connection = (HttpURLConnection) url.openConnection();
            connection.setRequestMethod("POST");
            connection.setConnectTimeout(10000);
            connection.setReadTimeout(10000);
            connection.setDoOutput(true);
            connection.setRequestProperty("Content-Type", "application/json");
            connection.setRequestProperty("apikey", BuildConfig.VACODE_PUBLISHABLE_KEY);
            JSONObject payload = new JSONObject()
                .put("provider", "google")
                .put("id_token", idToken)
                .put("nonce", rawNonce);
            byte[] data = payload.toString().getBytes(StandardCharsets.UTF_8);
            try (OutputStream output = connection.getOutputStream()) { output.write(data); }
            int status = connection.getResponseCode();
            if (status < 200 || status >= 300) {
                // Never display auth response bodies: they may contain sensitive data.
                throw new IllegalStateException("VACODE HTTP " + status);
            }
            try (InputStream stream = connection.getInputStream()) {
                JSONObject response = new JSONObject(readLimited(stream, 65536));
                String uid = response.getJSONObject("user").getString("id");
                UUID.fromString(uid); // Reject invalid or missing user identity.
                String newAccess = response.getString("access_token");
                String newRefresh = response.getString("refresh_token");
                if (newAccess.isEmpty() || newRefresh.isEmpty()) {
                    throw new IllegalStateException("Missing session");
                }
                accessToken = newAccess;
                refreshToken = newRefresh;
                signedInUserId = uid;
            }
            authenticating = false;
            updateButton();
            notifyUser("VACODE 계정 연결 완료");
        } catch (Exception ignored) {
            authFailed("VACODE 인증 서버 연결 또는 검증 실패");
        } finally {
            if (connection != null) connection.disconnect();
        }
    }

    private static String readLimited(InputStream input, int maximum) throws Exception {
        ByteArrayOutputStream out = new ByteArrayOutputStream();
        byte[] buffer = new byte[4096];
        int n;
        while ((n = input.read(buffer)) != -1) {
            if (out.size() + n > maximum) throw new IllegalStateException("Response too large");
            out.write(buffer, 0, n);
        }
        return out.toString(StandardCharsets.UTF_8.name());
    }

    private static String hex(byte[] bytes) {
        StringBuilder output = new StringBuilder(bytes.length * 2);
        for (byte b : bytes) output.append(String.format(java.util.Locale.ROOT, "%02x", b & 255));
        return output.toString();
    }

    private void authFailed(String message) {
        authenticating = false;
        updateButton();
        notifyUser(message);
    }

    @Override
    protected void onDestroy() {
        // Credentials never persist across application restarts in this first integration.
        signedInUserId = null;
        accessToken = null;
        refreshToken = null;
        authExecutor.shutdownNow();
        super.onDestroy();
    }
}
