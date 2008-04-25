#ifndef DataFormats_Provenance_BranchMapper_h
#define DataFormats_Provenance_BranchMapper_h

/*----------------------------------------------------------------------
  
BranchMapper: The mapping from per event product ID's to BranchID's.

$Id: BranchMapper.h,v 1.2 2008/01/25 21:06:48 paterno Exp $
----------------------------------------------------------------------*/
#include <iosfwd>
#include <vector>
#include "boost/shared_ptr.hpp"

#include "DataFormats/Provenance/interface/ProductID.h"
#include "DataFormats/Provenance/interface/BranchID.h"
#include "DataFormats/Provenance/interface/BranchMapperID.h"

/*
  BranchMapper

*/

namespace edm {
  struct BranchMapper {
    BranchMapper();

    ~BranchMapper() {}

    BranchMapperID id() const;

    void write(std::ostream& os) const;
   
    void push_back(BranchID const& bid) {mapping_.push_back(bid);}

    std::vector<BranchID> const& mapping() const {return mapping_;}

    std::vector<BranchID> mapping_;

  };
  
  inline
  std::ostream&
  operator<<(std::ostream& os, BranchMapper const& p) {
    p.write(os);
    return os;
  }

}
#endif
