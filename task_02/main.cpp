#include "common/binary_search_tree.hpp"
#include <cstddef>
#include <cctype>
#include <fstream>
#include <iostream>
#include <stdexcept>

class TextBuffer {
private:
    char* data_;
    std::size_t size_;
    std::size_t capacity_;

    void reserve(std::size_t capacity) {
        if (capacity <= capacity_) return;
        char* next = new char[capacity];
        for (std::size_t i = 0; i <= size_; ++i) next[i] = data_[i];
        delete[] data_;
        data_ = next;
        capacity_ = capacity;
    }

    void swap(TextBuffer& other) noexcept {
        char* data = data_; data_ = other.data_; other.data_ = data;
        std::size_t size = size_; size_ = other.size_; other.size_ = size;
        std::size_t capacity = capacity_; capacity_ = other.capacity_; other.capacity_ = capacity;
    }

public:
    TextBuffer() : data_(new char[32]), size_(0), capacity_(32) { data_[0] = '\0'; }
    TextBuffer(const TextBuffer& other) : data_(new char[other.capacity_]), size_(other.size_), capacity_(other.capacity_) {
        for (std::size_t i = 0; i <= size_; ++i) data_[i] = other.data_[i];
    }
    TextBuffer& operator=(const TextBuffer& other) { if (this != &other) { TextBuffer copy(other); swap(copy); } return *this; }
    ~TextBuffer() { delete[] data_; }

    void push(char symbol) {
        if (size_ + 1 >= capacity_) reserve(capacity_ * 2);
        data_[size_++] = symbol;
        data_[size_] = '\0';
    }
    void clear() { size_ = 0; data_[0] = '\0'; }
    void remove_final_carriage_return() {
        if (size_ != 0 && data_[size_ - 1] == '\r') data_[--size_] = '\0';
    }
    const char* data() const { return data_; }
    std::size_t size() const { return size_; }
};

char* copy_text(const char* text, std::size_t length) {
    char* copy = new char[length + 1];
    for (std::size_t i = 0; i < length; ++i) copy[i] = text[i];
    copy[length] = '\0';
    return copy;
}

class Message {
private:
    char* username_;
    char* text_;

    void swap(Message& other) noexcept {
        char* username = username_; username_ = other.username_; other.username_ = username;
        char* text = text_; text_ = other.text_; other.text_ = text;
    }

public:
    Message(const char* username, std::size_t username_length, const char* text)
        : username_(nullptr), text_(nullptr) {
        username_ = copy_text(username, username_length);
        try {
            std::size_t text_length = 0;
            while (text[text_length]) ++text_length;
            text_ = copy_text(text, text_length);
        } catch (...) {
            delete[] username_;
            throw;
        }
    }

    Message(const Message& other) : username_(nullptr), text_(nullptr) {
        std::size_t username_length = 0;
        std::size_t text_length = 0;
        while (other.username_[username_length]) ++username_length;
        while (other.text_[text_length]) ++text_length;
        username_ = copy_text(other.username_, username_length);
        try { text_ = copy_text(other.text_, text_length); }
        catch (...) { delete[] username_; throw; }
    }

    Message& operator=(const Message& other) {
        if (this != &other) { Message copy(other); swap(copy); }
        return *this;
    }

    ~Message() { delete[] username_; delete[] text_; }
    const char* username() const { return username_; }
    const char* text() const { return text_; }
};

struct DateTime {
    int day;
    int month;
    int year;
    int hour;
    int minute;
    int second;
};

int fixed_number(const char* text, std::size_t start, std::size_t count) {
    int value = 0;
    for (std::size_t i = 0; i < count; ++i) {
        const unsigned char symbol = static_cast<unsigned char>(text[start + i]);
        if (!std::isdigit(symbol)) throw std::invalid_argument("Invalid date or time digits");
        value = value * 10 + text[start + i] - '0';
    }
    return value;
}

bool leap_year(int year) {
    return year % 400 == 0 || (year % 4 == 0 && year % 100 != 0);
}

bool latin_letter(char symbol) {
    return (symbol >= 'A' && symbol <= 'Z') || (symbol >= 'a' && symbol <= 'z');
}

DateTime parse_datetime(const char* text) {
    if (text[2] != '.' || text[5] != '.' || text[10] != ' ' || text[13] != ':' || text[16] != ':')
        throw std::invalid_argument("Date must have format dd.MM.yyyy hh:mm:ss");

    DateTime result;
    result.day = fixed_number(text, 0, 2);
    result.month = fixed_number(text, 3, 2);
    result.year = fixed_number(text, 6, 4);
    result.hour = fixed_number(text, 11, 2);
    result.minute = fixed_number(text, 14, 2);
    result.second = fixed_number(text, 17, 2);

    if (result.year < 1 || result.month < 1 || result.month > 12 ||
        result.hour < 0 || result.hour > 23 || result.minute < 0 || result.minute > 59 ||
        result.second < 0 || result.second > 59)
        throw std::invalid_argument("Date or time value is out of range");

    int month_days = 31;
    if (result.month == 4 || result.month == 6 || result.month == 9 || result.month == 11) month_days = 30;
    if (result.month == 2) month_days = leap_year(result.year) ? 29 : 28;
    if (result.day < 1 || result.day > month_days) throw std::invalid_argument("Day is out of range");
    return result;
}

int compare_datetime(const DateTime& first, const DateTime& second) {
    if (first.year != second.year) return first.year < second.year ? -1 : 1;
    if (first.month != second.month) return first.month < second.month ? -1 : 1;
    if (first.day != second.day) return first.day < second.day ? -1 : 1;
    if (first.hour != second.hour) return first.hour < second.hour ? -1 : 1;
    if (first.minute != second.minute) return first.minute < second.minute ? -1 : 1;
    if (first.second != second.second) return first.second < second.second ? -1 : 1;
    return 0;
}

struct RecordKey {
    DateTime datetime;
    std::size_t input_order;
};

struct RecordKeyLess {
    bool operator()(const RecordKey& first, const RecordKey& second) const {
        const int time_order = compare_datetime(first.datetime, second.datetime);
        if (time_order != 0) return time_order < 0;
        return first.input_order < second.input_order;
    }
};

void write_two(std::ostream& output, int value) {
    output.put(static_cast<char>('0' + value / 10));
    output.put(static_cast<char>('0' + value % 10));
}

void write_four(std::ostream& output, int value) {
    output.put(static_cast<char>('0' + (value / 1000) % 10));
    output.put(static_cast<char>('0' + (value / 100) % 10));
    output.put(static_cast<char>('0' + (value / 10) % 10));
    output.put(static_cast<char>('0' + value % 10));
}

void write_record(std::ostream& output, const RecordKey& key, const Message& message) {
    write_two(output, key.datetime.day); output.put('.');
    write_two(output, key.datetime.month); output.put('.');
    write_four(output, key.datetime.year); output.put(' ');
    write_two(output, key.datetime.hour); output.put(':');
    write_two(output, key.datetime.minute); output.put(':');
    write_two(output, key.datetime.second);
    output << message.username() << ": " << message.text() << '\n';
}

void add_record(const TextBuffer& line,
                BinarySearchTree<RecordKey, Message, RecordKeyLess>& tree,
                std::size_t input_order) {
    if (line.size() < 22) throw std::invalid_argument("Record is too short");
    const char* text = line.data();
    const DateTime datetime = parse_datetime(text);

    std::size_t colon = 19;
    while (colon < line.size() && text[colon] != ':') ++colon;
    const std::size_t username_length = colon - 19;
    if (colon == line.size() || username_length == 0 || username_length > 15)
        throw std::invalid_argument("Username must contain from 1 to 15 letters");
    for (std::size_t i = 19; i < colon; ++i)
        if (!latin_letter(text[i]))
            throw std::invalid_argument("Username must contain only Latin letters");
    if (text[colon + 1] != ' ')
        throw std::invalid_argument("A colon and a space must follow username");

    RecordKey key{datetime, input_order};
    Message message(text + 19, username_length, text + colon + 2);
    tree.upsert(key, message);
}

void load_records(const char* path, BinarySearchTree<RecordKey, Message, RecordKeyLess>& tree) {
    std::ifstream input(path, std::ios::binary);
    if (!input) throw std::runtime_error("Cannot open input file");
    TextBuffer line;
    std::size_t line_number = 0;
    char symbol = 0;
    while (input.get(symbol)) {
        if (symbol == '\n') {
            line.remove_final_carriage_return();
            ++line_number;
            if (line.size() == 0) throw std::invalid_argument("Empty record line");
            try { add_record(line, tree, line_number); }
            catch (const std::exception& error) {
                std::cerr << "Line " << line_number << " error: " << error.what() << '\n';
                throw;
            }
            line.clear();
        } else {
            line.push(symbol);
        }
    }
    if (!input.eof()) throw std::runtime_error("Cannot read input file");
    if (line.size() != 0) {
        line.remove_final_carriage_return();
        ++line_number;
        try { add_record(line, tree, line_number); }
        catch (const std::exception& error) {
            std::cerr << "Line " << line_number << " error: " << error.what() << '\n';
            throw;
        }
    }
}

void save_records(const char* path, BinarySearchTree<RecordKey, Message, RecordKeyLess>& tree) {
    std::ofstream output(path, std::ios::binary | std::ios::trunc);
    if (!output) throw std::runtime_error("Cannot open file for rewriting");
    BinarySearchTree<RecordKey, Message, RecordKeyLess>::InfixIterator iterator = tree.infix_iterator();
    while (iterator.has_next()) {
        BinarySearchTree<RecordKey, Message, RecordKeyLess>::IteratorItem item = iterator.next();
        write_record(output, item.key, item.value);
        if (!output) throw std::runtime_error("Cannot write output file");
    }
}

int main(int argc, char* argv[]) {
    if (argc != 2 || argv[1][0] == '\0') {
        std::cerr << "Usage: task_02 messages_file\n";
        return 1;
    }
    try {
        RecordKeyLess comparison;
        BinarySearchTree<RecordKey, Message, RecordKeyLess> records(comparison);
        load_records(argv[1], records);
        save_records(argv[1], records);
        std::cout << "Records were sorted chronologically\n";
    } catch (const std::exception& error) {
        std::cerr << "Error: " << error.what() << '\n';
        return 1;
    }
    return 0;
}
