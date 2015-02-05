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
  std::cout << '[';
  for(Iterator iter = begin; iter != end; ++iter)
    std::cout << ' ' << *iter;
  std::cout << " ]" << std::endl;
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
  
  // gaml::integer are iterable integer values. They are used here as
  // the native sequence of values, from which more complex sequences
  // are built.
  gaml::integer begin = 0;
  gaml::integer end   = 50;

  std::cout << "Basic subset" << std::endl;

  // All further calls to display will be similar to this one, since
  // the actual template type is deduced from arguments. This is why
  // the display method should be defined as a template.
  display(begin,end);

  // Identity.... This may sound useless here, but it can help the
  // writing of degenerated cases in some templates.
  std::cout << "Identity" << std::endl;
  auto identity = gaml::identity(begin,end);
  display(identity.begin(),identity.end());

  std::cout << "Merging non overlapping subsets" << std::endl;

  // This merges two chunks into a single set (union of non
  // overlapping sets).
  auto merge = gaml::merge(begin +  2, begin + 10,
			   begin + 23, begin + 47);
  display(merge.begin(),merge.end());

  std::cout << "Mapping a function" << std::endl;
  
  // This is lazy, since the function is applied only when *iter is
  // invoked.
  auto map = gaml::map(begin,begin+10,
		       [] (const int& x) -> int {return x*x;});
  display(map.begin(),map.end());

  std::cout << "Bootstrap [0,10]" << std::endl;
  
  // The bootstrap consists in taking 20 samples randomly from [begin,
  // begin+10[.
  auto bootstrap = gaml::bootstrap(begin,begin+10,20);
  display(bootstrap.begin(),bootstrap.end());

  std::cout << "Shuffle" << std::endl;
  
  // This computes an acces to the elements as if the initial set were
  // shuffled.
  auto shuffle = gaml::shuffle(begin,begin+20);
  display(shuffle.begin(),shuffle.end());

  std::cout << "Packed shuffle" << std::endl;
  
  // This performs a shuffle tkat keeps some kind of locality, for
  // memory efficiency reasons. This may be needed if some cache
  // mecanism is involved.
  auto packed_shuffle = gaml::packed_shuffle(begin,begin+45,10);
  display(packed_shuffle.begin(),packed_shuffle.end());

  // We can filter the data according to a boolean function. The
  // function gaml::filter and gaml::reject provide lazy iterators. It
  // mean that the test is performed when ++it is called, and it is
  // not a random access iterator (i.e std::distance(it1,it2) iterates
  // and makes the test at each time).
  std::cout << "Filter" << std::endl;

  auto filter = gaml::filter(shuffle.begin(), shuffle.end(),
			     [](int i) -> bool {return i % 2 == 0;});
  display(filter.begin(),filter.end());

  // Split is similar to gaml::filter and gaml::reject, but the
  // collection is iterated once when gaml::split is called, in order
  // to determine which data pass the test. The iterator is thus a
  // random access iterator, and std::distance(it1,it2) do not need
  // any iteration on the data.
  std::cout << "Split" << std::endl;

  auto split = gaml::split(shuffle.begin(), shuffle.end(),
			   [](int i) -> bool {return i % 2 == 0;});
  display(split.true_values.begin(), split.true_values.end());
  display(split.false_values.begin(),split.false_values.end());
  

  // Let us try to make intricated interations. As opposed to the
  // previous use of gaml::shuffle on gaml::integer, for which the
  // size of the range can be computed directly from the (end-begin),
  // the shuffle here, when it is created, has to iterate explicitly
  // all the fsq content, since gaml::filter/gaml::reject iterator do
  // not allow for instantaneous (end-begin) size computation.
  std::cout << "Intrication" << std::endl;
  auto sq   = gaml::map     (begin,       begin+20,  [] (int x) -> int  {return x*x;       });
  auto fsq  = gaml::reject  (sq.begin(),  sq.end(),  [] (int i) -> bool {return i % 4 != 0;});
  auto fsqs = gaml::shuffle (fsq.begin(), fsq.end()                                         );
  display(fsqs.begin(),fsqs.end());

  std::cout << std::endl
	    << "Cache (contains two 7-sized pages)" << std::endl
	    << std::endl;

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

  std::cout << "Cache demo [0..5[" << std::endl;
  display(cached_squares.begin(),cached_squares.begin()+5);

  std::cout << "Cache demo [0..5[" << std::endl;
  display(cached_squares.begin(), cached_squares.begin()+5);

  std::cout << "Cache demo [5..10[" << std::endl;
  display(cached_squares.begin()+5,cached_squares.begin()+10);

  std::cout << "Cache demo [0..20[" << std::endl;
  display(cached_squares.begin(),cached_squares.end());

  std::cout << "Cache demo [10..20[" << std::endl;
  display(cached_squares.begin()+10,cached_squares.begin()+20);
  
  
  return EXIT_SUCCESS;
}
