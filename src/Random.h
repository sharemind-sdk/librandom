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
    void fillRange (Iter begin, Iter end) {
        typedef typename std::iterator_traits<Iter>::value_type value_type;
        fillRangeImpl_ (begin, end,
            typename std::iterator_traits<Iter>::iterator_category (),
            boost::is_pointer<Iter>(),
            boost::is_same<Iter, typename std::vector<value_type>::iterator>());
    }

private:

    template <typename Iter>
    void fillRangeImpl_ (Iter begin, Iter end,
                         std::forward_iterator_tag,
                         const boost::false_type& /* is pointer */,
                         const boost::false_type& /* is vector iterator */)
    {
        for (Iter i = begin; i != end; ++ i) {
            fillValue (*i);
        }
    }

    template <typename Iter>
    void fillRangeImpl_ (Iter begin, Iter end,
                         std::random_access_iterator_tag,
                         const boost::false_type& /* is pointer */,
                         const boost::true_type& /* is vector iterator */)
    {
        typedef typename std::iterator_traits<Iter>::difference_type diff_type;
        typedef typename std::iterator_traits<Iter>::value_type value_type;
        const diff_type num = end - begin;
        if (num > 0) {
            m_engine->fillBytes (&*begin, sizeof (value_type) * num);
        }
    }

    template <typename T>
    void fillRangeImpl_ (T* begin, T* end,
                         std::random_access_iterator_tag,
                         const boost::true_type& /* is pointer */,
                         const boost::false_type& /* is vector iterator */)
    {
        const ptrdiff_t num = end - begin;
        if (num > 0) {
            m_engine->fillBytes (begin, sizeof (T) * num);
        }
    }

private: /* Fields: */

    RandomEngine*  const  m_engine; ///< The actual engine used to generate randomness
};

} // namespace sharemind

#endif // SHAREMINDCOMMON_RANDOM_H
