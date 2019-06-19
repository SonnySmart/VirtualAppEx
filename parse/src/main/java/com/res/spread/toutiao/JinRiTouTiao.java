package com.res.spread.toutiao;

import com.lody.whale.xposed.IXposedHookLoadPackage;
import com.lody.whale.xposed.callbacks.XC_LoadPackage;

public class JinRiTouTiao implements IXposedHookLoadPackage {
    @Override
    public void handleLoadPackage(XC_LoadPackage.LoadPackageParam lpparam) throws Throwable {
        if (!lpparam.packageName.equals("com.ss.android.article.news"))
            return;


    }
}
