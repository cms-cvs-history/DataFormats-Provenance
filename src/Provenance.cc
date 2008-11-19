#include "DataFormats/Provenance/interface/Provenance.h"

/*----------------------------------------------------------------------

----------------------------------------------------------------------*/

namespace edm {

  Provenance::Provenance(BranchDescription const& p) :
    branchDescription_(p),
    productProvenancePtr_() {
  }

  Provenance::Provenance(ConstBranchDescription const& p) :
    branchDescription_(p),
    productProvenancePtr_() {
  }

  Provenance::Provenance(BranchDescription const& p, boost::shared_ptr<ProductProvenance> ei) :
    branchDescription_(p),
    productProvenancePtr_(ei)
  { }

  Provenance::Provenance(ConstBranchDescription const& p, boost::shared_ptr<ProductProvenance> ei) :
    branchDescription_(p),
    productProvenancePtr_(ei)
  { }

  Provenance::Provenance(BranchDescription const& p, boost::shared_ptr<RunLumiEntryInfo> ei) :
    branchDescription_(p),
    productProvenancePtr_(boost::shared_ptr<ProductProvenance>(
      new ProductProvenance(ei->branchID(), ei->productStatus()))) {
  }

  Provenance::Provenance(ConstBranchDescription const& p, boost::shared_ptr<RunLumiEntryInfo> ei) :
    branchDescription_(p),
    productProvenancePtr_(boost::shared_ptr<ProductProvenance>(
      new ProductProvenance(ei->branchID(), ei->productStatus()))) {
  }

  void
  Provenance::setProductProvenance(boost::shared_ptr<ProductProvenance> bei) const {
    assert(productProvenancePtr_.get() == 0);
    productProvenancePtr_ = bei;
  }

  boost::shared_ptr<ProductProvenance>
  Provenance::resolve () const {
    boost::shared_ptr<ProductProvenance> prov = store_->branchToEntryInfo(branchDescription_.branchID());
    setProductProvenance(prov);
    return prov;
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

