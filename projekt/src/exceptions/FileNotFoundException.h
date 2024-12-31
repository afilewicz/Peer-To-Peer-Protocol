#include <exception>

class FileNotFoundException : public std::exception {
public:
    const char* what() const noexcept override { return "File not found"; }
};
