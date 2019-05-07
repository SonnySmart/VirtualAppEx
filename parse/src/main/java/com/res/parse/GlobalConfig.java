package com.res.parse;

import android.content.SharedPreferences;

import org.json.JSONException;
import org.json.JSONObject;

/**
 * Created by beichen on 8/31 0031.
 */

public class GlobalConfig {
    public static String Inject_PackageName = "";                                   //待注入的包名
    public static String Inject_SoPath = "/data/data/com.res.parse/lib/libparse.so";//待注入的框架so的完整路径,此处不能在sd卡

    public static String Setting_FileName = "setting";
    public static String Setting_Key_pack_name = "pack_name";
    public static String Setting_Key_hook_name = "hook_name";
    public static String Setting_Key_dump_dll = "dump_dll";
    public static String Setting_Key_dump_lua = "dump_lua";
    public static String Setting_Key_dump_res = "dump_res";
    public static String Setting_Key_dump_res1 = "dump_res1";
    public static String Setting_Key_dump_xxtea = "dump_xxtea";

    public static String Current_PackageName = "com.res.parse";         //当前的包名,供Xposed读取配置用
    public static String Log_TAG = "TestInject";
    public static String Setting_SaveFileName = "myhook/config.json";                    //保存的文件名,共框架so读取配置


    public static SharedPreferences mSharePreferences;
    public static SharedPreferences.Editor editor;

    public static void init(SharedPreferences sharedPreferences)
    {
        mSharePreferences = sharedPreferences;
        editor = mSharePreferences.edit();
    }

    public static String getString(String key, String defValue)
    {
        return mSharePreferences.getString(key, defValue);
    }

    public static String getJson() throws JSONException {

        JSONObject object = new JSONObject();
        object.put(Setting_Key_pack_name, getString(Setting_Key_pack_name, ""));
        object.put(Setting_Key_hook_name, getString(Setting_Key_hook_name, ""));
        object.put(Setting_Key_dump_dll, getString(Setting_Key_dump_dll, ""));
        object.put(Setting_Key_dump_lua, getString(Setting_Key_dump_lua, ""));
        object.put(Setting_Key_dump_res, getString(Setting_Key_dump_res, ""));
        object.put(Setting_Key_dump_res1, getString(Setting_Key_dump_res1, ""));
        object.put(Setting_Key_dump_xxtea, getString(Setting_Key_dump_xxtea, ""));
        return object.toString();
    }

    public static void putString(String key, String value)
    {
        editor.putString(key, value);
    }

    public static void commit(String json) throws JSONException {
        if (json == null || json.isEmpty())
            return;

        JSONObject object = new JSONObject(json);
        putString(Setting_Key_pack_name, object.getString(Setting_Key_pack_name));
        putString(Setting_Key_hook_name, object.getString(Setting_Key_hook_name));
        putString(Setting_Key_dump_dll, object.getString(Setting_Key_dump_dll));
        putString(Setting_Key_dump_lua, object.getString(Setting_Key_dump_lua));
        putString(Setting_Key_dump_res, object.getString(Setting_Key_dump_res));
        putString(Setting_Key_dump_res1, object.getString(Setting_Key_dump_res1));
        putString(Setting_Key_dump_xxtea, object.getString(Setting_Key_dump_xxtea));
        editor.commit();
    }
}
