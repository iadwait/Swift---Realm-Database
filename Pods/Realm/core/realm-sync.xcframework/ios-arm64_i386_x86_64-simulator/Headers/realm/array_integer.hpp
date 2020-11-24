/*************************************************************************
 *
 * Copyright 2016 Realm Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 **************************************************************************/

#ifndef REALM_ARRAY_INTEGER_HPP
#define REALM_ARRAY_INTEGER_HPP

#include <realm/array.hpp>
#include <realm/util/safe_int_ops.hpp>
#include <realm/util/optional.hpp>

namespace realm {

class ArrayInteger : public Array, public ArrayPayload {
public:
    using value_type = int64_t;

    using Array::add;
    using Array::get;
    using Array::insert;
    using Array::move;
    using Array::set;

    explicit ArrayInteger(Allocator&) noexcept;
    ~ArrayInteger() noexcept override {}

    static value_type default_value(bool)
    {
        return 0;
    }

    void init_from_ref(ref_type ref) noexcept override
    {
        Array::init_from_ref(ref);
    }
    void set_parent(ArrayParent* parent, size_t ndx_in_parent) noexcept override
    {
        Array::set_parent(parent, ndx_in_parent);
    }

    // Disable copying, this is not allowed.
    ArrayInteger& operator=(const ArrayInteger&) = delete;
    ArrayInteger(const ArrayInteger&) = delete;

    void create()
    {
        Array::create(type_Normal, false, 0, 0);
    }

    bool is_null(size_t) const
    {
        return false;
    }
};


class ArrayIntNull : public Array, public ArrayPayload {
public:
    using value_type = util::Optional<int64_t>;

    explicit ArrayIntNull(Allocator&) noexcept;
    ~ArrayIntNull() noexcept override;

    static value_type default_value(bool nullable)
    {
        return nullable ? util::none : util::Optional<int64_t>(0);
    }

    /// Construct an array of the specified type and size, and return just the
    /// reference to the underlying memory. All elements will be initialized to
    /// the specified value.
    static MemRef create_array(Type, bool context_flag, size_t size, Allocator&);
    void create()
    {
        MemRef r = create_array(type_Normal, false, 0, m_alloc);
        init_from_mem(r);
    }

    void init_from_ref(ref_type) noexcept override;
    void set_parent(ArrayParent* parent, size_t ndx_in_parent) noexcept override
    {
        Array::set_parent(parent, ndx_in_parent);
    }
    void init_from_mem(MemRef) noexcept;
    void init_from_parent() noexcept;

    size_t size() const noexcept;
    bool is_empty() const noexcept;

    void insert(size_t ndx, value_type value);
    void add(value_type value);
    void set(size_t ndx, value_type value);
    value_type get(size_t ndx) const noexcept;
    static value_type get(const char* header, size_t ndx) noexcept;
    void get_chunk(size_t ndx, value_type res[8]) const noexcept;
    void set_null(size_t ndx);
    bool is_null(size_t ndx) const noexcept;
    int64_t null_value() const noexcept;

    void erase(size_t ndx);
    void erase(size_t begin, size_t end);
    void move(ArrayIntNull& dst, size_t ndx);
    void clear();

    void move(size_t begin, size_t end, size_t dest_begin);

    bool find(int cond, Action action, value_type value, size_t start, size_t end, size_t baseindex,
              QueryState<int64_t>* state) const;

    template <class cond, Action action, size_t bitwidth, class Callback>
    bool find(value_type value, size_t start, size_t end, size_t baseindex, QueryState<int64_t>* state,
              Callback callback) const;

    // This is the one installed into the m_finder slots.
    template <class cond, Action action, size_t bitwidth>
    bool find(int64_t value, size_t start, size_t end, size_t baseindex, QueryState<int64_t>* state) const;

    template <class cond, Action action, class Callback>
    bool find(value_type value, size_t start, size_t end, size_t baseindex, QueryState<int64_t>* state,
              Callback callback) const;

    // Optimized implementation for release mode
    template <class cond, Action action, size_t bitwidth, class Callback>
    bool find_optimized(value_type value, size_t start, size_t end, size_t baseindex, QueryState<int64_t>* state,
                        Callback callback) const;

    // Called for each search result
    template <Action action, class Callback>
    bool find_action(size_t index, value_type value, QueryState<int64_t>* state, Callback callback) const;

    template <Action action, class Callback>
    bool find_action_pattern(size_t index, uint64_t pattern, QueryState<int64_t>* state, Callback callback) const;

    // Wrappers for backwards compatibility and for simple use without
    // setting up state initialization etc
    template <class cond>
    size_t find_first(value_type value, size_t start = 0, size_t end = npos) const;

    void find_all(IntegerColumn* result, value_type value, size_t col_offset = 0, size_t begin = 0,
                  size_t end = npos) const;


    size_t find_first(value_type value, size_t begin = 0, size_t end = npos) const;

protected:
    void avoid_null_collision(int64_t value);

private:
    int_fast64_t choose_random_null(int64_t incoming) const;
    void replace_nulls_with(int64_t new_null);
    bool can_use_as_null(int64_t value) const;
};


// Implementation:

inline ArrayInteger::ArrayInteger(Allocator& allocator) noexcept
    : Array(allocator)
{
    m_is_inner_bptree_node = false;
}

inline ArrayIntNull::ArrayIntNull(Allocator& allocator) noexcept
    : Array(allocator)
{
}

inline ArrayIntNull::~ArrayIntNull() noexcept {}

inline size_t ArrayIntNull::size() const noexcept
{
    return Array::size() - 1;
}

inline bool ArrayIntNull::is_empty() const noexcept
{
    return size() == 0;
}

inline void ArrayIntNull::insert(size_t ndx, value_type value)
{
    if (value) {
        avoid_null_collision(*value);
        Array::insert(ndx + 1, *value);
    }
    else {
        Array::insert(ndx + 1, null_value());
    }
}

inline void ArrayIntNull::add(value_type value)
{
    if (value) {
        avoid_null_collision(*value);
        Array::add(*value);
    }
    else {
        Array::add(null_value());
    }
}

inline void ArrayIntNull::set(size_t ndx, value_type value)
{
    if (value) {
        avoid_null_collision(*value);
        Array::set(ndx + 1, *value);
    }
    else {
        Array::set(ndx + 1, null_value());
    }
}

inline void ArrayIntNull::set_null(size_t ndx)
{
    Array::set(ndx + 1, null_value());
}

inline ArrayIntNull::value_type ArrayIntNull::get(size_t ndx) const noexcept
{
    int64_t value = Array::get(ndx + 1);
    if (value == null_value()) {
        return util::none;
    }
    return util::some<int64_t>(value);
}

inline ArrayIntNull::value_type ArrayIntNull::get(const char* header, size_t ndx) noexcept
{
    int64_t null_value = Array::get(header, 0);
    int64_t value = Array::get(header, ndx + 1);
    if (value == null_value) {
        return util::none;
    }
    else {
        return util::some<int64_t>(value);
    }
}

inline bool ArrayIntNull::is_null(size_t ndx) const noexcept
{
    return !get(ndx);
}

inline int64_t ArrayIntNull::null_value() const noexcept
{
    return Array::get(0);
}

inline void ArrayIntNull::erase(size_t ndx)
{
    Array::erase(ndx + 1);
}

inline void ArrayIntNull::erase(size_t begin, size_t end)
{
    Array::erase(begin + 1, end + 1);
}

inline void ArrayIntNull::clear()
{
    Array::truncate(0);
    Array::add(0);
}

inline void ArrayIntNull::move(size_t begin, size_t end, size_t dest_begin)
{
    Array::move(begin + 1, end + 1, dest_begin + 1);
}

inline bool ArrayIntNull::find(int cond, Action action, value_type value, size_t start, size_t end, size_t baseindex,
                               QueryState<int64_t>* state) const
{
    if (value) {
        return Array::find(cond, action, *value, start, end, baseindex, state, true /*treat as nullable array*/,
                           false /*search parameter given in 'value' argument*/);
    }
    else {
        return Array::find(cond, action, 0 /* unused dummy*/, start, end, baseindex, state,
                           true /*treat as nullable array*/, true /*search for null, ignore value argument*/);
    }
}

template <class cond, Action action, size_t bitwidth, class Callback>
bool ArrayIntNull::find(value_type value, size_t start, size_t end, size_t baseindex, QueryState<int64_t>* state,
                        Callback callback) const
{
    if (value) {
        return Array::find<cond, action>(*value, start, end, baseindex, state, std::forward<Callback>(callback),
                                         true /*treat as nullable array*/,
                                         false /*search parameter given in 'value' argument*/);
    }
    else {
        return Array::find<cond, action>(0 /*ignored*/, start, end, baseindex, state,
                                         std::forward<Callback>(callback), true /*treat as nullable array*/,
                                         true /*search for null, ignore value argument*/);
    }
}


template <class cond, Action action, size_t bitwidth>
bool ArrayIntNull::find(int64_t value, size_t start, size_t end, size_t baseindex, QueryState<int64_t>* state) const
{
    return Array::find<cond, action>(value, start, end, baseindex, state, true /*treat as nullable array*/,
                                     false /*search parameter given in 'value' argument*/);
}


template <class cond, Action action, class Callback>
bool ArrayIntNull::find(value_type value, size_t start, size_t end, size_t baseindex, QueryState<int64_t>* state,
                        Callback callback) const
{
    if (value) {
        return Array::find<cond, action>(*value, start, end, baseindex, state, std::forward<Callback>(callback),
                                         true /*treat as nullable array*/,
                                         false /*search parameter given in 'value' argument*/);
    }
    else {
        return Array::find<cond, action>(0 /*ignored*/, start, end, baseindex, state,
                                         std::forward<Callback>(callback), true /*treat as nullable array*/,
                                         true /*search for null, ignore value argument*/);
    }
}


template <Action action, class Callback>
bool ArrayIntNull::find_action(size_t index, value_type value, QueryState<int64_t>* state, Callback callback) const
{
    if (value) {
        return Array::find_action<action, Callback>(index, *value, state, callback, true /*treat as nullable array*/,
                                                    false /*search parameter given in 'value' argument*/);
    }
    else {
        return Array::find_action<action, Callback>(index, 0 /* ignored */, state, callback,
                                                    true /*treat as nullable array*/,
                                                    true /*search for null, ignore value argument*/);
    }
}


template <Action action, class Callback>
bool ArrayIntNull::find_action_pattern(size_t index, uint64_t pattern, QueryState<int64_t>* state,
                                       Callback callback) const
{
    return Array::find_action_pattern<action, Callback>(index, pattern, state, callback,
                                                        true /*treat as nullable array*/,
                                                        false /*search parameter given in 'value' argument*/);
}


template <class cond>
size_t ArrayIntNull::find_first(value_type value, size_t start, size_t end) const
{
    QueryState<int64_t> state(act_ReturnFirst, 1);
    if (value) {
        Array::find<cond, act_ReturnFirst>(*value, start, end, 0, &state, Array::CallbackDummy(),
                                           true /*treat as nullable array*/,
                                           false /*search parameter given in 'value' argument*/);
    }
    else {
        Array::find<cond, act_ReturnFirst>(0 /*ignored*/, start, end, 0, &state, Array::CallbackDummy(),
                                           true /*treat as nullable array*/,
                                           true /*search for null, ignore value argument*/);
    }

    if (state.m_match_count > 0)
        return to_size_t(state.m_state);
    else
        return not_found;
}

inline size_t ArrayIntNull::find_first(value_type value, size_t begin, size_t end) const
{
    return find_first<Equal>(value, begin, end);
}
} // namespace realm

#endif // REALM_ARRAY_INTEGER_HPP
