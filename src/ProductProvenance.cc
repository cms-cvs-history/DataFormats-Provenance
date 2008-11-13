#include "DataFormats/Provenance/interface/ProductProvenance.h"
#include "DataFormats/Provenance/interface/EntryDescriptionRegistry.h"
#include <ostream>

/*----------------------------------------------------------------------

----------------------------------------------------------------------*/

namespace edm {
  ProductProvenance::Transients::Transients() :
    moduleDescriptionID_(),
    entryDescriptionPtr_(),
    noEntryDescription_(false)
  {}

  ProductProvenance::ProductProvenance() :
    branchID_(),
    productStatus_(productstatus::uninitialized()),
    entryDescriptionID_(),
    transients_()
  {}

  ProductProvenance::ProductProvenance(BranchID const& bid) :
    branchID_(bid),
    productStatus_(productstatus::uninitialized()),
    entryDescriptionID_(),
    transients_()
  {}

   ProductProvenance::ProductProvenance(BranchID const& bid,
				    ProductStatus status) :
    branchID_(bid),
    productStatus_(status),
    entryDescriptionID_(),
    transients_()
  {}

   ProductProvenance::ProductProvenance(BranchID const& bid,
				    ProductStatus status,
				    EntryDescriptionID const& edid) :
    branchID_(bid),
    productStatus_(status),
    entryDescriptionID_(edid),
    transients_()
  {}

   ProductProvenance::ProductProvenance(BranchID const& bid,
				    ProductStatus status,
				    boost::shared_ptr<EventEntryDescription> edPtr) :
    branchID_(bid),
    productStatus_(status),
    entryDescriptionID_(edPtr->id()),
    transients_() {
       moduleDescriptionID() = edPtr->moduleDescriptionID();
       entryDescriptionPtr() = edPtr;
       EntryDescriptionRegistry::instance()->insertMapped(*edPtr);
  }

  ProductProvenance::ProductProvenance(BranchID const& bid,
		   ProductStatus status,
		   ModuleDescriptionID const& mdid,
		   std::vector<BranchID> const& parents) :
    branchID_(bid),
    productStatus_(status),
    entryDescriptionID_(),
    transients_() {
      moduleDescriptionID() = mdid;
      entryDescriptionPtr() = boost::shared_ptr<EventEntryDescription>(new EventEntryDescription);
      entryDescriptionPtr()->parents() = parents;
      entryDescriptionPtr()->moduleDescriptionID() = mdid;
      entryDescriptionID_ = entryDescriptionPtr()->id();
      EntryDescriptionRegistry::instance()->insertMapped(*entryDescriptionPtr());
  }

  ProductProvenance::ProductProvenance(BranchID const& bid,
		   ProductStatus status,
		   ModuleDescriptionID const& mdid) :
    branchID_(bid),
    productStatus_(status),
    entryDescriptionID_(),
    transients_() {
      moduleDescriptionID() = mdid;
      noEntryDescription() = true;
    }

  ProductProvenance
  ProductProvenance::makeEntryInfo() const {
    return *this;
  }

  EventEntryDescription const &
  ProductProvenance::entryDescription() const {
    if (!entryDescriptionPtr()) {
      entryDescriptionPtr().reset(new EventEntryDescription);
      EntryDescriptionRegistry::instance()->getMapped(entryDescriptionID_, *entryDescriptionPtr());
      moduleDescriptionID() = entryDescriptionPtr()->moduleDescriptionID();
    }
    return *entryDescriptionPtr();
  }

  void
  ProductProvenance::setPresent() {
    if (productstatus::present(productStatus())) return;
    assert(productstatus::unknown(productStatus()));
    setStatus(productstatus::present());
  }

  void
  ProductProvenance::setNotPresent() {
    if (productstatus::neverCreated(productStatus())) return;
    if (productstatus::dropped(productStatus())) return;
    assert(productstatus::unknown(productStatus()));
    setStatus(productstatus::neverCreated());
  }

  void
  ProductProvenance::write(std::ostream& os) const {
    os << "branch ID = " << branchID() << '\n';
    os << "product status = " << static_cast<int>(productStatus()) << '\n';
    if (noEntryDescription()) {
      os << "module description ID = " << moduleDescriptionID() << '\n';
    } else {
      os << "entry description ID = " << entryDescriptionID() << '\n';
    }
  }
    
  bool
  operator==(ProductProvenance const& a, ProductProvenance const& b) {
    if (a.noEntryDescription() != b.noEntryDescription()) return false;
    if (a.noEntryDescription()) {
      return
        a.branchID() == b.branchID()
        && a.productStatus() == b.productStatus()
        && a.moduleDescriptionID() == b.moduleDescriptionID();
    }
    return
      a.branchID() == b.branchID()
      && a.productStatus() == b.productStatus()
      && a.entryDescriptionID() == b.entryDescriptionID();
  }
}
