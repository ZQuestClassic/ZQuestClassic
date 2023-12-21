#ifndef ZSCRIPT_OWNING_VECTOR_H
#define ZSCRIPT_OWNING_VECTOR_H

////////////////////////////////////////////////////////////////
// owning_vector - a quick replacement for vector<unique_ptr<T*> >.

// owning_vector<T> is like a vector<T*> that will delete its contents when
// deleted. Element must have a clone function for the owning_vector itself
// to be assignable.
template <class Element>
class owning_vector
{
public:
	typedef std::vector<Element*> base_type;
	typedef typename base_type::value_type value_type;
	typedef typename base_type::reference reference;
	typedef typename base_type::const_reference const_reference;
	typedef typename base_type::pointer pointer;
	typedef typename base_type::const_pointer const_pointer;
	typedef typename base_type::iterator iterator;
	typedef typename base_type::const_iterator const_iterator;
	typedef typename base_type::reverse_iterator reverse_iterator;
	typedef typename base_type::const_reverse_iterator const_reverse_iterator;
	typedef typename base_type::size_type size_type;
	
	owning_vector() {}
	template <class InputIterator>
	owning_vector(InputIterator first, InputIterator last)
			: vector_(first, last) {}
	owning_vector(owning_vector const& other)
			: vector_(other.cloneData()) {}
	owning_vector(owning_vector&& other)
	{
		clear();
		swap(other);
	}
	owning_vector& operator=(owning_vector&& other)
	{
		clear();
		swap(other);
	}
	~owning_vector() {deleteData();}
	owning_vector& operator=(owning_vector const& rhs)
	{
		deleteData();
		vector_ = rhs.cloneData();
		return *this;
	}

	inline base_type& data() {return vector_;}
	inline base_type const& data() const {return vector_;}

	// Iterators
	inline iterator begin() {return vector_.begin();}
	inline const_iterator cbegin() const {return vector_.begin();}
	inline iterator end() {return vector_.end();}
	inline const_iterator cend() const {return vector_.end();}
	inline reverse_iterator rbegin() {return vector_.rbegin();}
	inline const_reverse_iterator crbegin() const {return vector_.rbegin();}
	inline reverse_iterator rend() {return vector_.rend();}
	inline const_reverse_iterator crend() const {return vector_.rend();}

	// Capacity
	inline size_type size() const {return vector_.size();}
	inline size_type max_size() const {return vector_.max_size();}
	inline void resize(size_type n, value_type val = value_type()) {
		vector_.resize(n, val);}
	inline size_type capacity() const {return vector_.capacity();}
	inline bool empty() const {return vector_.empty();}
	inline void reserve(size_type n) {vector_.reserve(n);}

	// Element access
	inline reference operator[](size_type n) {return vector_[n];}
	inline const_reference operator[](size_type n) const {return vector_[n];}
	inline reference at(size_type n) {return vector_.at(n);}
	inline const_reference at(size_type n) const {return vector_.at(n);}
	inline reference front() {return vector_.front();}
	inline const_reference front() const {return vector_.front();}
	inline reference back() {return vector_.back();}
	inline const_reference back() const {return vector_.back();}

	// Modifiers
	template <class InputIterator>
	inline void assign(InputIterator first, InputIterator last) {
		deleteData();
		vector_.assign(first, last);}
	// void assign(size_type n, value_type const& val) - makes no sense
	inline void push_back(value_type const& val) {vector_.push_back(val);}
	inline void pop_back() {
		delete vector_.back();
		vector_.pop_back();}
	inline iterator insert(iterator position, value_type const& val) {
		return vector_.insert(position, val);}
	inline void insert(iterator position, size_type n,
	                   value_type const& val) {
		vector_.insert(position, n, val);}
	template <class InputIterator>
	inline void insert(iterator position, InputIterator first,
	                   InputIterator last) {
		vector_.insert(position, first, last);}
	inline iterator erase(iterator position) {
		delete *position;
		return vector_.erase(position);}
	inline iterator erase(iterator first, iterator last) {
		deleteData(first, last);
		return vector_.erase(first, last);}
	inline void swap(owning_vector& x) {vector_.swap(x.vector_);}
	inline void clear() {
		deleteData();
		vector_.clear();
		vector_.shrink_to_fit();
	}

private:
	base_type cloneData() const
	{
		base_type result;
		for (const_iterator it = vector_.begin(); it != vector_.end(); ++it)
			result.push_back((*it)->clone());
		return result;
	}
	void deleteData()
	{
		for (iterator it = vector_.begin(); it != vector_.end(); ++it)
			delete *it;
	}
	void deleteData(iterator first, iterator last)
	{
		for (iterator it = first; it != last; ++it)
			delete *it;
	}
	
	base_type vector_;
};

template <class Element>
inline bool operator==(owning_vector<Element> const& lhs,
                       owning_vector<Element> const& rhs) {
	return lhs.data() == rhs.data();}
template <class Element>
inline bool operator!=(owning_vector<Element> const& lhs,
                       owning_vector<Element> const& rhs) {
	return lhs.data() != rhs.data();}
template <class Element>
inline bool operator<(owning_vector<Element> const& lhs,
                      owning_vector<Element> const& rhs) {
	return lhs.data() < rhs.data();}
template <class Element>
inline bool operator<=(owning_vector<Element> const& lhs,
                       owning_vector<Element> const& rhs) {
	return lhs.data() <= rhs.data();}
template <class Element>
inline bool operator>(owning_vector<Element> const& lhs,
                      owning_vector<Element> const& rhs) {
	return lhs.data() > rhs.data();}
template <class Element>
inline bool operator>=(owning_vector<Element> const& lhs,
                       owning_vector<Element> const& rhs) {
	return lhs.data() >= rhs.data();}

template <class Element>
inline void swap(owning_vector<Element>& x, owning_vector<Element>& y) {
	swap(x.data(), y.data());}

#endif
