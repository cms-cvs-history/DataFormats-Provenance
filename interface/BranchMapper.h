#ifndef DataFormats_Provenance_BranchMapper_h
#define DataFormats_Provenance_BranchMapper_h

/*----------------------------------------------------------------------
  
BranchMapper: The mapping from per event product ID's to BranchID's.

----------------------------------------------------------------------*/
#include <iosfwd>
#include <memory>
#include <set>

#include "DataFormats/Provenance/interface/ProductID.h"
#include "DataFormats/Provenance/interface/BranchID.h"
#include "DataFormats/Provenance/interface/BranchEntryInfo.h"

/*
  BranchMapper

*/

namespace edm {
  class BranchMapper {
  public:
    BranchMapper();

    ~BranchMapper() {}

    void write(std::ostream& os) const;

    ProductID branchToProduct(BranchID const& bid) const;

    BranchID productToBranch(ProductID const& pid) const;
    
    std::auto_ptr<BranchEntryInfo> branchToEntryInfo(BranchID const& bid) const;

    void insert(BranchEntryInfo const& bei);

  private:
    typedef std::set<BranchEntryInfo> beiSet;
    typedef std::map<ProductID, beiSet::const_iterator> beiMap;

    beiSet branchEntryInfoSet_;

    beiMap branchEntryInfoMap_;
  };
  
  inline
  std::ostream&
  operator<<(std::ostream& os, BranchMapper const& p) {
    p.write(os);
    return os;
  }

}
#endif
