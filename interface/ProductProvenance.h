#ifndef DataFormats_Provenance_ProductProvenance_h
#define DataFormats_Provenance_ProductProvenance_h

/*----------------------------------------------------------------------
  
ProductProvenance: The event dependent portion of the description of a product
and how it came into existence, plus the status.

----------------------------------------------------------------------*/
#include <iosfwd>
#include <vector>

#include "boost/shared_ptr.hpp"

#include "DataFormats/Provenance/interface/BranchID.h"
#include "DataFormats/Provenance/interface/EntryDescriptionID.h"
#include "DataFormats/Provenance/interface/ProductStatus.h"
#include "DataFormats/Provenance/interface/ProvenanceFwd.h"
#include "DataFormats/Provenance/interface/Transient.h"

/*
  ProductProvenance
*/

namespace edm {
  class ProductProvenance {
  public:
    typedef std::vector<ProductProvenance> EntryInfoVector;
    ProductProvenance();
    explicit ProductProvenance(BranchID const& bid);
    ProductProvenance(BranchID const& bid,
		    ProductStatus status);
    ProductProvenance(BranchID const& bid,
		    ProductStatus status,
		    boost::shared_ptr<EventEntryDescription> edPtr);
    ProductProvenance(BranchID const& bid,
		    ProductStatus status,
		    EntryDescriptionID const& edid);

    ProductProvenance(BranchID const& bid,
		   ProductStatus status,
		   ModuleDescriptionID const& mdid,
		   std::vector<BranchID> const& parents);
    ProductProvenance(BranchID const& bid,
                   ProductStatus status,
                   ModuleDescriptionID const& mdid);

    ~ProductProvenance() {}

    ProductProvenance makeEntryInfo() const;

    void write(std::ostream& os) const;

    BranchID const& branchID() const {return branchID_;}
    ProductStatus const& productStatus() const {return productStatus_;}
    EntryDescriptionID const& entryDescriptionID() const {return entryDescriptionID_;}
    EventEntryDescription const& entryDescription() const;
    void setStatus(ProductStatus status) {productStatus_ = status;}
    void setPresent();
    void setNotPresent();
    void setModuleDescriptionID(ModuleDescriptionID const& mdid) {moduleDescriptionID() = mdid;}

    ModuleDescriptionID & moduleDescriptionID() const {return transients_.get().moduleDescriptionID_;}

    bool & noEntryDescription() const {return transients_.get().noEntryDescription_;}

    struct Transients {
      Transients();
      ModuleDescriptionID moduleDescriptionID_;
      boost::shared_ptr<EventEntryDescription> entryDescriptionPtr_;
      bool noEntryDescription_;
    };

    void setDefaultTransients() const {
	transients_ = Transients();
    };

  private:

    boost::shared_ptr<EventEntryDescription> & entryDescriptionPtr() const {return transients_.get().entryDescriptionPtr_;}

    BranchID branchID_;
    ProductStatus productStatus_;
    EntryDescriptionID entryDescriptionID_;
    mutable Transient<Transients> transients_;
  };

  inline
  bool
  operator < (ProductProvenance const& a, ProductProvenance const& b) {
    return a.branchID() < b.branchID();
  }
  
  inline
  std::ostream&
  operator<<(std::ostream& os, ProductProvenance const& p) {
    p.write(os);
    return os;
  }

  // Only the 'salient attributes' are testing in equality comparison.
  bool operator==(ProductProvenance const& a, ProductProvenance const& b);
  inline bool operator!=(ProductProvenance const& a, ProductProvenance const& b) { return !(a==b); }
  typedef std::vector<ProductProvenance> ProductProvenanceVector;
}
#endif
