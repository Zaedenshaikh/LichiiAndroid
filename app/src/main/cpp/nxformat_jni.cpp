#include "nxformat_wrapper.h"
#include <jni.h>
#include <string>
#include <vector>

#define JNI_CLASS "com/eden/lichii/nxformat/NxFormatNative"

extern "C" {

// Helper to get the wrapper instance
static NxFormatWrapper& getWrapper() {
    return NxFormatWrapper::instance();
}

// Initialize the wrapper with asset manager
JNIEXPORT void JNICALL Java_com_eden_lichii_nxformat_NxFormatNative_init
    (JNIEnv* env, jclass clazz, jobject assetManager) {
    getWrapper().init(env, assetManager);
}

// Open an NX archive from a file path
JNIEXPORT jboolean JNICALL Java_com_eden_lichii_nxformat_NxFormatNative_openArchive
    (JNIEnv* env, jclass clazz, jstring path) {
    const char* pathStr = env->GetStringUTFChars(path, nullptr);
    bool result = getWrapper().openArchive(pathStr);
    env->ReleaseStringUTFChars(path, pathStr);
    return static_cast<jboolean>(result);
}

// Open an NX archive from assets
JNIEXPORT jboolean JNICALL Java_com_eden_lichii_nxformat_NxFormatNative_openAsset
    (JNIEnv* env, jclass clazz, jobject assetManager, jstring assetName) {
    const char* nameStr = env->GetStringUTFChars(assetName, nullptr);
    bool result = getWrapper().openAsset(env, assetManager, nameStr);
    env->ReleaseStringUTFChars(assetName, nameStr);
    return static_cast<jboolean>(result);
}

// Close the current archive
JNIEXPORT void JNICALL Java_com_eden_lichii_nxformat_NxFormatNative_closeArchive
    (JNIEnv* env, jclass clazz) {
    getWrapper().closeArchive();
}

// Check if an archive is open
JNIEXPORT jboolean JNICALL Java_com_eden_lichii_nxformat_NxFormatNative_isOpen
    (JNIEnv* env, jclass clazz) {
    return static_cast<jboolean>(getWrapper().isOpen());
}

// Get archive metadata
JNIEXPORT jobject JNICALL Java_com_eden_lichii_nxformat_NxFormatNative_getArchiveInfo
    (JNIEnv* env, jclass clazz) {
    auto info = getWrapper().getArchiveInfo();
    if (!info) return nullptr;
    
    jclass infoClass = env->FindClass("com/eden/lichii/nxformat/ArchiveInfo");
    if (!infoClass) return nullptr;
    
    jmethodID constructor = env->GetMethodID(infoClass, "<init>", "(JJIIZI)V");
    if (!constructor) return nullptr;
    
    return env->NewObject(infoClass, constructor,
        static_cast<jlong>(info->generation),
        static_cast<jlong>(info->archiveSize),
        static_cast<jint>(info->blockCount),
        static_cast<jint>(info->mediaRecordCount),
        static_cast<jboolean>(info->hasMedia),
        0 // reserved
    );
}

// Get root directory handle
JNIEXPORT jlong JNICALL Java_com_eden_lichii_nxformat_NxFormatNative_getRootHandle
    (JNIEnv* env, jclass clazz) {
    auto handle = getWrapper().getRootHandle();
    return static_cast<jlong>(handle.value);
}

// List directory contents
JNIEXPORT jobjectArray JNICALL Java_com_eden_lichii_nxformat_NxFormatNative_listDirectory
    (JNIEnv* env, jclass clazz, jlong handleValue) {
    nx::DirectoryHandle handle;
    handle.value = static_cast<uint64_t>(handleValue);
    
    auto entries = getWrapper().listDirectory(handle);
    
    jclass entryClass = env->FindClass("com/eden/lichii/nxformat/DirectoryEntry");
    if (!entryClass) return nullptr;
    
    jmethodID constructor = env->GetMethodID(entryClass, "<init>", "(JZJLjava/lang/String;)V");
    if (!constructor) return nullptr;
    
    jobjectArray array = env->NewObjectArray(entries.size(), entryClass, nullptr);
    if (!array) return nullptr;
    
    for (size_t i = 0; i < entries.size(); ++i) {
        jstring name = env->NewStringUTF(entries[i].name.c_str());
        jobject entry = env->NewObject(entryClass, constructor,
            static_cast<jlong>(entries[i].fileId),
            static_cast<jboolean>(entries[i].isDirectory),
            static_cast<jlong>(entries[i].size),
            name
        );
        env->SetObjectArrayElement(array, i, entry);
        env->DeleteLocalRef(name);
        env->DeleteLocalRef(entry);
    }
    
    return array;
}

// Get file info
JNIEXPORT jobject JNICALL Java_com_eden_lichii_nxformat_NxFormatNative_getFileInfo
    (JNIEnv* env, jclass clazz, jlong fileId) {
    auto info = getWrapper().getFileInfo(static_cast<uint64_t>(fileId));
    if (!info) return nullptr;
    
    jclass infoClass = env->FindClass("com/eden/lichii/nxformat/FileInfo");
    if (!infoClass) return nullptr;
    
    jmethodID constructor = env->GetMethodID(infoClass, "<init>", "(JJI[B)V");
    if (!constructor) return nullptr;
    
    jbyteArray digestArray = env->NewByteArray(info->digest.size());
    if (digestArray) {
        env->SetByteArrayRegion(digestArray, 0, info->digest.size(),
            reinterpret_cast<const jbyte*>(info->digest.data()));
    }
    
    jobject result = env->NewObject(infoClass, constructor,
        static_cast<jlong>(info->id),
        static_cast<jlong>(info->size),
        static_cast<jint>(info->chunkLog2),
        digestArray
    );
    
    if (digestArray) env->DeleteLocalRef(digestArray);
    return result;
}

// Read file data
JNIEXPORT jbyteArray JNICALL Java_com_eden_lichii_nxformat_NxFormatNative_readFile
    (JNIEnv* env, jclass clazz, jlong fileId, jlong offset, jint length) {
    auto data = getWrapper().readFile(
        static_cast<uint64_t>(fileId),
        static_cast<uint64_t>(offset),
        static_cast<size_t>(length)
    );
    
    if (data.empty()) return nullptr;
    
    jbyteArray array = env->NewByteArray(data.size());
    if (array) {
        env->SetByteArrayRegion(array, 0, data.size(),
            reinterpret_cast<const jbyte*>(data.data()));
    }
    return array;
}

// Read entire file
JNIEXPORT jbyteArray JNICALL Java_com_eden_lichii_nxformat_NxFormatNative_readFileFull
    (JNIEnv* env, jclass clazz, jlong fileId) {
    auto data = getWrapper().readFile(static_cast<uint64_t>(fileId));
    
    if (data.empty()) return nullptr;
    
    jbyteArray array = env->NewByteArray(data.size());
    if (array) {
        env->SetByteArrayRegion(array, 0, data.size(),
            reinterpret_cast<const jbyte*>(data.data()));
    }
    return array;
}

// Get last error
JNIEXPORT jstring JNICALL Java_com_eden_lichii_nxformat_NxFormatNative_getLastError
    (JNIEnv* env, jclass clazz) {
    std::string error = getWrapper().getLastError();
    return env->NewStringUTF(error.c_str());
}

} // extern "C"