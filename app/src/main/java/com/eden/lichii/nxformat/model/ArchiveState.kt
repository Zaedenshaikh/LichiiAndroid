package com.eden.lichii.nxformat.model

import android.app.Application
import android.content.Context
import android.net.Uri
import android.os.Environment
import androidx.lifecycle.AndroidViewModel
import androidx.lifecycle.viewModelScope
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.setValue
import com.eden.lichii.nxformat.NxFormatNative
import com.eden.lichii.nxformat.ArchiveInfo
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.asStateFlow
import kotlinx.coroutines.flow.stateIn
import kotlinx.coroutines.flow.SharingStarted
import kotlinx.coroutines.launch

sealed interface UiState {
    data class ArchiveOpened(
        val currentEntries: List<ArchiveEntry> = emptyList(),
        val currentPath: String = "/",
        val currentHandle: Long = 0,
        val parentHandles: List<Long> = emptyList(),
        val parentNames: List<String> = emptyList(),
        val archiveInfo: ArchiveInfo? = null
    ) : UiState

    object Loading : UiState

    data class Error(val message: String) : UiState

    object Closed : UiState
}

data class ArchiveEntry(
    val fileId: Long,
    val name: String,
    val isDirectory: Boolean,
    val size: Long
)

class MainViewModel(
    application: Application,
    private val nxFormat: NxFormatNative
) : AndroidViewModel(application) {

    private val _uiState = MutableStateFlow<UiState>(UiState.Closed)
    val uiState = _uiState
        .stateIn(
            scope = viewModelScope,
            started = SharingStarted.WhileSubscribed(5000),
            initialValue = UiState.Closed
        )

    private var currentHandle = 0L
    private var parentHandles = mutableListOf<Long>()
    private var parentNames = mutableListOf<String>()

    private val context: Context = application.applicationContext

    fun onFileSelected(uri: Uri) {
        viewModelScope.launch {
            _uiState.value = UiState.Loading
            try {
                val contentResolver = context.contentResolver
                val fd = contentResolver.openFileDescriptor(uri, "r")?.fileDescriptor
                if (fd != null) {
                    val path = "/proc/self/fd/${fd}"
                    if (nxFormat.openArchive(path)) {
                        loadRoot()
                    } else {
                        _uiState.value = UiState.Error(nxFormat.getLastError())
                    }
                } else {
                    _uiState.value = UiState.Error("Failed to open file descriptor")
                }
            } catch (e: Exception) {
                _uiState.value = UiState.Error(e.message ?: "Unknown error")
            }
        }
    }

    fun openAsset(assetName: String) {
        viewModelScope.launch {
            _uiState.value = UiState.Loading
            try {
                if (nxFormat.openAsset(context.assets, assetName)) {
                    loadRoot()
                } else {
                    _uiState.value = UiState.Error(nxFormat.getLastError())
                }
            } catch (e: Exception) {
                _uiState.value = UiState.Error(e.message ?: "Unknown error")
            }
        }
    }

    fun closeArchive() {
        viewModelScope.launch {
            nxFormat.closeArchive()
            currentHandle = 0L
            parentHandles.clear()
            parentNames.clear()
            _uiState.value = UiState.Closed
        }
    }

    fun retry() {
        if (currentHandle != 0L) {
            loadDirectory(currentHandle)
        } else {
            _uiState.value = UiState.Closed
        }
    }

    fun navigateTo(fileId: Long, name: String) {
        viewModelScope.launch {
            _uiState.value = UiState.Loading
            parentHandles.add(currentHandle)
            parentNames.add(name)
            currentHandle = fileId
            loadDirectory(fileId)
        }
    }

    fun goToParent() {
        viewModelScope.launch {
            if (parentHandles.isNotEmpty()) {
                _uiState.value = UiState.Loading
                currentHandle = parentHandles.removeAt(parentHandles.lastIndex)
                parentNames.removeAt(parentNames.lastIndex)
                loadDirectory(currentHandle)
            }
        }
    }

    fun extractFile(fileId: Long, name: String) {
        viewModelScope.launch {
            try {
                val data = nxFormat.readFile(fileId)
                if (data != null) {
                    val downloadsDir = context.getExternalFilesDir(Environment.DIRECTORY_DOWNLOADS)
                        ?: context.filesDir
                    val file = java.io.File(downloadsDir, name)
                    file.writeBytes(data)
                    _uiState.value = UiState.Loading
                }
            } catch (e: Exception) {
                _uiState.value = UiState.Error("Failed to extract: ${e.message}")
            }
        }
    }

    fun showFileOptions(entry: ArchiveEntry) {
        // TODO: Show dialog with options
    }

    fun showAssetPicker() {
        // TODO: Show asset picker dialog
    }

    private fun loadRoot() {
        viewModelScope.launch {
            currentHandle = nxFormat.getRootHandle()
            loadDirectory(currentHandle)
        }
    }

    private fun loadDirectory(handle: Long) {
        viewModelScope.launch {
            try {
                val entries = nxFormat.listDirectory(handle)
                val archiveInfo = nxFormat.getArchiveInfo()
                
                val path = if (parentNames.isEmpty()) "/" else "/${parentNames.joinToString("/")}/"
                
                _uiState.value = UiState.ArchiveOpened(
                    currentEntries = entries.map { ArchiveEntry(
                        fileId = it.fileId,
                        name = it.name,
                        isDirectory = it.isDirectory,
                        size = it.size
                    ) },
                    currentPath = path,
                    currentHandle = handle,
                    parentHandles = parentHandles.toList(),
                    parentNames = parentNames.toList(),
                    archiveInfo = archiveInfo
                )
            } catch (e: Exception) {
                _uiState.value = UiState.Error(e.message ?: "Failed to load directory")
            }
        }
    }
}