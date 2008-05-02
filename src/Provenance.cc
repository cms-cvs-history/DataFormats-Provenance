#include "DataFormats/Provenance/interface/Provenance.h"

/*----------------------------------------------------------------------

----------------------------------------------------------------------*/

namespace edm {

  Provenance::Provenance(BranchDescription const& p, ProductStatus status, boost::shared_ptr<EntryDescription> entryDesc) :
    branchDescription_(p),
    branchEntryInfo_(p.branchID(), p.productIDtoAssign(), status, entryDesc)
  { }

  Provenance::Provenance(ConstBranchDescription const& p, ProductStatus status, boost::shared_ptr<EntryDescription> entryDesc) :
    branchDescription_(p),
    branchEntryInfo_(p.me().branchID(), p.productIDtoAssign(), status, entryDesc)
  { }

  void
  Provenance::setPresent() {
    if (productstatus::present(productStatus())) return;
    assert(productstatus::unknown(productStatus()));
    branchEntryInfo_.setStatus(productstatus::present());
  }

  void
  Provenance::setNotPresent() {
    if (productstatus::neverCreated(productStatus())) return;
    assert(productstatus::unknown(productStatus()));
    branchEntryInfo_.setStatus(productstatus::neverCreated());
  }

  void
  Provenance::write(std::ostream& os) const {
    // This is grossly inadequate, but it is not critical for the
    // first pass.
    product().write(os);
    entryDescription().write(os);
  }

    
  bool operator==(Provenance const& a, Provenance const& b) {
    return
      a.product() == b.product()
      && a.entryDescription() == b.entryDescription();
  }

}

