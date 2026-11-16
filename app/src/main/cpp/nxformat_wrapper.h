#pragma once

#include <jni.h>
#include <string>
#include <vector>
#include <memory>
#include <mutex>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>

namespace nx {

struct ResultBase {
    int error_code = 0;
    std::string error_message;
    
    bool ok() const { return error_code == 0; }
    operator bool() const { return ok(); }
};

template<typename T>
struct Result : ResultBase {
    T value;
    
    static Result<T> success(T&& v) {
        Result<T> r;
        r.value = std::move(v);
        return r;
    }
    
    static Result<T> failure(int code, const std::string& msg) {
        Result<T> r;
        r.error_code = code;
        r.error_message = msg;
        return r;
    }
};

struct FileInfo {
    uint64_t id;
    uint64_t size;
    uint32_t chunk_log2;
    std::array<uint8_t, 32> digest;
};

struct DirectoryEntryView {
    enum class EntryType { file = 1, directory = 2 };
    EntryType type;
    std::string name;
    uint64_t file;
};

struct DirectoryHandle {
    uint64_t value;
};

struct ReadCursor {
    // Opaque cursor state
};

class RandomAccessSource {
public:
    virtual ~RandomAccessSource() = default;
    virtual uint64_t size() const noexcept = 0;
    virtual bool read_at(uint64_t offset, uint8_t* output, size_t length) = 0;
};

class Session {
public:
    struct Options {
        size_t max_directories;
        size_t max_files;
        size_t max_entries;
        size_t max_blocks;
        size_t max_block_references;
        size_t max_manifest_bytes;
        size_t max_index_bytes;
        size_t max_cached_block_bytes;
        size_t max_cached_blocks;
        bool verify_read_blocks;
        bool collect_read_stats;
        bool collect_read_timing;
        bool zstd;
        void* zstd_context;
    };

    explicit Session(std::shared_ptr<RandomAccessSource> source);
    explicit Session(std::shared_ptr<RandomAccessSource> source, Options options);
    ~Session();

    Result<FileInfo> file_info(uint64_t file_id) const;
    
    struct ReadResult { size_t bytes; };
    
    ReadResult read(uint64_t file_id, uint64_t offset, size_t length, 
                    uint8_t* output, size_t output_size, 
                    std::shared_ptr<ReadCursor> cursor);
    
    size_t max_read_bytes() const;
    
    std::optional<DirectoryEntryView> lookup(DirectoryHandle handle, std::string_view name) const;
    void enumerate(DirectoryHandle handle, std::function<bool(const DirectoryEntryView&)> callback) const;
    
    DirectoryHandle root() const;
    
    void open();
    
private:
    std::shared_ptr<RandomAccessSource> source_;
    Options options_;
};

} // namespace nx

class NxFormatWrapper {
public:
    static NxFormatWrapper& instance();
    
    void init(JNIEnv* env, jobject assetManager);
    bool openArchive(const std::string& path);
    bool openAsset(JNIEnv* env, jobject assetManager, const std::string& assetName);
    void closeArchive();
    bool isOpen() const;
    
    nx::DirectoryHandle getRootHandle() const;
    
    struct Entry {
        std::string name;
        bool isDirectory;
        uint64_t size;
        uint64_t fileId;
    };
    
    std::vector<Entry> listDirectory(nx::DirectoryHandle handle);
    
    struct FileInfo {
        uint64_t id;
        uint64_t size;
        uint32_t chunkLog2;
        std::vector<uint8_t> digest;
    };
    
    std::optional<FileInfo> getFileInfo(uint64_t fileId);
    
    std::vector<uint8_t> readFile(uint64_t fileId, uint64_t offset, size_t length);
    std::vector<uint8_t> readFile(uint64_t fileId);
    
    struct ArchiveInfo {
        uint64_t generation;
        uint64_t archiveSize;
        uint32_t blockCount;
        bool hasMedia;
        uint32_t mediaRecordCount;
    };
    
    std::optional<ArchiveInfo> getArchiveInfo();
    
    std::string getLastError() const;
    
private:
    mutable std::mutex mutex_;
    std::unique_ptr<nx::Session> session_;
    std::shared_ptr<nx::RandomAccessSource> source_;
    std::string lastError_;
};