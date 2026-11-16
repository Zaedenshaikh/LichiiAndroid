#include "nxformat_wrapper.h"
#include <android/log.h>
#include <string>
#include <vector>
#include <memory>
#include <mutex>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

#define LOG_TAG "NxFormatWrapper"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__)

namespace nx {

// AssetSource implementation
class AssetSource : public RandomAccessSource {
public:
    explicit AssetSource(AAsset* asset) : asset_(asset) {
        assetSize_ = AAsset_getLength64(asset);
    }
    
    ~AssetSource() {
        if (asset_) AAsset_close(asset_);
    }
    
    uint64_t size() const noexcept override {
        return static_cast<uint64_t>(assetSize_);
    }
    
    bool read_at(uint64_t offset, uint8_t* output, size_t length) override {
        if (offset > static_cast<uint64_t>(assetSize_) || 
            length > static_cast<uint64_t>(assetSize_) - offset) return false;
        
        off_t seek = AAsset_seek64(asset_, static_cast<off64_t>(offset), SEEK_SET);
        if (seek < 0) return false;
        
        int read = AAsset_read(asset_, output, length);
        return read == static_cast<int>(length);
    }
    
private:
    AAsset* asset_;
    off64_t assetSize_;
};

// FileSource implementation
class FileSource : public RandomAccessSource {
public:
    explicit FileSource(const std::string& path) {
        fd_ = open(path.c_str(), O_RDONLY | O_CLOEXEC);
        if (fd_ < 0) throw std::runtime_error("Failed to open file: " + path);
        
        struct stat st;
        if (fstat(fd_, &st) < 0 || !S_ISREG(st.st_mode)) {
            close(fd_);
            throw std::runtime_error("Not a regular file: " + path);
        }
        fileSize_ = static_cast<uint64_t>(st.st_size);
    }
    
    ~FileSource() {
        if (fd_ >= 0) close(fd_);
    }
    
    uint64_t size() const noexcept override {
        return fileSize_;
    }
    
    bool read_at(uint64_t offset, uint8_t* output, size_t length) override {
        if (offset > fileSize_ || length > fileSize_ - offset) return false;
        ssize_t read = pread(fd_, output, length, static_cast<off_t>(offset));
        return read == static_cast<ssize_t>(length);
    }
    
private:
    int fd_ = -1;
    uint64_t fileSize_ = 0;
};

// Session implementation
Session::Session(std::shared_ptr<RandomAccessSource> source)
    : source_(std::move(source)) {
    Options default_options;
    options_ = default_options;
}

Session::Session(std::shared_ptr<RandomAccessSource> source, Options options)
    : source_(std::move(source)), options_(options) {
}

Session::~Session() = default;

Result<FileInfo> Session::file_info(uint64_t file_id) const {
    return Result<FileInfo>::failure(4, "Not implemented - requires full nxformat library");
}

Session::ReadResult Session::read(uint64_t file_id, uint64_t offset, size_t length,
                    uint8_t* output, size_t output_size,
                    std::shared_ptr<ReadCursor> cursor) {
    return {0};
}

size_t Session::max_read_bytes() const {
    return 1024 * 1024;
}

std::optional<DirectoryEntryView> Session::lookup(DirectoryHandle handle, std::string_view name) const {
    return std::nullopt;
}

void Session::enumerate(DirectoryHandle handle, std::function<bool(const DirectoryEntryView&)> callback) const {
}

DirectoryHandle Session::root() const {
    return DirectoryHandle{0};
}

void Session::open() {
    // Stub - real implementation would parse NX format
}

} // namespace nx

// NxFormatWrapper implementation
void NxFormatWrapper::init(JNIEnv* env, jobject assetManager) {
    LOGD("NxFormatWrapper initialized (stub implementation)");
}

bool NxFormatWrapper::openArchive(const std::string& path) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    try {
        closeArchive();
        source_ = std::make_shared<nx::FileSource>(path);
        session_ = std::make_unique<nx::Session>(source_);
        session_->open();
        lastError_.clear();
        LOGD("Opened archive: %s", path.c_str());
        return true;
    } catch (const std::exception& e) {
        lastError_ = e.what();
        LOGE("Failed to open archive: %s", e.what());
        return false;
    }
}

bool NxFormatWrapper::openAsset(JNIEnv* env, jobject assetManager, const std::string& assetName) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    AAssetManager* mgr = AAssetManager_fromJava(env, assetManager);
    if (!mgr) {
        lastError_ = "Failed to get asset manager";
        return false;
    }
    
    AAsset* asset = AAssetManager_open(mgr, assetName.c_str(), AASSET_MODE_BUFFER);
    if (!asset) {
        lastError_ = "Asset not found: " + assetName;
        return false;
    }
    
    try {
        closeArchive();
        source_ = std::make_shared<nx::AssetSource>(asset);
        session_ = std::make_unique<nx::Session>(source_);
        session_->open();
        lastError_.clear();
        LOGD("Opened asset: %s", assetName.c_str());
        return true;
    } catch (const std::exception& e) {
        AAsset_close(asset);
        lastError_ = e.what();
        LOGE("Failed to open asset: %s", e.what());
        return false;
    }
}

void NxFormatWrapper::closeArchive() {
    std::lock_guard<std::mutex> lock(mutex_);
    session_.reset();
    source_.reset();
    lastError_.clear();
}

bool NxFormatWrapper::isOpen() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return session_ != nullptr;
}

nx::DirectoryHandle NxFormatWrapper::getRootHandle() const {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!session_) return nx::DirectoryHandle{0};
    return session_->root();
}

std::vector<NxFormatWrapper::Entry> NxFormatWrapper::listDirectory(nx::DirectoryHandle handle) {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<Entry> entries;
    
    if (!session_) return entries;
    
    session_->enumerate(handle, [&](const nx::DirectoryEntryView& entry) {
        Entry e;
        e.name = entry.name;
        e.isDirectory = (entry.type == nx::DirectoryEntryView::EntryType::directory);
        e.fileId = entry.file;
        e.size = 0;
        entries.push_back(std::move(e));
        return true;
    });
    
    return entries;
}

std::optional<NxFormatWrapper::FileInfo> NxFormatWrapper::getFileInfo(uint64_t fileId) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!session_) return std::nullopt;
    
    auto result = session_->file_info(fileId);
    if (!result) return std::nullopt;
    
    FileInfo info;
    info.id = result.value.id;
    info.size = result.value.size;
    info.chunkLog2 = result.value.chunk_log2;
    info.digest.assign(result.value.digest.begin(), result.value.digest.end());
    return info;
}

std::vector<uint8_t> NxFormatWrapper::readFile(uint64_t fileId, uint64_t offset, size_t length) {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<uint8_t> result;
    
    if (!session_) return result;
    
    auto fileInfo = session_->file_info(fileId);
    if (!fileInfo) return result;
    
    if (offset > fileInfo.value.size) return result;
    length = std::min(length, static_cast<size_t>(fileInfo.value.size - offset));
    
    if (length == 0) return result;
    
    result.resize(length);
    auto cursor = std::make_shared<nx::ReadCursor>();
    auto readResult = session_->read(fileId, offset, length, result.data(), length, cursor);
    
    if (!readResult) {
        result.clear();
    } else if (readResult.bytes < length) {
        result.resize(readResult.bytes);
    }
    
    return result;
}

std::vector<uint8_t> NxFormatWrapper::readFile(uint64_t fileId) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!session_) return {};
    
    auto fileInfo = session_->file_info(fileId);
    if (!fileInfo) return {};
    
    return readFile(fileId, 0, static_cast<size_t>(fileInfo.value.size));
}

std::optional<NxFormatWrapper::ArchiveInfo> NxFormatWrapper::getArchiveInfo() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!session_) return std::nullopt;
    
    // Return dummy info for now
    ArchiveInfo info;
    info.generation = 0;
    info.archiveSize = 0;
    info.blockCount = 0;
    info.hasMedia = false;
    info.mediaRecordCount = 0;
    return info;
}

std::string NxFormatWrapper::getLastError() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return lastError_;
}