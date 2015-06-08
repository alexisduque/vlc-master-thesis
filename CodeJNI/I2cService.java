/*
 * Copyright (C) 2011 The Android Open Source Project
 *
 * Copyright (C) 2013, 2014 Google, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions an
 * limitations under the License.
 */

package com.projectara.server;

import java.io.File;
import java.io.FileFilter;
import java.util.Arrays;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.regex.Pattern;

import com.projectara.hardware.bridge.II2cManager;
import com.projectara.hardware.bridge.I2cTransaction;
import android.util.Log;

/*
 * FIXME: add mechanism to request "my" module's i2c device (by vid/pid or something)
 */

/** @hide */
public class I2cService extends II2cManager.Stub {

    private static final String TAG = "I2cService";

    private static final String sBusNamePattern = "^i2c-\\d+$";
    private static final String sBusDirectory = "/dev";

    // Prevent DoS attacks
    private static final int MAX_TRANSACTIONS = 50;

    private static final String[] sBuses;
    private static final HashMap<String, Object> sBusLocks;

    // FIXME: add hotplug support
    static {
        ArrayList<String> busNames = new ArrayList<String>();
        sBusLocks = new HashMap<String, Object>();
        // Get an array of files in sBusDirectory whose names match
        // sBusNamePattern and which we can read and write.
        File[] buses = new File(sBusDirectory).listFiles(new FileFilter() {
                @Override
                public boolean accept(File f) {
                    return (f.canRead() && f.canWrite() &&
                            Pattern.matches(sBusNamePattern, f.getName()));
                }
            });
        // For each discovered bus, make a lock object.
        for (File bus: buses) {
            String busPath = bus.getAbsolutePath();
            sBusLocks.put(busPath, new Object());
            busNames.add(busPath);
            Log.d(TAG, "discovered bus: " + busPath);
        }
        // Cache the discovered bus names.
        sBuses = busNames.toArray(new String[busNames.size()]);
    }

    public I2cService() {
    }

    // FIXME: remove? Or at least add hotplug callbacks.
    public String[] getI2cBuses() {
        return Arrays.copyOf(sBuses, sBuses.length);
    }

    public int performTransactions(String bus, int address, I2cTransaction[] txns) {
        // XXX: add and use mechanism for checking caller permission 
        if (!sBusLocks.containsKey(bus)) {
            throw new IllegalArgumentException("invalid bus: " + bus);
        }
        if (txns.length > MAX_TRANSACTIONS) {
            throw new IllegalArgumentException("too many transactions; maximum is " +
                                               MAX_TRANSACTIONS);
        }
        if (txns.length == 0) {
            return 0;
        }
        synchronized (sBusLocks.get(bus)) {
            return native_perform_i2c_transactions(bus, address, txns);
        }
    }

    private static native int native_perform_i2c_txns(String bus, int address,
                                                      I2cTransaction[] txns);
}
