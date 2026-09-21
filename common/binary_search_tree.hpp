#ifndef BINARY_SEARCH_TREE_HPP
#define BINARY_SEARCH_TREE_HPP

#include <cstddef>
#include <stdexcept>

template <typename TKey, typename TValue, typename TCompare>
class BinarySearchTree {
private:
    struct Node {
        TKey key;
        TValue value;
        Node* left;
        Node* right;

        Node(const TKey& new_key, const TValue& new_value)
            : key(new_key), value(new_value), left(nullptr), right(nullptr) {}
    };

public:
    class TreeError : public std::runtime_error {
    public:
        explicit TreeError(const char* message) : std::runtime_error(message) {}
    };

    struct IteratorItem {
        const TKey& key;
        TValue& value;
        std::size_t depth;

        IteratorItem(const TKey& item_key, TValue& item_value, std::size_t item_depth)
            : key(item_key), value(item_value), depth(item_depth) {}
    };

private:
    enum TraversalOrder { PREFIX, INFIX, POSTFIX };

    class TraversalIteratorBase {
    private:
        struct Entry {
            Node* node;
            std::size_t depth;
        };

        Entry* entries_;
        std::size_t size_;
        std::size_t position_;

        static std::size_t count_nodes(const Node* node) {
            if (!node) return 0;
            return 1 + count_nodes(node->left) + count_nodes(node->right);
        }

        static void fill(Node* node, std::size_t depth, TraversalOrder order,
                         Entry* entries, std::size_t& position) {
            if (!node) return;
            if (order == PREFIX) entries[position++] = Entry{node, depth};
            fill(node->left, depth + 1, order, entries, position);
            if (order == INFIX) entries[position++] = Entry{node, depth};
            fill(node->right, depth + 1, order, entries, position);
            if (order == POSTFIX) entries[position++] = Entry{node, depth};
        }

        void swap(TraversalIteratorBase& other) noexcept {
            Entry* entries = entries_; entries_ = other.entries_; other.entries_ = entries;
            std::size_t size = size_; size_ = other.size_; other.size_ = size;
            std::size_t position = position_; position_ = other.position_; other.position_ = position;
        }

    protected:
        TraversalIteratorBase(Node* root, TraversalOrder order)
            : entries_(nullptr), size_(count_nodes(root)), position_(0) {
            if (size_ != 0) {
                entries_ = new Entry[size_];
                std::size_t fill_position = 0;
                fill(root, 0, order, entries_, fill_position);
            }
        }

        TraversalIteratorBase(const TraversalIteratorBase& other)
            : entries_(nullptr), size_(other.size_), position_(other.position_) {
            if (size_ != 0) {
                entries_ = new Entry[size_];
                for (std::size_t i = 0; i < size_; ++i) entries_[i] = other.entries_[i];
            }
        }

        TraversalIteratorBase& operator=(const TraversalIteratorBase& other) {
            if (this != &other) { TraversalIteratorBase copy(other); swap(copy); }
            return *this;
        }

        ~TraversalIteratorBase() { delete[] entries_; }

    public:
        bool has_next() const { return position_ < size_; }

        IteratorItem next() {
            if (!has_next()) throw TreeError("Iterator has no more elements");
            Entry& entry = entries_[position_++];
            return IteratorItem(entry.node->key, entry.node->value, entry.depth);
        }
    };

public:
    class PrefixIterator : public TraversalIteratorBase {
        friend class BinarySearchTree;
        explicit PrefixIterator(Node* root) : TraversalIteratorBase(root, PREFIX) {}
    public:
        PrefixIterator(const PrefixIterator&) = default;
        PrefixIterator& operator=(const PrefixIterator&) = default;
        ~PrefixIterator() = default;
    };

    class InfixIterator : public TraversalIteratorBase {
        friend class BinarySearchTree;
        explicit InfixIterator(Node* root) : TraversalIteratorBase(root, INFIX) {}
    public:
        InfixIterator(const InfixIterator&) = default;
        InfixIterator& operator=(const InfixIterator&) = default;
        ~InfixIterator() = default;
    };

    class PostfixIterator : public TraversalIteratorBase {
        friend class BinarySearchTree;
        explicit PostfixIterator(Node* root) : TraversalIteratorBase(root, POSTFIX) {}
    public:
        PostfixIterator(const PostfixIterator&) = default;
        PostfixIterator& operator=(const PostfixIterator&) = default;
        ~PostfixIterator() = default;
    };

private:
    Node* root_;
    TCompare compare_;

    static void destroy(Node* node) noexcept {
        if (!node) return;
        destroy(node->left);
        destroy(node->right);
        delete node;
    }

    static Node* clone(const Node* node) {
        if (!node) return nullptr;
        Node* copy = new Node(node->key, node->value);
        try {
            copy->left = clone(node->left);
            copy->right = clone(node->right);
        } catch (...) {
            destroy(copy);
            throw;
        }
        return copy;
    }

    bool equal_keys(const TKey& first, const TKey& second) const {
        return !compare_(first, second) && !compare_(second, first);
    }

    void swap(BinarySearchTree& other) noexcept {
        Node* root = root_; root_ = other.root_; other.root_ = root;
        TCompare compare = compare_; compare_ = other.compare_; other.compare_ = compare;
    }

    Node* find_node(const TKey& key) const {
        Node* current = root_;
        while (current) {
            if (equal_keys(key, current->key)) return current;
            current = compare_(key, current->key) ? current->left : current->right;
        }
        return nullptr;
    }

public:
    explicit BinarySearchTree(const TCompare& compare) : root_(nullptr), compare_(compare) {}

    BinarySearchTree(const BinarySearchTree& other)
        : root_(clone(other.root_)), compare_(other.compare_) {}

    BinarySearchTree& operator=(const BinarySearchTree& other) {
        if (this != &other) { BinarySearchTree copy(other); swap(copy); }
        return *this;
    }

    ~BinarySearchTree() { destroy(root_); }

    bool empty() const { return root_ == nullptr; }

    void upsert(const TKey& key, const TValue& value) {
        Node** place = &root_;
        while (*place) {
            if (equal_keys(key, (*place)->key)) {
                (*place)->value = value;
                return;
            }
            place = compare_(key, (*place)->key) ? &((*place)->left) : &((*place)->right);
        }
        *place = new Node(key, value);
    }

    TValue& find(const TKey& key) {
        Node* node = find_node(key);
        if (!node) throw TreeError("Key was not found");
        return node->value;
    }

    const TValue& find(const TKey& key) const {
        Node* node = find_node(key);
        if (!node) throw TreeError("Key was not found");
        return node->value;
    }

    void remove(const TKey& key) {
        Node** place = &root_;
        while (*place && !equal_keys(key, (*place)->key))
            place = compare_(key, (*place)->key) ? &((*place)->left) : &((*place)->right);
        if (!*place) throw TreeError("Key was not found");

        Node* removed = *place;
        if (!removed->left) {
            *place = removed->right;
        } else if (!removed->right) {
            *place = removed->left;
        } else {
            Node** successor_place = &(removed->right);
            while ((*successor_place)->left) successor_place = &((*successor_place)->left);
            Node* successor = *successor_place;
            removed->key = successor->key;
            removed->value = successor->value;
            *successor_place = successor->right;
            delete successor;
            return;
        }
        delete removed;
    }

    PrefixIterator prefix_iterator() { return PrefixIterator(root_); }
    InfixIterator infix_iterator() { return InfixIterator(root_); }
    PostfixIterator postfix_iterator() { return PostfixIterator(root_); }
};

#endif
