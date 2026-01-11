#include <iostream>
#include <filesystem>
#include <string>
#include <iomanip>

namespace fs = std::filesystem;

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <directory_path>\n";
        return 1;
    }

    fs::path directory = argv[1];
    if (!fs::exists(directory) || !fs::is_directory(directory)) {
        std::cerr << "Error: Invalid directory path.\n";
        return 1;
    }

    int counter = 1;
    for (const auto& entry : fs::directory_iterator(directory)) {
        if (fs::is_regular_file(entry.path())) {
            fs::path old_path = entry.path();
            std::string extension = old_path.extension().string();

            std::ostringstream new_filename;
            new_filename << "file_" << std::setw(4) << std::setfill('0') << counter << extension;
            fs::path new_path = old_path.parent_path() / new_filename.str();

            try {
                fs::rename(old_path, new_path);
                std::cout << "Renamed: " << old_path.filename() << " -> " << new_path.filename() << '\n';
                ++counter;
            } catch (const fs::filesystem_error& e) {
                std::cerr << "Error renaming " << old_path.filename() << ": " << e.what() << '\n';
            }
        }
    }

    std::cout << "Renaming complete. Total files processed: " << counter - 1 << '\n';
    return 0;
}
#include <iostream>
#include <filesystem>
#include <vector>
#include <algorithm>
#include <iomanip>

namespace fs = std::filesystem;

void renameFilesSequentially(const fs::path& directory) {
    std::vector<fs::directory_entry> files;
    
    for (const auto& entry : fs::directory_iterator(directory)) {
        if (entry.is_regular_file()) {
            files.push_back(entry);
        }
    }
    
    std::sort(files.begin(), files.end(),
              [](const fs::directory_entry& a, const fs::directory_entry& b) {
                  return fs::last_write_time(a) < fs::last_write_time(b);
              });
    
    int counter = 1;
    for (const auto& file : files) {
        fs::path oldPath = file.path();
        fs::path extension = oldPath.extension();
        fs::path newName = directory / (std::to_string(counter) + extension.string());
        
        try {
            fs::rename(oldPath, newName);
            std::cout << "Renamed: " << oldPath.filename() << " -> " << newName.filename() << std::endl;
            ++counter;
        } catch (const fs::filesystem_error& e) {
            std::cerr << "Error renaming " << oldPath.filename() << ": " << e.what() << std::endl;
        }
    }
    
    std::cout << "Total files processed: " << counter - 1 << std::endl;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <directory_path>" << std::endl;
        return 1;
    }
    
    fs::path targetDir(argv[1]);
    
    if (!fs::exists(targetDir) || !fs::is_directory(targetDir)) {
        std::cerr << "Error: Invalid directory path." << std::endl;
        return 1;
    }
    
    renameFilesSequentially(targetDir);
    
    return 0;
}