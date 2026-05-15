#include <iterator>
#include <utility>
#include <vector>
#include <boost/container/static_vector.hpp>
#include <concepts>
#include <stack>
#include <pp_allocator.h>
#include <associative_container.h>
#include <initializer_list>
#include <optional>

#ifndef SYS_PROG_BS_PLUS_TREE_H
#define SYS_PROG_BS_PLUS_TREE_H

template <typename tkey, typename tvalue, comparator<tkey> compare = std::less<tkey>, std::size_t t = 5>
class BSP_tree final : private compare
{
public:

    using tree_data_type = std::pair<tkey, tvalue>;
    using tree_data_type_const = std::pair<const tkey, tvalue>;
    using value_type = tree_data_type_const;

private:

    static constexpr const size_t minimum_keys_in_root = 1;
    static constexpr const size_t maximum_keys_in_root = 4 * t - 1;

    static constexpr const size_t minimum_keys_in_node = 2 * t - 1;
    static constexpr const size_t maximum_keys_in_node = 3 * t - 1;

    // region comparators declaration

    inline bool compare_keys(const tkey& lhs, const tkey& rhs) const;
    inline bool compare_pairs(const tree_data_type& lhs, const tree_data_type& rhs) const;

    // endregion comparators declaration

    struct bsptree_node_base
    {
        bool _is_terminated;

        bsptree_node_base() noexcept;
        virtual ~bsptree_node_base() =default;
    };

    struct bsptree_node_term : public bsptree_node_base
    {
        bsptree_node_term* _next;
        boost::container::static_vector<tree_data_type, maximum_keys_in_root + 1> _data;
        bsptree_node_term() noexcept;

    };

    struct bsptree_node_middle : public bsptree_node_base
    {
        boost::container::static_vector<tkey, maximum_keys_in_root + 1> _keys;
        boost::container::static_vector<bsptree_node_base*, maximum_keys_in_root + 2> _pointers;
        bsptree_node_middle() noexcept;
    };

    pp_allocator<value_type> _allocator;
    bsptree_node_base* _root;
    size_t _size;

    pp_allocator<value_type> get_allocator() const noexcept;

public:

    // region constructors declaration

    explicit BSP_tree(const compare& cmp = compare(), pp_allocator<value_type> = pp_allocator<value_type>());

    explicit BSP_tree(pp_allocator<value_type> alloc, const compare& comp = compare());

    template<input_iterator_for_pair<tkey, tvalue> iterator>
    explicit BSP_tree(iterator begin, iterator end, const compare& cmp = compare(), pp_allocator<value_type> = pp_allocator<value_type>());

    BSP_tree(std::initializer_list<std::pair<tkey, tvalue>> data, const compare& cmp = compare(), pp_allocator<value_type> = pp_allocator<value_type>());

    // endregion constructors declaration

    // region five declaration

    BSP_tree(const BSP_tree& other);

    BSP_tree(BSP_tree&& other) noexcept;

    BSP_tree& operator=(const BSP_tree& other);

    BSP_tree& operator=(BSP_tree&& other) noexcept;

    ~BSP_tree() noexcept;

    // endregion five declaration

    // region iterators declaration

    class bsptree_iterator;
    class bsptree_const_iterator;

    class bsptree_iterator final
    {
        bsptree_node_term* _node;
        size_t _index;

    public:
        using value_type = tree_data_type_const;
        using reference = value_type&;
        using pointer = value_type*;
        using iterator_category = std::forward_iterator_tag;
        using difference_type = ptrdiff_t;
        using self = bsptree_iterator;

        friend class BSP_tree;
        friend class bsptree_const_iterator;

        reference operator*() const noexcept;
        pointer operator->() const noexcept;

        self& operator++();
        self operator++(int);

        bool operator==(const self& other) const noexcept;
        bool operator!=(const self& other) const noexcept;

        size_t current_node_keys_count() const noexcept;
        size_t index() const noexcept;

        explicit bsptree_iterator(bsptree_node_term* node = nullptr, size_t index = 0);

    };

    class bsptree_const_iterator final
    {
        const bsptree_node_term* _node;
        size_t _index;

    public:

        using value_type = tree_data_type_const;
        using reference = const value_type&;
        using pointer = const value_type*;
        using iterator_category = std::forward_iterator_tag;
        using difference_type = ptrdiff_t;
        using self = bsptree_const_iterator;

        friend class BSP_tree;
        friend class bsptree_iterator;

        bsptree_const_iterator(const bsptree_iterator& it) noexcept;

        reference operator*() const noexcept;
        pointer operator->() const noexcept;

        self& operator++();
        self operator++(int);

        bool operator==(const self& other) const noexcept;
        bool operator!=(const self& other) const noexcept;

        size_t current_node_keys_count() const noexcept;
        size_t index() const noexcept;

        explicit bsptree_const_iterator(const bsptree_node_term* node = nullptr, size_t index = 0);
    };

    friend class btree_iterator;
    friend class btree_const_iterator;

    // endregion iterators declaration

    // region element access declaration

    /*
     * Returns a reference to the mapped value of the element with specified key. If no such element exists, an exception of type std::out_of_range is thrown.
     */
    tvalue& at(const tkey&);
    const tvalue& at(const tkey&) const;

    /*
     * If key not exists, makes default initialization of value
     */
    tvalue& operator[](const tkey& key);
    tvalue& operator[](tkey&& key);

    // endregion element access declaration
    // region iterator begins declaration

    bsptree_iterator begin();
    bsptree_iterator end();

    bsptree_const_iterator begin() const;
    bsptree_const_iterator end() const;

    bsptree_const_iterator cbegin() const;
    bsptree_const_iterator cend() const;

    // endregion iterator begins declaration

    // region lookup declaration

    size_t size() const noexcept;
    bool empty() const noexcept;

    /*
     * Returns end() if not exist
     */

    bsptree_iterator find(const tkey& key);
    bsptree_const_iterator find(const tkey& key) const;

    bsptree_iterator lower_bound(const tkey& key);
    bsptree_const_iterator lower_bound(const tkey& key) const;

    bsptree_iterator upper_bound(const tkey& key);
    bsptree_const_iterator upper_bound(const tkey& key) const;

    bool contains(const tkey& key) const;

    // endregion lookup declaration

    // region modifiers declaration

    void clear() noexcept;

    /*
     * Does nothing if key exists, delegates to emplace.
     * Second return value is true, when inserted
     */
    std::pair<bsptree_iterator, bool> insert(const tree_data_type& data);
    std::pair<bsptree_iterator, bool> insert(tree_data_type&& data);

    template <typename ...Args>
    std::pair<bsptree_iterator, bool> emplace(Args&&... args);

    /*
     * Updates value if key exists, delegates to emplace.
     */
    bsptree_iterator insert_or_assign(const tree_data_type& data);
    bsptree_iterator insert_or_assign(tree_data_type&& data);

    template <typename ...Args>
    bsptree_iterator emplace_or_assign(Args&&... args);

    /*
     * Return iterator to node next ro removed or end() if key not exists
     */
    bsptree_iterator erase(bsptree_iterator pos);
    bsptree_iterator erase(bsptree_const_iterator pos);

    bsptree_iterator erase(bsptree_iterator beg, bsptree_iterator en);
    bsptree_iterator erase(bsptree_const_iterator beg, bsptree_const_iterator en);


    bsptree_iterator erase(const tkey& key);

    // endregion modifiers declaration

    // region helpers declaration
private:
    size_t key_index(bsptree_node_middle* node, const tkey& key) const;
    size_t key_index(bsptree_node_term* node, const tkey& key, bool strict) const;

    void merge_nodes(bsptree_node_middle* parent, size_t i);
    void erase_node(bsptree_node_middle* node, const tkey& k);

    static std::pair<bsptree_node_middle*, size_t> find_parent_recurse(bsptree_node_base* root, bsptree_node_base* child);
    std::pair<bsptree_node_middle*, size_t> find_parent_of(bsptree_node_base* child) const;
    bool borrow_leaf_right(bsptree_node_middle* parent, size_t i);
    bool borrow_leaf_left(bsptree_node_middle* parent, size_t i);
    bool borrow_internal_right(bsptree_node_middle* parent, size_t i);
    bool borrow_internal_left(bsptree_node_middle* parent, size_t i);
    void shrink_root_if_needed();
    size_t node_size(bsptree_node_base* n) const noexcept;

    bsptree_iterator bound(const tkey& key, bool strict = false);

    bool keys_equal(const tkey& a, const tkey& b) const noexcept;
    void insert_into_middle(bsptree_node_middle* parent, size_t child_idx, tkey sep, bsptree_node_base* right_child);

    void maybe_split_root();
    void split_leaf_child(bsptree_node_middle* parent, size_t child_index);
    void split_internal_child(bsptree_node_middle* parent, size_t child_index);
    void insert_nonfull(bsptree_node_base* node, tree_data_type&& data);
    void relink_leaves();
    void rebuild_separator_keys();
    void rebuild_separator_keys_recursive(bsptree_node_base* node);

    tkey subtree_first_key(bsptree_node_base* node) const;
    static void collect_leaves_inorder(bsptree_node_base* n, std::vector<bsptree_node_term*>& out);
    // endregion helpers declaration
};

template<std::input_iterator iterator, comparator<typename std::iterator_traits<iterator>::value_type::first_type> compare = std::less<typename std::iterator_traits<iterator>::value_type::first_type>,
        std::size_t t = 5, typename U>
BSP_tree(iterator begin, iterator end, const compare &cmp = compare(), pp_allocator<U> = pp_allocator<U>()) -> BSP_tree<typename std::iterator_traits<iterator>::value_type::first_type, typename std::iterator_traits<iterator>::value_type::second_type, compare, t>;

template<typename tkey, typename tvalue, comparator<tkey> compare = std::less<tkey>, std::size_t t = 5, typename U>
BSP_tree(std::initializer_list<std::pair<tkey, tvalue>> data, const compare &cmp = compare(), pp_allocator<U> = pp_allocator<U>()) -> BSP_tree<tkey, tvalue, compare, t>;

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool BSP_tree<tkey, tvalue, compare, t>::compare_pairs(const BSP_tree::tree_data_type &lhs,
                                                      const BSP_tree::tree_data_type &rhs) const
{
    return compare_keys(lhs.first, rhs.first);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool BSP_tree<tkey, tvalue, compare, t>::compare_keys(const tkey &lhs, const tkey &rhs) const
{
    return compare::operator()(lhs, rhs);
}

// region bsptree_node_base implementation

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BSP_tree<tkey, tvalue, compare, t>::bsptree_node_base::bsptree_node_base() noexcept: _is_terminated(false) {}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BSP_tree<tkey, tvalue, compare, t>::bsptree_node_term::bsptree_node_term() noexcept: _next(nullptr)
{
    this->_is_terminated = true;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BSP_tree<tkey, tvalue, compare, t>::bsptree_node_middle::bsptree_node_middle() noexcept = default;

// region BSP_tree constructor implementations

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
pp_allocator<typename BSP_tree<tkey, tvalue, compare, t>::value_type> BSP_tree<tkey, tvalue, compare, t>::
get_allocator() const noexcept
{
    return _allocator;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BSP_tree<tkey, tvalue, compare, t>::bsptree_const_iterator::bsptree_const_iterator(const bsptree_node_term *node,
    size_t index): _node(node), _index(index) {}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BSP_tree<tkey, tvalue, compare, t>::BSP_tree(const compare& cmp, pp_allocator<value_type> alloc):
compare(cmp), _allocator(alloc), _root(nullptr), _size(0) {}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BSP_tree<tkey, tvalue, compare, t>::BSP_tree(pp_allocator<value_type> alloc, const compare& cmp): BSP_tree(cmp, alloc) {}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
template<input_iterator_for_pair<tkey, tvalue> iterator>
BSP_tree<tkey, tvalue, compare, t>::BSP_tree(iterator begin, iterator end, const compare& cmp, pp_allocator<value_type> alloc):
BSP_tree(cmp, alloc)
{
    for (; begin != end; ++begin) insert(*begin);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BSP_tree<tkey, tvalue, compare, t>::BSP_tree(std::initializer_list<std::pair<tkey, tvalue>> data, const compare& cmp, pp_allocator<value_type> alloc):
BSP_tree(data.begin(), data.end(), cmp, alloc) {}

// endregion BSP_tree constructor implementations

// region BSP_tree copy and move constructors

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BSP_tree<tkey, tvalue, compare, t>::BSP_tree(const BSP_tree& other): compare(static_cast<const compare&>(other)),
_allocator(other._allocator), _root(nullptr), _size(0)
{
    try {
        for (auto it = other.cbegin(); it != other.cend(); ++it) insert({it->first, it->second});
    } catch (...) {
        clear();
        throw std::logic_error("Err while copy construct!");
    }
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BSP_tree<tkey, tvalue, compare, t>::BSP_tree(BSP_tree&& other) noexcept: compare(std::move(static_cast<compare&>(other))),
_allocator(std::move(other._allocator)), _root(std::exchange(other._root, nullptr)),
_size(std::exchange(other._size, 0)) {}

// endregion BSP_tree copy and move constructors

// region BSP_tree copy and move assignment operators

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BSP_tree<tkey, tvalue, compare, t>& BSP_tree<tkey, tvalue, compare, t>::operator=(const BSP_tree& other)
{
    if (this == &other) return *this;

    BSP_tree tmp(other);
    *this = std::move(tmp);
    return *this;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BSP_tree<tkey, tvalue, compare, t>& BSP_tree<tkey, tvalue, compare, t>::operator=(BSP_tree&& other) noexcept
{
    if (this == &other) return *this;
    BSP_tree tmp(std::move(other));
    std::swap(static_cast<compare&>(*this), static_cast<compare&>(tmp));
    std::swap(_allocator, tmp._allocator);
    std::swap(_root, tmp._root);
    std::swap(_size, tmp._size);
    return *this;
}

// endregion BSP_tree copy and move assignment operators

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BSP_tree<tkey, tvalue, compare, t>::~BSP_tree() noexcept
{
    clear();
}

// region BSP_tree iterators implementations

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BSP_tree<tkey, tvalue, compare, t>::bsptree_iterator::bsptree_iterator(bsptree_node_term* node, size_t index):
_node(node), _index(index) {}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BSP_tree<tkey, tvalue, compare, t>::bsptree_iterator::reference BSP_tree<tkey, tvalue, compare, t>::bsptree_iterator::operator*() const noexcept
{
    return *reinterpret_cast<pointer>(&_node->_data[_index]);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BSP_tree<tkey, tvalue, compare, t>::bsptree_iterator::pointer BSP_tree<tkey, tvalue, compare, t>::bsptree_iterator::operator->() const noexcept
{
    return &operator*();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BSP_tree<tkey, tvalue, compare, t>::bsptree_iterator& BSP_tree<tkey, tvalue, compare, t>::bsptree_iterator::operator++()
{
    if (!_node) return *this;

    if (_index + 1 < _node->_data.size()) {
        ++_index;
        return *this;
    }
    _node = _node->_next;
    _index = 0;
    return *this;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BSP_tree<tkey, tvalue, compare, t>::bsptree_iterator BSP_tree<tkey, tvalue, compare, t>::bsptree_iterator::operator++(int)
{
    self tmp = *this;
    ++*this;
    return tmp;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool BSP_tree<tkey, tvalue, compare, t>::bsptree_iterator::operator==(const self& other) const noexcept
{
    return _node == other._node && _index == other._index;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool BSP_tree<tkey, tvalue, compare, t>::bsptree_iterator::operator!=(const self& other) const noexcept
{
    return !(*this == other);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t BSP_tree<tkey, tvalue, compare, t>::bsptree_iterator::current_node_keys_count() const noexcept
{
    return _node ? _node->_data.size() : 0;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t BSP_tree<tkey, tvalue, compare, t>::bsptree_iterator::index() const noexcept
{
    return _index;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BSP_tree<tkey, tvalue, compare, t>::bsptree_const_iterator::bsptree_const_iterator(const bsptree_iterator& it) noexcept:
        _node(it._node), _index(it._index) {}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BSP_tree<tkey, tvalue, compare, t>::bsptree_const_iterator::reference BSP_tree<tkey, tvalue, compare, t>::bsptree_const_iterator::operator*() const noexcept
{
    return *reinterpret_cast<pointer>(&_node->_data[_index]);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BSP_tree<tkey, tvalue, compare, t>::bsptree_const_iterator::pointer BSP_tree<tkey, tvalue, compare, t>::bsptree_const_iterator::operator->() const noexcept
{
    return &operator*();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BSP_tree<tkey, tvalue, compare, t>::bsptree_const_iterator& BSP_tree<tkey, tvalue, compare, t>::bsptree_const_iterator::operator++()
{
    if (!_node) return *this;

    if (_index + 1 < _node->_data.size()) {
        ++_index;
        return *this;
    }
    _node = _node->_next;
    _index = 0;
    return *this;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BSP_tree<tkey, tvalue, compare, t>::bsptree_const_iterator BSP_tree<tkey, tvalue, compare, t>::bsptree_const_iterator::operator++(int)
{
    self tmp = *this;
    ++*this;
    return tmp;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool BSP_tree<tkey, tvalue, compare, t>::bsptree_const_iterator::operator==(const self& other) const noexcept
{
    return _node == other._node && _index == other._index;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool BSP_tree<tkey, tvalue, compare, t>::bsptree_const_iterator::operator!=(const self& other) const noexcept
{
    return !(*this == other);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t BSP_tree<tkey, tvalue, compare, t>::bsptree_const_iterator::current_node_keys_count() const noexcept
{
    return _node ? _node->_data.size() : 0;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t BSP_tree<tkey, tvalue, compare, t>::bsptree_const_iterator::index() const noexcept
{
    return _index;
}

// endregion BSP_tree iterators implementations

// region BSP_tree element access implementations

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
tvalue& BSP_tree<tkey, tvalue, compare, t>::at(const tkey& key)
{
    auto it = find(key);
    if (it == end()) throw std::out_of_range("key not found");
    return it->second;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
const tvalue& BSP_tree<tkey, tvalue, compare, t>::at(const tkey& key) const
{
    return const_cast<BSP_tree*>(this)->at(key);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
tvalue& BSP_tree<tkey, tvalue, compare, t>::operator[](const tkey& key)
{
    auto it = find(key);
    if (it == end()) it = insert({key, tvalue{}}).first;
    return it->second;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
tvalue& BSP_tree<tkey, tvalue, compare, t>::operator[](tkey&& key)
{
    auto it = find(key);
    if (it == end()) it = insert({std::move(key), tvalue{}}).first;
    return it->second;
}

// endregion BSP_tree element access implementations

// region BSP_tree iterator begins implementations

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BSP_tree<tkey, tvalue, compare, t>::bsptree_iterator BSP_tree<tkey, tvalue, compare, t>::begin()
{
    if (!_root) return end();

    auto current = _root;
    while (!current->_is_terminated) {
        current = static_cast<bsptree_node_middle*>(current)->_pointers.front();
    }
    auto leaf = static_cast<bsptree_node_term*>(current);
    if (leaf->_data.empty()) return end();
    return bsptree_iterator(leaf, 0);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BSP_tree<tkey, tvalue, compare, t>::bsptree_iterator BSP_tree<tkey, tvalue, compare, t>::end()
{
    return bsptree_iterator();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BSP_tree<tkey, tvalue, compare, t>::bsptree_const_iterator BSP_tree<tkey, tvalue, compare, t>::begin() const
{
    return bsptree_const_iterator(const_cast<BSP_tree*>(this)->begin());
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BSP_tree<tkey, tvalue, compare, t>::bsptree_const_iterator BSP_tree<tkey, tvalue, compare, t>::end() const
{
    return bsptree_const_iterator();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BSP_tree<tkey, tvalue, compare, t>::bsptree_const_iterator BSP_tree<tkey, tvalue, compare, t>::cbegin() const
{
    return bsptree_const_iterator(const_cast<BSP_tree*>(this)->begin());
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BSP_tree<tkey, tvalue, compare, t>::bsptree_const_iterator BSP_tree<tkey, tvalue, compare, t>::cend() const
{
    return bsptree_const_iterator(const_cast<BSP_tree*>(this)->end());
}

// endregion BSP_tree iterator begins implementations

// region BSP_tree lookup implementations

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t BSP_tree<tkey, tvalue, compare, t>::size() const noexcept
{
    return _size;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool BSP_tree<tkey, tvalue, compare, t>::empty() const noexcept
{
    return _size == 0;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BSP_tree<tkey, tvalue, compare, t>::bsptree_iterator BSP_tree<tkey, tvalue, compare, t>::find(const tkey& key)
{
    auto it = bound(key);
    return it != end() && !compare_keys(key, it->first) && !compare_keys(it->first, key) ? it : end();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BSP_tree<tkey, tvalue, compare, t>::bsptree_const_iterator BSP_tree<tkey, tvalue, compare, t>::find(const tkey& key) const
{
    return bsptree_const_iterator(const_cast<BSP_tree*>(this)->find(key));
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BSP_tree<tkey, tvalue, compare, t>::bsptree_iterator BSP_tree<tkey, tvalue, compare, t>::lower_bound(const tkey& key)
{
    return bound(key, false);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BSP_tree<tkey, tvalue, compare, t>::bsptree_const_iterator BSP_tree<tkey, tvalue, compare, t>::lower_bound(const tkey& key) const
{
    return bsptree_const_iterator(const_cast<BSP_tree*>(this)->lower_bound(key));
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BSP_tree<tkey, tvalue, compare, t>::bsptree_iterator BSP_tree<tkey, tvalue, compare, t>::upper_bound(const tkey& key)
{
    return bound(key, true);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BSP_tree<tkey, tvalue, compare, t>::bsptree_const_iterator BSP_tree<tkey, tvalue, compare, t>::upper_bound(const tkey& key) const
{
    return bsptree_const_iterator(const_cast<BSP_tree*>(this)->upper_bound(key));
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool BSP_tree<tkey, tvalue, compare, t>::contains(const tkey& key) const
{
    return find(key) != end();
}

// endregion BSP_tree lookup implementations

// region BSP_tree modifiers implementations

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
void BSP_tree<tkey, tvalue, compare, t>::clear() noexcept
{
    if (!_root) return;

    std::stack<bsptree_node_base*> nodes;
    nodes.push(_root);

    while (!nodes.empty()) {
        auto current = nodes.top();
        nodes.pop();

        if (!current->_is_terminated)
            for (auto child : static_cast<bsptree_node_middle*>(current)->_pointers) {
                if (child) nodes.push(child);
            }

        if (current->_is_terminated) {
            _allocator.template delete_object<bsptree_node_term>(static_cast<bsptree_node_term*>(current));
        } else {
            _allocator.template delete_object<bsptree_node_middle>(static_cast<bsptree_node_middle*>(current));
        }
    }

    _root = nullptr;
    _size = 0;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
std::pair<typename BSP_tree<tkey, tvalue, compare, t>::bsptree_iterator, bool> BSP_tree<tkey, tvalue, compare, t>::insert(const tree_data_type& data)
{
    return insert(tree_data_type(data));
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
std::pair<typename BSP_tree<tkey, tvalue, compare, t>::bsptree_iterator, bool>
BSP_tree<tkey, tvalue, compare, t>::insert(tree_data_type&& data)
{
    using leaf_t = bsptree_node_term;
    const tkey kcopy = data.first;

    if (!_root) {
        leaf_t* leaf = _allocator.template new_object<leaf_t>();
        leaf->_data.push_back(std::move(data));
        _root = leaf;
        ++_size;
        return {bsptree_iterator(leaf, 0), true};
    }

    const auto existing = find(kcopy);
    if (existing != end()) {
        return {existing, false};
    }

    insert_nonfull(_root, std::move(data));
    ++_size;

    maybe_split_root();
    rebuild_separator_keys();
    relink_leaves();
    return {find(kcopy), true};
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
template<typename ...Args>
std::pair<typename BSP_tree<tkey, tvalue, compare, t>::bsptree_iterator, bool> BSP_tree<tkey, tvalue, compare, t>::emplace(Args&&... args)
{
    return insert(tree_data_type(std::forward<Args>(args)...));
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BSP_tree<tkey, tvalue, compare, t>::bsptree_iterator BSP_tree<tkey, tvalue, compare, t>::insert_or_assign(const tree_data_type& data)
{
    return insert_or_assign(tree_data_type(data));
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BSP_tree<tkey, tvalue, compare, t>::bsptree_iterator BSP_tree<tkey, tvalue, compare, t>::insert_or_assign(tree_data_type&& data)
{
    auto it = find(data.first);
    return it != end() ? (it->second = std::move(data.second), it) : insert(std::move(data)).first;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
template<typename ...Args>
typename BSP_tree<tkey, tvalue, compare, t>::bsptree_iterator BSP_tree<tkey, tvalue, compare, t>::emplace_or_assign(Args&&... args)
{
    return insert_or_assign(tree_data_type(std::forward<Args>(args)...));
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BSP_tree<tkey, tvalue, compare, t>::bsptree_iterator BSP_tree<tkey, tvalue, compare, t>::erase(bsptree_iterator pos)
{
    if (pos == end()) return end();
    return erase(pos->first);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BSP_tree<tkey, tvalue, compare, t>::bsptree_iterator BSP_tree<tkey, tvalue, compare, t>::erase(bsptree_const_iterator pos)
{
    if (pos == cend()) return end();
    return erase(pos->first);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BSP_tree<tkey, tvalue, compare, t>::bsptree_iterator
BSP_tree<tkey, tvalue, compare, t>::erase(
    bsptree_iterator beg,
    bsptree_iterator en)
{
    if (beg == en)
        return beg;

    const bool has_end = (en != this->end());
    const tkey end_value = has_end ? en->first : tkey{};

    while (beg != this->end())
    {
        if (has_end && !compare_keys(beg->first, end_value))
            break;

        beg = erase(beg);
    }

    return has_end ? lower_bound(end_value) : this->end();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BSP_tree<tkey, tvalue, compare, t>::bsptree_iterator BSP_tree<tkey, tvalue, compare, t>::erase(bsptree_const_iterator beg, bsptree_const_iterator en)
{
    if (beg == en || beg == cend()) return end();
    return erase(lower_bound(beg->first), en == cend() ? end() : lower_bound(en->first));
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BSP_tree<tkey, tvalue, compare, t>::bsptree_iterator BSP_tree<tkey, tvalue, compare, t>::erase(const tkey& key)
{
    if (!_root) {
        return end();
    }

    const auto it = find(key);
    if (it == end()) {
        return end();
    }

    std::optional<tkey> next_key;
    const auto ub = upper_bound(key);
    if (ub != end()) {
        next_key = ub->first;
    }

    erase_node(nullptr, key);
    relink_leaves();

    if (!next_key) {
        return end();
    }
    return lower_bound(*next_key);
}

// endregion BSP_tree modifiers implementations

// region BSP_tree helpers implementations

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
void BSP_tree<tkey, tvalue, compare, t>::rebuild_separator_keys()
{
    if (!_root || _root->_is_terminated) {
        return;
    }

    rebuild_separator_keys_recursive(_root);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
void BSP_tree<tkey, tvalue, compare, t>::rebuild_separator_keys_recursive(
    bsptree_node_base* node)
{
    if (!node || node->_is_terminated) {
        return;
    }

    auto* middle = static_cast<bsptree_node_middle*>(node);

    for (auto* child : middle->_pointers) {
        rebuild_separator_keys_recursive(child);
    }

    middle->_keys.clear();

    for (size_t i = 1; i < middle->_pointers.size(); ++i) {
        middle->_keys.push_back(
            subtree_first_key(middle->_pointers[i]));
    }
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
tkey BSP_tree<tkey, tvalue, compare, t>::subtree_first_key(
    bsptree_node_base* node) const
{
    while (!node->_is_terminated) {
        node = static_cast<bsptree_node_middle*>(node)
                   ->_pointers.front();
    }

    return static_cast<bsptree_node_term*>(node)
        ->_data.front().first;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t BSP_tree<tkey, tvalue, compare, t>::key_index(bsptree_node_middle* node, const tkey& key) const
{
    size_t lo = 0;
    size_t hi = node->_keys.size();

    while (lo < hi) {
        const size_t mid = lo + (hi - lo) / 2;
        if (!compare_keys(key, node->_keys[mid])) {
            lo = mid + 1;
        } else {
            hi = mid;
        }
    }

    return lo;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t BSP_tree<tkey, tvalue, compare, t>::key_index(bsptree_node_term* node, const tkey& key, bool strict) const
{
    size_t lo = 0;
    size_t hi = node->_data.size();

    while (lo < hi) {
        const size_t mid = lo + (hi - lo) / 2;
        const auto mid_key = node->_data[mid].first;

        if (strict ? !compare_keys(key, mid_key) : compare_keys(mid_key, key)) {
            lo = mid + 1;
        } else {
            hi = mid;
        }
    }

    return lo;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
void BSP_tree<tkey, tvalue, compare, t>::merge_nodes(bsptree_node_middle* parent, size_t i)
{
    auto left_base = parent->_pointers[i];
    auto right_base = parent->_pointers[i + 1];

    if (left_base->_is_terminated) {
        auto left = static_cast<bsptree_node_term*>(left_base);
        auto right = static_cast<bsptree_node_term*>(right_base);

        for (auto& item : right->_data) left->_data.push_back(std::move(item));
        left->_next = right->_next;

        _allocator.template delete_object<bsptree_node_term>(right);
    } else {
        auto left = static_cast<bsptree_node_middle*>(left_base);
        auto right = static_cast<bsptree_node_middle*>(right_base);

        left->_keys.push_back(std::move(parent->_keys[i]));
        for (auto& key : right->_keys) left->_keys.push_back(std::move(key));
        for (auto ptr : right->_pointers) left->_pointers.push_back(ptr);

        _allocator.template delete_object<bsptree_node_middle>(right);
    }

    parent->_keys.erase(parent->_keys.begin() + i);
    parent->_pointers.erase(parent->_pointers.begin() + i + 1);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
std::pair<typename BSP_tree<tkey, tvalue, compare, t>::bsptree_node_middle*, size_t>
BSP_tree<tkey, tvalue, compare, t>::find_parent_recurse(bsptree_node_base* root, bsptree_node_base* child)
{
    if (!root || root == child || root->_is_terminated) {
        return {nullptr, 0};
    }
    auto* mid = static_cast<bsptree_node_middle*>(root);
    for (size_t j = 0; j < mid->_pointers.size(); ++j) {
        if (mid->_pointers[j] == child) {
            return {mid, j};
        }
        auto sub = find_parent_recurse(mid->_pointers[j], child);
        if (sub.first) {
            return sub;
        }
    }
    return {nullptr, 0};
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
std::pair<typename BSP_tree<tkey, tvalue, compare, t>::bsptree_node_middle*, size_t>
BSP_tree<tkey, tvalue, compare, t>::find_parent_of(bsptree_node_base* child) const
{
    return find_parent_recurse(_root, child);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t BSP_tree<tkey, tvalue, compare, t>::node_size(bsptree_node_base* n) const noexcept
{
    if (!n) {
        return 0;
    }
    if (n->_is_terminated) {
        return static_cast<bsptree_node_term*>(n)->_data.size();
    }
    return static_cast<bsptree_node_middle*>(n)->_keys.size();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool BSP_tree<tkey, tvalue, compare, t>::borrow_leaf_right(bsptree_node_middle* parent, size_t i)
{
    if (i + 1 >= parent->_pointers.size()) {
        return false;
    }
    auto* L = static_cast<bsptree_node_term*>(parent->_pointers[i]);
    auto* R = static_cast<bsptree_node_term*>(parent->_pointers[i + 1]);
    if (R->_data.size() <= minimum_keys_in_node) {
        return false;
    }
    L->_data.push_back(std::move(R->_data.front()));
    R->_data.erase(R->_data.begin());
    parent->_keys[i] = R->_data.front().first;
    return true;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool BSP_tree<tkey, tvalue, compare, t>::borrow_leaf_left(bsptree_node_middle* parent, size_t i)
{
    if (i == 0) {
        return false;
    }
    auto* L = static_cast<bsptree_node_term*>(parent->_pointers[i - 1]);
    auto* cur = static_cast<bsptree_node_term*>(parent->_pointers[i]);
    if (L->_data.size() <= minimum_keys_in_node) {
        return false;
    }
    cur->_data.insert(cur->_data.begin(), std::move(L->_data.back()));
    L->_data.pop_back();
    parent->_keys[i - 1] = cur->_data.front().first;
    return true;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool BSP_tree<tkey, tvalue, compare, t>::borrow_internal_right(bsptree_node_middle* parent, size_t i)
{
    if (i + 1 >= parent->_pointers.size()) {
        return false;
    }
    auto* z = static_cast<bsptree_node_middle*>(parent->_pointers[i]);
    auto* rs = static_cast<bsptree_node_middle*>(parent->_pointers[i + 1]);
    if (rs->_keys.size() <= minimum_keys_in_node) {
        return false;
    }
    z->_keys.push_back(std::move(parent->_keys[i]));
    parent->_keys[i] = std::move(rs->_keys.front());
    rs->_keys.erase(rs->_keys.begin());
    z->_pointers.push_back(rs->_pointers.front());
    rs->_pointers.erase(rs->_pointers.begin());
    return true;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool BSP_tree<tkey, tvalue, compare, t>::borrow_internal_left(bsptree_node_middle* parent, size_t i)
{
    if (i == 0) {
        return false;
    }
    auto* ls = static_cast<bsptree_node_middle*>(parent->_pointers[i - 1]);
    auto* z = static_cast<bsptree_node_middle*>(parent->_pointers[i]);
    if (ls->_keys.size() <= minimum_keys_in_node) {
        return false;
    }
    z->_keys.insert(z->_keys.begin(), std::move(parent->_keys[i - 1]));
    parent->_keys[i - 1] = std::move(ls->_keys.back());
    ls->_keys.pop_back();
    z->_pointers.insert(z->_pointers.begin(), ls->_pointers.back());
    ls->_pointers.pop_back();
    return true;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
void BSP_tree<tkey, tvalue, compare, t>::shrink_root_if_needed()
{
    if (!_root || _root->_is_terminated) {
        return;
    }
    auto* m = static_cast<bsptree_node_middle*>(_root);
    if (m->_pointers.size() == 1) {
        _root = m->_pointers[0];
        _allocator.template delete_object<bsptree_node_middle>(m);
    }
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
void BSP_tree<tkey, tvalue, compare, t>::erase_node(bsptree_node_middle* node, const tkey& k)
{
    (void)node;

    if (!_root) {
        return;
    }

    using leaf_t = bsptree_node_term;
    using middle_t = bsptree_node_middle;

    bsptree_node_base* cur = _root;
    while (!cur->_is_terminated) {
        auto* mid = static_cast<middle_t*>(cur);
        const size_t i = key_index(mid, k);
        cur = mid->_pointers[i];
    }

    auto* leaf = static_cast<leaf_t*>(cur);
    const size_t idx = key_index(leaf, k, false);
    if (idx >= leaf->_data.size() || !keys_equal(leaf->_data[idx].first, k)) {
        return;
    }

    leaf->_data.erase(leaf->_data.begin() + idx);
    --_size;

    if (_size == 0) {
        _allocator.template delete_object<leaf_t>(leaf);
        _root = nullptr;
        return;
    }

    cur = leaf;

    while (true) {
        const size_t sz = node_size(cur);
        const bool underflow = (cur != _root) && (sz < minimum_keys_in_node);
        if (!underflow) {
            shrink_root_if_needed();
            break;
        }

        auto pr = find_parent_of(cur);
        auto* parent = pr.first;
        const size_t ci = pr.second;
        if (!parent) {
            break;
        }

        if (cur->_is_terminated) {
            if (borrow_leaf_right(parent, ci) || borrow_leaf_left(parent, ci)) {
                break;
            }
            if (ci + 1 < parent->_pointers.size()) {
                merge_nodes(parent, ci);
            } else {
                merge_nodes(parent, ci - 1);
            }
        } else {
            if (borrow_internal_right(parent, ci) || borrow_internal_left(parent, ci)) {
                break;
            }
            if (ci + 1 < parent->_pointers.size()) {
                merge_nodes(parent, ci);
            } else {
                merge_nodes(parent, ci - 1);
            }
        }

        cur = parent;
    }

    shrink_root_if_needed();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BSP_tree<tkey, tvalue, compare, t>::bsptree_iterator
BSP_tree<tkey, tvalue, compare, t>::bound(const tkey& key, bool strict)
{
    if (!_root) return end();

    auto current = _root;

    while (!current->_is_terminated) {
        auto middle = static_cast<bsptree_node_middle*>(current);
        current = middle->_pointers[key_index(middle, key)];
    }

    auto leaf = static_cast<bsptree_node_term*>(current);
    const size_t index = key_index(leaf, key, strict);

    if (index < leaf->_data.size()) return bsptree_iterator(leaf, index);
    if (leaf->_next) return bsptree_iterator(leaf->_next, 0);
    return end();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool BSP_tree<tkey, tvalue, compare, t>::keys_equal(const tkey& a, const tkey& b) const noexcept
{
    return !compare_keys(a, b) && !compare_keys(b, a);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
void BSP_tree<tkey, tvalue, compare, t>::insert_into_middle(
    bsptree_node_middle* parent,
    size_t child_idx,
    tkey sep,
    bsptree_node_base* right_child)
{
    parent->_keys.insert(parent->_keys.begin() + child_idx, std::move(sep));
    parent->_pointers.insert(parent->_pointers.begin() + child_idx + 1, right_child);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
void BSP_tree<tkey, tvalue, compare, t>::maybe_split_root()
{
    if (!_root) return;

    if (!_root->_is_terminated) {
        const auto* mid = static_cast<bsptree_node_middle*>(_root);
        if (mid->_keys.size() <= maximum_keys_in_root) {
            return;
        }
    } else {
        const auto* leaf = static_cast<bsptree_node_term*>(_root);
        if (leaf->_data.size() <= maximum_keys_in_root) {
            return;
        }
    }

    auto* new_root = _allocator.template new_object<bsptree_node_middle>();
    new_root->_pointers.push_back(_root);
    _root = new_root;

    if (new_root->_pointers[0]->_is_terminated) {
        split_leaf_child(new_root, 0);
    } else {
        split_internal_child(new_root, 0);
    }
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
void BSP_tree<tkey, tvalue, compare, t>::split_leaf_child(bsptree_node_middle* parent, size_t i)
{
    auto* leaf = static_cast<bsptree_node_term*>(parent->_pointers[i]);
    auto* new_right = _allocator.template new_object<bsptree_node_term>();
    const size_t split_at = leaf->_data.size() / 2;
    for (size_t j = split_at; j < leaf->_data.size(); ++j) {
        new_right->_data.push_back(std::move(leaf->_data[j]));
    }
    leaf->_data.resize(split_at);
    new_right->_next = leaf->_next;
    leaf->_next = new_right;
    tkey sep = new_right->_data.front().first;
    insert_into_middle(parent, i, std::move(sep), new_right);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
void BSP_tree<tkey, tvalue, compare, t>::split_internal_child(bsptree_node_middle* parent, size_t i)
{
    auto* z = static_cast<bsptree_node_middle*>(parent->_pointers[i]);
    auto* y = _allocator.template new_object<bsptree_node_middle>();
    const size_t mid = z->_keys.size() / 2;
    tkey mid_key = std::move(z->_keys[mid]);
    for (size_t j = mid + 1; j < z->_keys.size(); ++j) {
        y->_keys.push_back(std::move(z->_keys[j]));
    }
    for (size_t j = mid + 1; j < z->_pointers.size(); ++j) {
        y->_pointers.push_back(z->_pointers[j]);
    }
    z->_keys.resize(mid);
    z->_pointers.resize(mid + 1);
    insert_into_middle(parent, i, std::move(mid_key), y);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
void BSP_tree<tkey, tvalue, compare, t>::insert_nonfull(bsptree_node_base* x, tree_data_type&& data)
{
    const tkey& k = data.first;

    if (x->_is_terminated) {
        auto* leaf = static_cast<bsptree_node_term*>(x);
        const size_t idx = key_index(leaf, k, false);
        leaf->_data.insert(leaf->_data.begin() + idx, std::move(data));
        return;
    }

    auto* mid = static_cast<bsptree_node_middle*>(x);
    size_t i = key_index(mid, k);
    bsptree_node_base* ch = mid->_pointers[i];
    const bool full = ch->_is_terminated
        ? static_cast<bsptree_node_term*>(ch)->_data.size() >= maximum_keys_in_node
        : static_cast<bsptree_node_middle*>(ch)->_keys.size() >= maximum_keys_in_node;

    if (full) {
        if (ch->_is_terminated) {
            split_leaf_child(mid, i);
        } else {
            split_internal_child(mid, i);
        }
        i = key_index(mid, k);
    }

    insert_nonfull(mid->_pointers[i], std::move(data));
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
void BSP_tree<tkey, tvalue, compare, t>::collect_leaves_inorder(bsptree_node_base* n, std::vector<bsptree_node_term*>& out)
{
    if (n->_is_terminated) {
        out.push_back(static_cast<bsptree_node_term*>(n));
        return;
    }
    auto* m = static_cast<bsptree_node_middle*>(n);
    for (size_t j = 0; j < m->_pointers.size(); ++j) {
        collect_leaves_inorder(m->_pointers[j], out);
    }
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
void BSP_tree<tkey, tvalue, compare, t>::relink_leaves()
{
    if (!_root) {
        return;
    }
    std::vector<bsptree_node_term*> leaves;
    leaves.reserve(_size + 1);
    collect_leaves_inorder(_root, leaves);
    for (size_t i = 0; i + 1 < leaves.size(); ++i) {
        leaves[i]->_next = leaves[i + 1];
    }
    if (!leaves.empty()) {
        leaves.back()->_next = nullptr;
    }
}

// endregion BSP_tree helpers implementations

#endif