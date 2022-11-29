
#pragma once

#include <functional>
#include <algorithm>
#include <vector>
#include <utility>


template < class Key, class Compare = std::less<Key>, class Alloc = std::allocator<Key> >
class vectorset : protected std::vector<Key, Alloc>
{
public:
    typedef vectorset<Key, Compare, Alloc>			this_type;
    typedef std::vector<Key, Alloc>					base_type;
    typedef Key										key_type;
    typedef Compare									key_compare;
    typedef key_compare								value_compare;
    typedef typename base_type::allocator_type		allocator_type;
    typedef typename base_type::size_type			size_type;
    typedef typename base_type::value_type			value_type;
    typedef typename base_type::pointer				pointer;
    typedef typename base_type::const_pointer		const_pointer;
    typedef typename base_type::reference			reference;
    typedef typename base_type::const_reference		const_reference;
    typedef typename base_type::iterator			iterator;
    typedef typename base_type::const_iterator		const_iterator;
    //for convenience
    typedef std::pair<iterator, bool>				insert_iterator_type;
    using base_type::begin;
    using base_type::end;
    using base_type::rbegin;
    using base_type::rend;
    using base_type::size;
    using base_type::max_size;
    using base_type::empty;
    using base_type::capacity;
    using base_type::clear;
    using base_type::front;
    using base_type::back;
    using base_type::reserve;
    using base_type::swap;
    using base_type::at;
    using base_type::operator[];
    explicit vectorset(const Compare& comp = Compare(), const allocator_type& alloc = allocator_type())
        : base_type(alloc), _compare(comp)
    {
    }
    vectorset(const this_type& v)
        : base_type(v), _compare(v._compare)
    {
    }
    this_type& operator =(const this_type& v)
    {
        if(this != &v)
        {
            base_type::operator =(v);
            _compare = v._compare;
        }
        return *this;
    }
    void push_back(const value_type& val)
    {
        insert(val);
    }
    std::pair<iterator, bool> insert(const value_type& val)
    {
        std::pair<iterator, bool> ret(end(), false);
        iterator it = lower_bound(val);
        if(it == end() || _compare(val, *it))
        {
            ret.first = base_type::insert(it, val);
            ret.second = true;
        }
        return ret;
    }
    iterator insert(iterator it, const value_type& val)
    {
        return insert(val).first;
    }
    template <class Iter>
    void insert(Iter first, Iter last)
    {
        base_type::insert(first, last);
        sort();
    }
    void erase(iterator it)
    {
        base_type::erase(it);
    }
    void erase(iterator first, iterator last)
    {
        base_type::erase(first, last);
    }
    size_type erase(const key_type& key)
    {
        size_type n(0);
        iterator it = find(key);
        if(it != end())
        {
            erase(it);
            ++n;
        }
        return n;
    }
    iterator find(const key_type& key)
    {
        iterator it = lower_bound(key);
        return it;
    }
    const_iterator find(const key_type& key) const
    {
        const_iterator it = lower_bound(key);
        return it;
    }
    size_type count(const key_type& key) const
    {
        return(find(key) != end() ? (size_type)1 : (size_type)0);
    }
    key_compare key_comp() const
    {
        return _compare;
    }
    value_compare value_comp() const
    {
        return key_comp();
    }
    std::pair<iterator, iterator> equal_range(const key_type& key)
    {
        iterator first = find(key);
        iterator last = end();
        if(first != end())
            last = first + 1;
        return std::pair<iterator, iterator>(first, last);
    }
    std::pair<const_iterator, const_iterator> equal_range(const key_type& key) const
    {
        const_iterator first = find(key);
        const_iterator last = end();
        if(first != end())
            last = first + 1;
        return std::pair<const_iterator, const_iterator>(first, last);
    }
    iterator lower_bound(const key_type& key)
    {
        return std::lower_bound(begin(), end(), key, _compare);
    }
    const_iterator lower_bound(const key_type& key) const
    {
        return std::lower_bound(begin(), end(), key, _compare);
    }
    iterator upper_bound(const key_type& key)
    {
        return std::upper_bound(begin(), end(), key, _compare);
    }
    const_iterator upper_bound(const key_type& key) const
    {
        return std::upper_bound(begin(), end(), key, _compare);
    }
    void sort()
    {
        if(size() > (size_type)1)
        {
            std::sort(begin(), end(), _compare);
            iterator it = std::unique(begin(), end());
            if(it != end())
                base_type::erase(it, end());
        }
    }
protected:
    key_compare _compare;
};


