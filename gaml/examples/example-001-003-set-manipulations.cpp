#include <gaml.hpp>
#include <iostream>
#include <iomanip>
#include <iterator>
#include <ctime>
#include <cstdlib>
#include <array>
#include <tuple>
#include <utility>

/*
  This example shows how the use of iterators allows a convenient
  manupilation of sets, through the tools proposed in the gaml
  library. 
*/

// For use of zip, we define a serialization operator for
// a std::tuple<std::string, int, std::pair<int, int> >
std::ostream& operator<<(std::ostream& os,
			 const std::tuple<std::string, int, std::pair<int, int>>& elem) {
  os << "(\"" << std::get<0>(elem) << "\"" 
     << ", "  << std::get<1>(elem)
     << ", (" << std::get<2>(elem).first << ", " << std::get<2>(elem).second << "))";
  return os;
}

// This template function displays values from begin to end.
template<typename Iterator>
void display(const std::string& title, Iterator begin, Iterator end) {
  
  std::cout << "### " << title << std::endl
	    << "    [";
  for(Iterator iter = begin; iter != end; ++iter)
    std::cout << ' ' << *iter;
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

  std::cout << std::endl << std::endl;

  // random seed initialization
  std::srand(std::time(0));

  // Let us build some fake data.
  std::vector<int> data(50);
  int i=0;
  for(auto& d : data) d = i++;
  auto begin = data.begin();
  auto end   = data.end();
  

  // All further calls to display will be similar to this one, since
  // the actual template type is deduced from arguments. This is why
  // the display method should be defined as a template.
  
  display("Initial set", begin, end);
  
  // This transforms any collection into a tabular collection. This is
  // only usefull in some specific cases where the algorithm needs to
  // have a tabular collection.
  
  auto identity = gaml::identity(begin, end);
  display("Identity", identity.begin(), identity.end());

  // This merges two chunks into a single set (union of non
  // overlapping sets).
  auto merge = gaml::merge(begin +  2, begin + 10,
  			   begin + 23, begin + 47);
  display("Merging non overlapping subsets", merge.begin(), merge.end());

  
  // This is lazy, since the function is applied only when *iter is
  // invoked.
  
  auto map = gaml::map(begin, begin+10,
  		       [] (const int& x) -> int {return x*x;});
  display("Mapping a function", map.begin(), map.end());

  // The zip iterator allows to browse several iterators in parallel
  auto s1 = gaml::map(begin, end, [] (const int& x) { return std::to_string(x);});
  auto s2 = gaml::map(begin, end, [] (const int& x) { return x*x;});
  auto s3 = gaml::map(begin, end, [] (const int& x) { return std::make_pair(x, 2*x);});
  auto zip = gaml::zip(gaml::range(s1.begin(), s1.end()),
		       gaml::range(s2.begin(), s2.end()),
		       gaml::range(s3.begin(), s3.end()));
  auto p = zip.begin();
  //display("Zip of 3 collections ", zip.begin(), zip.end());
  
  // The bootstrap consists in taking 20 samples randomly from [begin,
  // begin+10[.
  
  auto bootstrap = gaml::bootstrap(begin, begin+10, 20);
  display("Bootstrap [0, 10]", bootstrap.begin(), bootstrap.end());
  
  // This computes an acces to the elements as if the initial set were
  // shuffled.
  
  auto shuffle = gaml::shuffle(begin, begin+20);
  display("Shuffle", shuffle.begin(), shuffle.end());
  
  // This performs a shuffle tkat keeps some kind of locality, for
  // memory efficiency reasons. This may be needed if some cache
  // mecanism is involved.
  
  auto packed_shuffle = gaml::packed_shuffle(begin, begin+45, 10);
  display("Packed shuffle", packed_shuffle.begin(), packed_shuffle.end());

  // We can filter the data according to a boolean function. The
  // functions gaml::filter and gaml::reject provides iterators. It mean that the
  // test is performed when ++it is called, and it is not a random
  // access iterator (i.e std::distance(it1,it2) iterates and makes
  // the test at each time).

  auto filter = gaml::filter(shuffle.begin(), shuffle.end(),
  			     [](int i) -> bool {return i % 2 == 0;});
  display("Filter", filter.begin(), filter.end());

  // Split is similar to gaml::filter, but the
  // collection is iterated once when gaml::split is called, in order
  // to determine which data pass the test. The iterator is thus a
  // random access iterator, and std::distance(it1,it2) do not need
  // any iteration on the data.
  
  auto split = gaml::split(shuffle.begin(), shuffle.end(),
  			   [](int i) -> bool {return i % 2 == 0;});
  display("True side of split", split.true_values.begin(), split.true_values.end());
  display("False side of split", split.false_values.begin(), split.false_values.end());
  

  // Let us try to make intricated iterations. As opposed to the
  // previous use of gaml::shuffle on gaml::integer, for which the
  // size of the range can be computed directly from the (end-begin),
  // the shuffle here, when it is created, has to iterate explicitly
  // all the fsq content, since gaml::filter/gaml::reject iterator do
  // not allow for instantaneous (end-begin) size computation.
  auto sq   = gaml::map     (begin,       begin+20,  [] (int x) -> int  {return x*x;       });
  auto fsq  = gaml::reject  (sq.begin(),  sq.end(),  [] (int i) -> bool {return i % 4 != 0;});
  auto fsqs = gaml::shuffle (fsq.begin(), fsq.end()                                         );
  display("Intrication",fsqs.begin(),fsqs.end());
  
  // The following implements a cache. Values within pages are
  // computed (and copied) when the page is loaded in the cache.
  
  // Let us define a mapped collection, the mapped function being
  // verbose, so that one can see when the computation is actually
  // invoked.
  auto squares = gaml::map(begin,begin+20,
  			   [] (int x) -> int {
  			     std::cout << " (" << x*x << ')';
  			     return x*x;
  			   });
  // Let us now use a cache in order to pre-compute values within
  // pages.
  auto cached_squares = gaml::cache(squares.begin(),squares.end(),
  				    7,  // page size
  				    2); // number of pages in the cache.

  display("Cache demo [0..5[",   cached_squares.begin()     , cached_squares.begin() +  5);
  display("Cache demo [0..5[",   cached_squares.begin()     , cached_squares.begin() +  5);
  display("Cache demo [5..10[",  cached_squares.begin() +  5, cached_squares.begin() + 10);
  display("Cache demo [0..20[",  cached_squares.begin()     , cached_squares.end()       );
  display("Cache demo [10..20[", cached_squares.begin() + 10, cached_squares.begin() + 20);
  
  
  return EXIT_SUCCESS;
}
