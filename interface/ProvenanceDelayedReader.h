#ifndef DataFormats_Provenance_DelayedReader_h
#define DataFormats_Provenance_DelayedReader_h

/*----------------------------------------------------------------------
  
DelayedReader: The abstract interface through which the EventPrincipal
uses input sources to retrieve per-event provenance from external storage.

$Id: ProvenanceDelayedReader.h,v 1.4 2008/02/06 06:24:43 wmtan Exp $

----------------------------------------------------------------------*/

#include <memory>
#include "DataFormats/Provenance/interface/EntryDescription.h"

namespace edm {
  class BranchDescription;
  class EntryDescription;
  class ProvenanceDelayedReader {
  public:
    virtual ~ProvenanceDelayedReader();
    std::auto_ptr<EntryDescription> getProvenance(BranchDescription const& desc) const {
      return getProvenance_(desc);
    }
 private: 
    virtual std::auto_ptr<EntryDescription> getProvenance_(BranchDescription const& desc) const = 0;
  };
}

#endif
