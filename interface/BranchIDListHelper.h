#ifndef FWCore_Framework_BranchIDListHelper_h
#define FWCore_Framework_BranchIDListHelper_h

#include "DataFormats/Provenance/interface/BranchListIndex.h"
#include "DataFormats/Provenance/interface/ProvenanceFwd.h"
#include "DataFormats/Provenance/interface/ProductID.h"
#include <map>

namespace edm {
    
  class BranchIDListHelper {
  public:
    typedef std::pair<BranchListIndex, ProductIndex> IndexPair;
    typedef std::map<BranchID, IndexPair> BranchIDToIndexMap;
    BranchIDListHelper() : branchIDToIndexMap_(), currentIndex_(0) {}
    static void updateRegistry(ProductRegistry const& reg);
    static void clearRegistry();  // Use only for tests

    BranchListIndex currentIndex() const {return currentIndex_;}

    BranchIDToIndexMap const& branchIDToIndexMap() const {return branchIDToIndexMap_;}

  private:
    BranchIDToIndexMap branchIDToIndexMap_;
    BranchListIndex currentIndex_;
  };
}

#endif
