#ifndef DataFormats_Provenance_Provenance_h
#define DataFormats_Provenance_Provenance_h

/*----------------------------------------------------------------------
  
Provenance: The full description of a product and how it came into
existence.

$Id: Provenance.h,v 1.7.2.1 2008/04/28 17:58:32 wmtan Exp $
----------------------------------------------------------------------*/
#include <iosfwd>

#include "DataFormats/Provenance/interface/BranchDescription.h"
#include "DataFormats/Provenance/interface/BranchEntryInfo.h"
#include "DataFormats/Provenance/interface/EntryDescription.h"
#include "DataFormats/Provenance/interface/ConstBranchDescription.h"
#include "boost/shared_ptr.hpp"

/*
  Provenance

  definitions:
  Product: The EDProduct to which a provenance object is associated

  Creator: The EDProducer that made the product.

  Parents: The EDProducts used as input by the creator.
*/

namespace edm {
  class Provenance {
  public:
    Provenance(ConstBranchDescription const& p,
	       ProductStatus status,
	       boost::shared_ptr<EntryDescription> entryDesc = boost::shared_ptr<EntryDescription>()); 
    Provenance(BranchDescription const& p,
	       ProductStatus status,
	       boost::shared_ptr<EntryDescription> entryDesc = boost::shared_ptr<EntryDescription>()); 

    ~Provenance() {}

    EntryDescription const& event() const {return entryDescription();}
    BranchDescription const& product() const {return branchDescription_.me();}
    BranchEntryInfo const& branchEntryInfo() const {return branchEntryInfo_;}
    EntryDescription const& entryDescription() const {return branchEntryInfo_.entryDescription();}
    BranchID const& branchID() const {return product().branchID();}
    std::string const& branchName() const {return product().branchName();}
    std::string const& className() const {return product().className();}
    std::string const& moduleLabel() const {return product().moduleLabel();}
    std::string const& moduleName() const {return entryDescription().moduleName();}
    PassID const& passID() const {return entryDescription().passID();}
    std::string const& processName() const {return product().processName();}
    ProductID const& productID() const {return branchEntryInfo_.productID();}
    ProductStatus const& productStatus() const {return branchEntryInfo_.productStatus();}
    std::string const& productInstanceName() const {return product().productInstanceName();}
    std::string const& friendlyClassName() const {return product().friendlyClassName();}
    std::set<ParameterSetID> const& psetIDs() const {return product().psetIDs();}
    ParameterSetID const& psetID() const {return entryDescription().psetID();}
    ReleaseVersion const& releaseVersion() const {return entryDescription().releaseVersion();}
    std::set<std::string> const& branchAliases() const {return product().branchAliases();}
    ModuleDescriptionID const& moduleDescriptionID() const {return entryDescription().moduleDescriptionID();}
    ModuleDescription const& moduleDescription() const {return entryDescription().moduleDescription();}
    bool isPresent() const {return productstatus::present(productStatus());}

    std::vector<ProductID> const& parents() const {return entryDescription().parents();}

    void write(std::ostream& os) const;

    void setPresent();

    void setNotPresent();

  private:
    ConstBranchDescription const branchDescription_;
    BranchEntryInfo branchEntryInfo_;
  };
  
  inline
  std::ostream&
  operator<<(std::ostream& os, Provenance const& p) {
    p.write(os);
    return os;
  }

  bool operator==(Provenance const& a, Provenance const& b);
}
#endif
