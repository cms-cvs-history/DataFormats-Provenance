#ifndef DataFormats_Provenance_BranchEntryDescription_h
#define DataFormats_Provenance_BranchEntryDescription_h

/*----------------------------------------------------------------------
  
BranchEntryDescription: The event dependent portion of the description of a product
and how it came into existence.

----------------------------------------------------------------------*/
#include <iosfwd>
#include <vector>
#include "boost/shared_ptr.hpp"

#include "DataFormats/Provenance/interface/ProductID.h"
#include "DataFormats/Provenance/interface/EventEntryInfo.h"
#include "DataFormats/Provenance/interface/EntryDescription.h"
#include "DataFormats/Provenance/interface/ModuleDescription.h"

/*
  BranchEntryDescription

  definitions:
  Product: The EDProduct to which a provenance object is associated

  Creator: The EDProducer that made the product.

  Parents: The EDProducts used as input by the creator.
*/

namespace edm {
  struct BranchEntryDescription {
    enum CreatorStatus { Success = 0,
			 ApplicationFailure,
			 InfrastructureFailure,
			 CreatorNotRun };

    BranchEntryDescription();
    BranchEntryDescription(ProductID const& pid, CreatorStatus const& status);

    ~BranchEntryDescription() {}

    void write(std::ostream& os) const;

    ProductID const& productID() const {return productID_;}
    bool const& isPresent() const {return isPresent_;}
    CreatorStatus const& creatorStatus() const {return status_;}
    std::vector<ProductID> const& parents() const {return parents_;}

    ModuleDescriptionID const& moduleDescriptionID() const {return moduleDescriptionID_;}
    std::auto_ptr<EntryDescription> convertToEntryDescription() const;
  private:
    ProductID productID_;

    // The EDProduct IDs of the parents
    std::vector<ProductID> parents_;

    // a single identifier that describes all the conditions used
    unsigned int cid_; // frame ID?

    // the last of these is not in the roadmap, but is on the board

    // if modules can or will place an object in the event
    // even though something not good occurred, like a timeout, then
    // this may be useful - or if the I/O system makes blank or default
    // constructed objects and we need to distinguish between zero
    // things in a collection between nothing was found and the case
    // where a failure caused nothing to be in the collection.
    // Should a provenance be inserted even if a module fails to 
    // create the output it promised?
    CreatorStatus status_;

    // Is the object present in the event. This can be false if the object
    // was never created (status != Success), or if the branch containing
    // the product was dropped.
    bool isPresent_;

    ModuleDescriptionID moduleDescriptionID_;

  };
  
  inline
  std::ostream&
  operator<<(std::ostream& os, BranchEntryDescription const& p) {
    p.write(os);
    return os;
  }

  bool operator==(BranchEntryDescription const& a, BranchEntryDescription const& b);
}
#endif
