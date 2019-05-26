package com.res.spread;

import android.content.Context;

public abstract class ISpreadHook {

    public final static String TAG = "myhook";

    public abstract void execute(String processName, Context context);
}
