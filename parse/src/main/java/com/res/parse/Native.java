package com.res.parse;

import android.util.Log;

import com.lody.whale.xposed.IXposedHookLoadPackage;
import com.lody.whale.xposed.callbacks.XC_LoadPackage;
import com.res.spread.tieba.BaiduTieBa;
import com.res.spread.toutiao.JinRiTouTiao;

import static android.content.Context.MODE_PRIVATE;

public class Native implements IXposedHookLoadPackage {

    //public final static String TAG = Native.class.getSimpleName();
    public final static String TAG = "myhook";

    static boolean IsReadConfig = false;

    static {
        //System.loadLibrary("parse");
    }

    @Override
    public void handleLoadPackage(XC_LoadPackage.LoadPackageParam lpparam) throws Throwable {

        //Log.d(TAG, "handleLoadPackage: " + lpparam.processName);

        if (GlobalConfig.init(lpparam.context.getSharedPreferences(GlobalConfig.Setting_FileName, MODE_PRIVATE))) {
            IsReadConfig = GlobalConfig.readConfigFile();
        }

        String process =  GlobalConfig.getString(GlobalConfig.Setting_Key_pack_name, "");

        //Log.d(TAG, "tryHook: processName:" + processName + "config process:" + process);
        if (!process.equals(lpparam.processName))
            return;

        System.loadLibrary("parse");

        Log.d(TAG, "inject success . processName:" + lpparam.processName);

        new BaiduTieBa().handleLoadPackage(lpparam);
        new JinRiTouTiao().handleLoadPackage(lpparam);
    }
}
