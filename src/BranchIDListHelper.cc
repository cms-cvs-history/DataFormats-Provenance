#include "DataFormats/Provenance/interface/BranchIDListHelper.h"
#include "DataFormats/Provenance/interface/BranchIDListRegistry.h"
#include "DataFormats/Provenance/interface/ProductRegistry.h"

namespace edm {
  void
  BranchIDListHelper::updateRegistry(ProductRegistry const& preg) {
    BranchIDList bidlist;
    // Add entries for current process for ProductID to BranchID mapping.
    for (ProductRegistry::ProductList::const_iterator it = preg.productList().begin(), itEnd = preg.productList().end();
        it != itEnd; ++it) {
      if (it->second.produced()) {
        if (it->second.branchType() == InEvent) {
          bidlist.push_back(it->second.branchID().id());
        }
      }
    }
    BranchIDListRegistry& breg = *BranchIDListRegistry::instance();
    breg.extra().currentIndex_ = breg.data().size();
    breg.insertMapped(bidlist);

    // Add entries to aid BranchID to ProductID mapping
    BranchIDToIndexMap& branchIDToIndexMap = breg.extra().branchIDToIndexMap_;
    for (BranchIDLists::const_iterator it = breg.data().begin(), itEnd = breg.data().end(); it != itEnd; ++it) {
      BranchListIndex blix = it - breg.data().begin();
      for (BranchIDList::const_iterator i = it->begin(), iEnd = it->end(); i != iEnd; ++i) {
        ProductIndex pix = i - it->begin();
	branchIDToIndexMap.insert(std::make_pair(*i, std::make_pair(blix, pix)));
      }
    }
  }

  void
  BranchIDListHelper::clearRegistry() {
    BranchIDListRegistry& breg = *BranchIDListRegistry::instance();
    breg.data().clear();
    breg.extra().currentIndex_ = 0;
    breg.extra().branchIDToIndexMap_.clear();
  }
}
