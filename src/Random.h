/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
 */

#ifndef SHAREMINDCOMMON_RANDOM_H
#define SHAREMINDCOMMON_RANDOM_H

#include <boost/type_traits/is_same.hpp>
#include <boost/type_traits/is_pointer.hpp>
#include "../Logger/Logger.h"
#include "RandomEngine.h"


namespace sharemind {

/**
 * This class is a front end to randomness generation.
 */
class Random {

public: /* Methods: */

    Random(Logger& logger, RandomEngineType m_engine = RNG_SNOW2);

    virtual ~Random();

    template <typename T>
    void fillValue (T& value) {
        m_engine->fillBytes (&value, sizeof (T));
    }

    template <typename T>
    T generateValue () {
        T result;
        fillValue (result);
        return result;
    }

    /**
     * \brief Fills a subrange of vector with random values.
     */
    template <typename T>
    void fillVector (std::vector<T>& vec, size_t begin, size_t end) {
        if (begin < end && end <= vec.size ()) {
            fillRange (vec.begin () + begin, vec.begin () + end);
        }
    }

    /**
     * \brief Fills entire vector with random values.
     */
    template <typename T>
    void fillVector (std::vector<T>& vec) {
        fillRange (vec.begin (), vec.end ());
    }

    /**
     * \brief Fills the range [begin, end) with random values.
     * \param[in] begin an iterator
     * \param[in] end an iterator
     *
     * This supplies efficient implementation for raw pointer ranges, and std::vector iterators.
     * For regular forward iterators this may be slow due to virtual call overhead.
     */
    template <typename Iter>
    void fillRange (const Iter begin, const Iter end) {
        fillRangeImpl_ (begin, end, boost::is_pointer<Iter>());
    }

private:

    template <typename T>
    inline void fillRangeImpl_ (T* const begin, T* const end,
                                const boost::true_type& /* is_pointer */) {
        return fillRangePointer_ (begin, end);
    }

    template <typename Iter>
    inline void fillRangeImpl_ (const Iter begin, const Iter end,
                                const boost::false_type& /* is_pointer */) {
        typedef typename std::iterator_traits<Iter>::value_type value_type;
        return fillRangeIterator_ (begin, end,
                                   typename std::iterator_traits<Iter>::iterator_category (),
                                   boost::is_same<Iter, typename std::vector<value_type>::iterator>());
    }

    /* Specialization for non-vector non-pointer iterator: */
    template <typename Iter>
    inline void fillRangeIterator_ (const Iter begin, const Iter end,
                                    const std::forward_iterator_tag,
                                    const boost::false_type& /* is_vector_iterator */)
    {
        typedef typename std::iterator_traits<Iter>::value_type value_type;
        for (Iter i = begin; i != end; ++ i) {
            *i = generateValue<value_type> ();
        }
    }

    /* Specialization for vector iterator: */
    template <typename Iter>
    inline void fillRangeIterator_ (const Iter begin, const Iter end,
                                    const std::random_access_iterator_tag,
                                    const boost::true_type& /* is_vector_iterator */)
    {
        typedef typename std::iterator_traits<Iter>::difference_type diff_type;
        typedef typename std::iterator_traits<Iter>::value_type value_type;
        const diff_type num = end - begin;
        if (num > 0) {
            m_engine->fillBytes (&*begin, sizeof (value_type) * num);
        }
    }

    /* Specialization for non-void pointer iterator: */
    template <typename T>
    inline void fillRangePointer_ (T* const begin, T* const end) {
        const ptrdiff_t num = end - begin;
        if (num > 0) {
            m_engine->fillBytes (begin, sizeof (T) * num);
        }
    }

private: /* Fields: */

    /** The actual engine used to generate randomness. */
    RandomEngine *  const  m_engine;

}; /* class Random { */

/* Specialization for void pointer iterator: */
template <>
inline void Random::fillRangePointer_<void> (void* const begin, void* const end) {
    const ptrdiff_t num = static_cast<char*>(end) - static_cast<char*>(begin);
    if (num > 0) {
        m_engine->fillBytes (begin, num);
    }
}


} /* namespace sharemind { */

#endif /* SHAREMINDCOMMON_RANDOM_H */
