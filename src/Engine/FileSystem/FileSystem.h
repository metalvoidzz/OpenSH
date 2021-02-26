#ifndef FILE_SYSTEM_H_
#define FILE_SYSTEM_H_
/**
 * Wrapper file for ghc::filesystem, because screw you, whoever
 * decided defining macros with generic names like this in a library
 * would be a good idea!
 */
#if defined(__clang__)
#if __clang_major__ > 10
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wrange-loop-construct"
#endif  // __clang_major__ > 10
#endif  // __clang__

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wuseless-cast"
#endif  // GCC

#ifdef _MSC_VER
#endif  // _MSC_VER

#include <ghc/filesystem.hpp>

#ifdef __clang__
#if __clang_major__ > 10
#pragma GCC diagnostic pop
#endif  // __clang_major__ > 10
#endif  // __clang__

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic pop
#endif  // GCC

#ifdef _MSC_VER
#endif  //_MSC_VER

#undef ERROR
#undef min
#undef max

namespace FileSystem {
std::string GetFileExtension(ghc::filesystem::path path);
std::string GetFileStem(ghc::filesystem::path path);
std::string GetFilename(ghc::filesystem::path path);

bool IsFileHidden(ghc::filesystem::path path);
bool IsFolder(ghc::filesystem::path path);
bool DoesFileExist(ghc::filesystem::path path);
void CreateFolder(ghc::filesystem::path path);

ghc::filesystem::path GetDocumentsPath();

/**
 * Get all the files in a directory and its
 * sub directories
 */
std::vector<ghc::filesystem::path> GetDirectoryRecursive(
    ghc::filesystem::path path, const std::string &extension = "",
    bool recursive = true, bool includeFolders = false);
}  // namespace FileSystem
#endif  // FILE_SYSTEM_H_
