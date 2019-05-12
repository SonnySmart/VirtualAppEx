package com.res.parse;

import android.Manifest;
import android.app.Activity;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.pm.PackageManager;
import android.net.Uri;
import android.os.Bundle;
import android.os.Environment;
import android.util.Log;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.EditText;
import android.widget.Spinner;
import android.widget.TextView;
import android.widget.Toast;

import com.res.app.FileChooseUtil;

import org.json.JSONException;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.util.ArrayList;

import static com.res.parse.GlobalConfig.editor;

public class MainActivity extends Activity {
    private EditText et_inject_pckname, et_hook_soname;
    private Button bt_inject_pckname, btn_save, btn_start;
    private Spinner spinner_hook_soname;
    private CheckBox check_dump_dll;
    private CheckBox check_dump_lua;
    private CheckBox check_dump_res;
    private CheckBox check_dump_res1;
    private CheckBox check_dump_res2;
    private CheckBox check_dump_xxtea;
    private TextView tv_help;

    static final int REQUEST_CHOOSEFILE = 1;
    private static final int REQUEST_EXTERNAL_STORAGE = 1;
    private static String[] PERMISSIONS_STORAGE = {
            Manifest.permission.READ_EXTERNAL_STORAGE,
            Manifest.permission.WRITE_EXTERNAL_STORAGE
    };

    public static final String PACKAGE_NAME_CMD = "PACKAGE_NAME_CMD";

    static ArrayList<String> so_list = new ArrayList<String>();

    static
    {
        so_list.add("libmono.so");
        so_list.add("libqpry_lua.so");
        so_list.add("libcocos2dlua.so");
        so_list.add("libgame.so");
    }

    private BroadcastReceiver mBroadcastReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            if (action.equals(PACKAGE_NAME_CMD))
            {
                String packageString = intent.getStringExtra(PACKAGE_NAME_CMD);
                et_inject_pckname.setText(packageString);

                ArrayList<String> ret = findAppLibs(packageString, new ArrayList<String>());
                for (String lib : ret)
                {
                    for (String s : so_list)
                    {
                        if (lib.contains(s))
                        {
                            et_hook_soname.setText(s);
                            break;
                        }
                    }
                }
//                if (ret.size() > 1)
//                    initSpinnerHookSoName(ret);
            }
        }
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        //verifyStoragePermissions(this);

        setContentView(R.layout.activity_main);
        et_inject_pckname = findViewById(R.id.et_inject_pckname);
        et_hook_soname = findViewById(R.id.et_hook_soname);
        bt_inject_pckname = findViewById(R.id.bt_inject_pckname);
        spinner_hook_soname = findViewById(R.id.spinner_hook_soname);
        check_dump_dll = findViewById(R.id.check_dump_dll);
        check_dump_lua = findViewById(R.id.check_dump_lua);
        check_dump_res = findViewById(R.id.check_dump_res);
        check_dump_res1 = findViewById(R.id.check_dump_res1);
        check_dump_res2 = findViewById(R.id.check_dump_res2);
        check_dump_xxtea = findViewById(R.id.check_dump_xxtea);
        tv_help = findViewById(R.id.tv_help);
        btn_save = findViewById(R.id.btn_save);
        btn_start = findViewById(R.id.btn_start);
        GlobalConfig.init(getSharedPreferences(GlobalConfig.Setting_FileName, MODE_PRIVATE));
        bt_inject_pckname.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                onClickInjectPackage();
            }
        });
        btn_save.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                onClickSave();
            }
        });
        btn_start.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                onClickStart();
            }
        });

        initSpinnerHookSoName();

        tv_help.setText("帮助:\n" + "/sdcard/myhook/tmp为加密资源存放路径\n");

        IntentFilter intentFilter = new IntentFilter();
        intentFilter.addAction(PACKAGE_NAME_CMD);
        registerReceiver(mBroadcastReceiver, intentFilter);

        //读取配置文件
        init();
        readConfigFile();
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();

        unregisterReceiver(mBroadcastReceiver);
    }

//    public static void verifyStoragePermissions(Activity activity) {
//        // Check if we have write permission
//        int permission = ActivityCompat.checkSelfPermission(activity, Manifest.permission.WRITE_EXTERNAL_STORAGE);
//        if (permission != PackageManager.PERMISSION_GRANTED) {
//        // We don't have permission so prompt the user
//            ActivityCompat.requestPermissions(
//                    activity,
//                    PERMISSIONS_STORAGE,
//                    REQUEST_EXTERNAL_STORAGE
//            );
//        }
//    }

    void init()
    {
        et_inject_pckname.setText(GlobalConfig.getString(GlobalConfig.Setting_Key_pack_name, ""));
        et_hook_soname.setText(GlobalConfig.getString(GlobalConfig.Setting_Key_hook_name, ""));
        check_dump_dll.setChecked(GlobalConfig.getString(GlobalConfig.Setting_Key_dump_dll, "").equals("1"));
        check_dump_lua.setChecked(GlobalConfig.getString(GlobalConfig.Setting_Key_dump_lua, "").equals("1"));
        check_dump_res.setChecked(GlobalConfig.getString(GlobalConfig.Setting_Key_dump_res, "").equals("1"));
        check_dump_res1.setChecked(GlobalConfig.getString(GlobalConfig.Setting_Key_dump_res1, "").equals("1"));
        check_dump_res2.setChecked(GlobalConfig.getString(GlobalConfig.Setting_Key_dump_res2, "").equals("1"));
        check_dump_xxtea.setChecked(GlobalConfig.getString(GlobalConfig.Setting_Key_dump_xxtea, "").equals("1"));
    }

    void initSpinnerHookSoName()
    {
//        String packString = et_inject_pckname.getText().toString();
//        if (packString.isEmpty())
//            initSpinnerHookSoName(new ArrayList<String>(so_list));
//        else
//            initSpinnerHookSoName(findAppLibs(packString, new ArrayList<String>()));

        initSpinnerHookSoName(new ArrayList<String>(so_list));
    }

    void initSpinnerHookSoName(ArrayList<String> data_list)
    {
        data_list.add(0, "*.so");

        //适配器
        ArrayAdapter<String> arr_adapter= new ArrayAdapter<String>(this, android.R.layout.simple_spinner_item, data_list);
        //设置样式
        arr_adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        //加载适配器

        spinner_hook_soname.setAdapter(arr_adapter);
        spinner_hook_soname.setSelection(0, true);
        spinner_hook_soname.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
            @Override
            public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {
                if (position == 0)
                    return;
                String sInfo = spinner_hook_soname.getItemAtPosition(position).toString();
                et_hook_soname.setText(sInfo);
            }

            @Override
            public void onNothingSelected(AdapterView<?> parent) {
            }
        });
    }

    void onClickInjectPackage()
    {
        Intent intent = new Intent(this, AppActivity.class);
        startActivity(intent);
    }

    void onClickSave()
    {
        if (!et_inject_pckname.getText().toString().equals("")) {
            GlobalConfig.putString(GlobalConfig.Setting_Key_pack_name, et_inject_pckname.getText().toString());
            GlobalConfig.putString(GlobalConfig.Setting_Key_hook_name, et_hook_soname.getText().toString());
            GlobalConfig.putString(GlobalConfig.Setting_Key_dump_dll, check_dump_dll.isChecked() ? "1" : "0");
            GlobalConfig.putString(GlobalConfig.Setting_Key_dump_lua, check_dump_lua.isChecked() ? "1" : "0");
            GlobalConfig.putString(GlobalConfig.Setting_Key_dump_res, check_dump_res.isChecked() ? "1" : "0");
            GlobalConfig.putString(GlobalConfig.Setting_Key_dump_res1, check_dump_res1.isChecked() ? "1" : "0");
            GlobalConfig.putString(GlobalConfig.Setting_Key_dump_res2, check_dump_res2.isChecked() ? "1" : "0");
            GlobalConfig.putString(GlobalConfig.Setting_Key_dump_xxtea, check_dump_xxtea.isChecked() ? "1" : "0");
            editor.commit();
            writeSaveFile();
            Toast.makeText(MainActivity.this, "保存设置成功", Toast.LENGTH_SHORT).show();
        } else {
            Toast.makeText(MainActivity.this, "请完整输入路径和包名", Toast.LENGTH_SHORT).show();
        }
    }

    void onClickStart()
    {
        PackageManager packageManager = getPackageManager();
        Intent intent= new Intent();
        intent = packageManager.getLaunchIntentForPackage(et_inject_pckname.getText().toString());
        if(intent == null) {
            Toast.makeText(MainActivity.this, "未安装", Toast.LENGTH_LONG).show();
        }else{
            intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
            startActivity(intent);
        }
    }

/** 向 /sdcard/my_hookso.txt写入配置供注入的框架so读取
 *  第一行: 要注入的包名
 *  第二行: 要挂钩(Hook)的so名字,这里加载应用默认的库不需要完整路径,如果该应用采用其它方式加载则需要完整路径
 *  第三行: 实现实际hook功能的so,这个so里面包含了和框架so约定的导出函数
 */
    void writeSaveFile() {
        String sdStatus = Environment.getExternalStorageState();
        if (!sdStatus.equals(Environment.MEDIA_MOUNTED)) {
            Log.e(GlobalConfig.Log_TAG, "SD card is not avaiable/writeable right now.");
            Toast.makeText(MainActivity.this, "SD card is not avaiable/writeable right now", Toast.LENGTH_SHORT);
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

    void readConfigFile()
    {
        String sdStatus = Environment.getExternalStorageState();
        if (!sdStatus.equals(Environment.MEDIA_MOUNTED)) {
            Log.e(GlobalConfig.Log_TAG, "SD card is not avaiable/writeable right now.");
            Toast.makeText(MainActivity.this, "SD card is not avaiable/writeable right now", Toast.LENGTH_SHORT);
            return;
        }

        try {
            String absoultPath = Environment.getExternalStorageDirectory() + File.separator + GlobalConfig.Setting_SaveFileName;
            File file = new File(absoultPath);
            if (!file.exists()) {
                return;
            }

            String json = FileUtils.readFile(absoultPath);
            Log.d("myhook", json);

            GlobalConfig.commit(json);

            init();
        }
        catch (IOException e) {
            e.printStackTrace();
        } catch (JSONException e) {
            e.printStackTrace();
        }
    }

    ArrayList<String> findAppLibs(String packgeString, ArrayList<String> ignoreLists)
    {
        ArrayList<String> data_list = new ArrayList<String>();

        File file = new File("/data/data/" + packgeString + "/lib");
        for (File f : file.listFiles())
        {
            boolean ignore = false;
            for (String s  : ignoreLists)
            {
                if (f.getName().equals(s))
                {
                    ignore = true;
                    break;
                }
            }

            if (!ignore)
                data_list.add(f.getAbsolutePath());
        }
        return data_list;
    }
}