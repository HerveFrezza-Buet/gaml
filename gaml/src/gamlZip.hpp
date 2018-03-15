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

namespace gaml {

  template<typename T>
  struct Range {
    using value_type = std::iterator_traits<T>::value_type;
    using iterator_type = T;

    T begin;
    T end;

    Range() = default;
    Range(const T& begin, const T& end): begin(begin), end(end) {}
    Range(const Range&) = default;
    Range& operator=(const Range&) = default;
  };

  template<typename T>
  Range<T> range(const T& begin, const T& end) {
    return {begin, end};
  }

  

  template<typename... T> class ZipIterator;
  
  template<typename Head, typename... Tail>
  class ZipIterator<Head, Tail...> {
  private:
    typename Head::first_type head;
    ZipIterator<Tail...> tail;
    
  public:
    using difference_type = long;
    using value_type        = decltype(std::tuple_cat(std::declval<std::tuple<const typename Head::first_type&>>(),*tail));
    using pointer           = value_type*;
    using reference         = value_type&;
    using iterator_category = std::random_access_iterator_tag;

    ZipIterator() = default;
    ZipIterator(const ZipIterator& cp) = default;
    ZipIterator(const Head& head, ZipIterator<Tail...> tail) : head(head), tail(tail) {}
    ZipIterator& operator=(const ZipIterator& cp) = default;
    ZipIterator& operator++() {++head; ++tail; return *this;}
    ZipIterator& operator--() {--head; --tail; return *this;}
    ZipIterator& operator+=(difference_type diff) { std::advance(head, diff); tail+= diff; return *this;}
    ZipIterator& operator-=(difference_type diff) { std::advance(head, -diff); tail-=diff; return *this;}
    ZipIterator operator++(int) {ZipIterator res = *this; ++*this; return res;}
    ZipIterator operator--(int) {ZipIterator res = *this; --*this; return res;}
    difference_type operator-(const ZipIterator& i) const {return std::distance(i.head, head);}
    ZipIterator operator+(difference_type i) const {auto cpy = *this; return (cpy+=i);}
    ZipIterator operator-(difference_type i) const {auto cpy = *this; return (cpy-=i);}
    value_type operator*() const {return std::tuple<const Head&>(*head);}
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
    value_type operator*() const {return std::tuple<>();}
    bool operator==(const ZipIterator& i) const {return true;}
    bool operator!=(const ZipIterator& i) const {return false;}
  };
		    

  template<typename... T> class Zip;
  
  template<typename Head, typename... Tail>
  class Zip<Head, Tail...> {
  private:
    typename Head::first_type begin_;
    typename Head::second_type end_;
    Zip<Tail...> tail;

  public:
    Zip(const Head& range, const Tail&... tail): begin_(range.first), end_(range.second), tail(tail...){}
    
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

  template<typename... T>
  Zip<T...> zip(T&&... args) {
    return Zip<T...>(std::forward<T>(args)...);
  }

  // template<typename... T> class ZipIterator;
  
  // template<typename Head, typename... Tail>
  // class ZipIterator<Head, Tail...> {
  // private:
  //   Head head;
  //   ZipIterator<Tail...> tail;
    
  // public:
  //   using difference_type = long;
  //   using value_type        = decltype(std::tuple_cat(std::declval<std::tuple<const Head&>>(),*tail));
  //   using pointer           = value_type*;
  //   using reference         = value_type&;
  //   using iterator_category = std::random_access_iterator_tag;

  //   ZipIterator() = default;
  //   ZipIterator(const ZipIterator& cp) = default;
  //   ZipIterator(const Head& head, ZipIterator<Tail...> tail) : head(head), tail(tail) {}
  //   ZipIterator& operator=(const ZipIterator& cp) = default;
  //   ZipIterator& operator++() {++head; ++tail; return *this;}
  //   ZipIterator& operator--() {--head; --tail; return *this;}
  //   ZipIterator& operator+=(difference_type diff) { std::advance(head, diff); tail+= diff; return *this;}
  //   ZipIterator& operator-=(difference_type diff) { std::advance(head, -diff); tail-=diff; return *this;}
  //   ZipIterator operator++(int) {ZipIterator res = *this; ++*this; return res;}
  //   ZipIterator operator--(int) {ZipIterator res = *this; --*this; return res;}
  //   difference_type operator-(const ZipIterator& i) const {return std::distance(i.head, head);}
  //   ZipIterator operator+(difference_type i) const {auto cpy = *this; return (cpy+=i);}
  //   ZipIterator operator-(difference_type i) const {auto cpy = *this; return (cpy-=i);}
  //   value_type operator*() const {return std::tuple<const Head&>(*head);}
  //   bool operator==(const ZipIterator& i) const {return (head == i.head) && (tail == i.tail);}
  //   bool operator!=(const ZipIterator& i) const {return (head != i.head) || (tail != i.tail);}
  // };

  // template<typename Head>
  // class ZipIterator<Head> {
  // private:
  //   Head head;
  // public:
  //   using difference_type   = long;
  //   using value_type        = std::tuple<const Head&>;
  //   using pointer           = value_type*;
  //   using reference         = value_type&;
  //   using iterator_category = std::random_access_iterator_tag;

  //   ZipIterator() = default;
  //   ZipIterator(const ZipIterator& cp) = default;
  //   ZipIterator(const Head& head) : head(head) {}
  //   ZipIterator& operator=(const ZipIterator& cp) = default;
  //   ZipIterator& operator++() {++head; return *this;}
  //   ZipIterator& operator--() {--head; return *this;}
  //   ZipIterator& operator+=(difference_type diff) { std::advance(head, diff); return *this;}
  //   ZipIterator& operator-=(difference_type diff) { std::advance(head, -diff); return *this;}
  //   ZipIterator operator++(int) {ZipIterator res = *this; ++*this; return res;}
  //   ZipIterator operator--(int) {ZipIterator res = *this; --*this; return res;}
  //   difference_type operator-(const ZipIterator& i) const {return std::distance(i.head, head);}
  //   ZipIterator operator+(difference_type i) const {auto cpy = *this; return (cpy+=i);}
  //   ZipIterator operator-(difference_type i) const {auto cpy = *this; return (cpy-=i);}
  //   value_type operator*() const {return std::tuple<const Head&>(*head);}
  //   bool operator==(const ZipIterator& i) const {return head == i.head;}
  //   bool operator!=(const ZipIterator& i) const {return head != i.head;}
  // };
		    

  // template<typename... T> class Zip;
  
  // template<typename Head, typename... Tail>
  // class Zip<Head, Tail...> {
  // private:
  //   Head begin_;
  //   Head end_;
  //   Zip<Tail...> tail;

  // public:
  //   Zip(const Head& range, const Tail&... tail): begin_(range.first), end_(range.second), tail(tail...){}

  //   ZipIterator<Head, Tail...> begin() const {
  //     return ZipIterator<Head, Tail...>(begin_, tail.begin());
  //   }

  //   ZipIterator<Head, Tail...> end() const {
  //     return ZipIterator<Head, Tail...>(end_, tail.end());
  //   }    
  // };

  // template<typename Head>
  // class Zip<Head> {
  // private:
  //   Head begin_;
  //   Head end_;
    
  // public:
  //   Zip(const std::pair<Head, Head>& range): begin_(range.first), end_(range.second) {}

  //   ZipIterator<Head> begin() const {
  //     return ZipIterator<Head>(begin_);
  //   }

  //   ZipIterator<Head> end() const {
  //     return ZipIterator<Head>(end_);
  //   } 
  // };

  // template<typename... T>
  // Zip<T...> zip(T&&... args) {
  //   return Zip<T...>(std::forward<T>(args)...);
  // }
  
}
