
#include <iostream>
#include <filesystem>
#include <string>
#include <iomanip>
#include <vector>
#include <algorithm>

namespace fs = std::filesystem;

class FileRenamer {
public:
    FileRenamer(const std::string& directory, const std::string& prefix, int startNumber = 1)
        : targetDirectory(directory), filePrefix(prefix), counter(startNumber) {}

    bool processFiles() {
        if (!fs::exists(targetDirectory) || !fs::is_directory(targetDirectory)) {
            std::cerr << "Error: Directory does not exist or is not accessible.\n";
            return false;
        }

        std::vector<fs::directory_entry> files;
        for (const auto& entry : fs::directory_iterator(targetDirectory)) {
            if (entry.is_regular_file()) {
                files.push_back(entry);
            }
        }

        std::sort(files.begin(), files.end(),
                  [](const fs::directory_entry& a, const fs::directory_entry& b) {
                      return a.path().filename().string() < b.path().filename().string();
                  });

        for (const auto& file : files) {
            fs::path oldPath = file.path();
            std::string extension = oldPath.extension().string();

            std::ostringstream newFilename;
            newFilename << filePrefix << std::setw(4) << std::setfill('0') << counter << extension;
            fs::path newPath = oldPath.parent_path() / newFilename.str();

            try {
                fs::rename(oldPath, newPath);
                std::cout << "Renamed: " << oldPath.filename() << " -> " << newPath.filename() << "\n";
                counter++;
            } catch (const fs::filesystem_error& e) {
                std::cerr << "Failed to rename " << oldPath.filename() << ": " << e.what() << "\n";
                return false;
            }
        }

        std::cout << "Renaming completed successfully. Total files processed: " << (counter - 1) << "\n";
        return true;
    }

private:
    std::string targetDirectory;
    std::string filePrefix;
    int counter;
};

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cout << "Usage: " << argv[0] << " <directory> <prefix> [start_number]\n";
        std::cout << "Example: " << argv[0] << " ./photos vacation_ 1\n";
        return 1;
    }

    std::string directory = argv[1];
    std::string prefix = argv[2];
    int startNumber = (argc > 3) ? std::stoi(argv[3]) : 1;

    FileRenamer renamer(directory, prefix, startNumber);
    return renamer.processFiles() ? 0 : 1;
}