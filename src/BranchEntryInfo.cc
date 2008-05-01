#include "DataFormats/Provenance/interface/BranchEntryInfo.h"
#include "DataFormats/Provenance/interface/EntryDescriptionRegistry.h"
#include <ostream>

/*----------------------------------------------------------------------

$Id: BranchEntryInfo.cc,v 1.3.2.1 2008/04/25 17:20:40 wmtan Exp $

----------------------------------------------------------------------*/

namespace edm {
  BranchEntryInfo::BranchEntryInfo() :
    branchID_(),
    productID_(),
    productStatus_(),
    entryDescriptionID_(),
    entryDescriptionPtr_()
  { }

   BranchEntryInfo::BranchEntryInfo(BranchID const& bid,
				    ProductID const& pid,
				    ProductStatus status,
				    boost::shared_ptr<EntryDescription> edPtr) :
    branchID_(bid),
    productID_(pid),
    productStatus_(status),
    entryDescriptionID_(edPtr->id()),
    entryDescriptionPtr_(edPtr)
  { EntryDescriptionRegistry::instance()->insertMapped(*edPtr);}

  void
  BranchEntryInfo::write(std::ostream& os) const {
    os << "branch ID = " << branchID() << '\n';
    os << "product ID = " << productID() << '\n';
    os << "product status = " << productStatus() << '\n';
    os << "entry description ID = " << entryDescriptionID() << '\n';
  }
    
  bool
  operator==(BranchEntryInfo const& a, BranchEntryInfo const& b) {
    return
      a.branchID() == b.branchID()
      && a.productID() == b.productID()
      && a.productStatus() == b.productStatus()
      && a.entryDescriptionID() == b.entryDescriptionID();
  }
}