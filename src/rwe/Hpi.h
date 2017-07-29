#ifndef RWE_HPI_H
#define RWE_HPI_H

#include <cstdint>
#include <istream>
#include <vector>
#include <boost/variant.hpp>
#include <boost/optional.hpp>

namespace rwe
{
    /** The magic number at the start of the HPI header ("HAPI"). */
    static const unsigned int HpiMagicNumber = 0x49504148;

    /** The version number indicating a standard HPI file. */
    static const unsigned int HpiVersionNumber = 0x00010000;

    /** The version number indicating a saved game ("BANK"). */
    static const unsigned int HpiBankMagicNumber = 0x4B4E4142;

    /** The magic number at the start of HPI chunks ("SQSH"). */
    static const unsigned int HpiChunkMagicNumber = 0x48535153;

    class HpiException : public std::runtime_error
    {
    public:
        explicit HpiException(const char* message);
    };

#pragma pack(1) // don't pad members

    struct HpiVersion
    {
        /** Set to "HAPI". */
        uint32_t marker;

        /** Set to "BANK" if the file is a saved game. */
        uint32_t version;
    };

    struct HpiHeader
    {
        /** The size of the directory in bytes, including the directory header. */
        uint32_t directorySize;

        /** The decryption key. */
        uint32_t headerKey;

        /** Offset to the start of the directory. */
        uint32_t start;
    };

    struct HpiDirectoryData
    {
        uint32_t numberOfEntries;
        uint32_t entryListOffset;
    };

    struct HpiFileData
    {
        /** Pointer to the start of the file data. */
        uint32_t dataOffset;

        /** Size of the decompressed file in bytes. */
        uint32_t fileSize;

        /**
         * 0 for no compression, 1 for LZ77 compression, 2 for ZLib compression.
         */
        uint8_t compressionScheme;
    };

    struct HpiDirectoryEntry
    {
        /** Pointer to a null-terminated string containing the entry name. */
        uint32_t nameOffset;

        /**
         * Pointer to the data for the entry.
         * The actual data varies depending on whether this entry
         * is a file or a directory.
         */
        uint32_t dataOffset;

        /** 1 if this entry is a directory, 0 if it is a file. */
        uint8_t isDirectory;
    };

    struct HpiChunk
    {
        uint32_t marker;
        uint8_t version;
        uint8_t compressionScheme;
        uint8_t encrypted;
        uint32_t compressedSize;
        uint32_t decompressedSize;
        uint32_t checksum;
    };

#pragma pack()

    class HpiArchive
    {
    public:
        struct DirectoryEntry;
        struct File
        {
            enum class CompressionScheme
            {
                None = 0,
                LZ77,
                ZLib
            };
            CompressionScheme compressionScheme;
            std::size_t offset;
            std::size_t size;
        };
        struct Directory
        {
            std::vector<DirectoryEntry> entries;
        };
        struct DirectoryEntry
        {
            std::string name;
            boost::variant<File, Directory> data;
        };

    private:
        std::istream* stream;
        unsigned char decryptionKey;
        Directory _root;

    public:
        explicit HpiArchive(std::istream* stream);

        const Directory& root() const;

        boost::optional<const File&> findFile(const std::string& path) const;

        void extract(const File& file, char* buffer) const;

    private:
        HpiArchive::File convertFile(const HpiFileData& file);
        HpiArchive::DirectoryEntry convertDirectoryEntry(const HpiDirectoryEntry& entry, const char* buffer, std::size_t size);
        HpiArchive::Directory convertDirectory(const HpiDirectoryData& directory, const char buffer[], std::size_t size);
    };

    unsigned char transformKey(unsigned char key);
}

#endif
