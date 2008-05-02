#ifndef DataFormats_Provenance_BranchEntryInfo_h
#define DataFormats_Provenance_BranchEntryInfo_h

/*----------------------------------------------------------------------
  
BranchEntryInfo: The event dependent portion of the description of a product
and how it came into existence, plus the product identifier and the status.

$Id: BranchEntryInfo.h,v 1.1.2.2 2008/05/01 20:36:22 wdd Exp $
----------------------------------------------------------------------*/
#include <iosfwd>

#include "boost/shared_ptr.hpp"

#include "DataFormats/Provenance/interface/BranchID.h"
#include "DataFormats/Provenance/interface/EntryDescriptionID.h"
#include "DataFormats/Provenance/interface/ProductID.h"
#include "DataFormats/Provenance/interface/ProductStatus.h"
#include "DataFormats/Provenance/interface/ProvenanceFwd.h"

/*
  BranchEntryInfo
*/

namespace edm {
  class BranchEntryInfo {
  public:
    BranchEntryInfo();
    explicit BranchEntryInfo(BranchID const& bid);
    BranchEntryInfo(BranchID const& bid,
		    ProductID const& pid,
		    ProductStatus status,
		    boost::shared_ptr<EntryDescription> edPtr);

    ~BranchEntryInfo() {}

    void write(std::ostream& os) const;

    BranchID const& branchID() const {return branchID_;}
    ProductID const& productID() const {return productID_;}
    ProductStatus const& productStatus() const {return productStatus_;}
    EntryDescriptionID const& entryDescriptionID() const {return entryDescriptionID_;}
    EntryDescription const& entryDescription() const {return *entryDescriptionPtr_;}
    void setStatus(ProductStatus status) {productStatus_ = status;}

  private:
    BranchID branchID_;
    ProductID productID_;
    ProductStatus productStatus_;
    EntryDescriptionID entryDescriptionID_;
    mutable boost::shared_ptr<EntryDescription> entryDescriptionPtr_;
  };

  inline
  bool
  operator < (BranchEntryInfo const& a, BranchEntryInfo const& b) {
    return a.branchID() < b.branchID();
  }
  
  inline
  std::ostream&
  operator<<(std::ostream& os, BranchEntryInfo const& p) {
    p.write(os);
    return os;
  }

  // Only the 'salient attributes' are testing in equality comparison.
  bool operator==(BranchEntryInfo const& a, BranchEntryInfo const& b);
  inline bool operator!=(BranchEntryInfo const& a, BranchEntryInfo const& b) { return !(a==b); }

  typedef std::vector<BranchEntryInfo> BranchEntryInfoVector;
}
#endif
