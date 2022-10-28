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

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <list>
#include <array>
#include <tuple>
#include <stdexcept>
#include <iterator>
#include <utility>
#include <numeric>
#include <algorithm>
#include <cmath>

#include <cstdlib>
#include <gamlMerge.hpp>
#include <gamlPartition.hpp>
#include <gamlMap.hpp>
#include <iostream>
#include <iomanip>
#include <random>
#include <type_traits>

namespace gaml {


  namespace concepts {

    typedef int any;
    /**
     * @short This predicts a label from an input.
     */ 
    class Predictor {
    public:
      
      typedef any input_type;
      typedef any output_type;
      
      Predictor(const Predictor& other);
      Predictor& operator=(const Predictor& other);
      output_type operator()(const input_type& x) const;
    };

    /**
     * @short This learns from a data set.
     */ 
    class Learner {
    public:
      
      typedef any predictor_type;
      
      Learner(const Learner& other);
      Learner& operator=(const Learner& other);

      template<typename DataIterator, typename InputOf, typename OutputOf> 
      predictor_type operator()(const DataIterator& begin, const DataIterator& end,
				const InputOf&, const OutputOf&) const;
    };


    /**
     * @short Evaluation of a predictor on a data set.
     */
    class PredictorEvaluator {
    public:

      typedef double value_type;

      PredictorEvaluator(const PredictorEvaluator& other);

      template<typename Predictor, typename DataIterator, typename InputOf, typename OutputOf> 
      value_type operator()(const Predictor& predictor,const DataIterator& begin, const DataIterator& end,
			    const InputOf&, const OutputOf&) const;
    };

    
    /**
     * @short Evaluation of a predictor on a data set.
     */
    class LearnerEvaluator {
    public:

      typedef double value_type;

      LearnerEvaluator(const LearnerEvaluator& other);

      template<typename Learner, typename DataIterator, typename InputOf, typename OutputOf> 
      value_type operator()(const Learner& learner,const DataIterator& begin, const DataIterator& end,
			    const InputOf&, const OutputOf&) const;
    };
    
  }

  namespace risk {

    template<typename Predictor,typename DataIterator,
	     typename InputOf, typename OutputOf,
	     typename Loss, typename AccumIterator>
    double accumulation(const Predictor& predictor, 
			const DataIterator& begin, const DataIterator& end,
			const InputOf& inputOf,const OutputOf& outputOf, 
			const Loss& loss, AccumIterator& acc) {
      for(DataIterator it = begin; it != end; ++it) 
	*acc++ = loss(predictor(inputOf(*it)),
		      outputOf(*it));
      return (double)(acc());
    }
    
    template<typename LOSS>
    class Empirical {
    private:

      LOSS loss;

    public:

      Empirical(const LOSS& l) : loss(l) {}
      Empirical(const Empirical& other) : loss(other.loss) {}

      template<typename Predictor, typename DataIterator, typename InputOf, typename OutputOf> 
      double operator()(const Predictor& predictor,const DataIterator& begin, const DataIterator& end,
			const InputOf& inputOf, const OutputOf& outputOf) const {
	unsigned int nb = 0;
	double sum = 0;
	for(DataIterator it = begin; it != end; ++it) {
	  auto& data = *it;
	  sum += loss(predictor(inputOf(data)),
		      outputOf(data));
	  ++nb;
	}

	return sum/nb;
      }
    };

    template<typename LOSS>
    Empirical<LOSS> empirical(const LOSS& loss) {return Empirical<LOSS>(loss);}
    

    template<typename LOSS, typename PARTITION>
    class CrossValidation {
    private:
      
      LOSS loss;
      PARTITION partition;
      bool verbose;

    public:


      CrossValidation(const LOSS& l, const PARTITION& part,bool verbosity) 
	: loss(l), partition(part), verbose(verbosity) {}
      CrossValidation(const CrossValidation& other) : loss(other.loss), partition(other.partition), verbose(other.verbose) {}

      template<typename Learner, typename DataIterator, typename InputOf, typename OutputOf> 
      double operator()(const Learner& learner,const DataIterator& begin, const DataIterator& end,
			const InputOf& inputOf, const OutputOf& outputOf) const {

	auto built_partition = partition.build(begin,end);
	double sum = 0;
	
	if(verbose)
	  std::cout << "Splitting the database into " << built_partition.size() << " sets." << std::endl;
	for(unsigned int i = 0; i < built_partition.size(); ++i) {
	  
	  auto begin  = built_partition.begin(i);
	  auto end    = built_partition.end(i);
	  auto _begin = built_partition.complement_begin(i);
	  auto _end   = built_partition.complement_end(i);
	  
	  if(verbose)
	    std::cout << std::setw(6) << i+1 << '/' << built_partition.size() 
		      << " : learning...\r" << std::flush;
	  
	  auto predictor = learner(_begin, _end, inputOf, outputOf);
	  

	  double risk=0;
	  for(DataIterator it = begin; it != end; ++it) {
	    auto& data = *it;
	    risk += loss(predictor(inputOf(data)),
			 outputOf(data));
	  }

	  if(verbose) {
	    auto size = std::distance(begin,end);
	    std::cout << std::setw(6) << i+1 << '/' << built_partition.size() 
		      << " : risk = " << risk / size << " (" 
		      << size << "-sized test set)" << std::endl;
	  }
	  
	  sum += risk;
	}
	
	return (double)(sum/(double)(built_partition.data_size()));
      }
    };
    
    template<typename LOSS, typename PARTITION>
    CrossValidation<LOSS,PARTITION> cross_validation(const LOSS& l, const PARTITION& part,bool verbosity) {
      return CrossValidation<LOSS,PARTITION>(l,part,verbosity);
    }

  }

  /**
   * This iterator is not adapted to serve as a basis for tabular collections, since it do not refer to a value that exists independently from the iterator.
   */
  class integer {
  private:
    int j;
  public:
     
    using difference_type   = long;
    using value_type        = int;
    using pointer           = value_type*;
    using reference         = value_type&;
    using iterator_category = std::random_access_iterator_tag;

    
    integer() : j(0) {}
    integer(const integer& cp) : j(cp.j) {}
    integer(int i) : j(i) {}
    integer& operator=(int i) {j=i; return *this;}
    integer& operator=(const integer& cp) {j=cp.j; return *this;}
    integer& operator++() {++j; return *this;}
    integer& operator--() {--j; return *this;}
    integer& operator+=(int diff) {j+=diff; return *this;}
    integer& operator-=(int diff) {j-=diff; return *this;}
    integer operator++(int) {integer res = *this; ++*this; return res;}
    integer operator--(int) {integer res = *this; --*this; return res;}
    int operator-(const integer& i) const {return j - i.j;}
    integer operator+(int i) const {return integer(j+i);}
    integer operator-(int i) const {return integer(j-i);}
    const int& operator*() const {return j;}
    bool operator==(const integer& i) const {return j == i.j;}
    bool operator!=(const integer& i) const {return j != i.j;}
  };

  namespace functor {
    class average {
    public:
      typedef double output_type;
      template<typename DataIterator,typename ValueOf> 
      double operator()(const DataIterator& begin, const DataIterator& end, const ValueOf& value_of) const {
	if(begin == end)
	  throw std::runtime_error("Average called on an empty collection");

	// std::accumulate does not work... Is there a problem in map iterator définition ?
    
	// auto values = gaml::map(begin,end,value_of);
	// return (Y)(std::accumulate(values.begin(),values.end(),0)/(double)nb);

	// The version without std::accumulate does not require nb.
	double sum = 0;
	unsigned int nb = 0;
	for(auto it = begin; it != end; ++it, ++nb) sum += (double)(value_of(*it));
	return sum/(double)nb;
      }
    };
  }

  /**
   * @returns The average of the values in the collection. Error in
   * case of empty collection is not tested.
   */
  template<typename DataIterator,typename ValueOf> 
  double average(const DataIterator& begin, const DataIterator& end, const ValueOf& value_of) {
    functor::average avg;
    return avg(begin,end,value_of);
  }

  namespace functor {
    class variance {
    public:
      typedef double output_type;
      template<typename DataIterator,typename ValueOf> 
      double operator()(const DataIterator& begin, const DataIterator& end, const ValueOf& value_of) const {
	double n    = 0;
	double mean = 0;
	double M2   = 0;
	for(auto it = begin; it != end; ++it) {
	  double x =  (double)(value_of(*it));
	  double delta = x - mean;
	  mean = mean + delta/(++n);
	  M2 = M2 + delta*(x - mean);
	}
	if (n < 2) return 0;
	return M2/(n - 1);
      }
    };
  }

  /**
   * @returns The variance of the values in the collection. Error in
   * case of empty collection is not tested.
   */
  template<typename DataIterator,typename ValueOf> 
  double variance(const DataIterator& begin, const DataIterator& end, const ValueOf& value_of) {
    functor::variance var;
    return var(begin,end,value_of);
  }

  namespace by_default {
    template<typename VALUE>
    struct LesserThan {
      bool operator()(const VALUE& v1, const VALUE& v2) const {
	return v1 < v2;
      }
    };
  }

  namespace functor {
    template<typename VALUE, typename COMP = by_default::LesserThan<VALUE> > 
    class frequencies {
    public:
      template<typename DataIterator, typename ValueOf>
      std::map<VALUE,double,COMP> operator()(const DataIterator& begin, const DataIterator& end, const ValueOf& value_of) const {

	if(begin == end)
	  throw std::runtime_error("Frequencies called on an empty collection");

	std::map<VALUE,double,COMP> frequencies;
	for(auto it = begin; it != end; ++it) {
	  auto value = value_of(*it);
	  auto mapit = frequencies.find(value);
	  if(mapit == frequencies.end())
	    frequencies[value] = 1;
	  else
	    ++(mapit->second);
	}

	double size = (double)(std::distance(begin,end));
	for(auto& kv : frequencies) kv.second /= size;
	return frequencies;
      }
    };
  }

  /**
   * @returns The map of (value,frequency) pairs.
   */
  template<typename VALUE, typename COMP = by_default::LesserThan<VALUE>, typename DataIterator, typename ValueOf> 
  std::map<VALUE,double,COMP> frequencies(const DataIterator& begin, const DataIterator& end, const ValueOf& value_of) {
    functor::frequencies<VALUE,COMP> f;
    return f(begin,end,value_of);
  }
  
  /**
   * @returns The value in the (value,frequency) map which have the highest frequency.
   * case of empty collection is not tested.
   */
  template<typename Map>
  typename Map::key_type most_frequent(const Map& frequencies) {
    
    auto it = std::max_element(frequencies.begin(), frequencies.end(),
			       [](const std::pair<typename Map::key_type,double>& e1,
				  const std::pair<typename Map::key_type,double>& e2) -> bool {return e1.second < e2.second;});
    return it->first;
  }
  

  namespace functor {
    template<typename VALUE, typename COMP = by_default::LesserThan<VALUE>>
      class most_frequent {
      public:
	typedef VALUE output_type;
	template<typename DataIterator, typename ValueOf>
	VALUE operator()(const DataIterator& begin, const DataIterator& end, const ValueOf& value_of) const {
	  return (VALUE)(gaml::most_frequent(gaml::frequencies<VALUE,COMP>(begin,end,value_of)));
	}
      };
  }

  /**
   * @returns The (value,frequency) pair of the most frequent value from values in the collection. Error in
   * case of empty collection is not tested.
   */
  template<typename VALUE, typename COMP = by_default::LesserThan<VALUE>, typename DataIterator, typename ValueOf> 
  VALUE most_frequent(const DataIterator& begin, const DataIterator& end, const ValueOf& value_of) {
    functor::most_frequent<VALUE,COMP> mf;
    return mf(begin,end,value_of);
  }
  

  namespace functor {
    template<typename VALUE>
    class highest_cumulated_frequency {
    public:
      typedef VALUE output_type;
      template<typename DataIterator, typename ValueOf>
      VALUE operator()(const DataIterator& begin, const DataIterator& end, const ValueOf& value_of) const {
	auto it = begin;
	auto cumulated_frequencies = value_of(*it);
	for(++it; it != end; ++it)
	  for(auto& kv : value_of(*it)) {
	    auto fit = cumulated_frequencies.find(kv.first);
	    if(fit == cumulated_frequencies.end())
	      cumulated_frequencies[kv.first] = kv.second;
	    else
	      fit->second += kv.second;
	  }
	return (VALUE)(gaml::most_frequent(cumulated_frequencies));
      }
    };
  }

  /**
   * @param begin,end iterates on a collection of frequencies (i.e label/freq maps).
   * @returns the label for which the cumulated frequencies is the highest.
   */
  template<typename VALUE, typename DataIterator, typename ValueOf> 
  VALUE highest_cumulated_frequency(const DataIterator& begin, const DataIterator& end, const ValueOf& value_of) {
    functor::highest_cumulated_frequency<VALUE> hcf;
    return hcf(begin,end,value_of);
  }

  /**
   * Returns the classification entropy of a data set.
   */
  template<typename Class, typename DataIterator, typename ClassComp = by_default::LesserThan<Class>, typename ClassOf> 
  double classification_entropy(const DataIterator& begin, const DataIterator& end, const ClassOf& class_of) {
    auto freq = gaml::frequencies<Class,ClassComp>(begin,end,class_of);
    double H = 0;
    for(auto& kv : freq) {
      double p = kv.second;
      H -= p*std::log2(p);
    }
    return H;
  }
}
