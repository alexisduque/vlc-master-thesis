#define LOG_TAG "I2cServiceJNI"
#include "utils/Log.h"

#include "jni.h"
#include "JNIHelp.h"

#include <fcntl.h>
#include <sys/ioctl.h>
#include <stdint.h>
#include <uapi/linux/i2c.h>
#include <uapi/linux/i2c-dev.h>

namespace android
{

// These I/O direction and status enums must match those in Java
// (see I2cTransaction.java)

enum {
    IO_DIRECTION_WRITE = 0,
    IO_DIRECTION_READ = 1,
};

enum {
    STATUS_NOT_STARTED = 1,
    STATUS_OK = 0,
    STATUS_ERR = -1,
};

static struct i2c_transaction_offsets_t
{
    jfieldID mIoDirection;
    jfieldID mStatus;
    jfieldID mData;
} gI2cTransactionOffsets;

static jint com_projectara_server_I2cService_native_perform_i2c_txns(
    JNIEnv *env, jobject thiz, jstring name, jint address, jobjectArray txns)
{
    jint ret = 0;

    const char *pathStr = env->GetStringUTFChars(name, NULL);
    int fd = open(pathStr, O_RDWR);
    if (fd < 0) {
        ALOGE("can't open %s", pathStr);
        env->ReleaseStringUTFChars(name, pathStr);
        return fd;
    }
    env->ReleaseStringUTFChars(name, pathStr);

    jsize nTxns = env->GetArrayLength(txns);
    struct i2c_msg messages[nTxns];
    for (jsize i = 0; i < nTxns; i++) {
        jobject txn = env->GetObjectArrayElement(txns, i);
        jint iod = env->GetIntField(txn, gI2cTransactionOffsets.mIoDirection);
        jobject dataObj =
            env->GetObjectField(txn, gI2cTransactionOffsets.mData);
        jbyteArray *dataAp = reinterpret_cast<jbyteArray*>(&dataObj);
        jsize dataLength = env->GetArrayLength(*dataAp);
        jbyte *datap = env->GetByteArrayElements(*dataAp, NULL);

        messages[i].addr = (__u16)address;
        messages[i].flags = (iod == IO_DIRECTION_READ) ? I2C_M_RD : 0;
        messages[i].len = (__u16)dataLength;
        messages[i].buf = (__u8*)datap;

        env->DeleteLocalRef(txn);
    }

    struct i2c_rdwr_ioctl_data packets;
    packets.msgs = messages;
    packets.nmsgs = nTxns;
    int io_rc = ioctl(fd, I2C_RDWR, &packets);
    if (io_rc < 0) {
        ALOGE("I/O error: %d", io_rc);
        ret = io_rc;
    }

    for (jsize i = 0; i < nTxns; i++) {
        jobject txn = env->GetObjectArrayElement(txns, i);
        jobject dataObj =
            env->GetObjectField(txn, gI2cTransactionOffsets.mData);
        jbyteArray *dataAp = reinterpret_cast<jbyteArray*>(&dataObj);

        env->ReleaseByteArrayElements(*dataAp, (jbyte*)messages[i].buf, 0);
        env->SetIntField(txn, gI2cTransactionOffsets.mStatus,
                         ret < 0 ? STATUS_ERR : STATUS_OK);

        env->DeleteLocalRef(txn);
    }

    close(fd);
    return ret;
}

static JNINativeMethod method_table[] = {
    { "native_perform_i2c_txns",
      "(Ljava/lang/String;I[Lcom/projectara/hardware/bridge/I2cTransaction;)I",
      (void*)com_projectara_server_I2cService_native_perform_i2c_txns },
};

int register_com_projectara_server_I2cService(JNIEnv *env)
{
    const char *serviceName = "com/projectara/server/I2cService";
    const char *txnName = "com/projectara/hardware/bridge/I2cTransaction";

    jclass clazz = env->FindClass(serviceName);
    if (clazz == NULL) {
        ALOGE("Can't find %s", serviceName);
        return -1;
    }
    clazz = env->FindClass(txnName);
    LOG_FATAL_IF(clazz == NULL,
                 "Can't find com.projectara.hardware.bridge.I2cTransaction");
    gI2cTransactionOffsets.mIoDirection = env->GetFieldID(clazz,
                                                          "ioDirection",
                                                          "I");
    gI2cTransactionOffsets.mStatus = env->GetFieldID(clazz, "status", "I");
    gI2cTransactionOffsets.mData = env->GetFieldID(clazz, "data", "[B");
    jniRegisterNativeMethods(env, serviceName, method_table,
                             NELEM(method_table));
    return 0;
}

}
