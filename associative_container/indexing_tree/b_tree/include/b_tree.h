#ifndef SYS_PROG_B_TREE_H
#define SYS_PROG_B_TREE_H

// .\build\associative_container\indexing_tree\b_tree\tests\Debug\sys_prog_assctv_cntnr_indxng_tr_b_tr_tests.exe
#include <iterator>
#include <utility>
#include <boost/container/static_vector.hpp>
#include <stack>
#include <pp_allocator.h>
#include <associative_container.h>
#include <not_implemented.h>
#include <initializer_list>
#include <vector>

template <typename tkey, typename tvalue, comparator<tkey> compare = std::less<tkey>, std::size_t t = 5>
class B_tree final : private compare // EBCO
{
public:

    using tree_data_type = std::pair<tkey, tvalue>;
    using tree_data_type_const = std::pair<const tkey, tvalue>;
    using value_type = tree_data_type_const;

private:

    static constexpr const size_t minimum_keys_in_node = t - 1;
    static constexpr const size_t maximum_keys_in_node = 2 * t - 1;

    // region comparators declaration

    inline bool compare_keys(const tkey& lhs, const tkey& rhs) const;
    inline bool compare_pairs(const tree_data_type& lhs, const tree_data_type& rhs) const;
    inline bool equal(const tkey& lhs, const tkey& rhs) const { return !compare_keys(lhs, rhs) && !compare_keys(rhs, lhs);}

    // endregion comparators declaration


    struct btree_node
    {
        boost::container::static_vector<tree_data_type, maximum_keys_in_node + 1> _keys;
        boost::container::static_vector<btree_node*, maximum_keys_in_node + 2> _pointers;
        btree_node() noexcept;
    };

    pp_allocator<value_type> _allocator;
    btree_node* _root;
    size_t _size;

    pp_allocator<value_type> get_allocator() const noexcept;

public:

    // region constructors declaration
    explicit B_tree(const compare& cmp = compare(), pp_allocator<value_type> = pp_allocator<value_type>());

    explicit B_tree(pp_allocator<value_type> alloc, const compare& comp = compare());

    template<input_iterator_for_pair<tkey, tvalue> iterator>
    explicit B_tree(iterator begin, iterator end, const compare& cmp = compare(), pp_allocator<value_type> = pp_allocator<value_type>());

    B_tree(std::initializer_list<std::pair<tkey, tvalue>> data, const compare& cmp = compare(), pp_allocator<value_type> = pp_allocator<value_type>());

    // endregion constructors declaration

    // region five declaration

    B_tree(const B_tree& other);

    B_tree(B_tree&& other) noexcept;

    B_tree& operator=(const B_tree& other);

    B_tree& operator=(B_tree&& other) noexcept;

    ~B_tree() noexcept;

    // endregion five declaration

    // region iterators declaration

    class btree_iterator;
    class btree_reverse_iterator;
    class btree_const_iterator;
    class btree_const_reverse_iterator;

    class btree_iterator final
    {
        std::stack<std::pair<btree_node**, size_t>> _path;
        size_t _index;

    public:
        using value_type = tree_data_type_const;
        using reference = value_type&;
        using pointer = value_type*;
        using iterator_category = std::bidirectional_iterator_tag;
        using difference_type = ptrdiff_t;
        using self = btree_iterator;

        friend class B_tree;
        friend class btree_reverse_iterator;
        friend class btree_const_iterator;
        friend class btree_const_reverse_iterator;

        reference operator*() const noexcept;
        pointer operator->() const noexcept;

        self& operator++();
        self operator++(int);

        self& operator--();
        self operator--(int);

        bool operator==(const self& other) const noexcept;
        bool operator!=(const self& other) const noexcept;

        size_t depth() const noexcept;
        size_t current_node_keys_count() const noexcept;
        bool is_terminate_node() const noexcept;
        size_t index() const noexcept;

        explicit btree_iterator(const std::stack<std::pair<btree_node**, size_t>>& path = std::stack<std::pair<btree_node**, size_t>>(), size_t index = 0);

    };

    class btree_const_iterator final
    {
        std::stack<std::pair<btree_node* const*, size_t>> _path;
        size_t _index;

    public:

        using value_type = tree_data_type_const;
        using reference = const value_type&;
        using pointer = const value_type*;
        using iterator_category = std::bidirectional_iterator_tag;
        using difference_type = ptrdiff_t;
        using self = btree_const_iterator;

        friend class B_tree;
        friend class btree_reverse_iterator;
        friend class btree_iterator;
        friend class btree_const_reverse_iterator;

        btree_const_iterator(const btree_iterator& it) noexcept;

        reference operator*() const noexcept;
        pointer operator->() const noexcept;

        self& operator++();
        self operator++(int);

        self& operator--();
        self operator--(int);

        bool operator==(const self& other) const noexcept;
        bool operator!=(const self& other) const noexcept;

        size_t depth() const noexcept;
        size_t current_node_keys_count() const noexcept;
        bool is_terminate_node() const noexcept;
        size_t index() const noexcept;

        explicit btree_const_iterator(const std::stack<std::pair<btree_node* const*, size_t>>& path = std::stack<std::pair<btree_node* const*, size_t>>(), size_t index = 0);
    };

    class btree_reverse_iterator final
    {
        std::stack<std::pair<btree_node**, size_t>> _path;
        size_t _index;

    public:

        using value_type = tree_data_type_const;
        using reference = value_type&;
        using pointer = value_type*;
        using iterator_category = std::bidirectional_iterator_tag;
        using difference_type = ptrdiff_t;
        using self = btree_reverse_iterator;

        friend class B_tree;
        friend class btree_iterator;
        friend class btree_const_iterator;
        friend class btree_const_reverse_iterator;

        btree_reverse_iterator(const btree_iterator& it) noexcept;
        operator btree_iterator() const noexcept;

        reference operator*() const noexcept;
        pointer operator->() const noexcept;

        self& operator++();
        self operator++(int);

        self& operator--();
        self operator--(int);

        bool operator==(const self& other) const noexcept;
        bool operator!=(const self& other) const noexcept;

        size_t depth() const noexcept;
        size_t current_node_keys_count() const noexcept;
        bool is_terminate_node() const noexcept;
        size_t index() const noexcept;

        explicit btree_reverse_iterator(const std::stack<std::pair<btree_node**, size_t>>& path = std::stack<std::pair<btree_node**, size_t>>(), size_t index = 0);
    };

    class btree_const_reverse_iterator final
    {
        std::stack<std::pair<btree_node* const*, size_t>> _path;
        size_t _index;

    public:

        using value_type = tree_data_type_const;
        using reference = const value_type&;
        using pointer = const value_type*;
        using iterator_category = std::bidirectional_iterator_tag;
        using difference_type = ptrdiff_t;
        using self = btree_const_reverse_iterator;

        friend class B_tree;
        friend class btree_reverse_iterator;
        friend class btree_const_iterator;
        friend class btree_iterator;

        btree_const_reverse_iterator(const btree_reverse_iterator& it) noexcept;
        operator btree_const_iterator() const noexcept;

        reference operator*() const noexcept;
        pointer operator->() const noexcept;

        self& operator++();
        self operator++(int);

        self& operator--();
        self operator--(int);

        bool operator==(const self& other) const noexcept;
        bool operator!=(const self& other) const noexcept;

        size_t depth() const noexcept;
        size_t current_node_keys_count() const noexcept;
        bool is_terminate_node() const noexcept;
        size_t index() const noexcept;

        explicit btree_const_reverse_iterator(const std::stack<std::pair<btree_node* const*, size_t>>& path = std::stack<std::pair<btree_node* const*, size_t>>(), size_t index = 0);
    };

    friend class btree_iterator;
    friend class btree_const_iterator;
    friend class btree_reverse_iterator;
    friend class btree_const_reverse_iterator;

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

    btree_iterator begin();
    btree_iterator end();

    btree_const_iterator begin() const;
    btree_const_iterator end() const;

    btree_const_iterator cbegin() const;
    btree_const_iterator cend() const;

    btree_reverse_iterator rbegin();
    btree_reverse_iterator rend();

    btree_const_reverse_iterator rbegin() const;
    btree_const_reverse_iterator rend() const;

    btree_const_reverse_iterator crbegin() const;
    btree_const_reverse_iterator crend() const;

    // endregion iterator begins declaration

    // region lookup declaration

    size_t size() const noexcept;
    bool empty() const noexcept;

    /*
     * Returns end() if not exist
     */

    btree_iterator find(const tkey& key);
    btree_const_iterator find(const tkey& key) const;

    btree_iterator lower_bound(const tkey& key);
    btree_const_iterator lower_bound(const tkey& key) const;

    btree_iterator upper_bound(const tkey& key);
    btree_const_iterator upper_bound(const tkey& key) const;

    bool contains(const tkey& key) const;

    // endregion lookup declaration

    // region modifiers declaration

    void clear() noexcept;

    /*
     * Does nothing if key exists, delegates to emplace.
     * Second return value is true, when inserted
     */
    std::pair<btree_iterator, bool> insert(const tree_data_type& data);
    std::pair<btree_iterator, bool> insert(tree_data_type&& data);

    template <typename ...Args>
    std::pair<btree_iterator, bool> emplace(Args&&... args);

    /*
     * Updates value if key exists, delegates to emplace.
     */
    btree_iterator insert_or_assign(const tree_data_type& data);
    btree_iterator insert_or_assign(tree_data_type&& data);

    template <typename ...Args>
    btree_iterator emplace_or_assign(Args&&... args);

    /*
     * Return iterator to node next ro removed or end() if key not exists
     */
    btree_iterator erase(btree_iterator pos);
    btree_iterator erase(btree_const_iterator pos);

    btree_iterator erase(btree_iterator beg, btree_iterator en);
    btree_iterator erase(btree_const_iterator beg, btree_const_iterator en);


    btree_iterator erase(const tkey& key);

    // endregion modifiers declaration

private:
    // helper functions declaration

    // navigation hepler functions
    void find_path(const tkey& key, std::stack<std::pair<btree_node*, size_t>>& path);
    std::stack<std::pair<btree_node* const*, size_t>>
    build_extreme_path(bool go_left) const;
    size_t find_key_position(const btree_node* node, const tkey& key) const;
    template <typename T_OUT, typename T_IN>
    static std::stack<std::pair<T_OUT, size_t>> transform_path(std::stack<std::pair<T_IN, size_t>> path);
    
    // node state helper functions
    bool is_node_full(const btree_node* node) const noexcept;
    bool is_node_underfull(const btree_node* node, bool check_for_borrow = false) const noexcept;

    // insert rebalancing helper functions
    void rebalance_insert(btree_node* curr, std::stack<std::pair<btree_node*, size_t>>& path);
    void split_overflowed_node(btree_node* node, btree_node* parent, size_t index_in_parent);
    void grow_tree();
    
    // delete rebalancing helper functions
    void rebalance_delete(btree_node* node, std::stack<std::pair<btree_node*, size_t>>& path);
    bool borrow_sibling(btree_node* curr, btree_node* parent, size_t idx_in_parent);
    void merge_sibling(btree_node* curr, btree_node* parent, size_t idx_in_parent);
    void shrink_root();

    // memory management helper functions
    void swap(B_tree& other) noexcept;
    void delete_node(btree_node* node) noexcept;

    // endregion helper functions declaration
};

template<std::input_iterator iterator, comparator<typename std::iterator_traits<iterator>::value_type::first_type> compare = std::less<typename std::iterator_traits<iterator>::value_type::first_type>,
        std::size_t t = 5, typename U>
B_tree(iterator begin, iterator end, const compare &cmp = compare(), pp_allocator<U> = pp_allocator<U>()) -> B_tree<typename std::iterator_traits<iterator>::value_type::first_type, typename std::iterator_traits<iterator>::value_type::second_type, compare, t>;

template<typename tkey, typename tvalue, comparator<tkey> compare = std::less<tkey>, std::size_t t = 5, typename U>
B_tree(std::initializer_list<std::pair<tkey, tvalue>> data, const compare &cmp = compare(), pp_allocator<U> = pp_allocator<U>()) -> B_tree<tkey, tvalue, compare, t>;

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool B_tree<tkey, tvalue, compare, t>::compare_pairs(const B_tree::tree_data_type &lhs,
                                                     const B_tree::tree_data_type &rhs) const
{
    return compare_keys(lhs.first, rhs.first);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool B_tree<tkey, tvalue, compare, t>::compare_keys(const tkey &lhs, const tkey &rhs) const
{
    return compare::operator()(lhs, rhs);
}


template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
B_tree<tkey, tvalue, compare, t>::btree_node::btree_node() noexcept
{
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
pp_allocator<typename B_tree<tkey, tvalue, compare, t>::value_type> B_tree<tkey, tvalue, compare, t>::get_allocator() const noexcept
{
    return _allocator;
}

// region constructors implementation

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
B_tree<tkey, tvalue, compare, t>::B_tree(
        const compare& cmp,
        pp_allocator<value_type> alloc) : compare(cmp), _allocator(alloc), _root(nullptr), _size(0) 
{
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
B_tree<tkey, tvalue, compare, t>::B_tree(
        pp_allocator<value_type> alloc,\
        const compare& comp): _allocator(alloc), compare(comp), _root(nullptr), _size(0) 
{
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
template<input_iterator_for_pair<tkey, tvalue> iterator>
B_tree<tkey, tvalue, compare, t>::B_tree(
        iterator begin,
        iterator end,
        const compare& cmp,
        pp_allocator<value_type> alloc): compare(cmp), _allocator(alloc), _root(nullptr), _size(0)
{
    for (auto it = begin; it!=end; ++it){
        insert(*begin);
    }
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
B_tree<tkey, tvalue, compare, t>::B_tree(
        std::initializer_list<std::pair<tkey, tvalue>> data,
        const compare& cmp,
        pp_allocator<value_type> alloc):B_tree(data.begin(), data.end(), cmp, alloc)
{
}

// endregion constructors implementation

// region five implementation

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
B_tree<tkey, tvalue, compare, t>::~B_tree() noexcept
{
    clear();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
B_tree<tkey, tvalue, compare, t>::B_tree(const B_tree& other): B_tree(other.begin(), other.end(), other, other._allocator)
{
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
B_tree<tkey, tvalue, compare, t>& B_tree<tkey, tvalue, compare, t>::operator=(const B_tree& other)
{
    if (this == &other) return *this;
    B_tree tmp(other);
    swap(tmp);
    return *this;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
B_tree<tkey, tvalue, compare, t>::B_tree(B_tree&& other) noexcept: compare(std::move(other)), _allocator(std::move(other._allocator)), _root(other._root), _size(other._size)
{
    other._root = nullptr;
    other._size = 0;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
B_tree<tkey, tvalue, compare, t>& B_tree<tkey, tvalue, compare, t>::operator=(B_tree&& other) noexcept
{
    if (this == &other) return *this;
    swap(other);
    return *this;
}
// endregion five implementation

// region iterators implementation

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
B_tree<tkey, tvalue, compare, t>::btree_iterator::btree_iterator(
        const std::stack<std::pair<btree_node**, size_t>>& path, size_t index):_path(path), _index(index)
{
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_iterator::reference
B_tree<tkey, tvalue, compare, t>::btree_iterator::operator*() const noexcept
{
    auto& data = (*(_path.top().first))->_keys[_index];
    return reinterpret_cast<reference>(data);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_iterator::pointer
B_tree<tkey, tvalue, compare, t>::btree_iterator::operator->() const noexcept
{
    return &(operator*());
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_iterator&
B_tree<tkey, tvalue, compare, t>::btree_iterator::operator++()
{
    if (_path.empty()) return *this;

    auto* curr = *(_path.top().first);
    if (!is_terminate_node()){
        _path.push({&(curr->_pointers[_index + 1]), _index + 1});
        auto* next_curr = *(_path.top().first);
        while (!next_curr->_pointers.empty())
        {
            _path.push({&(next_curr->_pointers[0]), 0});
            next_curr = *(_path.top().first);
        }
        _index = 0;
    }
    else{
        if (_index + 1 < curr->_keys.size()){
            _index++;
        }
        else{
            while (true)
            {
                auto child_idx = _path.top().second;
                _path.pop();
                if (_path.empty())
                {
                    _index = 0;
                    break;
                }
                if (child_idx < (*(_path.top().first))->_keys.size())
                {
                    _index = child_idx;
                    break; 
                }
            }
        }
    }
    return *this;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_iterator
B_tree<tkey, tvalue, compare, t>::btree_iterator::operator++(int)
{
    auto tmp = *this;
    ++(*this);
    return tmp;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_iterator&
B_tree<tkey, tvalue, compare, t>::btree_iterator::operator--()
{
    if (_path.empty()) return *this; 

    auto* curr = *(_path.top().first);
    if (!is_terminate_node()) {
        _path.push({&(curr->_pointers[_index]), _index});
        auto* next_curr = *(_path.top().first);
        while (!next_curr->_pointers.empty()) {
            auto last_idx = next_curr->_pointers.size() - 1;
            _path.push({&(next_curr->_pointers[last_idx]), last_idx});
            next_curr = *(_path.top().first);
        }
        _index = next_curr->_keys.size() - 1;
    }
    else {
        if (_index > 0) {
            _index--;
        } 
        else {
            while (true)
            {
                auto child_idx = _path.top().second;
                _path.pop();
                if (_path.empty())
                {
                    _index = 0; 
                    break;
                }
                btree_node* parent = *(_path.top().first);
                if (child_idx > 0)
                {
                    _index = child_idx - 1;
                    break;
                }
            }
        }
    }
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_iterator
B_tree<tkey, tvalue, compare, t>::btree_iterator::operator--(int)
{
    auto tmp = *this;
    --(*this);
    return tmp;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool B_tree<tkey, tvalue, compare, t>::btree_iterator::operator==(const self& other) const noexcept
{
    return _index == other._index && _path == other._path;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool B_tree<tkey, tvalue, compare, t>::btree_iterator::operator!=(const self& other) const noexcept
{
    return !(*this == other);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t B_tree<tkey, tvalue, compare, t>::btree_iterator::depth() const noexcept
{
     return !_path.empty() ? _path.size() - 1 : 0;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t B_tree<tkey, tvalue, compare, t>::btree_iterator::current_node_keys_count() const noexcept
{
    return (!_path.empty()) ? (*(_path.top().first))->_keys.size() : 0;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool B_tree<tkey, tvalue, compare, t>::btree_iterator::is_terminate_node() const noexcept
{
    return !_path.empty() ? (*(_path.top().first))->_pointers.empty() : false;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t B_tree<tkey, tvalue, compare, t>::btree_iterator::index() const noexcept
{
    return _index;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
B_tree<tkey, tvalue, compare, t>::btree_const_iterator::btree_const_iterator(
        const std::stack<std::pair<btree_node* const*, size_t>>& path, size_t index): _path(path), _index(index) 
{
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
B_tree<tkey, tvalue, compare, t>::btree_const_iterator::btree_const_iterator(
        const btree_iterator& it) noexcept: _index(it._index), _path(transform_path<btree_node* const*>(it._path))
{
    if (it._path.empty()) return;
    auto tmp_path = it._path; 

    std::vector<std::pair<btree_node* const*, size_t>> buf;
    buf.reserve(tmp_path.size());

    while (!tmp_path.empty())
    {
        buf.push_back({tmp_path.top().first, tmp_path.top().second});
        tmp_path.pop();
    }
    for (auto i = buf.rbegin(); i != buf.rend(); ++i)
    {
        _path.push(*i);
    }
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_const_iterator::reference
B_tree<tkey, tvalue, compare, t>::btree_const_iterator::operator*() const noexcept
{
    auto& data = (*(_path.top().first))->_keys[_index];
    return reinterpret_cast<reference>(data);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_const_iterator::pointer
B_tree<tkey, tvalue, compare, t>::btree_const_iterator::operator->() const noexcept
{
    return &(operator*());
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_const_iterator&
B_tree<tkey, tvalue, compare, t>::btree_const_iterator::operator++()
{
    if (_path.empty()) return *this;

    auto* curr = *(_path.top().first);
    if (!is_terminate_node()){
        _path.push({&(curr->_pointers[_index + 1]), _index + 1});
        auto* next_curr = *(_path.top().first);
        while (!next_curr->_pointers.empty())
        {
            _path.push({&(next_curr->_pointers[0]), 0});
            next_curr = *(_path.top().first);
        }
        _index = 0;
    }
    else{
        if (_index + 1 < curr->_keys.size()){
            _index++;
        }
        else{
            while (true)
            {
                auto new_idx = _path.top().second;
                _path.pop();
                if (_path.empty())
                {
                    _index = 0;
                    break;
                }
                auto* parent = *(_path.top().first);
                if (new_idx < parent->_keys.size())
                {
                    _index = new_idx;
                    break; 
                }
            }
        }
    }
    return *this;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_const_iterator
B_tree<tkey, tvalue, compare, t>::btree_const_iterator::operator++(int)
{
    auto tmp = *this;
    ++(*this);
    return tmp;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_const_iterator&
B_tree<tkey, tvalue, compare, t>::btree_const_iterator::operator--()
{
    if (_path.empty()) return *this; 

    auto* curr = *(_path.top().first);
    if (!is_terminate_node()){
        _path.push({&(curr->_pointers[_index]), _index});
        auto* next_curr = *(_path.top().first);
        while (!next_curr->_pointers.empty())
        {
            auto last_idx = next_curr->_pointers.size() - 1;
            _path.push({&(next_curr->_pointers[last_idx]), last_idx});
            next_curr = *(_path.top().first);
        }
        _index = next_curr->_keys.size() - 1;
    }
    else{
        if (_index > 0)
        {
            _index--;
        }
        else{
            while (true)
            {
                auto came_from_child_idx = _path.top().second;
                _path.pop();
                if (_path.empty())
                {
                    _index = 0; 
                    break;
                }
                auto* parent = *(_path.top().first);
                if (came_from_child_idx > 0)
                {
                    _index = came_from_child_idx - 1;
                    break;
                }
            }
        }
    }
    return *this;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_const_iterator
B_tree<tkey, tvalue, compare, t>::btree_const_iterator::operator--(int)
{
    auto tmp = *this;
    --(*this);
    return tmp;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool B_tree<tkey, tvalue, compare, t>::btree_const_iterator::operator==(const self& other) const noexcept
{
    return _index == other._index && _path == other._path;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool B_tree<tkey, tvalue, compare, t>::btree_const_iterator::operator!=(const self& other) const noexcept
{
    return !(*this == other);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t B_tree<tkey, tvalue, compare, t>::btree_const_iterator::depth() const noexcept
{
    return !_path.empty() ? _path.size() - 1 : 0;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t B_tree<tkey, tvalue, compare, t>::btree_const_iterator::current_node_keys_count() const noexcept
{
    return !_path.empty() ? (*(_path.top().first))->_keys.size() : 0;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool B_tree<tkey, tvalue, compare, t>::btree_const_iterator::is_terminate_node() const noexcept
{
    return !_path.empty() ? (*(_path.top().first))->_pointers.empty() : false;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t B_tree<tkey, tvalue, compare, t>::btree_const_iterator::index() const noexcept
{
    return _index;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator::btree_reverse_iterator(
        const std::stack<std::pair<btree_node**, size_t>>& path, size_t index): _path(path), _index(index) 
{
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator::btree_reverse_iterator(
        const btree_iterator& it) noexcept: _path(it._path), _index(it._index)
{
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator::operator B_tree<tkey, tvalue, compare, t>::btree_iterator() const noexcept
{
    return btree_iterator(_path, _index);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator::reference
B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator::operator*() const noexcept
{
    return *btree_iterator(*this);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator::pointer
B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator::operator->() const noexcept
{
    return &(operator*());
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator&
B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator::operator++()
{
    auto it = static_cast<btree_iterator>(*this);
    --it;
    _path = std::move(it._path);
    _index = it._index;
    return *this;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator
B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator::operator++(int)
{
    auto tmp = *this;
    ++(*this);
    return tmp;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator&
B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator::operator--()
{
    auto it = static_cast<btree_iterator>(*this);
    ++it;
    _path = std::move(it._path);
    _index = it._index;
    return *this;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator
B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator::operator--(int)
{
    auto tmp = *this;
    --(*this);
    return tmp;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator::operator==(const self& other) const noexcept
{
    return _index == other._index && _path == other._path;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator::operator!=(const self& other) const noexcept
{
    return !(*this == other);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator::depth() const noexcept
{
    return _path.empty() ? 0 : _path.size() - 1;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator::current_node_keys_count() const noexcept
{
    return !_path.empty() ? (*(_path.top().first))->_keys.size() : 0;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator::is_terminate_node() const noexcept
{
    return !_path.empty() ? (*(_path.top().first))->_pointers.empty() : false;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator::index() const noexcept
{
    return _index;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator::btree_const_reverse_iterator(
        const std::stack<std::pair<btree_node* const*, size_t>>& path, size_t index):_path(path), _index(index)
{
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator::btree_const_reverse_iterator(
        const btree_reverse_iterator& it) noexcept
{
    auto normal_it = static_cast<btree_iterator>(it);
    auto const_it(normal_it);
    _path = std::move(const_it._path);
    _index = const_it._index;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator::operator B_tree<tkey, tvalue, compare, t>::btree_const_iterator() const noexcept
{
    return btree_const_iterator(_path, _index);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator::reference
B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator::operator*() const noexcept
{
    return *btree_const_iterator(*this);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator::pointer
B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator::operator->() const noexcept
{
    return &(operator*());
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator&
B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator::operator++()
{
    auto it = static_cast<btree_const_iterator>(*this);
    --it;
    _path = std::move(it._path);
    _index = it._index;
    return *this;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator
B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator::operator++(int)
{
    auto tmp = *this;
    ++(*this);
    return tmp;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator&
B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator::operator--()
{
    auto it = static_cast<btree_const_iterator>(*this);
    ++it;
    _path = std::move(it._path);
    _index = it._index;
    return *this;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator
B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator::operator--(int)
{
    auto tmp = *this;
    --(*this);
    return tmp;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator::operator==(const self& other) const noexcept
{
    return _index == other._index && _path == other._path;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator::operator!=(const self& other) const noexcept
{
    return !(*this == other);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator::depth() const noexcept
{
    return _path.empty() ? 0 : _path.size() - 1;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator::current_node_keys_count() const noexcept
{
    return !_path.empty() ? (*(_path.top().first))->_keys.size() : 0;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator::is_terminate_node() const noexcept
{
    return !_path.empty() ? (*(_path.top().first))->_pointers.empty() : false;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator::index() const noexcept
{
    return _index;
}

// endregion iterators implementation

// region element access implementation

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
tvalue& B_tree<tkey, tvalue, compare, t>::at(const tkey& key)
{
    auto it = find(key);
    if (it == end()) throw std::out_of_range("key not found");
    return it->second;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
const tvalue& B_tree<tkey, tvalue, compare, t>::at(const tkey& key) const
{
    auto it = find(key);
    if (it == end()) throw std::out_of_range("key not found");
    return it->second;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
tvalue& B_tree<tkey, tvalue, compare, t>::operator[](const tkey& key)
{
    auto [it, inserted] = emplace(key, tvalue()); 
    return it->second;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
tvalue& B_tree<tkey, tvalue, compare, t>::operator[](tkey&& key)
{
    auto [it, inserted] = emplace(std::move(key), tvalue()); 
    return it->second;
}

// endregion element access implementation

// region iterator begins implementation

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_iterator B_tree<tkey, tvalue, compare, t>::begin()
{
    auto cit = static_cast<const B_tree*>(this)->cbegin();
    if (cit == end()) return rend();
    return btree_iterator(transform_path<btree_node**>(cit._path), cit._index);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_iterator B_tree<tkey, tvalue, compare, t>::end()
{
    return btree_iterator();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_const_iterator B_tree<tkey, tvalue, compare, t>::begin() const
{
    return cbegin();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_const_iterator B_tree<tkey, tvalue, compare, t>::end() const
{
    return cend();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_const_iterator B_tree<tkey, tvalue, compare, t>::cbegin() const
{
    return btree_const_iterator(build_extreme_path(true), 0);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_const_iterator B_tree<tkey, tvalue, compare, t>::cend() const
{
    return btree_const_iterator();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator B_tree<tkey, tvalue, compare, t>::rbegin()
{
    auto crit = static_cast<const B_tree*>(this)->crbegin();
    if (crit == rend()) return rend();
    return btree_reverse_iterator(transform_path<btree_node**>(crit._path), crit._index);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator B_tree<tkey, tvalue, compare, t>::rend()
{
    return btree_reverse_iterator();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator B_tree<tkey, tvalue, compare, t>::rbegin() const
{
    return crbegin();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator B_tree<tkey, tvalue, compare, t>::rend() const
{
    return crend();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator B_tree<tkey, tvalue, compare, t>::crbegin() const
{
    if (_root == nullptr) return crend();
    auto path = build_extreme_path(false);
    auto* leaf = *(path.top().first);
    return btree_const_reverse_iterator(path, leaf->_keys.size() - 1);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator B_tree<tkey, tvalue, compare, t>::crend() const
{
    return btree_const_reverse_iterator();
}


// endregion iterator begins implementation

// region lookup implementation

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t B_tree<tkey, tvalue, compare, t>::size() const noexcept
{
    return _size;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool B_tree<tkey, tvalue, compare, t>::empty() const noexcept
{
    return _size == 0;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_iterator B_tree<tkey, tvalue, compare, t>::find(const tkey& key)
{
    auto c_it = static_cast<const B_tree*>(this)->find(key);
    if (c_it == end()) return end();
    return btree_iterator(transform_path<btree_node**>(c_it._path), c_it._index);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_const_iterator B_tree<tkey, tvalue, compare, t>::find(const tkey& key) const
{
    if (_root == nullptr) return end();
    std::stack<std::pair<btree_node* const*, size_t>> path;
    path.push({&_root, 0});
    auto* curr = _root;
    while (curr)
    {
        auto left = find_key_position(curr, key);

        if (left > 0 && equal(curr->_keys[left - 1].first, key)) return btree_const_iterator(path, left - 1);

        if (curr->_pointers.empty()) break;

        path.push({&(curr->_pointers[left]), left});
        curr = curr->_pointers[left];
    }
    return end();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_iterator B_tree<tkey, tvalue, compare, t>::lower_bound(const tkey& key)
{
    auto c_it = static_cast<const B_tree*>(this)->lower_bound(key);
    if (c_it == end()) return end();
    return btree_iterator(transform_path<btree_node**>(c_it._path), c_it._index);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_const_iterator B_tree<tkey, tvalue, compare, t>::lower_bound(const tkey& key) const
{
if (_root == nullptr) return end();

    std::stack<std::pair<btree_node* const*, size_t>> path;
    std::stack<std::pair<btree_node* const*, size_t>> best_path;
    
    size_t best_idx = 0;
    bool found = false;
    
    path.push({&_root, 0});
    auto* curr = _root;

    while (curr)
    {
        auto left = find_key_position(curr, key);
        if (left > 0 && equal(curr->_keys[left - 1].first, key)) 
        {
            return btree_const_iterator(path, left - 1);
        }
        if (left < curr->_keys.size())
        {
            best_path = path;
            best_idx = left;
            found = true;
        }

        if (curr->_pointers.empty()) break;

        path.push({&(curr->_pointers[left]), left});
        curr = curr->_pointers[left];
    }
    return found ? btree_const_iterator(best_path, best_idx) : end();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_iterator B_tree<tkey, tvalue, compare, t>::upper_bound(const tkey& key)
{
    auto c_it = static_cast<const B_tree*>(this)->upper_bound(key);
    if (c_it == end()) return end();
    return btree_iterator(transform_path<btree_node**>(c_it._path), c_it._index);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_const_iterator B_tree<tkey, tvalue, compare, t>::upper_bound(const tkey& key) const
{
    auto it = lower_bound(key);
    if (it != end() && equal(it->first, key)) ++it;
    return it;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool B_tree<tkey, tvalue, compare, t>::contains(const tkey& key) const
{
    return find(key) != end();
}

// endregion lookup implementation

// region modifiers implementation

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
void B_tree<tkey, tvalue, compare, t>::clear() noexcept
{
    if (_root)
    {
        delete_node(_root);
        _root = nullptr;
        _size = 0;
    }
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
std::pair<typename B_tree<tkey, tvalue, compare, t>::btree_iterator, bool>
B_tree<tkey, tvalue, compare, t>::insert(const tree_data_type& data)
{
    return emplace(data);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
std::pair<typename B_tree<tkey, tvalue, compare, t>::btree_iterator, bool>
B_tree<tkey, tvalue, compare, t>::insert(tree_data_type&& data)
{
    return emplace(std::move(data));
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
template<typename... Args>
std::pair<typename B_tree<tkey, tvalue, compare, t>::btree_iterator, bool>
B_tree<tkey, tvalue, compare, t>::emplace(Args&&... args)
{
    tree_data_type new_data(std::forward<Args>(args)...);
    const auto& key = new_data.first;

    if (!_root) {
        _root = _allocator.template new_object<btree_node>();
        _root->_keys.push_back(std::move(new_data));
        _size = 1;
        return {begin(), true};
    }

    std::stack<std::pair<btree_node*, size_t>> path;
    find_path(key, path);
    auto leaf = path.top().first;

    auto left = find_key_position(leaf, key);
    if (left > 0 && equal(leaf->_keys[left - 1].first, key)) {
        return {find(key), false};
    }
    leaf->_keys.insert(leaf->_keys.begin() + left, std::move(new_data));
    _size++;
    if (!path.empty()) path.pop();

    rebalance_insert(leaf, path);
    return {find(key), true};
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_iterator
B_tree<tkey, tvalue, compare, t>::insert_or_assign(const tree_data_type& data)
{
    return emplace_or_assign(data);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_iterator
B_tree<tkey, tvalue, compare, t>::insert_or_assign(tree_data_type&& data)
{
    return emplace_or_assign(std::move(data));
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
template<typename... Args>
typename B_tree<tkey, tvalue, compare, t>::btree_iterator
B_tree<tkey, tvalue, compare, t>::emplace_or_assign(Args&&... args)
{
    tree_data_type new_data(std::forward<Args>(args)...);
    auto [it, inserted] = emplace(std::move(new_data));
    if (!inserted) it->second = std::move(new_data.second);
    return it;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_iterator
B_tree<tkey, tvalue, compare, t>::erase(btree_iterator pos)
{
    return erase(btree_const_iterator(pos));
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_iterator
B_tree<tkey, tvalue, compare, t>::erase(btree_const_iterator pos)
{
    if (pos == end()) return end();
    auto key = pos->first;

    std::stack<std::pair<btree_node*, size_t>> path;
    find_path(key, path);

    if (path.empty()) return end();

    auto [node, idx] = path.top();
    auto next_it = pos;
    ++next_it;
    tkey next_key;

    if (next_it != end()) next_key = next_it->first;

    if (!node->_pointers.empty()) {
        auto* next_node = node->_pointers[idx]; 
        path.push({next_node, 0});
        while (!next_node->_pointers.empty()) {
            next_node = next_node->_pointers[0];
            path.push({next_node, 0});
        }
        std::swap(node->_keys[idx - 1], next_node->_keys[0]);
        node = next_node;
        idx = 1;
    }

    node->_keys.erase(node->_keys.begin() + (idx - 1));
    _size--;
    rebalance_delete(node, path);
    return next_it != end() ? find(next_key) : end();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_iterator
B_tree<tkey, tvalue, compare, t>::erase(btree_iterator beg, btree_iterator en)
{
    return erase(btree_const_iterator(beg), btree_const_iterator(en));
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_iterator
B_tree<tkey, tvalue, compare, t>::erase(btree_const_iterator beg, btree_const_iterator en)
{
    auto curr = beg;
    while (curr != en)
    {
        curr = erase(curr);
    }
    if (curr == end()) return end();
    return btree_iterator(transform_path<btree_node**>(curr._path), curr._index);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_iterator
B_tree<tkey, tvalue, compare, t>::erase(const tkey& key)
{
    auto it = find(key);
    if (it == end()) return end();
    return erase(it);
}

// endregion modifiers implementation

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool compare_pairs(const typename B_tree<tkey, tvalue, compare, t>::tree_data_type &lhs,
                   const typename B_tree<tkey, tvalue, compare, t>::tree_data_type &rhs)
{
    return compare_keys(lhs.first, rhs.first);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool compare_keys(const tkey &lhs, const tkey &rhs)
{
    return compare::operator()(lhs, rhs);
}

// start helper functions implementation

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
void B_tree<tkey, tvalue, compare, t>::find_path(const tkey& key, std::stack<std::pair<btree_node*, size_t>>& path)
{
    auto* curr = _root;
    while (curr) {
        auto left = find_key_position(curr, key);
        path.push({curr, left});

        if (left > 0 && equal(curr->_keys[left - 1].first, key)) {
            return;
        }
        if (curr->_pointers.empty()) break;
        curr = curr->_pointers[left];
    }
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
std::stack<std::pair<typename B_tree<tkey, tvalue, compare, t>::btree_node* const*, size_t>>
B_tree<tkey, tvalue, compare, t>::build_extreme_path(bool go_left) const
{
    std::stack<std::pair<btree_node* const*, size_t>> path;
    if (_root == nullptr) return path;

    path.push({&_root, 0});
    auto* curr = _root;

    while (!curr->_pointers.empty())
    {
        size_t idx = go_left ? 0 : (curr->_pointers.size() - 1);

        path.push({&(curr->_pointers[idx]), idx});
        curr = curr->_pointers[idx];
    }

    return path;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t B_tree<tkey, tvalue, compare, t>::find_key_position(const btree_node* node, const tkey& key) const
{
    size_t left = 0;
    size_t right = node->_keys.size();
    
    while (left < right) {
        auto mid = left + (right - left) / 2;
        if (compare_keys(key, node->_keys[mid].first)){
            right = mid;
        }
        else{
            left = mid + 1;
        }
    }
    return left;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
template<typename T_OUT, typename T_IN>
std::stack<std::pair<T_OUT, size_t>> 
B_tree<tkey, tvalue, compare, t>::transform_path(std::stack<std::pair<T_IN, size_t>> path)
{
    if (path.empty()) return {};

    std::vector<std::pair<T_OUT, size_t>> buffer;
    buffer.reserve(path.size());

    while (!path.empty())
    {
        auto top = path.top();
        buffer.push_back({ const_cast<T_OUT>(top.first), top.second });
        path.pop();
    }
    std::stack<std::pair<T_OUT, size_t>> result;
    for (auto it = buffer.rbegin(); it != buffer.rend(); ++it)
    {
        result.push(*it);
    }
    return result;
}

template <typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool B_tree<tkey, tvalue, compare, t>::is_node_full(const btree_node* node) const noexcept
{
    if (node == nullptr) return false;
    return node->_keys.size() > maximum_keys_in_node;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool B_tree<tkey, tvalue, compare, t>::is_node_underfull(const btree_node* node, bool check_for_borrow) const noexcept
{
    if (node == _root) return false; 
    auto size = node->_keys.size();
    if (check_for_borrow) return size > (minimum_keys_in_node);
    return size < (minimum_keys_in_node);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
void B_tree<tkey, tvalue, compare, t>::rebalance_insert(btree_node* curr, std::stack<std::pair<btree_node*, size_t>>& path)
{
    while (is_node_full(curr)) {
        if (path.empty()) {
            grow_tree(); 
            return;
        }
        
        auto [parent, idx] = path.top();
        path.pop();
        
        split_overflowed_node(curr, parent, idx);
        curr = parent;
    }
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
void B_tree<tkey, tvalue, compare, t>::split_overflowed_node(btree_node* node, btree_node* parent, size_t index_in_parent)
{
    auto* sibling = _allocator.template new_object<btree_node>();
    
    size_t median_idx = node->_keys.size() / 2;
    auto median_pair = std::move(node->_keys[median_idx]);

    for (size_t i = median_idx + 1; i < node->_keys.size(); ++i) {
        sibling->_keys.push_back(std::move(node->_keys[i]));
    }
    if (!node->_pointers.empty()) {
        for (size_t i = median_idx + 1; i < node->_pointers.size(); ++i) {
            sibling->_pointers.push_back(node->_pointers[i]);
        }
    }

    node->_keys.erase(node->_keys.begin() + median_idx, node->_keys.end());
    if (!node->_pointers.empty()) {
        node->_pointers.erase(node->_pointers.begin() + median_idx + 1, node->_pointers.end());
    }
    parent->_keys.insert(parent->_keys.begin() + index_in_parent, std::move(median_pair));
    parent->_pointers.insert(parent->_pointers.begin() + index_in_parent + 1, sibling);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
void B_tree<tkey, tvalue, compare, t>::grow_tree()
{
    auto* new_root = _allocator.template new_object<btree_node>();
    auto* old_root = _root;
    new_root->_pointers.push_back(old_root);
    _root = new_root;
    split_overflowed_node(old_root, new_root, 0);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
void B_tree<tkey, tvalue, compare, t>::rebalance_delete(btree_node* curr, std::stack<std::pair<btree_node*, size_t>>& path)
{
    while (curr != _root && is_node_underfull(curr, false)) {
        path.pop();
        if (path.empty()) break;

        auto [parent, idx_in_parent] = path.top();
        if (borrow_sibling(curr, parent, idx_in_parent)) return;
        merge_sibling(curr, parent, idx_in_parent);
        curr = parent;
    }
    if (curr == _root) shrink_root();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool B_tree<tkey, tvalue, compare, t>::borrow_sibling(btree_node* curr, btree_node* parent, size_t idx)
{
    if (idx > 0) {
        auto* left = parent->_pointers[idx - 1];
        if (is_node_underfull(left, true)) {
            curr->_keys.insert(curr->_keys.begin(), std::move(parent->_keys[idx - 1]));
            parent->_keys[idx - 1] = std::move(left->_keys.back());
            left->_keys.pop_back();
            if (!left->_pointers.empty()) {
                curr->_pointers.insert(curr->_pointers.begin(), left->_pointers.back());
                left->_pointers.pop_back();
            }
            return true;
        }
    }
    if (idx < parent->_pointers.size() - 1) {
        auto* right = parent->_pointers[idx + 1];
        if (is_node_underfull(right, true)) {
            curr->_keys.push_back(std::move(parent->_keys[idx]));
            parent->_keys[idx] = std::move(right->_keys.front());
            right->_keys.erase(right->_keys.begin());
            if (!right->_pointers.empty()) {
                curr->_pointers.push_back(right->_pointers.front());
                right->_pointers.erase(right->_pointers.begin());
            }
            return true;
        }
    }
    return false;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
void B_tree<tkey, tvalue, compare, t>::merge_sibling(btree_node* curr, btree_node* parent, size_t idx)
{
    size_t left_idx = (idx > 0) ? idx - 1 : idx;

    auto* l_node = parent->_pointers[left_idx];
    auto* r_node = parent->_pointers[left_idx + 1];

    l_node->_keys.push_back(std::move(parent->_keys[left_idx]));
    for (auto& k : r_node->_keys){ 
        l_node->_keys.push_back(std::move(k));
    }
    for (auto* p : r_node->_pointers){ 
        l_node->_pointers.push_back(p);
    }

    parent->_keys.erase(parent->_keys.begin() + left_idx);
    parent->_pointers.erase(parent->_pointers.begin() + left_idx + 1);

    r_node->_pointers.clear();
    _allocator.template delete_object<btree_node>(r_node);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
void B_tree<tkey, tvalue, compare, t>::shrink_root()
{
    if (_root && _root->_keys.empty() && !_root->_pointers.empty()) {
        auto* old_root = _root;
        _root = _root->_pointers[0];
        old_root->_pointers.clear();
        _allocator.template delete_object<btree_node>(old_root);
    }
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
void B_tree<tkey, tvalue, compare, t>::swap(B_tree& other) noexcept
{
    std::swap(_root, other._root);
    std::swap(_size, other._size);
    std::swap(_allocator, other._allocator);
    std::swap(static_cast<compare&>(*this), static_cast<compare&>(other));
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
void B_tree<tkey, tvalue, compare, t>::delete_node(btree_node* node) noexcept
{
    if (node == nullptr) return;
    for (auto* child : node->_pointers)
    {
        delete_node(child);
    }
    _allocator.template delete_object<btree_node>(node);
}

// endregion helper functions implementation

#endif