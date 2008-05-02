#include "DataFormats/Provenance/interface/Provenance.h"

/*----------------------------------------------------------------------

----------------------------------------------------------------------*/

namespace edm {

  Provenance::Provenance(BranchDescription const& p, boost::shared_ptr<BranchEntryInfo> bei) :
    branchDescription_(p),
    branchEntryInfoPtr_(bei)
  { }

  Provenance::Provenance(ConstBranchDescription const& p, boost::shared_ptr<BranchEntryInfo> bei) :
    branchDescription_(p),
    branchEntryInfoPtr_(bei)
  { }

  void
  Provenance::setPresent() {
    if (productstatus::present(productStatus())) return;
    assert(productstatus::unknown(productStatus()));
    branchEntryInfoPtr_->setStatus(productstatus::present());
  }

  void
  Provenance::setNotPresent() {
    if (productstatus::neverCreated(productStatus())) return;
    assert(productstatus::unknown(productStatus()));
    branchEntryInfoPtr_->setStatus(productstatus::neverCreated());
  }

  void
  Provenance::setBranchEntryInfo(boost::shared_ptr<BranchEntryInfo> bei) const {
    assert(branchEntryInfoPtr_.get() == 0);
    branchEntryInfoPtr_ = bei;
  }

  void
  Provenance::write(std::ostream& os) const {
    // This is grossly inadequate, but it is not critical for the
    // first pass.
    product().write(os);
    branchEntryInfo().write(os);
  }

    
  bool operator==(Provenance const& a, Provenance const& b) {
    return
      a.product() == b.product()
      && a.entryDescription() == b.entryDescription();
  }

}

