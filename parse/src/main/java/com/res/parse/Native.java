package com.res.parse;

import android.util.Log;

public class Native {

    public final static String TAG = Native.class.getSimpleName();

    static {
        System.loadLibrary("parse");
    }

    public static void init() {
        Log.d(TAG, "init: ");
    }
}
