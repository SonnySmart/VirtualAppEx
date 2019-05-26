package com.res.parse;

import android.content.Context;
import android.util.Log;

import com.res.spread.tieba.BaiduTieBa;
import com.res.spread.ISpreadHook;

import static android.content.Context.MODE_PRIVATE;

public class Native {

    //public final static String TAG = Native.class.getSimpleName();
    public final static String TAG = "myhook";

    static boolean IsReadConfig = false;

    static {
        //System.loadLibrary("parse");
    }

    // start hook
    public static void tryHook(String processName, Context context) {

        if (GlobalConfig.init(context.getSharedPreferences(GlobalConfig.Setting_FileName, MODE_PRIVATE))) {
            IsReadConfig = GlobalConfig.readConfigFile();
        }

        String process =  GlobalConfig.getString(GlobalConfig.Setting_Key_pack_name, "");

        //Log.d(TAG, "tryHook: processName:" + processName + "config process:" + process);
        if (!process.equals(processName))
            return;

        Log.d(TAG, "success . processName:" + processName);

        ISpreadHook iSpreadHook = new BaiduTieBa();
        iSpreadHook.execute(processName, context);
    }
}
