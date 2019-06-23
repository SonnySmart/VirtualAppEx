package com.res.parse;

import android.content.SharedPreferences;
import android.os.Environment;
import android.util.Log;
import android.widget.Toast;

import org.json.JSONException;
import org.json.JSONObject;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;

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
    public static String Setting_Key_dump_res2 = "dump_res2";
    public static String Setting_Key_dump_xxtea = "dump_xxtea";
    public static String Setting_Key_send_msg = "send_msg";

    public static String Current_PackageName = "com.res.parse";         //当前的包名,供Xposed读取配置用
    public static String Log_TAG = "TestInject";
    public static String Setting_SaveFileName = "myhook/config.json";                    //保存的文件名,共框架so读取配置


    public static SharedPreferences mSharePreferences;
    public static SharedPreferences.Editor editor;

    public static boolean init(SharedPreferences sharedPreferences)
    {
        if (sharedPreferences == null)
            return false;

        if (sharedPreferences.equals(mSharePreferences))
            return false;

        mSharePreferences = sharedPreferences;
        editor = mSharePreferences.edit();

        return true;
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
        object.put(Setting_Key_dump_res2, getString(Setting_Key_dump_res2, ""));
        object.put(Setting_Key_dump_xxtea, getString(Setting_Key_dump_xxtea, ""));
        object.put(Setting_Key_send_msg, getString(Setting_Key_send_msg, ""));
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
        putString(Setting_Key_dump_res2, object.getString(Setting_Key_dump_res2));
        putString(Setting_Key_dump_xxtea, object.getString(Setting_Key_dump_xxtea));
        putString(Setting_Key_send_msg, object.getString(Setting_Key_send_msg));
        editor.commit();
    }

    public static boolean readConfigFile()
    {
        String sdStatus = Environment.getExternalStorageState();
        if (!sdStatus.equals(Environment.MEDIA_MOUNTED)) {
            Log.e(GlobalConfig.Log_TAG, "SD card is not avaiable/writeable right now.");
            return false;
        }

        try {
            String absoultPath = Environment.getExternalStorageDirectory() + File.separator + GlobalConfig.Setting_SaveFileName;
            File file = new File(absoultPath);
            if (!file.exists()) {
                return false;
            }

            String json = FileUtils.readFile(absoultPath);
            //Log.d("myhook", json);

            GlobalConfig.commit(json);
        }
        catch (IOException e) {
            e.printStackTrace();
            return false;
        } catch (JSONException e) {
            e.printStackTrace();
            return false;
        }

        return true;
    }

    /** 向 /sdcard/my_hookso.txt写入配置供注入的框架so读取
     *  第一行: 要注入的包名
     *  第二行: 要挂钩(Hook)的so名字,这里加载应用默认的库不需要完整路径,如果该应用采用其它方式加载则需要完整路径
     *  第三行: 实现实际hook功能的so,这个so里面包含了和框架so约定的导出函数
     */
    public static void writeSaveFile() {
        String sdStatus = Environment.getExternalStorageState();
        if (!sdStatus.equals(Environment.MEDIA_MOUNTED)) {
            Log.e(GlobalConfig.Log_TAG, "SD card is not avaiable/writeable right now.");
            return;
        }
        try {
            String absoultPath = Environment.getExternalStorageDirectory() + File.separator + GlobalConfig.Setting_SaveFileName;
            File file = new File(absoultPath);
            if (!file.getParentFile().isDirectory()) {
                file.getParentFile().mkdir();
            }
            if (file.exists()) {
                file.delete();
            }
            Log.e(GlobalConfig.Log_TAG, "Create the file : " + absoultPath);
            file.createNewFile();
            FileOutputStream out = new FileOutputStream(file);
            String json = null;
            try {
                json = GlobalConfig.getJson();
                Log.d("myhook", json);
            } catch (JSONException e) {
                e.printStackTrace();
                return;
            }
            byte[] buf = json.getBytes();
            out.write(buf);
            //close
            out.close();
        } catch (IOException e) {
            e.printStackTrace();
            Log.e(GlobalConfig.Log_TAG, "write file failed!");
        }
    }

    public static String getSendMessage() {
        return getString(Setting_Key_send_msg, "");
    }
}
