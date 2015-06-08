package com.projectara.hardware.bridge;

import android.content.Context;
import android.os.RemoteException;
import android.util.Log;

import java.io.IOException;

/**
 * @brief This class allows module support apps to control I2C devices
 *        on modules through bridge ASICs.
 *
 * Typical use (e.g. from an Activity):
 *
 * <pre>
 * // Get an I2cManager object, and find out the available I2C buses.
 * I2cManager i2c = ...;
 *
 * // Next, set up the I2C I/O you want to do.
 * //
 * // First, we'll write the three bytes 0x00, 0x01, 0x02.
 * I2cTransaction wTxn = I2cTransaction.newWrite(0x00, 0x01, 0x02);
 * // Then, we'll read 15 bytes back.
 * I2cTransaction rTxn = I2cTransaction.newRead(15);
 *
 * // Now, actually do the I/O. We pick the I2C bus arbitrarily,
 * // as the first one we got back.
 * I2cTransaction[] results;
 * try {
 *     results = i2c.performTransactions(buses[0], deviceAddr7bit,
 *                                       wTxn, rTxn);
 * } catch (IOException e) {
 *     // Something went wrong. Do your error handling here.
 * }
 *
 * // Otherwise, everything completed successfully.
 * // results[0] is the result of the write transaction.
 * // results[1] is the result of the read transaction.
 * for (byte b: results[1].data) {
 *     doSomethingWith(b);
 * }
 * </pre>
 */
public class I2cManager {
    private static final String TAG = "I2cManager";

    private final Context mContext; // currently ignored; likely necessary later
    private final II2cManager mService;

    /**
     * @hide
     */
    public I2cManager(Context context, II2cManager service) {
        mContext = context;
        mService = service;
    }

    /**
     * Get the names of all available I2C buses, or null if none are
     * available.
     */
    public String[] getI2cBuses() {
        try {
            return mService.getI2cBuses();
        } catch (RemoteException e) {
            Log.e(TAG, "RemoteException in getI2cBuses", e);
            return null;
        }
    }

    /**
     * Perform a sequence of I2C transactions on a given bus.
     *
     * @param bus Name of I2C bus to perform transactions on
     * @param address Address of I2C slave, 7-bit, in bottom 7 bits
     * @param txns Transactions to perform
     *
     * @return Transaction results. The returned array is parallel to
     *         txns. That is, if txns[i] is a read transaction, the
     *         returned_array[i].data contains the read data.
     *
     * @see I2cTransaction
     */
    public I2cTransaction[] performTransactions(String bus, int address,
                                                I2cTransaction... txns)
        throws IOException {
        String errorMsg = "error while performing I2C transaction";
        checkAddrOk(address);
        int status;
        try {
            status = mService.performTransactions(bus, address, txns);
        } catch (RemoteException e) {
            throw new IOException(errorMsg, e);
        }
        if (status < 0) {
            throw new IOException(errorMsg);
        }
        return txns;
    }

    private static void checkAddrOk(int addr) {
        if (addr < 0 || (addr & ~0xFF) != 0) {
            throw new IllegalArgumentException("invalid address " + addr);
        }
    }
}
