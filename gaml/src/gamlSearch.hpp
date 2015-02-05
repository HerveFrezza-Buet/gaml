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

#include <algorithm>
#include <limits>
#include <map>

namespace gaml {

  template<bool minimize = true>
  class Search {
  protected:
    typedef std::set<int> elts_set_type;
    typedef elts_set_type::iterator iterator;

    bool verbose_;

    void setVerbose(bool verbose) {
      verbose_ = verbose;
    }

    double getWorstScore() const {
      if (minimize)
	return std::numeric_limits<double>::max();
      else
	return std::numeric_limits<double>::min();
    }

    bool firstScoreIsStrictlyBetter(double score1, double score2) const {
      if (minimize)
	return score1 < score2;
      else
	return score1 > score2;
    }

    class Complement {
      int n_;
      elts_set_type set_;

      class Iterator {
	elts_set_type::const_iterator it_;
	elts_set_type::const_iterator end_;
	int i_;

	void skip() {
	  while (it_ != end_) {
	    int j = *it_;
	    if (j <= i_) {
	      if (j == i_)
		++i_;
	      ++it_;
	    } else
	      break;
	  }
	}

      public:
	Iterator(const elts_set_type& set, int i) :
	  it_(set.cbegin()), end_(set.cend()), i_(i) {
	  skip();
	}
	Iterator(const Iterator& other) :
	  it_(other.it_), end_(other.end_), i_(other.i_) {
	}

	bool operator!=(const Iterator& other) {
	  return i_ != other.i_;
	}
	int operator*(void) {
	  return i_;
	}
	Iterator& operator++(void) {
	  ++i_;
	  skip();
	  return *this;
	}
      };
    public:
      typedef Iterator iterator;
      Complement(const elts_set_type& set, int n) :
	n_(n), set_(set) {
      }
      Complement(const Complement& other) :
	n_(other.n_), set_(other.set_) {
      }
      Iterator begin() {
	return Iterator(set_, 0);
      }
      Iterator end() {
	return Iterator(set_, n_);
      }
    };

    class ForwardPolicy {
      Complement attrs_;
    public:
      ForwardPolicy(elts_set_type& set, int n) :
	attrs_(set, n) {
      }

      static void setInitAttributeSubset(elts_set_type& set, int n) {
	set.clear();
      }

      typedef typename Complement::iterator attribute_iterator;

      attribute_iterator attr_begin() {
	return attrs_.begin();
      }
      attribute_iterator attr_end() {
	return attrs_.end();
      }

      static void apply(elts_set_type& set, int attribute) {
	set.insert(attribute);
      }

      struct IteratorPolicy {
	elts_set_type::iterator position_;

	IteratorPolicy() :
	  position_() {
	}
	IteratorPolicy(const IteratorPolicy& other) :
	  position_(other.position_) {
	}

	void doit(elts_set_type& set, attribute_iterator& attrIt) {
	  std::pair<elts_set_type::iterator, bool> res = set.insert(
								    *attrIt);
	  position_ = res.first;
	}

	void undoit(elts_set_type& set) {
	  set.erase(position_);
	}

	int attribute() {
	  return *position_;
	}
      };
    };

    class BackwardPolicy {
      elts_set_type& set_;

    public:
      BackwardPolicy(elts_set_type& set, int n) :
	set_(set) {
      }

      static void setInitAttributeSubset(elts_set_type& set, int n) {
	set.clear();
	for (int i = 0; i != n; ++i)
	  set.insert(i);
      }

      typedef typename elts_set_type::iterator attribute_iterator;

      attribute_iterator attr_begin() {
	return set_.begin();
      }
      attribute_iterator attr_end() {
	return set_.end();
      }

      static void apply(elts_set_type& set, int attribute) {
	set.erase(attribute);
      }

      struct IteratorPolicy {
	attribute_iterator* attrIt_;
	int attribute_;

	IteratorPolicy() :
	  attrIt_(), attribute_() {
	}
	IteratorPolicy(const IteratorPolicy& other) :
	  attrIt_(other.attrIt_), attribute_(other.attribute_) {
	}

	void doit(elts_set_type& set, attribute_iterator& attrIt) {
	  attrIt_ = &attrIt;
	  attribute_ = *attrIt;
	  attribute_iterator it = attrIt;
	  ++attrIt;
	  set.erase(it);
	}
	void undoit(elts_set_type& set) {
	  set.insert(attribute_);
	  --(*attrIt_);
	}

	int attribute() {
	  return attribute_;
	}
      };
    };

    template<typename Policy>
    class SuccessorGenerator: public Policy {
    protected:
      elts_set_type& set_;
      int n_;
      typedef typename Policy::IteratorPolicy iterator_policy;

      class Iterator: public iterator_policy {
	typedef typename Policy::attribute_iterator attribute_iterator;

	elts_set_type& set_;
	attribute_iterator it_;
	bool done_;

	void doit() {
	  if (done_)
	    return;
	  iterator_policy::doit(set_, it_);
	  done_ = true;
	}
	void undoit() {
	  if (!done_)
	    return;
	  iterator_policy::undoit(set_);
	  done_ = false;
	}

      public:
	Iterator(elts_set_type& set, const attribute_iterator& it) :
	  iterator_policy(), set_(set), it_(it), done_(false) {
	}
	Iterator(const Iterator& other) :
	  iterator_policy(other), set_(other.set_), it_(other.it_), done_(
									  other.done_) {
	}
	~Iterator() {
	  undoit();
	}

	bool operator!=(const Iterator& other) {
	  return it_ != other.it_;
	}
	Iterator& operator++(void) {
	  undoit();
	  ++it_;
	  return *this;
	}
	elts_set_type& operator*() {
	  doit();
	  return set_;
	}
      };

    public:
      typedef Iterator iterator;
      SuccessorGenerator(elts_set_type& set, int n) :
	Policy(set, n), set_(set), n_(n) {
      }
      Iterator begin() {
	return Iterator(set_, Policy::attr_begin());
      }
      Iterator end() {
	return Iterator(set_, Policy::attr_end());
      }
    };

    struct ForwardGenerator: public SuccessorGenerator<ForwardPolicy> {
      ForwardGenerator(elts_set_type& set, int n) :
	SuccessorGenerator<ForwardPolicy>(set, n) {
      }
    };

    struct BackwardGenerator: public SuccessorGenerator<BackwardPolicy> {
      BackwardGenerator(elts_set_type& set, int n) :
	SuccessorGenerator<BackwardPolicy>(set, n) {
      }
    };

    Search() :
      verbose_(false) {
    }
  };

  template<bool forward = true, bool minimize = true>
  class GreedySearch: public Search<minimize> {
    typedef Search<minimize> parent_type;
    typedef typename parent_type::elts_set_type elts_set_type;

    template<typename _SolutionOutputIterator, typename _Evaluator,
	     typename _Generator>
    double search(int n, _SolutionOutputIterator& outputIt,
		  _Evaluator& evaluator) const {
      elts_set_type elts;
      _Generator::setInitAttributeSubset(elts, n);
      double bestScore = evaluator(elts.cbegin(), elts.cend());
      while (true) {
	double newBestScore = parent_type::getWorstScore();
	int bestElt;

	_Generator generator(elts, n);
	for (auto it = generator.begin(); it != generator.end(); ++it) {
	  elts_set_type& elts = *it;
	  double score = evaluator(elts.cbegin(), elts.cend());
	  bool update = parent_type::firstScoreIsStrictlyBetter(score,
								newBestScore);
	  if (update) {
	    newBestScore = score;
	    bestElt = it.attribute();
	  }
	  if (parent_type::verbose_) {
	    std::for_each(elts.cbegin(), elts.cend(),
			  [](int elt) -> void {std::cout << elt << ' ';});
	    std::cout << "= " << score;
	    if (update)
	      std::cout << " (new optimum)";
	    std::cout << std::endl;
	  }
	}

	if (parent_type::firstScoreIsStrictlyBetter(newBestScore,
						    bestScore)) {
	  bestScore = newBestScore;
	  elts.insert(bestElt);
	} else
	  break;
      }

      for (auto it = elts.begin(); it != elts.end(); ++it)
	*outputIt++ = *it;
      return bestScore;
    }

    template<bool direction, typename _SolutionOutputIterator,
	     typename _Evaluator>
    struct SearchFunctor {
      double operator()(const GreedySearch& algo, int n,
			_SolutionOutputIterator& outputIt,
			_Evaluator& evaluator) const {
	return algo.search<_SolutionOutputIterator, _Evaluator,
			   typename parent_type::ForwardGenerator>(n, outputIt,
								   evaluator);
      }
    };

    template<typename _SolutionOutputIterator, typename _Evaluator>
    struct SearchFunctor<false, _SolutionOutputIterator, _Evaluator> {
      double operator()(const GreedySearch& algo, int n,
			_SolutionOutputIterator& outputIt,
			_Evaluator& evaluator) const {
	return algo.search<_SolutionOutputIterator, _Evaluator,
			   typename parent_type::BackwardGenerator>(n, outputIt,
								    evaluator);
      }
    };

  public:

    GreedySearch() :
      Search<minimize>() {
    }
    GreedySearch& verbose(bool verbose = true) {
      parent_type::setVerbose(verbose);
      return *this;
    }

    template<typename _SolutionOutputIterator, typename _Evaluator>
    double operator()(int n, _SolutionOutputIterator& outputIt,
		      _Evaluator& evaluator) const {
      SearchFunctor<forward, _SolutionOutputIterator, _Evaluator> func;
      return func(*this, n, outputIt, evaluator);
    }
  };

  template<bool forward = true, bool minimize = true>
  class BidirectionalGreedySearch: public Search<minimize> {
    typedef Search<minimize> parent_type;
    typedef typename parent_type::elts_set_type elts_set_type;

    template<typename _Generator, typename _Evaluator> std::pair<double, int> generate(
										       _Generator& generator, _Evaluator& evaluator) const {
      double newBestScore = parent_type::getWorstScore();
      int bestElt = 0;

      for (auto it = generator.begin(); it != generator.end(); ++it) {
	elts_set_type& elts = *it;
	double score = evaluator(elts.cbegin(), elts.cend());
	bool update = parent_type::firstScoreIsStrictlyBetter(score,
							      newBestScore);
	if (update) {
	  newBestScore = score;
	  bestElt = it.attribute();
	}
	if (parent_type::verbose_) {
	  std::for_each(elts.cbegin(), elts.cend(),
			[](int elt) -> void {std::cout << elt << ' ';});
	  std::cout << "= " << score;
	  if (update)
	    std::cout << " (new optimum)";
	  std::cout << std::endl;
	}
      }
      return std::pair<double, int>(newBestScore, bestElt);
    }

    template<typename _SolutionOutputIterator, typename _Evaluator,
	     typename _FirstGenerator, typename _SecondGenerator>
    double search(int n, _SolutionOutputIterator& outputIt,
		  _Evaluator& evaluator) const {
      elts_set_type bestSubset;
      _FirstGenerator::setInitAttributeSubset(bestSubset, n);
      bool firstGenerator = true;
      int nScoreIncrease = 0;
      double bestScore = evaluator(bestSubset.cbegin(), bestSubset.cend());
      while (true) {
	std::pair<double, int> best;
	if (firstGenerator) {
	  _FirstGenerator generator(bestSubset, n);
	  best = generate(generator, evaluator);
	} else {
	  _SecondGenerator generator(bestSubset, n);
	  best = generate(generator, evaluator);
	}

	if (parent_type::firstScoreIsStrictlyBetter(best.first,
						    bestScore)) {
	  ++nScoreIncrease;
	  bestScore = best.first;
	  if (firstGenerator)
	    _FirstGenerator::apply(bestSubset, best.second);
	  else
	    _SecondGenerator::apply(bestSubset, best.second);
	} else {
	  if (nScoreIncrease == 0)
	    break;
	  else {
	    nScoreIncrease = 0;
	    firstGenerator = !firstGenerator;
	  }
	}
      }

      for (auto it = bestSubset.begin(); it != bestSubset.end(); ++it)
	*outputIt++ = *it;
      return bestScore;
    }

    template<bool direction, typename _SolutionOutputIterator,
	     typename _Evaluator>
    struct SearchFunctor {
      double operator()(const BidirectionalGreedySearch& algo, int n,
			_SolutionOutputIterator& outputIt,
			_Evaluator& evaluator) const {
	return algo.search<_SolutionOutputIterator, _Evaluator,
			   typename parent_type::ForwardGenerator,
			   typename parent_type::BackwardGenerator>(n, outputIt,
								    evaluator);
      }
    };

    template<typename _SolutionOutputIterator, typename _Evaluator>
    struct SearchFunctor<false, _SolutionOutputIterator, _Evaluator> {
      double operator()(const BidirectionalGreedySearch& algo, int n,
			_SolutionOutputIterator& outputIt,
			_Evaluator& evaluator) const {
	return algo.search<_SolutionOutputIterator, _Evaluator,
			   typename parent_type::BackwardGenerator,
			   typename parent_type::ForwardGenerator>(n, outputIt,
								   evaluator);
      }
    };

  public:

    BidirectionalGreedySearch() :
      Search<minimize>() {
    }
    BidirectionalGreedySearch& verbose(bool verbose = true) {
      parent_type::setVerbose(verbose);
      return *this;
    }

    template<typename _SolutionOutputIterator, typename _Evaluator>
    double operator()(int n, _SolutionOutputIterator& outputIt,
		      _Evaluator& evaluator) const {
      SearchFunctor<forward, _SolutionOutputIterator, _Evaluator> func;
      return func(*this, n, outputIt, evaluator);
    }
  };

  template<bool forward = true, bool minimize = true>
  class BestFirstSearch: public Search<minimize> {
    typedef Search<minimize> parent_type;
    typedef typename parent_type::elts_set_type elts_set_type;

    int k_;
    double filteringRatio_;

    template<typename _SolutionOutputIterator, typename _Evaluator,
	     typename _Generator>
    double search(int n, _SolutionOutputIterator& outputIt,
		  _Evaluator& evaluator) const {
      typedef std::multimap<double, elts_set_type, std::less<double>> queue_type;
      typedef typename queue_type::iterator iterator;
      typedef typename queue_type::value_type queue_entry;

      queue_type queue;
      elts_set_type bestSubset, previousInsertedSubset;
      _Generator::setInitAttributeSubset(bestSubset, n);
      double bestScore = evaluator(bestSubset.cbegin(), bestSubset.cend());
      queue.insert(queue_entry(bestScore, bestSubset));
      previousInsertedSubset = bestSubset;

      while (!queue.empty()) {
	iterator head = queue.begin();
	iterator chead = head;
	elts_set_type& currentSubset = head->second;

	_Generator generator(currentSubset, n);
	for (auto it = generator.begin(); it != generator.end(); ++it) {
	  elts_set_type& newSubset = *it;
	  double newScore = evaluator(newSubset.cbegin(),
				      newSubset.cend());
	  if (parent_type::firstScoreIsStrictlyBetter(newScore,
						      filteringRatio_ * bestScore)) {
	    bool update = parent_type::firstScoreIsStrictlyBetter(
								  newScore, bestScore);
	    if (update) {
	      bestScore = newScore;
	      bestSubset = newSubset;
	    }
	    if (newSubset != previousInsertedSubset) {
	      previousInsertedSubset = newSubset;
	      queue.insert(queue_entry(newScore, newSubset));
	      if (parent_type::verbose_) {
		std::cout << "Pushing ";
		std::for_each(newSubset.cbegin(), newSubset.cend(),
			      [](int elt) -> void {std::cout << elt << ' ';});
		std::cout << "= " << newScore;

		if (update) {
		  std::cout << " (new optimum)";
		}
		std::cout << std::endl;
	      }
	    }
	  }
	}
	queue.erase(chead);
	if (queue.size() > (size_t) k_) {
	  iterator pos = queue.begin();
	  for (int i = 0; i != k_; ++i)
	    ++pos;
	  queue.erase(pos, queue.end());
	}
      }

      for (auto it = bestSubset.begin(); it != bestSubset.end(); ++it)
	*outputIt++ = *it;
      return bestScore;
    }

    template<bool direction, typename _SolutionOutputIterator,
	     typename _Evaluator>
    struct SearchFunctor {
      double operator()(const BestFirstSearch& algo, int n,
			_SolutionOutputIterator& outputIt,
			_Evaluator& evaluator) const {
	return algo.search<_SolutionOutputIterator, _Evaluator,
			   typename parent_type::ForwardGenerator>(n, outputIt,
								   evaluator);
      }
    };

    template<typename _SolutionOutputIterator, typename _Evaluator>
    struct SearchFunctor<false, _SolutionOutputIterator, _Evaluator> {
      double operator()(const BestFirstSearch& algo, int n,
			_SolutionOutputIterator& outputIt,
			_Evaluator& evaluator) const {
	return algo.search<_SolutionOutputIterator, _Evaluator,
			   typename parent_type::BackwardGenerator>(n, outputIt,
								    evaluator);
      }
    };

  public:

    BestFirstSearch(int k, double ratio = 1.) :
      Search<minimize>(), k_(k), filteringRatio_(ratio) {
    }
    BestFirstSearch& verbose(bool verbose = true) {
      parent_type::setVerbose(verbose);
      return *this;
    }

    template<typename _SolutionOutputIterator, typename _Evaluator>
    double operator()(int n, _SolutionOutputIterator& outputIt,
		      _Evaluator& evaluator) const {
      SearchFunctor<forward, _SolutionOutputIterator, _Evaluator> func;
      return func(*this, n, outputIt, evaluator);
    }
  };
}
