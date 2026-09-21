#include "common/binary_search_tree.hpp"
#include <climits>
#include <cctype>
#include <fstream>
#include <iostream>
#include <stdexcept>

struct IntLess {
    bool operator()(const int& first, const int& second) const { return first < second; }
};

class CommandParser {
private:
    const char* text_;
    std::size_t position_;

public:
    explicit CommandParser(const char* text) : text_(text), position_(0) {}

    void spaces() {
        while (std::isspace(static_cast<unsigned char>(text_[position_]))) ++position_;
    }

    bool empty() {
        spaces();
        return text_[position_] == '\0';
    }

    void word(char* result, std::size_t capacity) {
        spaces();
        std::size_t size = 0;
        while (std::isalpha(static_cast<unsigned char>(text_[position_]))) {
            if (size + 1 >= capacity) throw std::invalid_argument("Command is too long");
            result[size++] = text_[position_++];
        }
        if (size == 0) throw std::invalid_argument("Command expected");
        result[size] = '\0';
    }

    int integer() {
        spaces();
        bool negative = false;
        if (text_[position_] == '+' || text_[position_] == '-') {
            negative = text_[position_] == '-';
            ++position_;
        }
        if (!std::isdigit(static_cast<unsigned char>(text_[position_])))
            throw std::invalid_argument("Integer expected");

        long long value = 0;
        const long long limit = negative ? -(static_cast<long long>(INT_MIN)) : INT_MAX;
        while (std::isdigit(static_cast<unsigned char>(text_[position_]))) {
            value = value * 10 + text_[position_] - '0';
            if (value > limit) throw std::out_of_range("Integer is out of range");
            ++position_;
        }
        return negative ? static_cast<int>(-value) : static_cast<int>(value);
    }

    void end() {
        spaces();
        if (text_[position_] != '\0') throw std::invalid_argument("Unexpected command arguments");
    }
};

bool same_text(const char* first, const char* second) {
    std::size_t position = 0;
    while (first[position] && second[position] && first[position] == second[position]) ++position;
    return first[position] == second[position];
}

template <typename TIterator>
void print_traversal(TIterator iterator) {
    while (iterator.has_next()) {
        typename BinarySearchTree<int, int, IntLess>::IteratorItem item = iterator.next();
        for (std::size_t i = 0; i < item.depth; ++i) std::cout << "  ";
        std::cout << item.key << ": " << item.value << '\n';
    }
}

void execute(const char* line, BinarySearchTree<int, int, IntLess>& tree) {
    CommandParser parser(line);
    if (parser.empty()) return;

    char command[32];
    parser.word(command, sizeof(command));
    if (same_text(command, "upsert")) {
        const int key = parser.integer();
        const int value = parser.integer();
        parser.end();
        tree.upsert(key, value);
    } else if (same_text(command, "find")) {
        const int key = parser.integer();
        parser.end();
        std::cout << key << ": " << tree.find(key) << '\n';
    } else if (same_text(command, "remove")) {
        const int key = parser.integer();
        parser.end();
        tree.remove(key);
    } else if (same_text(command, "TraversePref")) {
        parser.end();
        std::cout << "Prefix traversal:\n";
        print_traversal(tree.prefix_iterator());
    } else if (same_text(command, "TraverseInf")) {
        parser.end();
        std::cout << "Infix traversal:\n";
        print_traversal(tree.infix_iterator());
    } else if (same_text(command, "TraversePost")) {
        parser.end();
        std::cout << "Postfix traversal:\n";
        print_traversal(tree.postfix_iterator());
    } else {
        throw std::invalid_argument("Unknown command");
    }
}

void run_commands(std::istream& input, BinarySearchTree<int, int, IntLess>& tree) {
    char line[512];
    std::size_t line_number = 0;
    while (input.getline(line, sizeof(line))) {
        ++line_number;
        try {
            execute(line, tree);
        } catch (const std::exception& error) {
            std::cerr << "Line " << line_number << " error: " << error.what() << '\n';
        }
    }
    if (!input.eof()) throw std::runtime_error("A command line is too long");
}

int main(int argc, char* argv[]) {
    if (argc > 2) {
        std::cerr << "Usage: task_01 [commands_file]\n";
        return 1;
    }

    try {
        IntLess comparison;
        BinarySearchTree<int, int, IntLess> tree(comparison);
        if (argc == 2) {
            std::ifstream input(argv[1]);
            if (!input) throw std::runtime_error("Cannot open command file");
            run_commands(input, tree);
        } else {
            std::cout << "Enter commands, then press Ctrl+Z and Enter:\n";
            run_commands(std::cin, tree);
        }
    } catch (const std::exception& error) {
        std::cerr << "Error: " << error.what() << '\n';
        return 1;
    }
    return 0;
}
