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


  template<typename... T> class ZipIterator;
  
  template<typename Head, typename... Tail,
	   typename = std::enable_if_t<sizeof...(Tail) != 0>>
  class ZipIterator {
  public:
    using difference_type = long;
  };

  template<typename Head>
  class ZipIterator {
  private:
    Head it;
  public:
    using difference_type   = long;
    using value_type        = std::tuple<Head>;
    using pointer           = value_type*;
    using reference         = value_type&;
    using iterator_category = std::random_acess_iterator_tag;

    ZipIterator() = default;
    ZipIterator(const ZipIterator& cp) = default;
    ZipIterator(const Head& it) : it(it) {}
    ZipIterator& operator=(const ZipIterator& cp) = default;
    ZipIterator& operator++() {++it; return *this;}
    ZipIterator& operator--() {--it; return *this;}
    ZipIterator& operator+=(difference_type diff) { std::advance(it, diff); return *this;}
    ZipIterator& operator-=(difference_type diff) { it-=diff; return *this;}
    ZipIterator operator++(int) {ZipIterator res = *this; ++*this; return res;}
    ZipIterator operator--(int) {ZipIterator res = *this; --*this; return res;}
    int operator-(const ZipIterator& i) const {return j - i.j;}
    ZipIterator operator+(int i) const {return ZipIterator(j+i);}
    ZipIterator operator-(int i) const {return ZipIterator(j-i);}
    const int& operator*() const {return j;}
    bool operator==(const ZipIterator& i) const {return j == i.j;}
    bool operator!=(const ZipIterator& i) const {return j != i.j;}
  };
		    

  template<typename... T> class zip;
  
  template<typename Head, typename... Tail,
	   typename = std::enable_if_t<sizeof...(Tail) != 0> >
  class zip {
  private:
    Head begin;
    Head end;
    zip<Tail...> tail;

  public:
    zip(const Head& range, const Tail&... tail): begin(range.first), end(range.second), tail(tail...){}

    ZipIterator<Head, Tail...> begin() const {
      return ZipIterator(begin, tail.begin());
    }

    ZipIterator<Head, Tail...> end() const {
      return ZipIterator(end, tail.end());
    }    
  };

  template<typename Head>
  class zip {
  private:
    Head begin;
    Head end;
    
  public:
    zip(const std::pair<Head, Head>& range): begin(range.first), end(range.second) {}

    ZipIterator<Head> begin() const {
      return ZipIterator<Head>(begin);
    }

    ZipIterator<Head> end() const {
      return ZipIterator<Head>(end);
    } 
  };

  
}
