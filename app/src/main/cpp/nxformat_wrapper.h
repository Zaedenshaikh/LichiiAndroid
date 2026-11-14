#pragma once

#include <jni.h>
#include <string>
#include <vector>
#include <memory>
#include <mutex>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>

// Forward declarations for minimal nxformat types
namespace nx {
    enum class ErrorCode : int {
        ok = 0,
        io = 1,
        format = 2,
        bounds = 3,
        unsupported = 4,
        integrity = 5,
        resource_limit = 6,
        decompression = 7,
        cancelled = 8,
    };

    struct ResultBase {
        ErrorCode error_code;
        std::string error_message;
        
        bool ok() const { return error_code == ErrorCode::ok; }
        operator bool() const { return ok(); }
    };

    template<typename T>
    struct Result : ResultBase {
        T value;
        
        static Result<T> success(T&& v) {
            Result<T> r;
            r.error_code = ErrorCode::ok;
            r.value = std::move(v);
            return r;
        }
        
        static Result<T> failure(ErrorCode code, const std::string& msg) {
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
            size_t max_directories = 100000;
            size_t max_files = 1000000;
            size_t max_entries = 2000000;
            size_t max_blocks = 1000000;
            size_t max_block_references = 5000000;
            size_t max_manifest_bytes = 64 * 1024 * 1024;
            size_t max_index_bytes = 64 * 1024 * 1024;
            size_t max_cached_block_bytes = 1024 * 1024;
            size_t max_cached_blocks = 1024;
            bool verify_read_blocks = true;
            bool collect_read_stats = false;
            bool collect_read_timing = false;
            bool zstd = true;
            void* zstd_context = nullptr;
        };

        explicit Session(std::shared_ptr<RandomAccessSource> source, Options options = {});
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
    };

} // namespace nx

// JNI wrapper class
class NxFormatWrapper {
public:
    static NxFormatWrapper& instance() {
        static NxFormatWrapper wrapper;
        return wrapper;
    }
    
    // Initialize with asset manager for loading libnxformat.so if needed
    void init(JNIEnv* env, jobject assetManager);
    
    // Open an NX archive from a file path
    bool openArchive(const std::string& path);
    
    // Open from an Android asset
    bool openAsset(JNIEnv* env, jobject assetManager, const std::string& assetName);
    
    // Close current archive
    void closeArchive();
    
    // Check if archive is open
    bool isOpen() const;
    
    // Get root directory handle
    nx::DirectoryHandle getRootHandle() const;
    
    // List directory contents
    struct Entry {
        std::string name;
        bool isDirectory;
        uint64_t size;
        uint64_t fileId;
    };
    
    std::vector<Entry> listDirectory(nx::DirectoryHandle handle);
    
    // Get file info
    struct FileInfo {
        uint64_t id;
        uint64_t size;
        uint32_t chunkLog2;
        std::vector<uint8_t> digest;
    };
    
    std::optional<FileInfo> getFileInfo(uint64_t fileId);
    
    // Read file data
    std::vector<uint8_t> readFile(uint64_t fileId, uint64_t offset, size_t length);
    
    // Read entire file
    std::vector<uint8_t> readFile(uint64_t fileId);
    
    // Get archive metadata
    struct ArchiveInfo {
        uint64_t generation;
        uint64_t archiveSize;
        uint32_t blockCount;
        bool hasMedia;
        uint32_t mediaRecordCount;
    };
    
    std::optional<ArchiveInfo> getArchiveInfo();
    
    // Get last error
    std::string getLastError() const;

private:
    NxFormatWrapper() = default;
    ~NxFormatWrapper() = default;
    
    std::mutex mutex_;
    std::unique_ptr<nx::Session> session_;
    std::shared_ptr<nx::RandomAccessSource> source_;
    std::string lastError_;
    
    // Asset-based source for reading from APK assets
    class AssetSource : public nx::RandomAccessSource {
    public:
        AssetSource(AAsset* asset);
        ~AssetSource();
        uint64_t size() const noexcept override;
        bool read_at(uint64_t offset, uint8_t* output, size_t length) override;
    private:
        AAsset* asset_;
        off64_t assetSize_;
    };
    
    // File-based source
    class FileSource : public nx::RandomAccessSource {
    public:
        FileSource(const std::string& path);
        ~FileSource();
        uint64_t size() const noexcept override;
        bool read_at(uint64_t offset, uint8_t* output, size_t length) override;
    private:
        int fd_;
        uint64_t fileSize_;
    };
};