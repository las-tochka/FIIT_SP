#ifndef SYS_PROG_B_TREE_H
#define SYS_PROG_B_TREE_H

#include <iterator>
#include <utility>
#include <boost/container/static_vector.hpp>
#include <stack>
#include <pp_allocator.h>
#include <associative_container.h>
#include <not_implemented.h>
#include <initializer_list>

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

    // endregion comparators declaration


    struct btree_node
    {
        bool is_leaf = true;
        boost::container::static_vector<tree_data_type, maximum_keys_in_node + 1> _keys;
        boost::container::static_vector<btree_node*, maximum_keys_in_node + 2> _pointers;
        btree_node() noexcept;
        void check_invariants() const
        {
            assert(_keys.size() <= maximum_keys_in_node);

            if (!is_leaf)
                assert(_pointers.size() == _keys.size() + 1);
        }
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
        std::stack<std::pair<btree_node*, size_t>> _path;
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

        explicit btree_iterator(const std::stack<std::pair<btree_node*, size_t>>& path = std::stack<std::pair<btree_node*, size_t>>(), size_t index = 0);

    };

    class btree_const_iterator final
    {
        std::stack<std::pair<const btree_node*, size_t>> _path;
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

        explicit btree_const_iterator(const std::stack<std::pair<const btree_node*, size_t>>& path = std::stack<std::pair<const btree_node*, size_t>>(), size_t index = 0);
    };

    class btree_reverse_iterator final
    {
        std::stack<std::pair<btree_node*, size_t>> _path;
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

        explicit btree_reverse_iterator(const std::stack<std::pair<btree_node*, size_t>>& path = std::stack<std::pair<btree_node*, size_t>>(), size_t index = 0);
    };

    class btree_const_reverse_iterator final
    {
        std::stack<std::pair<const btree_node*, size_t>> _path;
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

        explicit btree_const_reverse_iterator(const std::stack<std::pair<const btree_node*, size_t>>& path = std::stack<std::pair<const btree_node*, size_t>>(), size_t index = 0);
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
    // region heplers declaration
    btree_node* clone_node(const btree_node* node);
    static void destroy_node(btree_node* node) noexcept;
    size_t find_key_index(const btree_node* node, const tkey& key) const noexcept;
    void split_child(btree_node* parent, size_t index, btree_node* child);
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
    _keys.clear();
    _pointers.clear();
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
        pp_allocator<value_type> alloc)
    : compare(cmp)
    , _allocator(alloc)
    , _root(nullptr)
    , _size(0)
{
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
B_tree<tkey, tvalue, compare, t>::B_tree(
        pp_allocator<value_type> alloc,\
        const compare& comp)
    : B_tree(comp, alloc)
{
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
template<input_iterator_for_pair<tkey, tvalue> iterator>
B_tree<tkey, tvalue, compare, t>::B_tree(
        iterator begin,
        iterator end,
        const compare& cmp,
        pp_allocator<value_type> alloc)
    : compare(cmp)
    , _allocator(alloc)
    , _root(nullptr)
    , _size(0)
{
    for (; begin != end; ++begin)
    {
        insert(*begin);
    }
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
B_tree<tkey, tvalue, compare, t>::B_tree(
        std::initializer_list<std::pair<tkey, tvalue>> data,
        const compare& cmp,
        pp_allocator<value_type> alloc)
    : compare(cmp)
    , _allocator(alloc)
    , _root(nullptr)
    , _size(0)
{
    for (const auto& el : data)
    {
        insert(el);
    }
}

// endregion constructors implementation

// region five implementation

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
B_tree<tkey, tvalue, compare, t>::~B_tree() noexcept
{
    clear();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
B_tree<tkey, tvalue, compare, t>::B_tree(const B_tree& other)
    : compare(other), _allocator(other._allocator), _root(nullptr), _size(0)
{
    if (!other._root)
        return;

    _root = clone_node(other._root);
    _size = other._size;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
B_tree<tkey, tvalue, compare, t>& B_tree<tkey, tvalue, compare, t>::operator=(const B_tree& other)
{
    if (this == &other)
        return *this;

    clear();
    compare::operator=(other);

    _root = clone_node(other._root);
    _size = other._size;

    return *this;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
B_tree<tkey, tvalue, compare, t>::B_tree(B_tree&& other) noexcept
    : compare(std::move(other)),
      _allocator(std::move(other._allocator)),
      _root(other._root),
      _size(other._size)
{
    other._root = nullptr;
    other._size = 0;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
B_tree<tkey, tvalue, compare, t>& B_tree<tkey, tvalue, compare, t>::operator=(B_tree&& other) noexcept
{
    if (this == &other)
        return *this;

    clear();

    _root = other._root;
    _size = other._size;
    _allocator = std::move(other._allocator);

    other._root = nullptr;
    other._size = 0;

    return *this;
}
// endregion five implementation

// region iterators implementation

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
B_tree<tkey, tvalue, compare, t>::btree_iterator::btree_iterator(
        const std::stack<std::pair<btree_node*, size_t>>& path, size_t index)
    : _path(path)
    , _index(index)
{

}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_iterator::reference
B_tree<tkey, tvalue, compare, t>::btree_iterator::operator*() const noexcept
{
    assert(!_path.empty());

    auto* node = _path.top().first;

    assert(node != nullptr);
    assert(_index < node->_keys.size());
    return reinterpret_cast<reference>(node->_keys[_index]);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_iterator::pointer
B_tree<tkey, tvalue, compare, t>::btree_iterator::operator->() const noexcept
{
    auto* node = _path.top().first;
    return &node->_keys[_index];
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_iterator&
B_tree<tkey, tvalue, compare, t>::btree_iterator::operator++()
{

    if (_path.empty())
        return *this;

    auto [node_ptr, idx] = _path.top();
    assert(node_ptr != nullptr);
    assert(*node_ptr != nullptr);
    btree_node* node = *node_ptr;
    assert(node != nullptr);
    assert(idx < node->_keys.size());

    static int counter = 0;

    std::cout
        << "ITER STEP " << counter++
        << " node=" << node
        << " idx=" << idx
        << " keys=" << node->_keys.size()
        << " ptrs=" << node->_pointers.size()
        << std::endl;

    assert(counter < 1000);

    if (node->_pointers.size() > idx + 1 &&
        node->_pointers[idx + 1] != nullptr)
    {
        btree_node* cur = node->_pointers[idx + 1];

        while (true)
        {
            assert(cur != nullptr);

            if (cur->is_leaf)
            {
                _path.push({ node_ptr, idx + 1 });
                break;
            }

            assert(!cur->_pointers.empty());
            assert(cur->_pointers[0] != nullptr);

            _path.push({ cur->_pointers[0], 0 });

            cur = cur->_pointers[0];
        }

        _index = 0;
        return *this;
    }

    if (idx + 1 < node->_keys.size())
    {
        _path.pop();
        _path.push({ node_ptr, idx + 1 });

        _index = idx + 1;
        return *this;
    }

    _path.pop();

    while (!_path.empty())
    {
        auto [parent_ptr, parent_idx] = _path.top();
        btree_node* parent = *parent_ptr;

        if (parent_idx < parent->_keys.size())
        {
            _index = parent_idx;
            return *this;
        }

        _path.pop();
    }

    _index = 0;
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
    using node = typename B_tree<tkey, tvalue, compare, t>::node;

    // 1. end() → самый правый элемент
    if (!this->_node)
    {
        this->_node = this->_tree->root();
        if (!this->_node)
            return *this;

        while (!this->_node->is_leaf)
            this->_node = this->_node->children[this->_node.size()];

        this->_index = this->_node.size() ? this->_node.size() - 1 : 0;
        return *this;
    }

    // 2. есть элемент слева в текущем узле
    if (this->_index > 0)
    {
        --this->_index;

        node* cur = this->_node->children[this->_index + 1];

        while (cur && !cur->is_leaf)
            cur = cur->children[cur.size()];

        if (cur)
        {
            this->_node = cur;
            this->_index = cur.size() ? cur.size() - 1 : 0;
        }

        return *this;
    }

    // 3. поднимаемся вверх
    node* cur = this->_node;

    while (true)
    {
        node* parent = cur->parent;

        if (!parent)
        {
            this->_node = nullptr;
            return *this;
        }

        size_t i = 0;
        while (i <= parent.size() && parent->children[i] != cur)
            ++i;

        if (i > 0)
        {
            node* target = parent->children[i - 1];

            while (target && !target->is_leaf)
                target = target->children[target.size()];

            this->_node = target;
            this->_index = target ? target.size() - 1 : 0;
            return *this;
        }

        cur = parent;
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
    if (_path.empty() && other._path.empty())
        return true;

    if (_path.empty() || other._path.empty())
        return false;

    return _path == other._path && _index == other._index;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool B_tree<tkey, tvalue, compare, t>::btree_iterator::operator!=(const self& other) const noexcept
{
    return !(*this == other);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t B_tree<tkey, tvalue, compare, t>::btree_iterator::depth() const noexcept
{
    return _path.size();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t B_tree<tkey, tvalue, compare, t>::btree_iterator::current_node_keys_count() const noexcept
{
    return (*_path.top().first)->_keys.size();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool B_tree<tkey, tvalue, compare, t>::btree_iterator::is_terminate_node() const noexcept
{
    return (*_path.top().first)->_pointers.empty();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t B_tree<tkey, tvalue, compare, t>::btree_iterator::index() const noexcept
{
    return _index;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
B_tree<tkey, tvalue, compare, t>::btree_const_iterator::btree_const_iterator(
        const std::stack<std::pair<const btree_node*, size_t>>& path,
        size_t index)
    : _path(path),
      _index(index)
{
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
B_tree<tkey, tvalue, compare, t>::btree_const_iterator::btree_const_iterator(
        const btree_iterator& it) noexcept
    : _path(it._path),
      _index(it._index)
{
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_const_iterator::reference
B_tree<tkey, tvalue, compare, t>::btree_const_iterator::operator*() const noexcept
{
    auto top = _path.top();
    auto node = top.first;

    return node->_keys[_index];
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_const_iterator::pointer
B_tree<tkey, tvalue, compare, t>::btree_const_iterator::operator->() const noexcept
{
    return &(**this);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_const_iterator&
B_tree<tkey, tvalue, compare, t>::btree_const_iterator::operator++()
{
    if (_path.empty())
        return *this;

    while (true)
    {
        auto [node_ptr_ptr, idx] = _path.top();
        btree_node* node = node_ptr_ptr;

        // 1) если есть следующий ключ в узле
        if (idx + 1 < node->_keys.size())
        {
            ++idx;

            _path.pop();
            _path.emplace(node_ptr_ptr, idx);

            // если не лист → идём в правое поддерево
            if (!node->is_leaf)
            {
                btree_node* child = node->_pointers[idx];

                while (child)
                {
                    _path.emplace(&child, 0);

                    if (child->is_leaf)
                        break;

                    child = child->_pointers[0];
                }
            }

            return *this;
        }

        // 2) иначе поднимаемся вверх
        _path.pop();

        while (!_path.empty())
        {
            auto [p_ptr, p_idx] = _path.top();
            btree_node* parent = p_ptr;

            if (p_idx + 1 < parent->_keys.size())
            {
                _path.pop();

                ++p_idx;
                _path.emplace(p_ptr, p_idx);

                btree_node* child = parent->_pointers[p_idx];

                while (child)
                {
                    _path.emplace(&child, 0);

                    if (child->is_leaf)
                        break;

                    child = child->_pointers[0];
                }

                return *this;
            }

            _path.pop();
        }

        // end()
        return *this;
    }
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_const_iterator
B_tree<tkey, tvalue, compare, t>::btree_const_iterator::operator++(int)
{
    btree_const_iterator tmp(*this);
    ++(*this);
    return tmp;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_const_iterator&
B_tree<tkey, tvalue, compare, t>::btree_const_iterator::operator--()
{
    if (_path.empty())
    {
        return *this;
    }

    auto current_pair = _path.top();
    auto current = *current_pair.first;

    //
    // предыдущий ключ в текущем узле
    //
    if (_index > 0)
    {
        --_index;
        return *this;
    }

    //
    // поднимаемся вверх
    //
    while (!_path.empty())
    {
        auto top = _path.top();
        _path.pop();

        if (_path.empty())
        {
            _index = 0;
            return *this;
        }

        auto parent_pair = _path.top();
        auto parent = *parent_pair.first;

        size_t child_index = top.second;

        if (child_index > 0)
        {
            _index = child_index - 1;
            return *this;
        }
    }

    return *this;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_const_iterator
B_tree<tkey, tvalue, compare, t>::btree_const_iterator::operator--(int)
{
    btree_const_iterator tmp(*this);
    --(*this);
    return tmp;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool B_tree<tkey, tvalue, compare, t>::btree_const_iterator::operator==(const self& other) const noexcept
{
    return _index == other._index &&
           _path == other._path;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool B_tree<tkey, tvalue, compare, t>::btree_const_iterator::operator!=(const self& other) const noexcept
{
    return !(*this == other);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t B_tree<tkey, tvalue, compare, t>::btree_const_iterator::depth() const noexcept
{
    if (_path.empty())
    {
        return 0;
    }

    btree_node* current = const_cast<btree_node*>(*_path.top().first);
    return current->_keys.size();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t B_tree<tkey, tvalue, compare, t>::btree_const_iterator::current_node_keys_count() const noexcept
{
    if (_path.empty())
    {
        return 0;
    }

    btree_node* current = _path.top().first;
    return current->_keys.size();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool B_tree<tkey, tvalue, compare, t>::btree_const_iterator::is_terminate_node() const noexcept
{
    if (_path.empty())
    {
        return false;
    }

    auto current = *_path.top().first;

    return current->is_leaf;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t B_tree<tkey, tvalue, compare, t>::btree_const_iterator::index() const noexcept
{
    return _index;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator::btree_reverse_iterator(
        const std::stack<std::pair<btree_node*, size_t>>& path, size_t index)
    : _path(path),
      _index(index)
{
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator::btree_reverse_iterator(
        const btree_iterator& it) noexcept
    : _path(it._path),
      _index(it._index)
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
    auto node = *_path.top().first;
    auto& kv = node->_keys[_index];

    return *reinterpret_cast<const tree_data_type_const*>(&kv);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator::pointer
B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator::operator->() const noexcept
{
    return &(**this);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator&
B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator::operator++()
{
    --(*this);
    return *this;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator
B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator::operator++(int)
{
    btree_reverse_iterator tmp(*this);
    ++(*this);
    return tmp;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator&
B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator::operator--()
{
    ++(*this);
    return *this;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator
B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator::operator--(int)
{
    btree_reverse_iterator tmp(*this);
    --(*this);
    return tmp;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator::operator==(const self& other) const noexcept
{
    return _index == other._index &&
           _path == other._path;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator::operator!=(const self& other) const noexcept
{
    return !(*this == other);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator::depth() const noexcept
{
    return _path.size();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator::current_node_keys_count() const noexcept
{
    if (_path.empty())
    {
        return 0;
    }

    auto node = *_path.top().first;

    return node.size();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator::is_terminate_node() const noexcept
{
    if (_path.empty())
    {
        return false;
    }

    auto node = *_path.top().first;

    return node->is_leaf;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator::index() const noexcept
{
    return _index;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator::btree_const_reverse_iterator(
        const std::stack<std::pair<const btree_node*, size_t>>& path, size_t index)
    : _path(path), _index(index)
{}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator::btree_const_reverse_iterator(
        const btree_reverse_iterator& it) noexcept
    : _path(it._path), _index(it._index)
{}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator::operator B_tree<tkey, tvalue, compare, t>::btree_const_iterator() const noexcept
{
    return btree_const_iterator(_path, _index);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator::reference
B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator::operator*() const noexcept
{
    auto top = _path.top();
    auto node = *top.first;

    return node->_keys[_index];
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator::pointer
B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator::operator->() const noexcept
{
    return &(**this);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator&
B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator::operator++()
{
    btree_const_iterator tmp(_path, _index);
    --tmp;

    _path = tmp._path;
    _index = tmp._index;

    return *this;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator
B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator::operator++(int)
{
    self tmp = *this;
    ++(*this);
    return tmp;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator&
B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator::operator--()
{
    btree_const_iterator tmp(_path, _index);
    ++tmp;

    _path = tmp._path;
    _index = tmp._index;

    return *this;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator
B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator::operator--(int)
{
    self tmp = *this;
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
    return _path.size();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator::current_node_keys_count() const noexcept
{
    if (_path.empty())
    {
        return 0;
    }

    auto top = _path.top();
    auto node = *top.first;

    return node.size();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator::is_terminate_node() const noexcept
{
    if (_path.empty())
    {
        return false;
    }

    auto top = _path.top();
    auto node = *top.first;

    return node->is_leaf;
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

    if (it == end())
    {
        throw std::out_of_range("B_tree::at: key not found");
    }

    return (*it).second;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
const tvalue& B_tree<tkey, tvalue, compare, t>::at(const tkey& key) const
{
    auto it = find(key);

    if (it == cend())
    {
        throw std::out_of_range("B_tree::at: key not found");
    }

    return (*it).second;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
tvalue& B_tree<tkey, tvalue, compare, t>::operator[](const tkey& key)
{
    auto result = insert(value_type(key, tvalue{}));
    return (*result.first).second;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
tvalue& B_tree<tkey, tvalue, compare, t>::operator[](tkey&& key)
{
    auto result = insert(value_type(std::move(key), tvalue{}));
    return (*result.first).second;
}

// endregion element access implementation

// region iterator begins implementation

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_iterator B_tree<tkey, tvalue, compare, t>::begin()
{
    if (!_root)
        return end();

    btree_node* current = _root;

    std::stack<std::pair<btree_node*, size_t>> path;

    // идём в самый левый лист
    while (!current->_pointers.empty())
    {
        path.push({ current, 0 });
        current = current->_pointers[0];
    }

    return btree_iterator(path, 0);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_iterator B_tree<tkey, tvalue, compare, t>::end()
{
    return btree_iterator();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_const_iterator
B_tree<tkey, tvalue, compare, t>::begin() const
{
    return cbegin();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_const_iterator
B_tree<tkey, tvalue, compare, t>::end() const
{
    return cend();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_const_iterator
B_tree<tkey, tvalue, compare, t>::cbegin() const
{
    if (_root == nullptr || _root->_keys.empty())
        return cend();

    std::stack<std::pair<const btree_node*, size_t>> path;

    btree_node* current = _root;

    while (*current != nullptr)
    {
        path.push({ current, 0 });

        // ЛИСТ = нет детей
        if ((*current)->_pointers.empty())
            break;

        current = &((*current)->_pointers[0]);
    }

    return btree_const_iterator(path, 0);
}
template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_const_iterator
B_tree<tkey, tvalue, compare, t>::cend() const
{
    return btree_const_iterator();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator
B_tree<tkey, tvalue, compare, t>::rbegin()
{
    if (_root == nullptr || _root->_keys.size() == 0)
    {
        return rend();
    }

    std::stack<std::pair<btree_node*, size_t>> path;

    btree_node* current = _root;

    while (current)
    {
        size_t idx = (*current).size();

        path.push({ current, idx - 1 });

        if ((*current)->is_leaf)
        {
            break;
        }

        current = &((*current)->subtrees[idx]);
    }

    return btree_reverse_iterator(path, path.top().second);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator
B_tree<tkey, tvalue, compare, t>::rend()
{
    return btree_reverse_iterator();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator
B_tree<tkey, tvalue, compare, t>::rbegin() const
{
    return crbegin();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator
B_tree<tkey, tvalue, compare, t>::rend() const
{
    return crend();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator
B_tree<tkey, tvalue, compare, t>::crbegin() const
{
    if (_root == nullptr || _root->_keys.size() == 0)
    {
        return crend();
    }

    std::stack<std::pair<const btree_node*, size_t>> path;

    btree_node* current = _root;

    while (current)
    {
        size_t idx = (*current).size();

        path.push({ current, idx - 1 });

        if ((*current)->is_leaf)
        {
            break;
        }

        current = &((*current)->subtrees[idx]);
    }

    return btree_const_reverse_iterator(path, path.top().second);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator
B_tree<tkey, tvalue, compare, t>::crend() const
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
    btree_node* current = _root;
    std::stack<std::pair<btree_node*, size_t>> path;

    while (current)
    {
        size_t i = 0;

        while (i < current->_keys.size() &&
               compare::operator()(current->_keys[i].first, key))
        {
            ++i;
        }

        // нашли ключ
        if (i < current->_keys.size() &&
            !compare::operator()(key, current->_keys[i].first) &&
            !compare::operator()(current->_keys[i].first, key))
        {
            return btree_iterator(path, i);
        }

        // если leaf — не найден
        if (current->_pointers.empty())
        {
            return end();
        }

        // спускаемся вниз
        path.push({current, i});
        current = current->_pointers[i];
    }

    return end();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_const_iterator
B_tree<tkey, tvalue, compare, t>::find(const tkey& key) const
{
    if (_root == nullptr)
    {
        return cend();
    }

    std::stack<std::pair<const btree_node*, size_t>> path;

    btree_node* current = _root;

    while (current)
    {
        auto* node = current;

        size_t i = 0;

        while (i < node.size() &&
               compare_instance(node->keys[i].first, key))
        {
            ++i;
        }

        path.push({ current, i });

        if (i < node.size() &&
            !compare_instance(key, node->keys[i].first) &&
            !compare_instance(node->keys[i].first, key))
        {
            return btree_const_iterator(path, i);
        }

        if (node->is_leaf)
        {
            break;
        }

        current = node->_pointers[i];
    }

    return cend();
}
template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_iterator B_tree<tkey, tvalue, compare, t>::lower_bound(const tkey& key)
{
    btree_node* current = _root;
    std::stack<std::pair<btree_node*, size_t>> path;

    btree_node* candidate_node = nullptr;
    size_t candidate_index = 0;

    while (current)
    {
        size_t i = 0;

        while (i < current->_keys.size() &&
               compare::operator()(current->_keys[i].first, key))
        {
            ++i;
        }

        // если нашли равный или больший — потенциальный кандидат
        if (i < current->_keys.size())
        {
            candidate_node = current;
            candidate_index = i;
        }

        // если leaf — остановка
        if (current->_pointers.empty())
            break;

        path.push({current, i});
        current = current->_pointers[i];
    }

    if (!candidate_node)
        return end();

    return btree_iterator(path, candidate_index);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_const_iterator B_tree<tkey, tvalue, compare, t>::lower_bound(const tkey& key) const
{
    return const_cast<B_tree*>(this)->lower_bound(key);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_iterator B_tree<tkey, tvalue, compare, t>::upper_bound(const tkey& key)
{
    btree_node* current = _root;
    std::stack<std::pair<btree_node*, size_t>> path;

    btree_node* candidate = nullptr;
    size_t candidate_index = 0;

    while (current)
    {
        size_t i = 0;

        while (i < current->_keys.size() &&
               !compare::operator()(key, current->_keys[i].first))
        {
            ++i;
        }

        if (i < current->_keys.size())
        {
            candidate = current;
            candidate_index = i;
        }

        if (current->_pointers.empty())
            break;

        path.push({current, i});
        current = current->_pointers[i];
    }

    if (!candidate)
        return end();

    return btree_iterator(path, candidate_index);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_const_iterator
B_tree<tkey, tvalue, compare, t>::upper_bound(const tkey& key) const
{
    auto it = cbegin();

    while (it != cend())
    {
        if (compare_instance(key, (*it).first))
        {
            return it;
        }

        ++it;
    }

    return cend();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool B_tree<tkey, tvalue, compare, t>::contains(const tkey& key) const
{
    return const_cast<B_tree*>(this)->find(key) != end();
}

// endregion lookup implementation

// region modifiers implementation

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
void B_tree<tkey, tvalue, compare, t>::clear() noexcept
{
    if (_root)
    {
        destroy_node(_root);
        _root = nullptr;
    }

    _size = 0;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
std::pair<typename B_tree<tkey, tvalue, compare, t>::btree_iterator, bool>
B_tree<tkey, tvalue, compare, t>::insert(const tree_data_type& data)
{
    if (!_root)
    {
        _root = new btree_node();
        _root->_keys.push_back(std::move(data));
        ++_size;
        return { begin(), true };
    }
    _root->check_invariants();

    return emplace(data.first, data.second);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
std::pair<typename B_tree<tkey, tvalue, compare, t>::btree_iterator, bool>
B_tree<tkey, tvalue, compare, t>::insert(tree_data_type&& data)
{
    auto existing = find(data.first);

    if (existing != end())
    {
        return { btree_iterator(existing), false };
    }

    if (_root == nullptr)
    {
        _root = new btree_node();

        _root->is_leaf = true;
        _root->_keys.push_back(std::move(data));
        _root->keys_and_values[0] = std::move(data);

        ++_size;

        std::stack<std::pair<btree_node*, size_t>> path;
        path.push({ _root, 0 });

        return { btree_iterator(path, 0), true };
    }

    btree_node* current = _root;
    std::stack<std::pair<btree_node*, size_t>> path;

    while (!current->is_leaf)
    {
        size_t i = 0;

        while (i < current.size() &&
               compare_instance(current->keys_and_values[i].first, data.first))
        {
            ++i;
        }

        path.push({ current, i });

        current = current->subtrees[i];
    }

    size_t pos = 0;

    while (pos < current.size() &&
           compare_instance(current->keys_and_values[pos].first, data.first))
    {
        ++pos;
    }

    for (size_t i = current.size(); i > pos; --i)
    {
        current->keys_and_values[i] = std::move(current->keys_and_values[i - 1]);
    }

    current->keys_and_values[pos] = std::move(data);
    current->check_invariants();
    ++current.size();
    ++_size;

    std::stack<std::pair<btree_node*, size_t>> result_path;
    result_path.push({ current, pos });

    return { btree_iterator(result_path, pos), true };
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
template<typename... Args>
std::pair<typename B_tree<tkey, tvalue, compare, t>::btree_iterator, bool>
B_tree<tkey, tvalue, compare, t>::emplace(Args&&... args)
{
    tree_data_type data(std::forward<Args>(args)...);

    if (!_root)
    {
        _root = new btree_node();
        _root->_keys.push_back(std::move(data));
        ++_size;
        return { begin(), true };
    }

    // ROOT SPLIT CHECK
    if (_root->_keys.size() == maximum_keys_in_node)
    {
        auto* new_root = new btree_node();
        new_root->_pointers.push_back(_root);

        split_child(new_root, 0, _root);

        _root = new_root;
    }

    btree_node* current = _root;

    while (true)
    {
        size_t i = find_key_index(current, data.first);

        if (i < current->_keys.size() &&
            !compare::operator()(data.first, current->_keys[i].first) &&
            !compare::operator()(current->_keys[i].first, data.first))
        {
            return { find(data.first), false };
        }

        // SPLIT CHILD BEFORE DESCEND
        if (!current->_pointers.empty() &&
            current->_pointers[i]->_keys.size() == maximum_keys_in_node)
        {
            split_child(current, i, current->_pointers[i]);

            if (compare::operator()(current->_keys[i].first, data.first))
                ++i;
        }

        if (current->_pointers.empty())
        {
            current->_keys.insert(current->_keys.begin() + i, std::move(data));
            ++_size;
            return { btree_iterator(), true };
        }

        current = current->_pointers[i];
    }
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_iterator
B_tree<tkey, tvalue, compare, t>::insert_or_assign(const tree_data_type& data)
{
    auto it = find(data.first);

    if (it != end())
    {
        (*it).second = data.second;
        return it;
    }

    return insert(data).first;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_iterator
B_tree<tkey, tvalue, compare, t>::insert_or_assign(tree_data_type&& data)
{
    auto it = find(data.first);

    if (it != end())
    {
        (*it).second = std::move(data.second);
        return it;
    }

    return insert(std::move(data)).first;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
template<typename... Args>
typename B_tree<tkey, tvalue, compare, t>::btree_iterator
B_tree<tkey, tvalue, compare, t>::emplace_or_assign(Args&&... args)
{
    tree_data_type data(std::forward<Args>(args)...);
    return insert_or_assign(std::move(data));
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_iterator
B_tree<tkey, tvalue, compare, t>::erase(btree_iterator pos)
{
    if (pos == end())
    {
        return end();
    }

    auto next = pos;
    ++next;

    // временная заглушка
    return next;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_iterator
B_tree<tkey, tvalue, compare, t>::erase(btree_const_iterator pos)
{
    return erase(btree_iterator(pos));
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_iterator
B_tree<tkey, tvalue, compare, t>::erase(btree_iterator beg, btree_iterator en)
{
    while (beg != en)
    {
        beg = erase(beg);
    }

    return en;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_iterator
B_tree<tkey, tvalue, compare, t>::erase(btree_const_iterator beg, btree_const_iterator en)
{
    return erase(btree_iterator(beg), btree_iterator(en));
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_iterator
B_tree<tkey, tvalue, compare, t>::erase(const tkey& key)
{
    auto it = find(key);

    if (it == end())
    {
        return end();
    }

    return erase(it);
}

// endregion modifiers implementation

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool compare_pairs(
        const typename B_tree<tkey, tvalue, compare, t>::tree_data_type& lhs,
        const typename B_tree<tkey, tvalue, compare, t>::tree_data_type& rhs)
{
    compare cmp;
    return cmp(lhs.first, rhs.first);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool compare_keys(const tkey& lhs, const tkey& rhs)
{
    compare cmp;
    return cmp(lhs, rhs);
}

// region  heplers implementation

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_node*
B_tree<tkey, tvalue, compare, t>::clone_node(const btree_node* node)
{
    if (!node)
        return nullptr;

    auto* new_node = new btree_node();
    new_node->_keys = node->_keys;

    new_node->_pointers.resize(node->_pointers.size());

    for (size_t i = 0; i < node->_pointers.size(); ++i)
    {
        new_node->_pointers[i] = clone_node(node->_pointers[i]);
    }

    return new_node;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
void B_tree<tkey, tvalue, compare, t>::destroy_node(btree_node* node) noexcept
{
    if (!node) return;

    for (auto child : node->_pointers)
        destroy_node(child);

    delete node;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t B_tree<tkey, tvalue, compare, t>::find_key_index(
    const btree_node* node,
    const tkey& key) const noexcept
{
    size_t i = 0;

    while (i < node->_keys.size() &&
           compare::operator()(node->_keys[i].first, key))
    {
        ++i;
    }

    return i;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
void B_tree<tkey, tvalue, compare, t>::split_child(
    btree_node* parent,
    size_t index,
    btree_node* child)
{
    auto* new_node = new btree_node();

    const size_t mid = t - 1;

    // -----------------------------------
    // new node inherits leaf status
    // -----------------------------------
    new_node->is_leaf = child->is_leaf;

    // -----------------------------------
    // 1. save middle key
    // -----------------------------------
    tree_data_type middle_key = std::move(child->_keys[mid]);

    // -----------------------------------
    // 2. move right-half keys
    // -----------------------------------
    for (size_t i = mid + 1; i < child->_keys.size(); ++i)
    {
        new_node->_keys.push_back(std::move(child->_keys[i]));
    }

    // left half remains in child
    child->_keys.resize(mid);

    // -----------------------------------
    // 3. split children
    // -----------------------------------
    if (!child->is_leaf)
    {
        const size_t child_count = child->_pointers.size();

        for (size_t i = mid + 1; i < child_count; ++i)
        {
            new_node->_pointers.push_back(child->_pointers[i]);
        }

        child->_pointers.resize(mid + 1);
    }

    // -----------------------------------
    // 4. insert new child into parent
    // -----------------------------------
    parent->_pointers.insert(
        parent->_pointers.begin() + index + 1,
        new_node
    );

    // -----------------------------------
    // 5. promote middle key
    // -----------------------------------
    parent->_keys.insert(
        parent->_keys.begin() + index,
        std::move(middle_key)
    );

    // -----------------------------------
    // 6. invariants check
    // -----------------------------------
    child->check_invariants();
    new_node->check_invariants();
    parent->check_invariants();
}

#endif