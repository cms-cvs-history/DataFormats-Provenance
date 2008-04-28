#ifndef DataFormats_Provenance_BranchMapper_h
#define DataFormats_Provenance_BranchMapper_h

/*----------------------------------------------------------------------
  
BranchMapper: The mapping from per event product ID's to BranchID's.

$Id: BranchMapper.h,v 1.1.2.1 2008/04/25 17:21:36 wmtan Exp $
----------------------------------------------------------------------*/
#include <iosfwd>
#include <map>
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

    ProductID branchToProduct(BranchID const& bid) const;

    BranchID productToBranch(ProductID const& pid) const;
    
    void insert(ProductID const& pid, BranchID const& bid);

    void expand() const;

    std::vector<BranchID> const& mapping() const {return mapping_;}

    std::vector<BranchID> mapping_;

    std::map<BranchID, ProductID> branchToProduct_; //! transient

    ProductID highWaterMark_; //!transient
  };
  
  inline
  std::ostream&
  operator<<(std::ostream& os, BranchMapper const& p) {
    p.write(os);
    return os;
  }

}
#endif
