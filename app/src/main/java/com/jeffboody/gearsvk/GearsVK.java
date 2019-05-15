/*
 * Copyright (c) 2019 Jeff Boody
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

package com.jeffboody.gearsvk;

import android.app.NativeActivity;
import android.content.Context;
import android.content.Intent;
import android.media.AudioManager;
import android.net.Uri;
import android.os.Bundle;
import android.os.Handler;
import android.os.Handler.Callback;
import android.os.Message;
import android.util.Log;
import java.util.LinkedList;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

public class GearsVK
extends NativeActivity
implements Handler.Callback
{
	private static final String TAG = "gears";

	// native commands
	private static final int GEARS_CMD_PLAY_CLICK = 1;
	private static final int GEARS_CMD_EXIT       = 2;
	private static final int GEARS_CMD_LOADURL    = 3;

	private static LinkedList<Integer> mCmdQueue = new LinkedList<Integer>();
	private static Lock                mCmdLock  = new ReentrantLock();

	// "singleton" used for callbacks
	// handler is used to trigger commands on UI thread
	private static Handler mHandler = null;
	private static String  mURL     = "";

	/*
	 * Command Queue - A queue is needed to ensure commands
	 * are not lost between the rendering thread and the
	 * main thread when pausing the app.
	 */

	private void DrainCommandQueue(boolean handler)
	{
		mCmdLock.lock();
		try
		{
			while(mCmdQueue.size() > 0)
			{
				int cmd = mCmdQueue.remove();
				if(cmd == GEARS_CMD_PLAY_CLICK)
				{
					if(handler)
					{
						AudioManager am = (AudioManager) getSystemService(Context.AUDIO_SERVICE);
						am.playSoundEffect(AudioManager.FX_KEY_CLICK, 0.5F);
					}
				}
				else if(cmd == GEARS_CMD_LOADURL)
				{
					try
					{
						Intent intent = new Intent(Intent.ACTION_VIEW, Uri.parse(mURL));
						startActivity(intent);
					}
					catch(Exception e)
					{
						// ignore
					}
				}
				else if(cmd == GEARS_CMD_EXIT)
				{
					if(handler)
					{
						finish();
					}
				}
				else
				{
					Log.w(TAG, "unknown cmd=" + cmd);
				}
			}
		}
		catch(Exception e)
		{
			Log.e(TAG, "exception: " + e);
		}
		mCmdLock.unlock();
	}

	/*
	 * Callback interface
	 */

	private static void CallbackCmd(int cmd, String msg)
	{
		try
		{
			if(cmd == GEARS_CMD_LOADURL)
			{
				mURL = msg;
			}

			mCmdLock.lock();
			mHandler.sendMessage(Message.obtain(mHandler, cmd));
			mCmdQueue.add(cmd);
			mCmdLock.unlock();
		}
		catch(Exception e)
		{
			// ignore
		}
	}

	public boolean handleMessage(Message msg)
	{
		DrainCommandQueue(true);
		return true;
	}

	/*
	 * Activity interface
	 */

	@Override
	public void onCreate(Bundle savedInstanceState)
	{
		Log.i(TAG, "onCreate");
		super.onCreate(savedInstanceState);

		mHandler = new Handler(this);
	}

	@Override
	protected void onResume()
	{
		Log.i(TAG, "onResume");
		super.onResume();
	}

	@Override
	protected void onPause()
	{
		Log.i(TAG, "onPause");
		DrainCommandQueue(false);
		super.onPause();
	}

	@Override
	protected void onDestroy()
	{
		Log.i(TAG, "onDestroy1");
		mHandler = null;
		super.onDestroy();
		Log.i(TAG, "onDestroy2");
	}
}
