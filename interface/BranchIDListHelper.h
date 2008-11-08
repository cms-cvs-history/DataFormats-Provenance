#ifndef FWCore_Framework_BranchIDListHelper_h
#define FWCore_Framework_BranchIDListHelper_h

#include "DataFormats/Provenance/interface/BranchListIndex.h"
#include "DataFormats/Provenance/interface/ProvenanceFwd.h"

namespace edm {
    
  class BranchIDListHelper {
  public:
    BranchIDListHelper() : currentIndex_(0) {}
    static void initializeRegistry(ProductRegistry const& reg);

    BranchListIndex currentIndex() const {return currentIndex_;}
    BranchListIndex& currentIndex() {return currentIndex_;}

  private:
    BranchListIndex currentIndex_;
  };
}

#endif
