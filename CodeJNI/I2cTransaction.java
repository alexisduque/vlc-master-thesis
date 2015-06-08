package com.projectara.hardware.bridge;

import android.os.Parcel;
import android.os.Parcelable;

import java.util.Arrays;

/**
 * The I2cTransaction class represents a fixed amount of data to read
 * or write on an I2C bus. It is intended to be used with {@link
 * performTransactions I2cManager#performTransactions}.
 *
 * Typical use:
 *
 * <pre>
 *     // Makes a transaction which will write the tree bytes 0xAA, 0xBB,
 *     // and 0xCC, in that order.
 *     I2cTransaction writeTxn = I2cTransaction.newWrite(0xAA, 0xBB, 0xCC);
 *
 *     // Makes a transaction which will read two bytes
 *     I2cTransaction readTxn = I2cTransaction.newRead(2);
 * </pre>
 *
 * @see I2cManager
 */
public class I2cTransaction implements Parcelable {

    // These I/O directions and status definitions must keep up to date
    // with JNI (currently in com_projectara_server_I2cService.cpp).

    /** Transaction should read data. */
    public static final int IO_DIRECTION_READ = 1;
    /** Transaction has data to write. */
    public static final int IO_DIRECTION_WRITE = 0;

    /** Transaction hasn't been sent */
    public static final int STATUS_NOT_STARTED = 1;
    /** Transaction finished correctly (data was read/written) */
    public static final int STATUS_OK = 0;
    /** Error occurred while handling transaction */
    public static final int STATUS_ERR = -1;

    /**
     * Whether this transaction is a read or write.
     * @see IO_DIRECTION_READ
     * @see IO_DIRECTION_WRITE
     */
    public final int ioDirection;

    /**
     * Status of this I2C transaction.
     *
     * For platform internal use only.
     *
     * @hide
     */
    public int status;

    /**
     * Data read from or written to.
     *
     * Invalid until status==STATUS_OK.
     */
    public final byte[] data; // TODO: replace with ByteBuffer?

    // To prevent DoS attacks
    private static int MAX_TRANSACTION_SIZE = 512;

    private I2cTransaction(int ioDirection, byte[] data) {
        this(ioDirection, STATUS_NOT_STARTED, data);
    }

    private I2cTransaction(int ioDirection, int status, byte[] data) {
        this.ioDirection = ioDirection;
        this.status = status;
        this.data = data;
    }

    /**
     * Create a new read transaction.
     *
     * @param nBytes number of bytes to read
     */
    public static I2cTransaction newRead(int nBytes) {
        if (nBytes < 0) {
            throw new IllegalArgumentException("nBytes must be positive: " +
                                               nBytes);
        }
        if (nBytes > MAX_TRANSACTION_SIZE) {
            throw new IllegalArgumentException("nBytes " + nBytes +
                                               " is too large");
        }
        return new I2cTransaction(IO_DIRECTION_READ, new byte[nBytes]);
    }

    /**
     * Create a new write transaction.
     *
     * @param data Bytes to write
     */
    public static I2cTransaction newWrite(byte... data) {
        if (data == null) {
            throw new IllegalArgumentException("data must not be null");
        }
        if (data.length > MAX_TRANSACTION_SIZE) {
            throw new IllegalArgumentException("data.length " + data.length +
                                               " is too large");
        }
        if (data.length == 0) {
            throw new IllegalArgumentException("no data specified");
        }
        return new I2cTransaction(IO_DIRECTION_WRITE,
                                  Arrays.copyOf(data, data.length));
    }

    /**
     * Create a new write transaction.
     *
     * This is a convenience function to avoid having to cast literal
     * arguments to bytes. It works the same way as newWrite(byte...).
     *
     * @param data data to write; each value is converted to a byte first.
     */
    public static I2cTransaction newWrite(int... data) {
        if (data == null) {
            throw new IllegalArgumentException("data must not be null");
        }
        if (data.length > MAX_TRANSACTION_SIZE) {
            throw new IllegalArgumentException("data.length " + data.length +
                                               " is too large");
        }
        if (data.length == 0) {
            throw new IllegalArgumentException("no data specified");
        }
        byte[] bData = new byte[data.length];
        for (int i = 0; i < data.length; i++) {
            bData[i] = (byte)data[i];
        }
        return newWrite(bData);
    }

    // --------------- Parcelable API -------------------------------

    @Override
    public int describeContents() {
        return 0;
    }

    @Override
    public void writeToParcel(Parcel out, int flags) {
        out.writeInt(ioDirection);
        out.writeInt(status);
        out.writeInt(data.length);
        out.writeByteArray(data);
    }

    public static final Parcelable.Creator<I2cTransaction> CREATOR =
        new Parcelable.Creator<I2cTransaction>() {

        @Override
        public I2cTransaction createFromParcel(Parcel source) {
            int ioDirection = source.readInt();
            int status = source.readInt();
            int dataLen = source.readInt();
            byte[] data = new byte[dataLen];
            source.readByteArray(data);
            return new I2cTransaction(ioDirection, status, data);
        }

        @Override
        public I2cTransaction[] newArray(int size) {
            return new I2cTransaction[size];
        }
    };
}
