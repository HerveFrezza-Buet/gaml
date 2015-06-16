#include <gaml.hpp>
#include <iostream>
#include <iomanip>
#include <iterator>
#include <ctime>
#include <cstdlib>

/*
  This example shows how the use of iterators allows a convenient
  manupilation of sets, through the tools proposed in the gaml
  library. 
*/


// This template function displays values from begin to end.
template<typename Iterator>
void display(Iterator begin, Iterator end) {
  std::cout << gaml::iterator_traits<Iterator>::tag_type::to_string() << std::endl;
  std::cout << '[';
  for(Iterator iter = begin; iter != end; ++iter)
    std::cout << ' ' << *iter;
  std::cout << " ] (i.e. " << std::distance(begin,end) << " elements)" << std::endl
	    << std::endl;
}

// This template function displays values from begin to end.
template<typename Iterator>
void display_idx(Iterator begin, Iterator end) {
  std::cout << gaml::iterator_traits<Iterator>::tag_type::to_string() << std::endl;
  std::cout << '[';
  for(Iterator iter = begin; iter != end; ++iter)
    std::cout << ' ' << *iter;
  std::cout << " ] (i.e. " << std::distance(begin,end) << " elements)" << std::endl;
  std::cout << '[';
  for(Iterator iter = begin; iter != end; ++iter)
    std::cout << ' ' << *(iter.idx_iter());
  std::cout << " ]" << std::endl
	    << std::endl;
}


// Iterators are a lazy way to compute values. It means that the value
// is computed when *iter is called. Of course, if *iter is an acces
// to a previously stored collection of values, this on demand
// evaluation is just a memory access. Nevertheless, iterators are
// more general, as the ones provided by gaml for set manipulation
// show.

int main(int argc, char* argv[]) {

  // random seed initialization
  std::srand(std::time(0));
  
  gaml::integer begin = 0;
  gaml::integer end   = 50;

  display(begin,end);

  std::cout << "split1" << std::endl;
  auto split1 = gaml::split(begin, end,
  			   [](int i) -> bool {return i % 2 == 0;});
  display_idx(split1.true_values.begin(), split1.true_values.end());

  std::cout << "split2" << std::endl;
  auto split2 = gaml::split(split1.true_values.begin(), split1.true_values.end(),
  			   [](int i) -> bool {return i < 25 ;});
  display_idx(split2.true_values.begin(), split2.true_values.end());

  std::cout << "split3" << std::endl;
  auto split3 = gaml::split(split2.true_values.begin(), split2.true_values.end(),
  			   [](int i) -> bool {return i % 3 == 0 ;});
  display_idx(split3.true_values.begin(), split3.true_values.end());

  std::cout << "seq1" << std::endl;
  auto seq1 = gaml::virtualized::sequence(split1.true_values.begin(),split1.true_values.end());
  display_idx(seq1.begin(), seq1.end());

  std::cout << "split4" << std::endl;
  auto split4 = gaml::split(seq1.begin(), seq1.end(),
  			   [](int i) -> bool {return i % 3 == 0 ;});
  display_idx(split4.true_values.begin(), split4.true_values.end());

  std::cout << "seq2" << std::endl;
  auto id   = gaml::identity(begin,end);
  auto seq2 = gaml::virtualized::sequence(id.begin(),id.end());
  display(seq2.begin(), seq2.end());

  std::cout << "split5" << std::endl;
  auto split5 = gaml::split(seq2.begin(), seq2.end(),
  			   [](int i) -> bool {return i % 5 == 0 ;});
  display_idx(split5.true_values.begin(), split5.true_values.end());

  std::cout << "split6" << std::endl;
  auto split6 = gaml::split(split5.true_values.begin(), split5.true_values.end(),
  			   [](int i) -> bool {return i % 2 == 0 ;});
  display_idx(split6.true_values.begin(), split6.true_values.end());
  
  return EXIT_SUCCESS;
}
