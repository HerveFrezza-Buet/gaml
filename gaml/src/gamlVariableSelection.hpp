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

#include <gamlDimensions.hpp>
#include <vector>
#include <algorithm>
#include <limits>
#include <iterator>

namespace gaml {
  namespace varsel {
    // General variable selection algorithm
    template<typename Evaluator, typename AttributeSet, typename Searcher> 
    double search(Evaluator& evaluator, AttributeSet& variableSubset, Searcher& searcher, bool verbose) {

      int n = evaluator.getAttributeNumber();
      if(verbose) { searcher.verbose(); }
      auto outputIt = std::inserter(variableSubset, variableSubset.end());
      return searcher(n, outputIt, evaluator);
    }

    // Sequential Forward Selection
    template<typename Evaluator, typename AttributeSet> 
    double SFS(Evaluator& evaluator, AttributeSet& variableSubset, bool verbose = false) {
      gaml::GreedySearch<true, Evaluator::toMinimize> strategy;
      return search(evaluator, variableSubset, strategy, verbose);
    }

    // Sequential Backward Selection
    template<typename Evaluator, typename AttributeSet> 
    double SBS(Evaluator& evaluator, AttributeSet& variableSubset, bool verbose = false) {
      gaml::GreedySearch<false, Evaluator::toMinimize> strategy;
      return search(evaluator, variableSubset, strategy, verbose);
    }

    // Sequential Floating Forward Selection
    template<typename Evaluator, typename AttributeSet> 
    double SFFS(Evaluator& evaluator, AttributeSet& variableSubset, bool verbose = false) {
      gaml::BidirectionalGreedySearch<true, Evaluator::toMinimize> strategy;
      return search(evaluator, variableSubset, strategy, verbose);
    }

    // Sequential Floating Backward Selection
    template<typename Evaluator, typename AttributeSet> 
    double SFBS(Evaluator& evaluator, AttributeSet& variableSubset, bool verbose = false) {
      gaml::BidirectionalGreedySearch<false, Evaluator::toMinimize> strategy;
      return search(evaluator, variableSubset, strategy, verbose);
    }

    // Sequential Beam Forward Selection
    template<typename Evaluator, typename AttributeSet> 
    double SBFS(Evaluator& evaluator, AttributeSet& variableSubset, int k, double filteringRatio, bool verbose = false) {
      gaml::BestFirstSearch<true, Evaluator::toMinimize> strategy(k, filteringRatio);
      return search(evaluator, variableSubset, strategy, verbose);
    }

    // Sequential Beam Backward Selection
    template<typename Evaluator, typename AttributeSet> 
    double SBBS(Evaluator& evaluator, AttributeSet& variableSubset, int k, double filteringRatio, bool verbose = false) {
      gaml::BestFirstSearch<false, Evaluator::toMinimize> strategy(k, filteringRatio);
      return search(evaluator, variableSubset, strategy, verbose);
    }

  }
}
