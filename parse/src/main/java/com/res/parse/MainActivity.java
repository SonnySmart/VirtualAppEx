package com.res.parse;

import android.Manifest;
import android.app.Activity;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.os.Build;
import android.os.Bundle;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.EditText;
import android.widget.Spinner;
import android.widget.TextView;
import android.widget.Toast;

import java.io.File;
import java.util.ArrayList;

import static com.res.parse.GlobalConfig.editor;

public class MainActivity extends Activity {
    private EditText et_inject_pckname, et_hook_soname, et_send_msg;
    private Button bt_inject_pckname;
    private Button btn_save;
    private Button btn_start;
    private Button btn_stop;
    private Button btn_del;
    private Spinner spinner_hook_soname;
    private CheckBox check_dump_dll;
    private CheckBox check_dump_lua;
    private CheckBox check_dump_res;
    private CheckBox check_dump_res1;
    private CheckBox check_dump_res2;
    private CheckBox check_dump_xxtea;
    private CheckBox check_dump_inject;
    private TextView tv_help;

    public static MainActivity Instance = null;

    static final int REQUEST_CHOOSEFILE = 1;
    private static final int REQUEST_EXTERNAL_STORAGE = 1;
    private static String[] PERMISSIONS_STORAGE = {
            Manifest.permission.READ_EXTERNAL_STORAGE,
            Manifest.permission.WRITE_EXTERNAL_STORAGE
    };

    static ArrayList<String> so_list = new ArrayList<String>();

    static
    {
        so_list.add("libmono.so");
        so_list.add("libqpry_lua.so");
        so_list.add("libcocos2dlua.so");
        so_list.add("libgame.so");
        so_list.add("libcocos2djs.so");
    }

    public void setPackageString(final String packageString) {

        et_inject_pckname.setText(packageString);

        ArrayList<String> ret = findAppLibs(packageString, new ArrayList<String>());
        for (String lib : ret)
        {
            for (final String s : so_list)
            {
                if (lib.contains(s))
                {
                    et_hook_soname.setText(s);
                    break;
                }
            }
        }

        initSpinnerHookSoName(ret);
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        Instance = this;

        verifyStoragePermissions(this);

        setContentView(R.layout.activity_main);
        et_inject_pckname = findViewById(R.id.et_inject_pckname);
        et_hook_soname = findViewById(R.id.et_hook_soname);
        et_send_msg = findViewById(R.id.et_send_msg);
        et_inject_pckname.setSaveEnabled(false);
        et_hook_soname.setSaveEnabled(false);
        et_send_msg.setSaveEnabled(false);
        bt_inject_pckname = findViewById(R.id.bt_inject_pckname);
        spinner_hook_soname = findViewById(R.id.spinner_hook_soname);
        check_dump_dll = findViewById(R.id.check_dump_dll);
        check_dump_lua = findViewById(R.id.check_dump_lua);
        check_dump_res = findViewById(R.id.check_dump_res);
        check_dump_res1 = findViewById(R.id.check_dump_res1);
        check_dump_res2 = findViewById(R.id.check_dump_res2);
        check_dump_xxtea = findViewById(R.id.check_dump_xxtea);
        check_dump_inject = findViewById(R.id.check_dump_inject);
        tv_help = findViewById(R.id.tv_help);
        btn_save = findViewById(R.id.btn_save);
        btn_start = findViewById(R.id.btn_start);
        btn_stop = findViewById(R.id.btn_stop);
        btn_del = findViewById(R.id.btn_del);
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
        btn_stop.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                onClickStop();
            }
        });
        btn_del.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                onClickDel();
            }
        });

        tv_help.setText("帮助说明:\n" +
                "/sdcard/myhook/config.json 为配置文件\n" +
                "/sdcard/myhook/log.txt 为log文件可以查看破解是否完成" +
                "/sdcard/myhook/tmp 把需要解密的文件拷贝进这个文件夹\n" +
                "/sdcard/myhook/Cocos2dAsset/包名/ 解密完成去这个目录拷贝文件\n" +
                "1.选择注入包名\n" +
                "2.选择注入so之后\n" +
                "3.选择勾选项" +
                "4.点击保存配置\n" +
                "5.从VirtualAppEx程序列表启动app既可" +
                "作者QQ99939534有问题可以联系");

        initConfig();
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
    }

    void verifyStoragePermissions(Activity activity) {
        /**
         * 动态获取权限，Android 6.0 新特性，一些保护权限，除了要在AndroidManifest中声明权限，还要使用如下代码动态获取
         */
        if (Build.VERSION.SDK_INT >= 23) {
            int REQUEST_CODE_CONTACT = 101;
            String[] permissions = {Manifest.permission.WRITE_EXTERNAL_STORAGE};
            //验证是否许可权限
            for (String str : permissions) {
                if (this.checkSelfPermission(str) != PackageManager.PERMISSION_GRANTED) {
                    //申请权限
                    this.requestPermissions(permissions, REQUEST_CODE_CONTACT);
                    return;
                }
            }
        }
    }

    void initConfig()
    {
        //读取配置文件
        GlobalConfig.readConfigFile();

        et_inject_pckname.setText(GlobalConfig.getString(GlobalConfig.Setting_Key_pack_name, ""));
        et_hook_soname.setText(GlobalConfig.getString(GlobalConfig.Setting_Key_hook_name, ""));
        et_send_msg.setText(GlobalConfig.getString(GlobalConfig.Setting_Key_send_msg, "哈喽，你好啊～"));
        check_dump_dll.setChecked(GlobalConfig.getString(GlobalConfig.Setting_Key_dump_dll, "").equals("1"));
        check_dump_lua.setChecked(GlobalConfig.getString(GlobalConfig.Setting_Key_dump_lua, "").equals("1"));
        check_dump_res.setChecked(GlobalConfig.getString(GlobalConfig.Setting_Key_dump_res, "").equals("1"));
        check_dump_res1.setChecked(GlobalConfig.getString(GlobalConfig.Setting_Key_dump_res1, "").equals("1"));
        check_dump_res2.setChecked(GlobalConfig.getString(GlobalConfig.Setting_Key_dump_res2, "").equals("1"));
        check_dump_xxtea.setChecked(GlobalConfig.getString(GlobalConfig.Setting_Key_dump_xxtea, "").equals("1"));
        check_dump_inject.setChecked(GlobalConfig.getString(GlobalConfig.Setting_Key_dump_inject, "").equals("1"));

        makeDir();

        initSpinnerHookSoName();
    }

    void makeDir() {
        //创建文件夹
        File file = new File(GlobalConfig.Setting_TmpPath);
        if (!file.exists())
            file.mkdir();
        file = new File(GlobalConfig.Setting_InjectPath);
        if (!file.exists())
            file.mkdir();
    }

    void initSpinnerHookSoName()
    {
        String packString = et_inject_pckname.getText().toString();
        if (packString.isEmpty())
            initSpinnerHookSoName(new ArrayList<String>(so_list));
        else
            initSpinnerHookSoName(findAppLibs(packString, new ArrayList<String>()));

        //initSpinnerHookSoName(new ArrayList<String>(so_list));
    }

    void initSpinnerHookSoName(ArrayList<String> data_list)
    {
        spinner_hook_soname.removeAllViewsInLayout();

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
            GlobalConfig.putString(GlobalConfig.Setting_Key_send_msg, et_send_msg.getText().toString());
            GlobalConfig.putString(GlobalConfig.Setting_Key_dump_dll, check_dump_dll.isChecked() ? "1" : "0");
            GlobalConfig.putString(GlobalConfig.Setting_Key_dump_lua, check_dump_lua.isChecked() ? "1" : "0");
            GlobalConfig.putString(GlobalConfig.Setting_Key_dump_res, check_dump_res.isChecked() ? "1" : "0");
            GlobalConfig.putString(GlobalConfig.Setting_Key_dump_res1, check_dump_res1.isChecked() ? "1" : "0");
            GlobalConfig.putString(GlobalConfig.Setting_Key_dump_res2, check_dump_res2.isChecked() ? "1" : "0");
            GlobalConfig.putString(GlobalConfig.Setting_Key_dump_xxtea, check_dump_xxtea.isChecked() ? "1" : "0");
            GlobalConfig.putString(GlobalConfig.Setting_Key_dump_inject, check_dump_inject.isChecked() ? "1" : "0");
            editor.commit();
            GlobalConfig.writeSaveFile();
            Toast.makeText(MainActivity.this, "保存设置成功", Toast.LENGTH_SHORT).show();
        } else {
            Toast.makeText(MainActivity.this, "请完整输入路径和包名", Toast.LENGTH_SHORT).show();
        }
    }

    void onClickStart() {
        String packageName = et_inject_pckname.getText().toString();
        PackageManager packageManager = getPackageManager();
        Intent intent = new Intent();
        intent = packageManager.getLaunchIntentForPackage(packageName);
        if (intent == null) {
            Toast.makeText(MainActivity.this, "未安装", Toast.LENGTH_LONG).show();
        } else {
            intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
            startActivity(intent);
        }
    }

    void onClickStop() {

    }

    void onClickDel() {
        String packString = et_inject_pckname.getText().toString();
        deleteFile(new File(GlobalConfig.Setting_TmpPath));
        if (!packString.isEmpty()) {
            deleteFile(new File(String.format("/sdcard/myhook/Cocos2dAsset/%s", packString)));
        }
        makeDir();
        Toast.makeText(this, String.format("删除%stmp完成", GlobalConfig.Setting_TmpPath), Toast.LENGTH_LONG).show();
    }

    ArrayList<String> findAppLibs(String packgeString, ArrayList<String> ignoreLists)
    {
        ArrayList<String> data_list = new ArrayList<String>();

        String fileDir = getFilesDir().getAbsolutePath();

        //virtual/data/app/packgeString/lib
        String libDir = String.format("%s/../virtual/data/app/%s/lib", fileDir, packgeString);

        File file = new File(libDir);

        if (file == null || file.listFiles() == null || file.listFiles().length == 0)
            return data_list;

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
                data_list.add(f.getName());
        }
        return data_list;
    }

    //flie：要删除的文件夹的所在位置
    private void deleteFile(File file) {
        if (file.isDirectory()) {
            File[] files = file.listFiles();
            for (int i = 0; i < files.length; i++) {
                File f = files[i];
                deleteFile(f);
            }
            file.delete();//如要保留文件夹，只删除文件，请注释这行
        } else if (file.exists()) {
            file.delete();
        }
    }
}