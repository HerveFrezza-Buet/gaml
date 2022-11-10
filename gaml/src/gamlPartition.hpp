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

#include <gamlMerge.hpp>
#include <iterator>

namespace gaml {
  namespace concepts {
    template<typename DataIterator>
    class Partition {
    public:

      using value_type = typename std::iterator_traits<DataIterator>::value_type ;

      Partition(const Partition& copy);

      using iterator = DataIterator;
      using complement_iterator = DataIterator;

      /**
       * This gives the number of subsets in the parition.
       */
      unsigned int size(void) const;

      /**
       * This gives the number of elements in the whole set.
       */
      unsigned int data_size(void) const;

      /**
       * This gives the begin iterator for subset number i (subset number is in [O..size[).
       */
      iterator begin(unsigned int i) const;

      /**
       * This gives the end iterator for subset number i (subset number is in [O..size[).
       */
      iterator end(unsigned int i) const;

      /**
       * This gives the begin iterator for the complement of subset number i (subset number is in [O..size[).
       */
      complement_iterator complement_begin(unsigned int i) const;

      /**
       * This gives the end iterator for the complement of subset number i (subset number is in [O..size[).
       */
      complement_iterator complement_end(unsigned int i) const;
    };
  }

  namespace partition {


    template<typename DataIterator>
    class KFold {
    public:

      using value_type = typename std::iterator_traits<DataIterator>::value_type ;
      using iterator = DataIterator;
      using complement_iterator = gaml::MergeIterator<DataIterator,DataIterator>;

    private:

      unsigned int dsize;
      unsigned int last_set_id;
      unsigned int K;
      iterator begin_iter;
      iterator end_iter;

      gaml::Merge<DataIterator,DataIterator> complement(unsigned int i) const {
	if(i==0) 
	  return gaml::Merge<DataIterator,DataIterator>(end(0),end(0),end(0),end_iter);
	if(i==last_set_id)
	  return gaml::Merge<DataIterator,DataIterator>(begin_iter,begin(last_set_id),begin(last_set_id),begin(last_set_id));
	return  gaml::Merge<DataIterator,DataIterator>(begin_iter,begin(i),end(i),end_iter);
      }

    public:

      KFold(const KFold& cp) 
	: dsize(cp.dsize),
	  last_set_id(cp.last_set_id),
	  K(cp.K),
	  begin_iter(cp.begin_iter),
	  end_iter(cp.end_iter) {}

      /**
       * This fits gaml::concepts::Partition
       * @param k The number of subsets.
       */
      KFold(const iterator& data_begin,
	    const iterator& data_end,
	    unsigned int k) 
	: dsize(std::distance(data_begin,data_end)),
	  last_set_id(k-1),
	  K(k),
	  begin_iter(data_begin),
	  end_iter(data_end) { }

      unsigned int size(void) const {
	return K;
      }

      unsigned int data_size(void) const {
	return dsize;
      }

      iterator begin(unsigned int i) const {
	iterator res = begin_iter;
	std::advance(res,(int)(i*dsize/(double)K+.5));
	return res;
      }

      iterator end(unsigned int i) const {
	return begin(i+1);
      }

      complement_iterator complement_begin(unsigned int i) const {
	return complement(i).begin();
      }

      complement_iterator complement_end(unsigned int i) const {
	return complement(i).end();
      }
    };


    template<typename DataIterator>
    class Chunk {
    public:

      using value_type = typename std::iterator_traits<DataIterator>::value_type;
      using iterator = DataIterator;
      using complement_iterator = gaml::MergeIterator<DataIterator,DataIterator>;

    private:

      unsigned int dsize;
      unsigned int set_size;
      unsigned int nb_sets;
      unsigned int last_set_id;
      iterator begin_iter;
      iterator end_iter;

      gaml::Merge<DataIterator,DataIterator> complement(unsigned int i) const {
	if(i==0) 
	  return gaml::Merge<DataIterator,DataIterator>(end(0),end(0),end(0),end_iter);
	if(i==last_set_id)
	  return gaml::Merge<DataIterator,DataIterator>(begin_iter,begin(last_set_id),begin(last_set_id),begin(last_set_id));
	return  gaml::Merge<DataIterator,DataIterator>(begin_iter,begin(i),end(i),end_iter);
      }

    public:

      Chunk(const Chunk& cp) 
	: dsize(cp.dsize),
	  set_size(cp.set_size),
	  nb_sets(cp.nb_sets),
	  last_set_id(cp.last_set_id),
	  begin_iter(cp.begin_iter),
	  end_iter(cp.end_iter) {}

      /**
       * This fits gaml::concepts::Partition
       * @param chunk_size The size of each chunk. The last one may be smaller.
       */
      Chunk(const iterator& data_begin,
	    const iterator& data_end,
	    unsigned int chunk_size) 
	: dsize(std::distance(data_begin,data_end)),
	  set_size(chunk_size),
	  begin_iter(data_begin),
	  end_iter(data_end) {
	nb_sets = dsize / chunk_size;
	if(dsize % chunk_size != 0)
	  ++nb_sets;
	last_set_id = nb_sets-1;
      }

      unsigned int size(void) const {
	return nb_sets;
      }

      unsigned int data_size(void) const {
	return dsize;
      }

      iterator begin(unsigned int i) const {
	iterator res = begin_iter;
	std::advance(res,i*set_size);
	return res;
      }

      iterator end(unsigned int i) const {
	if(i==last_set_id)
	  return end_iter;
	iterator res = begin_iter;
	std::advance(res,(i+1)*set_size);
	return res;
      }

      complement_iterator complement_begin(unsigned int i) const {
	return complement(i).begin();
      }

      complement_iterator complement_end(unsigned int i) const {
	return complement(i).end();
      }
    };

    namespace internal {

      class KFold {
      private:
	unsigned int k;
      public:

	KFold(unsigned int nb) : k(nb) {}
	KFold(const KFold& other) : k(other.k) {}
	KFold& operator=(const KFold& other) {k = other.k;return *this;}

	template<typename DataIterator>
	gaml::partition::KFold<DataIterator> build(const DataIterator& data_begin,
						 const DataIterator& data_end) const {
	  return gaml::partition::KFold<DataIterator>(data_begin,data_end,k);
	}
      };

      
      class Chunk {
      private:
	unsigned int k;
      public:

	Chunk(unsigned int nb) : k(nb) {}
	Chunk(const Chunk& other) : k(other.k) {}
	Chunk& operator=(const Chunk& other) {k = other.k;return *this;}

	template<typename DataIterator>
	gaml::partition::Chunk<DataIterator> build(const DataIterator& data_begin,
						 const DataIterator& data_end) const {
	  return gaml::partition::Chunk<DataIterator>(data_begin,data_end,k);
	}
      };
    }

    inline internal::KFold kfold(unsigned int k) {return internal::KFold(k);}
    inline internal::Chunk chunk(unsigned int k) {return internal::Chunk(k);}
    inline internal::Chunk leave_one_out(void)   {return internal::Chunk(1);}
  }

}
