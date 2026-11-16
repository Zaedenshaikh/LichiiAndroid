package com.eden.lichii.nxformat.ui

import android.os.Bundle
import android.net.Uri
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.activity.result.contract.ActivityResultContracts
import androidx.compose.foundation.background
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.items
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material3.*
import androidx.compose.material.icons.filled.Default
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.vector.ImageVector
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import androidx.lifecycle.viewmodel.compose.viewModel
import com.eden.lichii.nxformat.NxFormatNative
import com.eden.lichii.nxformat.model.ArchiveEntry
import com.eden.lichii.nxformat.model.ArchiveState
import com.eden.lichii.nxformat.model.MainViewModel
import com.eden.lichii.nxformat.model.UiState
import androidx.compose.ui.unit.TextOverflow
import kotlinx.coroutines.launch

class MainActivity : ComponentActivity() {

    private val nxFormat = NxFormatNative.getInstance(application.applicationContext)
    private val mainViewModel: MainViewModel by viewModel {
        MainViewModel(application, nxFormat)
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContent {
            MainScreen(
                viewModel = mainViewModel,
                onPickFile = { /* handled by ActivityResultLauncher */ },
                onOpenAsset = { mainViewModel.openAsset(it) }
            )
        }
    }
}

@Composable
fun MainScreen(
    viewModel: MainViewModel,
    onPickFile: () -> Unit,
    onOpenAsset: (String) -> Unit
) {
    val state by viewModel.uiState.collectAsState()
    val context = LocalContext.current

    Scaffold(
        topBar = {
            TopAppBar(
                title = { Text("NX Archive Explorer") },
                colors = TopAppBarDefaults.topAppBarColors(
                    containerColor = MaterialTheme.colorScheme.surfaceContainer
                ),
                actions = {
                    if (state is ArchiveState.ArchiveOpened) {
                        IconButton(onClick = { viewModel.closeArchive() }) {
                            Icon(imageVector = Icons.Default.Close, contentDescription = "Close Archive")
                        }
                        IconButton(onClick = onPickFile) {
                            Icon(imageVector = Icons.Default.FolderOpen, contentDescription = "Open Archive")
                        }
                    } else {
                        IconButton(onClick = onPickFile) {
                            Icon(imageVector = Icons.Default.FolderOpen, contentDescription = "Open Archive")
                        }
                        IconButton(onClick = { viewModel.showAssetPicker() }) {
                            Icon(imageVector = Icons.Default.Storage, contentDescription = "Open Asset")
                        }
                    }
                }
            )
        },
        floatingActionButton = {
            if (state is ArchiveState.ArchiveOpened && state.currentHandle != 0L && state.currentEntries.isNotEmpty()) {
                ExtendedFloatingActionButton(
                    onClick = { viewModel.goToParent() },
                    icon = { Icon(imageVector = Icons.Default.ArrowUpward, contentDescription = "Parent Directory") },
                    text = { Text(text = "Up") },
                    containerColor = MaterialTheme.colorScheme.primaryContainer,
                    contentColor = MaterialTheme.colorScheme.onPrimaryContainer
                )
            }
        }
    ) { padding ->
        Box(modifier = Modifier.fillMaxSize().padding(padding)) {
            when (state) {
                is UiState.Closed -> EmptyState(onPickFile = onPickFile, onOpenAsset = onOpenAsset)
                is UiState.Loading -> LoadingState()
                is UiState.Error -> ErrorState(error = state.message, onRetry = { viewModel.retry() })
                is UiState.ArchiveOpened -> ArchiveBrowser(
                    entries = state.currentEntries,
                    currentPath = state.currentPath,
                    onEntryClick = { entry ->
                        if (entry.isDirectory) {
                            viewModel.navigateTo(entry.fileId, entry.name)
                        } else {
                            viewModel.extractFile(entry.fileId, entry.name)
                        }
                    },
                    onLongClick = { entry ->
                        if (!entry.isDirectory) {
                            viewModel.showFileOptions(entry)
                        }
                    }
                )
            }
        }
    }
}

@Composable
fun EmptyState(
    onPickFile: () -> Unit,
    onOpenAsset: (String) -> Unit
) {
    Column(
        modifier = Modifier.fillMaxSize(),
        verticalArrangement = Arrangement.Center,
        horizontalAlignment = Alignment.CenterHorizontally
    ) {
        Icon(
            imageVector = Icons.Default.Description,
            contentDescription = null,
            tint = MaterialTheme.colorScheme.primary,
            modifier = Modifier.size(96.dp)
        )
        Spacer(modifier = Modifier.padding(16.dp))
        Text(
            text = "No Archive Opened",
            fontSize = 24.sp,
            fontWeight = FontWeight.Medium,
            color = MaterialTheme.colorScheme.onSurface
        )
        Spacer(modifier = Modifier.padding(8.dp))
        Text(
            text = "Open an .nx game archive to explore its contents",
            fontSize = 16.sp,
            color = MaterialTheme.colorScheme.onSurfaceVariant,
            textAlign = androidx.compose.ui.text.TextAlign.Center
        )
        Spacer(modifier = Modifier.padding(24.dp))
        Row(horizontalArrangement = Arrangement.spacedBy(12.dp)) {
            FilledButton(onClick = onPickFile) {
                Row(Modifier.padding(horizontal = 24.dp, vertical = 12.dp)) {
                    Icon(imageVector = Icons.Default.FolderOpen, contentDescription = null)
                    Spacer(modifier = Modifier.width(8.dp))
                    Text(text = "Open Archive")
                }
            }
            OutlinedButton(onClick = { onOpenAsset("sample.nx") }) {
                Row(Modifier.padding(horizontal = 24.dp, vertical = 12.dp)) {
                    Icon(imageVector = Icons.Default.Storage, contentDescription = null)
                    Spacer(modifier = Modifier.width(8.dp))
                    Text(text = "Open Sample Asset")
                }
            }
        }
    }
}

@Composable
fun LoadingState() {
    Column(
        modifier = Modifier.fillMaxSize(),
        verticalArrangement = Arrangement.Center,
        horizontalAlignment = Alignment.CenterHorizontally
    ) {
        CircularProgressIndicator()
        Spacer(modifier = Modifier.padding(16.dp))
        Text(text = "Loading archive...", color = MaterialTheme.colorScheme.onSurfaceVariant)
    }
}

@Composable
fun ErrorState(error: String, onRetry: () -> Unit) {
    Column(
        modifier = Modifier.fillMaxSize(),
        verticalArrangement = Arrangement.Center,
        horizontalAlignment = Alignment.CenterHorizontally
    ) {
        Icon(
            imageVector = Icons.Default.Error,
            contentDescription = null,
            tint = MaterialTheme.colorScheme.error,
            modifier = Modifier.size(64.dp)
        )
        Spacer(modifier = Modifier.padding(16.dp))
        Text(text = "Error", fontSize = 24.sp, fontWeight = FontWeight.Medium)
        Spacer(modifier = Modifier.padding(8.dp))
        Text(
            text = error,
            fontSize = 16.sp,
            color = MaterialTheme.colorScheme.onSurfaceVariant,
            textAlign = androidx.compose.ui.text.TextAlign.Center,
            modifier = Modifier.padding(horizontal = 32.dp)
        )
        Spacer(modifier = Modifier.padding(24.dp))
        FilledButton(onClick = onRetry) {
            Text(text = "Retry")
        }
    }
}

@Composable
fun ArchiveBrowser(
    entries: List<ArchiveEntry>,
    currentPath: String,
    onEntryClick: (ArchiveEntry) -> Unit,
    onLongClick: (ArchiveEntry) -> Unit
) {
    Column {
        Card(
            modifier = Modifier.fillMaxWidth(),
            elevation = CardDefaults.cardElevation(defaultElevation = 2.dp),
            colors = CardDefaults.cardColors(
                containerColor = MaterialTheme.colorScheme.surfaceContainerLow
            )
        ) {
            Row(
                modifier = Modifier
                    .fillMaxWidth()
                    .padding(16.dp)
            ) {
                Text(
                    text = currentPath,
                    fontSize = 14.sp,
                    color = MaterialTheme.colorScheme.primary,
                    fontWeight = FontWeight.Medium
                )
            }
        }
        Spacer(modifier = Modifier.padding(8.dp))

        if (entries.isEmpty()) {
            Box(
                modifier = Modifier.fillMaxSize().padding(32.dp)
            ) {
                Column(
                    verticalArrangement = Arrangement.Center,
                    horizontalAlignment = Alignment.CenterHorizontally
                ) {
                    Icon(
                        imageVector = Icons.Default.Inbox,
                        contentDescription = null,
                        tint = MaterialTheme.colorScheme.onSurfaceVariant.copy(alpha = 0.5f),
                        modifier = Modifier.size(48.dp)
                    )
                    Spacer(modifier = Modifier.padding(16.dp))
                    Text(
                        text = "This directory is empty",
                        color = MaterialTheme.colorScheme.onSurfaceVariant
                    )
                }
            }
        } else {
            LazyColumn(
                modifier = Modifier.fillMaxSize().padding(horizontal = 8.dp)
            ) {
                items(entries) { entry ->
                    ArchiveEntryItem(
                        entry = entry,
                        onClick = { onEntryClick(entry) },
                        onLongClick = { onLongClick(entry) }
                    )
                }
            }
        }
    }
}

@Composable
fun ArchiveEntryItem(
    entry: ArchiveEntry,
    onClick: () -> Unit,
    onLongClick: () -> Unit
) {
    val icon = when {
        entry.isDirectory -> Icons.Default.Folder
        entry.name.lowercase().endsWith(".nca") -> Icons.Default.Storage
        entry.name.lowercase().endsWith(".nso") -> Icons.Default.Code
        entry.name.lowercase().endsWith(".nro") -> Icons.Default.Code
        entry.name.lowercase().endsWith(".png") || entry.name.lowercase().endsWith(".jpg") -> Icons.Default.Image
        else -> Icons.Default.Description
    }

    Card(
        modifier = Modifier
            .fillMaxWidth()
            .padding(vertical = 4.dp)
            .clickable(onClick = onClick)
            .combinedClickable(
                onLongClick = onLongClick,
                onClick = onClick
            ),
        colors = CardDefaults.cardColors(
            containerColor = MaterialTheme.colorScheme.surfaceContainerLowest
        )
    ) {
        Row(
            modifier = Modifier.padding(16.dp),
            horizontalArrangement = Arrangement.spacedBy(16.dp)
        ) {
            Icon(
                imageVector = icon,
                contentDescription = null,
                tint = MaterialTheme.colorScheme.primary,
                modifier = Modifier.size(24.dp)
            )
            Column(modifier = Modifier.weight(1f)) {
                Text(
                    text = entry.name,
                    fontSize = 16.sp,
                    maxLines = 1,
                    overflow = TextOverflow.Ellipsis
                )
                if (!entry.isDirectory) {
                    Text(
                        text = formatFileSize(entry.size),
                        fontSize = 12.sp,
                        color = MaterialTheme.colorScheme.onSurfaceVariant
                    )
                }
            }
            if (entry.isDirectory) {
                Icon(
                    imageVector = Icons.Default.ChevronRight,
                    contentDescription = null,
                    tint = MaterialTheme.colorScheme.onSurfaceVariant
                )
            }
        }
    }
}

@Composable
fun FileOptionsDialog(
    entry: ArchiveEntry,
    onExtract: () -> Unit,
    onViewHex: () -> Unit,
    onDismiss: () -> Unit
) {
    AlertDialog(
        onDismissRequest = onDismiss,
        title = { Text(text = entry.name) },
        text = { Text(text = "Choose an action") },
        confirmButton = {
            TextButton(onClick = { onExtract(); onDismiss() }) {
                Text(text = "Extract")
            }
        },
        dismissButton = {
            TextButton(onClick = { onViewHex(); onDismiss() }) {
                Text(text = "View Hex")
            }
        }
    )
}

fun formatFileSize(bytes: Long): String {
    return when {
        bytes < 1024 -> "$bytes B"
        bytes < 1024 * 1024 -> String.format("%.1f KB", bytes / 1024.0)
        bytes < 1024 * 1024 * 1024 -> String.format("%.1f MB", bytes / (1024.0 * 1024.0))
        else -> String.format("%.1f GB", bytes / (1024.0 * 1024.0 * 1024.0))
    }
}

data class ArchiveEntry(
    val fileId: Long,
    val name: String,
    val isDirectory: Boolean,
    val size: Long
)