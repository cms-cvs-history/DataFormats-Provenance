#include "DataFormats/Provenance/interface/BranchMapper.h"
#include "FWCore/Utilities/interface/Algorithms.h"

/*----------------------------------------------------------------------

----------------------------------------------------------------------*/

namespace edm {
  BranchMapper::BranchMapper() :
    branchEntryInfoSet_(),
    branchEntryInfoMap_()
  { }

  void
  BranchMapper::insert(BranchEntryInfo const& bei) {
    branchEntryInfoSet_.insert(bei);
    if (!branchEntryInfoMap_.empty()) {
      branchEntryInfoMap_.insert(std::make_pair(bei.productID(), branchEntryInfoSet_.find(bei)));
    }
  }
    
  ProductID 
  BranchMapper::branchToProduct(BranchID const& bid) const {
    BranchEntryInfo bei(bid);
    beiSet::const_iterator it = branchEntryInfoSet_.find(bei);
    if (it == branchEntryInfoSet_.end()) return ProductID();
    return it->productID();
  }

  BranchID 
  BranchMapper::productToBranch(ProductID const& pid) const {
    if (branchEntryInfoMap_.empty()) {
      beiMap & map = const_cast<beiMap &>(branchEntryInfoMap_);
      for (beiSet::const_iterator i = branchEntryInfoSet_.begin(), iEnd = branchEntryInfoSet_.end();
	  i != iEnd; ++i) {
	map.insert(std::make_pair(i->productID(), i));
      }
    }
    beiMap::const_iterator it = branchEntryInfoMap_.find(pid);
    if (it == branchEntryInfoMap_.end()) return BranchID();
    return it->second->branchID();
  }

}
