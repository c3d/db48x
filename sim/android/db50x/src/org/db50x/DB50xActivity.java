package org.db50x;

import android.os.Bundle;
import android.util.Log;
import android.window.OnBackInvokedDispatcher;
import org.qtproject.qt.android.bindings.QtActivity;

public class DB50xActivity extends QtActivity {
    private native void nativeKeyPush(int key);

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        if (android.os.Build.VERSION.SDK_INT >= 33) {
            getOnBackInvokedDispatcher().registerOnBackInvokedCallback(
                OnBackInvokedDispatcher.PRIORITY_DEFAULT,
                () -> {
                    Log.d("db50x", "OnBackInvokedCallback called");
                    nativeKeyPush(17); // KB_BKS = backspace
                }
            );
        }
    }

    @Override
    @SuppressWarnings("deprecation")
    public void onBackPressed() {
        Log.d("db50x", "onBackPressed called");
        nativeKeyPush(17); // KB_BKS = backspace, fallback for Android < 13
    }
}
