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

#include <vector>
#include <set>
#include <map>
#include <sstream>
#include <iterator>
#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <type_traits>
#include <gamlException.hpp>
#include <gamlTabular.hpp>
#include <gamlIdentity.hpp>
#include <random>

namespace gaml {

  
  template<typename Iterator, typename RANDOM_DEVICE>
  Tabular<Iterator, typename is_secondary_iterator<Iterator>::type> bootstrap(const Iterator& begin, const Iterator& end,
									      unsigned int size, RANDOM_DEVICE& rd) {
    auto init = [begin,end,size,&rd](std::vector<tabular_index_type>& indices) -> void {
      indices.resize(size);
      tabular_index_type range = std::distance(begin,end);
      std::uniform_int_distribution<tabular_index_type> uniform(0, range-1);
      for(auto& idx : indices)
	idx = uniform(rd);
    };
    return Tabular<Iterator, typename is_secondary_iterator<Iterator>::type> (begin,init);
  }

  namespace risk {
    namespace bootstrap {

      template<typename LOSS>
      class LeaveOneOut {
      private:

	LOSS loss;
	unsigned int nb_sets;
	bool verbose;

      public:

	LeaveOneOut(const LOSS& l, unsigned int nb_bootstrapped_sets, bool verbosity) 
	  : loss(l), nb_sets(nb_bootstrapped_sets), verbose(verbosity) {}
	LeaveOneOut(const LeaveOneOut& other)
	  : loss(other.loss), nb_sets(other.nb_sets), verbose(other.verbose) {}
	
	template<typename Learner, typename DataIterator, typename InputOf, typename OutputOf> 
	double operator()(const Learner& learner,const DataIterator& begin, const DataIterator& end,
			  const InputOf& inputOf, const OutputOf& outputOf) const {
	  unsigned int size = std::distance(begin,end);
	  unsigned int b;

	  // sets[i] is the ith bootstrapped dataset.
	  std::vector< decltype(gaml::bootstrap(begin,end,size)) > sets;

	  // f[i] is the predictor associated to set i.
	  std::vector<typename Learner::predictor_type> f;
	  
	  // C[i] contains the indices of sets that can be used to test index i.
	  std::map<tabular_index_type, std::set<tabular_index_type>> C;
	  

	  if(verbose)
	    std::cout << "Making " << nb_sets 
		      << " bootstrapped sets." << std::endl;

	  auto set_out = std::back_inserter(sets);
	  for(b = 0; b < nb_sets; ++b) *(set_out++) = gaml::bootstrap(begin,end,size);
	  
	  if(verbose) {
	    for(b = 0; b < nb_sets; ++b) {
	      std::cout << "  Set " << std::setw(3) << b+1 << '/' << nb_sets << " :";

	      for(auto it_elem = sets[b].begin_index(); it_elem != sets[b].end_index(); ++it_elem)
	      	std::cout << ' ' << std::setw(3) << *it_elem;
	      std::cout << std::endl;
	    }
	  }
	    

	  if(verbose)
	    std::cout << "Computing the non-belong-to set list for each example." << std::endl;

	  // We need identity here in order to have the main set as a
	  // tabular collection. Now indexes fit with subset indexes.
	  auto main_set = gaml::identity(begin,end);
	  
	  for(auto it = main_set.begin_index(); it != main_set.end_index(); ++it) {
	    auto i = *it;
	    auto& Ci = C[i];
	    for(b = 0; b < nb_sets; ++b)
	      if(!(sets[b].has(i)))
		Ci.insert(Ci.begin(),b);
	    // Ci contains the indices of sets that can be used to test i.
	  }

	  if(verbose) {
	    for(auto it = main_set.begin_index(); it != main_set.end_index(); ++it) {
	      auto i = *it;
	      std::cout << "  sample " << std::setw(3) << i << " is not in sets {";
	      for(auto elem : C[i])
	      	std::cout << ' ' << elem;
	      std::cout << " }" << std::endl;
	    }
	  }

	  if(verbose)
	    std::cout << "Learning on these sets." << std::endl;
	  
	  for(b = 0; b < nb_sets; ++b) {
	    std::ostringstream ostr;
	    if(verbose) {
	      ostr << "  Set " << std::setw(3) << b+1 << '/' << nb_sets << " : ";
	      std::cout << ostr.str() << "learning...\r" << std::flush;
	    }
	    auto& Zb = sets[b];
	    f.push_back(learner(Zb.begin(),Zb.end(),inputOf,outputOf));
	    if(verbose)
	      std::cout << ostr.str() << "Done.      " << std::endl;
	  }

	  if(verbose)
	    std::cout << "Testing each sample" << std::endl;

	  unsigned int cardD = 0;
	  double       sumD  = 0;

	  DataIterator zi;
	  auto it = main_set.begin_index();
	  for(zi = begin; it != main_set.end_index(); ++it, ++zi) {
	    auto i = *it;
	    if(verbose)
	      std::cout << "  " << std::setw(4) << i+1 << '/' << size << " : ";
	    
	    auto& Ci = C[i];
	    
	    if(Ci.empty()) {
	      if(verbose)
		std::cout << "belongs to all sets." << std::endl;
	    }
	    else {
	      const typename Learner::predictor_type::input_type& xi = inputOf(*zi);
	      const typename Learner::predictor_type::output_type yi = outputOf(*zi);
	      double sum = 0;
	      for(auto j : Ci)
		sum += loss(f[j](xi),yi);
	      sum /= Ci.size();
	      if(verbose)
		std::cout << sum << " (using " << Ci.size() << " samples)" << std::endl;
	      ++cardD;
	      sumD += sum;
	    }
	  }

	  if(cardD == 0)
	    throw gaml::exception::Bootstrap("Every sample belongs to all bootstrapped sets");
       
	  return sumD/(double)cardD;
	}
      };
      
      
      template<typename LOSS>
      LeaveOneOut<LOSS> leave_one_out(const LOSS& l, unsigned int nb_sets, bool verbosity) {
	return LeaveOneOut<LOSS>(l,nb_sets,verbosity);
      }
      

      template<typename LOSS>
      class R632 {
      private: 

      	LOSS loss;
      	unsigned int nb_sets;
      	bool verbose;

      public:

      	R632(const LOSS& l, unsigned int nb_bootstrapped_sets, bool verbosity) 
      	  : loss(l), nb_sets(nb_bootstrapped_sets), verbose(verbosity) {}
      	R632(const R632& other)
      	  : loss(other.loss), nb_sets(other.nb_sets), verbose(other.verbose) {}
	
      	template<typename Learner, typename DataIterator, typename InputOf, typename OutputOf> 
      	double operator()(const Learner& learner,const DataIterator& begin, const DataIterator& end,
			  const InputOf& inputOf, const OutputOf& outputOf) const {
	  auto predictor = learner(begin,end,inputOf,outputOf);
      	  auto empirical = gaml::risk::empirical(loss);
      	  double Rn      = empirical(predictor,begin,end,inputOf,outputOf);
      	  auto estimated = gaml::risk::bootstrap::leave_one_out(loss,nb_sets,false);
      	  double R       = estimated(learner,begin,end,inputOf,outputOf);
      	  if(verbose)
      	    std::cout << "Empirical risk    : Rn     : " << Rn << std::endl
      		      << "Bootstrapped risk : R      : " << R << std::endl
      		      << "                  : R - Rn : " << R-Rn << std::endl;
      	  return 0.368*Rn + 0.632*R;
      	}
      };

      template<typename LOSS>
      R632<LOSS> r632(const LOSS& l, unsigned int nb_sets, bool verbosity) {
      	return R632<LOSS>(l,nb_sets,verbosity);
      }


      template<typename LOSS>
      class R632Plus {
      private:

      	LOSS loss;
      	unsigned int nb_sets;
      	bool verbose;

      public:

      	R632Plus(const LOSS& l, unsigned int nb_bootstrapped_sets, bool verbosity) 
      	  : loss(l), nb_sets(nb_bootstrapped_sets), verbose(verbosity) {}
      	R632Plus(const R632Plus& other)
      	  : loss(other.loss), nb_sets(other.nb_sets), verbose(other.verbose) {}
	
      	template<typename Learner, typename DataIterator, typename InputOf, typename OutputOf> 
      	double operator()(const Learner& learner,const DataIterator& begin, const DataIterator& end,
      			      const InputOf& inputOf, const OutputOf& outputOf) const {
	  auto predictor = learner(begin,end,inputOf,outputOf);
      	  auto empirical = gaml::risk::empirical(loss);
      	  double Rn      = empirical(predictor,begin,end,inputOf,outputOf);
      	  auto estimated = gaml::risk::bootstrap::leave_one_out(loss,nb_sets,false);
      	  double R       = estimated(learner,begin,end,inputOf,outputOf);
      	  unsigned int N = end-begin;
      
      	  if(N == 0)
      	    throw gaml::exception::Bootstrap("Empty database provided.");
	

      	  DataIterator i,j;

      	  double gamma = 0;

      	  for(i = begin; i != end; ++i) {
      	    const typename Learner::predictor_type::output_type yi = predictor(inputOf(*i));
      	    for(j = begin; j != end; ++j)
      	      gamma += loss(outputOf(*j),yi);
      	  }
      	  gamma /= N*N;

      	  double r = (R-Rn)/(gamma-Rn);
      	  double omega = 0.632/(1-0.368*r);

      	  if(verbose)
      	    std::cout << "Empirical risk           : Rn                        : " << Rn << std::endl
      		      << "Bootstrapped risk        : R                         : " << R << std::endl
      		      << "0-information error rate : gamma                     : " << gamma << std::endl
      		      << "Overfitting rate         : r = (R-Rn)/(gamma-Rn)     : " << r << std::endl
      		      << "Weight                   : omega = 0.632/(1-0.368*r) : " << omega << std::endl;
      	  return (1-omega)*Rn + omega*R;
      	}
      };

      template<typename LOSS>
      R632Plus<LOSS> r632plus(const LOSS& l, unsigned int nb_sets, bool verbosity) {
      	return R632Plus<LOSS>(l,nb_sets,verbosity);
      }
    }
  }
}
