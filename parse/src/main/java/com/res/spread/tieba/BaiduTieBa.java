package com.res.spread.tieba;

import android.os.Bundle;
import android.util.Log;

import com.lody.whale.xposed.IXposedHookLoadPackage;
import com.lody.whale.xposed.XC_MethodReplacement;
import com.lody.whale.xposed.XposedBridge;
import com.lody.whale.xposed.XposedHelpers;
import com.lody.whale.xposed.callbacks.XC_LoadPackage;

import java.lang.reflect.Constructor;
import java.lang.reflect.InvocationTargetException;
import java.util.HashMap;
import java.util.LinkedList;

public class BaiduTieBa implements IXposedHookLoadPackage, Runnable {

    final static String TAG = "myhook";

    static ClassLoader classLoader = null;

    static HashMap mUserMap = new HashMap();

    SendQueue mSendQueue = new SendQueue();

    //package com.baidu.tbadk.data.MetaData;
    public class MetaData {
        public String userId;
        public String userName;
        public String name_show;
        public String portrait;
    }

    //发送队列
    public class SendQueue {
        private LinkedList list = new LinkedList();
        public void clear()//销毁队列
        {
            list.clear();
        }
        public boolean QueueEmpty()//判断队列是否为空
        {
            return list.isEmpty();
        }
        public void enQueue(Object o)//进队
        {
            list.addLast(o);
        }
        public Object deQueue()//出队
        {
            if(!list.isEmpty())
            {
                return list.removeFirst();
            }
            return null;
        }
        public int QueueLength()//获取队列长度
        {
            return list.size();
        }
        public Object QueuePeek()//查看队首元素
        {
            return list.getFirst();
        }
    }

    @Override
    public void run()
    {
        while (true)
        {
            synchronized (mSendQueue)
            {
                if (mSendQueue.QueueEmpty())
                    continue;

                MetaData metaData = (MetaData) mSendQueue.deQueue();
                if (metaData == null)
                    continue;
                String userId = metaData.userId;
                try
                {
                    Object PersonalMsglistModel = genPersonalMsglistModel(metaData);
                    if (PersonalMsglistModel != null)
                    {
                        //滴滴滴，niunaishipin点抗，永久免费，安全无毒更新快！小哥哥来喔～
                        Object ChatMessage = XposedHelpers.callMethod(PersonalMsglistModel, "createMessage", (short)1, (String)"你好啊～");
                        if (ChatMessage != null)
                        {
                            XposedHelpers.callMethod(PersonalMsglistModel, "doSendText", ChatMessage);
                            Log.i(TAG, "doSendText: userId:" + userId + " 发送成功 ！");
                        }
                        else
                        {
                            Log.e(TAG, "ChatMessage is null");
                        }
                    }
                    else
                    {
                        Log.e(TAG, "PersonalMsglistModel is null");
                    }
                }
                catch (Exception e)
                {
                    XposedBridge.log(e);
                }
            }

            try {
                Thread.sleep(1000);
            } catch (InterruptedException e) {
                XposedBridge.log(e);
            }
        }
    }

    @Override
    public void handleLoadPackage(XC_LoadPackage.LoadPackageParam lpparam) throws Throwable {

        if (!lpparam.packageName.equals("com.baidu.tieba"))
            return;

        classLoader = lpparam.classLoader;

        mUserMap.clear();

        mSendQueue.clear();

        XposedHelpers.findAndHookMethod("com.baidu.tieba.LogoActivity",
                classLoader,
                "onCreate",
                Bundle.class,
                new XC_MethodReplacement() {

                    @Override
                    protected Object replaceHookedMethod(MethodHookParam param) throws Throwable {

                        Object ret = XposedBridge.invokeOriginalMethod(param.method, param.thisObject, param.args);

                        Log.d(TAG, "注入成功");

                        return ret;
                    }
                });

        //发送聊天数据接口
        XposedHelpers.findAndHookMethod("com.baidu.tieba.im.model.MsglistModel",
                classLoader,
                "sendTextMessage",
                String.class,
                new XC_MethodReplacement() {

                    @Override
                    protected Object replaceHookedMethod(MethodHookParam param) throws Throwable {
                        String msg = (String)param.args[0];
                        Log.d(TAG, "replaceHookedMethod: msg:" + msg);

                        XposedBridge.invokeOriginalMethod(param.method, param.thisObject, param.args);

                        return null;
                    }
                });

        //设置解析用户数据接口
        //package com.baidu.tbadk.data.MetaData;
        //public void parserProtobuf(User arg9) {
        XposedHelpers.findAndHookMethod("com.baidu.tbadk.data.MetaData",
                classLoader,
                "parserProtobuf",
                XposedHelpers.findClass("tbclient.User", classLoader),
                new XC_MethodReplacement() {

                    @Override
                    protected Object replaceHookedMethod(MethodHookParam param) throws Throwable {

                        XposedBridge.invokeOriginalMethod(param.method, param.thisObject, param.args);

                        String userId = (String)XposedHelpers.getObjectField(param.thisObject, "userId");

                        if (userId.isEmpty())
                            return null;

                        //Log.d(TAG, "userId:" + userId);

                        if (!mUserMap.containsKey(userId))
                        {
                            MetaData metaData = new MetaData();
                            metaData.userId = userId;
                            metaData.userName = (String)XposedHelpers.getObjectField(param.thisObject, "userName");
                            metaData.name_show = (String)XposedHelpers.getObjectField(param.thisObject, "name_show");
                            metaData.portrait = (String)XposedHelpers.getObjectField(param.thisObject, "portrait");

                            Log.d(TAG, "metaData.userId: " + metaData.userId);
                            Log.d(TAG, "metaData.userName: " + metaData.userName);
                            //Log.d(TAG, "metaData.name_show: " + metaData.name_show);
                            //Log.d(TAG, "metaData.portrait: " + metaData.portrait);

                            mUserMap.put(userId, metaData);

                            synchronized (mSendQueue)
                            {
                                mSendQueue.enQueue(metaData);
                            }
                        }

                        return null;
                    }
                });

        Log.d(TAG, "handleLoadPackage: ok ..... ");

        //启动发送线程
        new Thread(this).start();
    }

    //package com.baidu.tieba.imMessageCenter.im.model;
    //public class PersonalMsglistModel extends CommonPersonalMsglistModel {
    Object genPersonalMsglistModel(MetaData metaData)
    {
        //创建对象
        Object instance = null;
        Constructor constructor = XposedHelpers.findConstructorExact(
                "com.baidu.tieba.imMessageCenter.im.model.PersonalMsglistModel",
                classLoader);
        try {
            instance = constructor.newInstance();
        } catch (InstantiationException e) {
            XposedBridge.log(e);
        } catch (IllegalAccessException e) {
            XposedBridge.log(e);
        } catch (InvocationTargetException e) {
            XposedBridge.log(e);
        }

        //设置聊天类型为2
        XposedHelpers.setIntField(instance, "customGroupType", 2);

        Object MsgPageData = null;
        try {
            MsgPageData = XposedHelpers.findConstructorExact(
                    "com.baidu.tieba.im.data.MsgPageData",
                    classLoader).newInstance();
        } catch (InstantiationException e) {
            XposedBridge.log(e);
        } catch (IllegalAccessException e) {
            XposedBridge.log(e);
        } catch (InvocationTargetException e) {
            XposedBridge.log(e);
        }
        XposedHelpers.setObjectField(instance, "mDatas", MsgPageData);

        //设置用户数据
        //com.baidu.tbadk.core.data.UserData
        Constructor ctorUserData = XposedHelpers.findConstructorExact("com.baidu.tbadk.core.data.UserData", classLoader);
        try {
            Object UserData = ctorUserData.newInstance();
            XposedHelpers.callMethod(UserData, "setUserName", metaData.userName);
            XposedHelpers.callMethod(UserData, "setUserId", metaData.userId);
            XposedHelpers.callMethod(UserData, "setName_show", metaData.name_show);
            XposedHelpers.callMethod(UserData, "setPortrait", metaData.portrait);
            XposedHelpers.callMethod(instance, "setUser", UserData);
        } catch (InstantiationException e) {
            XposedBridge.log(e);
        } catch (IllegalAccessException e) {
            XposedBridge.log(e);
        } catch (InvocationTargetException e) {
            XposedBridge.log(e);
        }

        return instance;
    }
}
