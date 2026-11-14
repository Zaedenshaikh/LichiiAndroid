package com.eden.lichii.nxformat

import android.content.Context
import android.content.res.AssetManager
import androidx.annotation.Keep

/**
 * JNI interface for the nxformat native library.
 * Provides access to NX archive format reading capabilities.
 */
class NxFormatNative private constructor() {

    companion object {
        @Volatile private var instance: NxFormatNative? = null
        
        fun getInstance(context: Context): NxFormatNative {
            return instance ?: synchronized(this) {
                instance ?: NxFormatNative().also {
                    it.init(context.assets)
                    instance = it
                }
            }
        }
        
        fun reset() {
            instance?.closeArchive()
            instance = null
        }
    }

    private var isLoaded = false

    init {
        System.loadLibrary("nxformat-jni")
        isLoaded = true
    }

    private external fun nativeInit(assetManager: AssetManager)
    private external fun nativeOpenArchive(path: String): Boolean
    private external fun nativeOpenAsset(assetManager: AssetManager, assetName: String): Boolean
    private external fun nativeCloseArchive()
    private external fun nativeIsOpen(): Boolean
    private external fun nativeGetArchiveInfo(): ArchiveInfo?
    private external fun nativeGetRootHandle(): Long
    private external fun nativeListDirectory(handle: Long): Array<DirectoryEntry>
    private external fun nativeGetFileInfo(fileId: Long): FileInfo?
    private external fun nativeReadFile(fileId: Long, offset: Long, length: Int): ByteArray?
    private external fun nativeReadFileFull(fileId: Long): ByteArray?
    private external fun nativeGetLastError(): String

    fun init(assetManager: AssetManager) {
        if (isLoaded) {
            nativeInit(assetManager)
        }
    }

    fun openArchive(path: String): Boolean {
        return isLoaded && nativeOpenArchive(path)
    }

    fun openAsset(assetManager: AssetManager, assetName: String): Boolean {
        return isLoaded && nativeOpenAsset(assetManager, assetName)
    }

    fun closeArchive() {
        if (isLoaded) {
            nativeCloseArchive()
        }
    }

    fun isOpen(): Boolean {
        return isLoaded && nativeIsOpen()
    }

    fun getArchiveInfo(): ArchiveInfo? {
        return if (isLoaded) nativeGetArchiveInfo() else null
    }

    fun getRootHandle(): Long {
        return if (isLoaded) nativeGetRootHandle() else 0
    }

    fun listDirectory(handle: Long): List<DirectoryEntry> {
        return if (isLoaded) nativeListDirectory(handle).toList() else emptyList()
    }

    fun getFileInfo(fileId: Long): FileInfo? {
        return if (isLoaded) nativeGetFileInfo(fileId) else null
    }

    fun readFile(fileId: Long, offset: Long = 0, length: Int = -1): ByteArray? {
        if (!isLoaded) return null
        return if (length < 0) {
            nativeReadFileFull(fileId)
        } else {
            nativeReadFile(fileId, offset, length)
        }
    }

    fun getLastError(): String {
        return if (isLoaded) nativeGetLastError() else "Native library not loaded"
    }
}

@Keep
data class ArchiveInfo(
    val generation: Long,
    val archiveSize: Long,
    val blockCount: Int,
    val mediaRecordCount: Int,
    val hasMedia: Boolean,
    val reserved: Int
)

@Keep
data class DirectoryEntry(
    val fileId: Long,
    val isDirectory: Boolean,
    val size: Long,
    val name: String
)

@Keep
data class FileInfo(
    val id: Long,
    val size: Long,
    val chunkLog2: Int,
    val digest: ByteArray
)