#pragma once

/*
 *   Copyright (C) 2012,  Supelec
 *
 *   Author : Hervé Frezza-Buet, Frédéric Pennerath 
 *
 *   Contributor :
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU General Public
 *   License (GPL) as published by the Free Software Foundation; either
 *   version 3 of the License, or any later version.
 *   
 *   This library is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 *   General Public License for more details.
 *   
 *   You should have received a copy of the GNU General Public
 *   License along with this library; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 *   Contact : herve.frezza-buet@supelec.fr, frederic.pennerath@supelec.fr
 *
 */

#include <iterator>
#include <type_traits>
#include <tuple>
#include <optional>

namespace gaml {

  template<typename T>
  struct Range {
    using value_type = typename std::iterator_traits<T>::value_type;
    using iterator_type = T;

    T begin;
    T end;

    Range() = default;
    Range(const T& begin, const T& end): begin(begin), end(end) {}
    Range(Range&&) = default;
    Range& operator=(Range&&) = default;
  };

  template<typename T>
  Range<T> range(const T& begin, const T& end) {
    return {begin, end};
  }

  template<typename... T> class ZipIterator;
  
  template<typename Head, typename... Tail>
  class ZipIterator<Head, Tail...> {
  private:
    typename Head::iterator_type head;
    ZipIterator<Tail...> tail;

  public:
    using difference_type = long;
    using value_type = decltype(std::tuple_cat(std::declval<std::tuple<typename Head::value_type>>(), *tail)); // const typename Head::value_type& generates bad memory read.
    using pointer           = value_type*;
    using reference         = value_type&;
    using iterator_category = std::random_access_iterator_tag;

  private:

    mutable std::optional<value_type> value;
    
  public:

    ZipIterator() = default;
    ZipIterator(const ZipIterator& cp) = default;
    ZipIterator(const typename Head::iterator_type& head, ZipIterator<Tail...> tail) : head(head), tail(tail) {}
    ZipIterator& operator=(const ZipIterator& cp) = default;
    ZipIterator& operator++() {++head; ++tail; value.reset(); return *this;}
    ZipIterator& operator--() {--head; --tail; value.reset(); return *this;}
    ZipIterator& operator+=(difference_type diff) {std::advance(head,  diff); tail += diff; value.reset(); return *this;}
    ZipIterator& operator-=(difference_type diff) {std::advance(head, -diff); tail -= diff; value.reset(); return *this;}
    ZipIterator operator++(int) {ZipIterator res = *this; ++*this; return res;}
    ZipIterator operator--(int) {ZipIterator res = *this; --*this; return res;}
    difference_type operator-(const ZipIterator& i) const {return std::distance(i.head, head);}
    ZipIterator operator+(difference_type i) const {auto cpy = *this; return (cpy+=i);}
    ZipIterator operator-(difference_type i) const {auto cpy = *this; return (cpy-=i);}
    const value_type& operator*() const {
      if(!value)
	value = std::tuple_cat(std::tuple<typename Head::value_type>(*head), *tail);
      return value.value();} 
    bool operator==(const ZipIterator& i) const {return (head == i.head) && (tail == i.tail);}
    bool operator!=(const ZipIterator& i) const {return (head != i.head) || (tail != i.tail);}
  };

  template<>
  class ZipIterator<> {
  public:
    using difference_type   = long;
    using value_type        = std::tuple<>;
    using pointer           = value_type*;
    using reference         = value_type&;
    using iterator_category = std::random_access_iterator_tag;
    
  private:

    mutable value_type value;
    
  public:

    ZipIterator() = default;
    ZipIterator(const ZipIterator& cp) = default;
    ZipIterator& operator=(const ZipIterator& cp) = default;
    ZipIterator& operator++() {return *this;}
    ZipIterator& operator--() {return *this;}
    ZipIterator& operator+=(difference_type diff) { return *this;}
    ZipIterator& operator-=(difference_type diff) { return *this;}
    ZipIterator operator++(int) {ZipIterator res = *this; ++*this; return res;}
    ZipIterator operator--(int) {ZipIterator res = *this; --*this; return res;}
    //difference_type operator-(const ZipIterator& i) const {return 0;}
    ZipIterator operator+(difference_type i) const {auto cpy = *this; return (cpy+=i);}
    ZipIterator operator-(difference_type i) const {auto cpy = *this; return (cpy-=i);}
    const value_type& operator*() const {value = std::tuple<>(); return value;}
    bool operator==(const ZipIterator& i) const {return true;}
    bool operator!=(const ZipIterator& i) const {return false;}
  };
		    

  template<typename... T> class Zip;
  
  template<typename Head, typename... Tail>
  class Zip<Head, Tail...> {
  private:
    using iterator_type = typename Head::iterator_type;
    
    iterator_type begin_;
    iterator_type end_;
    Zip<Tail...> tail;

  public:
    Zip(const Head& range, const Tail&... tail): begin_(range.begin), end_(range.end), tail(tail...){}
    
    ZipIterator<Head, Tail...> begin() const {
      return ZipIterator<Head, Tail...>(begin_, tail.begin());
    }

    ZipIterator<Head, Tail...> end() const {
      return ZipIterator<Head, Tail...>(end_, tail.end());
    } 
    

  };

  template<>
  class Zip<> {

  public:
    Zip() {}

    ZipIterator<> begin() const {
      return ZipIterator<>();
    }

    ZipIterator<> end() const {
      return ZipIterator<>();
    } 

  };

  /**
   * This zips collections. <b>Warning : </b> For the sake of
   * efficiency, if one of the iteratrors to be zipped is a random
   * access iterator, put it first.
   */
  template<typename... T>
  Zip<T...> zip(T&&... args) {
    return Zip<T...>(std::forward<T>(args)...);
  }

}
