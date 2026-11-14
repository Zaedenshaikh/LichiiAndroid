#include "nxformat_wrapper.h"
#include <android/asset_manager.h>
#include <android/log.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <cstring>

#define LOG_TAG "NxFormatWrapper"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__)

// Include the actual nxformat headers
// These would normally come from the nxformat library
// For now, we provide minimal implementations that would need to be replaced
// with the actual nxformat library when available

extern "C" {
    // Minimal Zstandard declarations - in reality these come from libzstd
    typedef struct ZSTD_DCtx_s ZSTD_DCtx;
    ZSTD_DCtx* ZSTD_createDCtx(void);
    void ZSTD_freeDCtx(ZSTD_DCtx* dctx);
    size_t ZSTD_decompressDCtx(ZSTD_DCtx* dctx, void* dst, size_t dstCapacity,
                                const void* src, size_t srcSize);
    size_t ZSTD_getFrameContentSize(const void* src, size_t srcSize);
    unsigned ZSTD_getDictID_fromFrame(const void* src, size_t srcSize);
    size_t ZSTD_findFrameCompressedSize(const void* src, size_t srcSize);
    size_t ZSTD_DCtx_setParameter(ZSTD_DCtx* dctx, int param, int value);
    int ZSTD_isError(size_t code);
    const char* ZSTD_getErrorName(size_t code);
    #define ZSTD_d_windowLogMax 100
}

// Since we can't include the actual nxformat headers (they're in the Lichii repo),
// we'll provide a simplified JNI interface that demonstrates the structure.
// The actual implementation would link against libnxformat.so

namespace nx {

// Simplified implementations for demonstration
// In production, these would come from the nxformat library

class Archive {
public:
    struct Block {
        uint64_t offset;
        uint32_t stored;
        uint32_t logical;
        uint8_t codec;
        uint32_t first_check;
    };
    
    struct File {
        uint64_t size;
        uint32_t first;
        uint32_t count;
        uint8_t chunk_log2;
        std::array<uint8_t, 32> digest;
    };
    
    struct Entry {
        uint64_t name_offset;
        uint32_t name_length;
        uint32_t target;
        uint8_t kind;
    };
    
    struct Directory {
        uint32_t first;
        uint32_t count;
    };
    
    // These would be implemented by the actual nxformat library
    Result<std::shared_ptr<RandomAccessSource>> open_source(const std::string& path);
    Status open();
    Status mount();
    DirectoryHandle root() const;
    std::optional<DirectoryEntryView> lookup(DirectoryHandle, std::string_view) const;
    void enumerate(DirectoryHandle, std::function<bool(const DirectoryEntryView&)>) const;
    Result<FileInfo> file_info(uint64_t) const;
    Result<std::vector<uint8_t>> read_media(uint32_t) const;
    struct ReadResult { size_t bytes; };
    ReadResult read(uint64_t, uint64_t, size_t, uint8_t*, size_t, std::shared_ptr<ReadCursor>);
    size_t max_read_bytes() const;
    
    uint64_t generation() const { return generation_; }
    uint64_t archive_size() const { return archive_size_; }
    uint64_t block_count() const { return block_count_; }
    bool has_media() const { return media_size_ > 0; }
    uint32_t media_record_count() const { return media_record_count_; }
    
private:
    uint64_t generation_ = 0;
    uint64_t archive_size_ = 0;
    uint64_t block_count_ = 0;
    uint64_t media_size_ = 0;
    uint32_t media_record_count_ = 0;
};

class Session {
public:
    explicit Session(std::shared_ptr<RandomAccessSource> source, Options options = {}) 
        : source_(std::move(source)), options_(options) {}
    
    ~Session() = default;
    
    const Archive& archive() const { return archive_; }
    
    Result<FileInfo> file_info(uint64_t file_id) const {
        return archive_.file_info(file_id);
    }
    
    Result<std::vector<uint8_t>> read_media(uint32_t index) const {
        return archive_.read_media(index);
    }
    
    struct ReadResult { size_t bytes; };
    
    ReadResult read(uint64_t file_id, uint64_t offset, size_t length,
                    uint8_t* output, size_t output_size,
                    std::shared_ptr<ReadCursor> cursor) {
        return archive_.read(file_id, offset, length, output, output_size, cursor);
    }
    
    size_t max_read_bytes() const { return 1024 * 1024; }
    
    std::optional<DirectoryEntryView> lookup(DirectoryHandle handle, std::string_view name) const {
        return archive_.lookup(handle, name);
    }
    
    void enumerate(DirectoryHandle handle, std::function<bool(const DirectoryEntryView&)> callback) const {
        archive_.enumerate(handle, callback);
    }
    
    DirectoryHandle root() const { return archive_.root(); }
    
    void open() {
        auto st = archive_.open();
        if (!st) throw std::runtime_error("Failed to open archive: " + std::string(st.error_message));
        st = archive_.mount();
        if (!st) throw std::runtime_error("Failed to mount archive: " + std::string(st.error_message));
    }

private:
    std::shared_ptr<RandomAccessSource> source_;
    Options options_;
    Archive archive_;
};

// Simplified RandomAccessSource implementations
class FileSource : public RandomAccessSource {
public:
    FileSource(const std::string& path) {
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
    
    uint64_t size() const noexcept override { return fileSize_; }
    
    bool read_at(uint64_t offset, uint8_t* output, size_t length) override {
        if (offset > fileSize_ || length > fileSize_ - offset) return false;
        ssize_t read = pread(fd_, output, length, static_cast<off_t>(offset));
        return read == static_cast<ssize_t>(length);
    }
    
private:
    int fd_ = -1;
    uint64_t fileSize_ = 0;
};

class AssetSource : public RandomAccessSource {
public:
    AssetSource(AAsset* asset) : asset_(asset) {
        assetSize_ = AAsset_getLength64(asset);
    }
    
    ~AssetSource() {
        if (asset_) AAsset_close(asset_);
    }
    
    uint64_t size() const noexcept override { return static_cast<uint64_t>(assetSize_); }
    
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

} // namespace nx

// NxFormatWrapper implementation
void NxFormatWrapper::init(JNIEnv* env, jobject assetManager) {
    // Store asset manager for later use if needed
    // In a real implementation, we might load libnxformat.so from assets here
    LOGD("NxFormatWrapper initialized");
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
        e.size = 0; // Would need file_info to get size
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
    
    const auto& archive = session_->archive();
    ArchiveInfo info;
    info.generation = archive.generation();
    info.archiveSize = archive.archive_size();
    info.blockCount = static_cast<uint32_t>(archive.block_count());
    info.hasMedia = archive.has_media();
    info.mediaRecordCount = archive.media_record_count();
    return info;
}

std::string NxFormatWrapper::getLastError() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return lastError_;
}