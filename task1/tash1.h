#include <initializer_list>
#include <vector>
#include <cstddef>
#include <stdexcept>

template<class KeyType, class ValueType, class Hash = std::hash<KeyType>>
class HashMap {
private:
	const static size_t START_SIZE = 42;

	typedef std::pair<const KeyType, ValueType> ItemType;
	typedef std::pair<KeyType, ValueType> NCItemType;

	class Cell {
	public:
		enum class Type {
			FREE,
			DUMP,
			FULL,
		};

		Cell() {
		}

		Cell(const NCItemType& item) : item(item), type(Type::FULL) {
		}

		NCItemType item;
		Type type = Type::FREE;
		size_t bibka = 0;
	};

	template<typename DataType>
	class Iterator {
	public:
		Iterator() {}

		Iterator(const Cell* ptr) : ptr(const_cast<Cell*>(ptr)) {
		}

		bool operator==(const Iterator& other) const {
			return ptr == other.ptr;
		}

		bool operator!=(const Iterator& other) const {
			return ptr != other.ptr;
		}

		DataType& operator*() const {
			return *operator->();
		}

		DataType* operator->() const {
			return reinterpret_cast<DataType*>(&ptr->item);
		}

		Iterator& operator++() {
			for (++ptr; ptr->type != Cell::Type::FULL; ++ptr) {
			}
			return *this;
		}

		Iterator operator++(int) {
			Iterator other(*this);
			++(*this);
			return other;
		}

	private:
		Cell* ptr;
	};

	inline size_t probe(size_t &i) const {
		return i = (i + 1 == capacity_ ? 0 : i + 1);
	}

	void rebuild() {
		auto other = *this;
		clear(data_.size() * 2);
		for (const auto& item : other) {
			insert(item);
		}
	}

	size_t find_index(const KeyType& key) const {
		size_t h = hash_(key);
		for (size_t i = h % capacity_; data_[i].type != Cell::Type::FREE; probe(i)) {
			if (data_[i].type != Cell::Type::DUMP && data_[i].item.first == key) {
				return i;
			}
		}
		return capacity_;
	}

public:
	typedef Iterator<ItemType> iterator;
	typedef Iterator<const ItemType> const_iterator;

	HashMap(const Hash& hash = Hash()) : hash_(hash) {
		clear();
	}

	template<class ItemIterator>
	HashMap(ItemIterator begin, ItemIterator end, const Hash& hash = Hash()) : hash_(hash) {
		clear();
		for (auto it = begin; it != end; ++it)
			insert(*it);
	}

	HashMap(const std::initializer_list<ItemType>& items, const Hash& hash = Hash()) : hash_(hash) {
		clear();
		for (const auto& item : items)
			insert(item);
	}

	size_t size() const {
		return size_;
	}

	bool empty() const {
		return size_ == 0;
	}

	Hash hash_function() const {
		return hash_;
	}

	void insert(const ItemType& item) {
		if (find(item.first) != end()) {
			return;
		}
		if (size_ * 2 >= capacity_) {
			rebuild();
		}
		++size_;
		size_t i = hash_(item.first) % capacity_;
		Cell cell(item);
		for (; data_[i].type == Cell::Type::FULL; probe(i)) {
			if (data_[i].bibka + 1 < cell.bibka) {
				std::swap(data_[i], cell);
			} else {
				++cell.bibka;
			}
		}
		data_[i] = cell;
	}

	void erase(const KeyType& key) {
		size_t i = find_index(key);
		if (i != capacity_) {
			data_[i].type = Cell::Type::DUMP;
			--size_;
		}
	}

	iterator begin() {
		iterator other(data_.data() - 1);
		return ++other;
	}

	const_iterator begin() const {
		const_iterator other(data_.data() - 1);
		return ++other;
	}

	iterator end() {
		return iterator(data_.data() + capacity_);
	}

	const_iterator end() const {
		return const_iterator(data_.data() + capacity_);
	}

	iterator find(const KeyType& key) {
		return iterator(data_.data() + find_index(key));
	}

	const_iterator find(const KeyType& key) const {
		return const_iterator(data_.data() + find_index(key));
	}

	ValueType& operator[](const KeyType& key) {
		iterator it = find(key);
		if (it != end()) {
			return it->second;
		}
		insert({key, ValueType()});
		return find(key)->second;
	}

	const ValueType& at(const KeyType& key) const {
		const_iterator it = find(key);
		if (it != end()) {
			return it->second;
		}
		throw std::out_of_range("tyan by");
	}

	void clear(size_t real_size = START_SIZE) {
		size_ = 0;
		capacity_ = real_size - 1;
		data_.assign(real_size, Cell());
		data_.back().type = Cell::Type::FULL;
	}

private:
	Hash hash_;
	std::vector<Cell> data_;
	size_t size_;
	size_t capacity_;
};
