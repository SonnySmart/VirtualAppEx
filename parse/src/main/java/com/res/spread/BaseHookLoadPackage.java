package com.res.spread;

import android.app.Activity;
import android.os.Bundle;
import android.widget.Toast;

import com.lody.whale.xposed.IXposedHookLoadPackage;
import com.lody.whale.xposed.XC_MethodReplacement;
import com.lody.whale.xposed.XposedBridge;
import com.lody.whale.xposed.XposedHelpers;
import com.lody.whale.xposed.callbacks.XC_LoadPackage;

import java.util.LinkedList;

public class BaseHookLoadPackage implements IXposedHookLoadPackage, Runnable {

    final long MSG_SEND_INTERVAL = 2000;

    protected final static String TAG = "myhook";

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

    public interface SendQueueCallback {
        void onSend(Object o);
    }

    SendQueue mSendQueue = new SendQueue();

    SendQueueCallback mSendQueueCallback = null;

    XC_LoadPackage.LoadPackageParam mParam = null;

    protected ClassLoader classLoader = null;

    protected Activity mActivity = null;

    @Override
    public void handleLoadPackage(XC_LoadPackage.LoadPackageParam lpparam) throws Throwable {

        mParam = lpparam;

        classLoader = lpparam.classLoader;

        mSendQueue.clear();

        doPreHook();

        new Thread(this).start();
    }

    @Override
    public void run() {

        while (true) {

            try {
                Thread.sleep(MSG_SEND_INTERVAL);
            } catch (InterruptedException e) {
                XposedBridge.log(e);
            }

            synchronized (mSendQueue) {
                if (mSendQueue.QueueEmpty())
                    continue;

                if (mSendQueueCallback == null)
                    continue;

                Object o = mSendQueue.deQueue();
                if (o == null)
                    continue;

                mSendQueueCallback.onSend(o);

                ShowToast(String.format("发送成功,队列剩余%d条.", mSendQueue.QueueLength()));
            }
        }
    }

    private void doPreHook() {
        XposedHelpers.findAndHookMethod(Activity.class, "onCreate", Bundle.class, new XC_MethodReplacement() {
            @Override
            protected Object replaceHookedMethod(MethodHookParam param) throws Throwable {

                Object ret = XposedBridge.invokeOriginalMethod(param.method, param.thisObject, param.args);

                if (mActivity == null) {
                    mActivity = (Activity) param.thisObject;
                    ShowToast("注入成功，Activity 找到。");
                }

                return ret;
            }
        });
    }

    protected void ShowToast(final String content) {
        if (mActivity != null) {
            mActivity.runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    Toast.makeText(mActivity, content, Toast.LENGTH_SHORT).show();
                }
            });
        }
    }

    protected void registerSendQueueCallback(SendQueueCallback callback) {
        mSendQueueCallback = callback;
    }

    protected void enQueue(Object o) {
        synchronized (mSendQueue) {
            mSendQueue.enQueue(o);
        }
    }

    protected boolean isProcess(String process) {
        if (mParam == null)
            return false;
        return mParam.packageName.equals(process);
    }
}
