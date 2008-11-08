#include "DataFormats/Provenance/interface/BranchIDListHelper.h"
#include "DataFormats/Provenance/interface/BranchIDListRegistry.h"
#include "DataFormats/Provenance/interface/ProductRegistry.h"

namespace edm {
  void
  BranchIDListHelper::initializeRegistry(ProductRegistry const& preg) {
    BranchIDListRegistry& breg = *BranchIDListRegistry::instance();
    BranchIDLists& branchIDLists_ = breg.data();
    breg.extra().currentIndex() = branchIDLists_.size();
    branchIDLists_.push_back(BranchIDList());
    BranchIDList& branchIDList = branchIDLists_.back();
    for (ProductRegistry::ProductList::const_iterator it = preg.productList().begin(), itEnd = preg.productList().end();
        it != itEnd; ++it) {
      if (it->second.produced()) {
        if (it->second.branchType() == InEvent) {
          branchIDList.push_back(it->second.branchID().id());
        }
      }
    }
  }
}
