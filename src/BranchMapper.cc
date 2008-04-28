#include "DataFormats/Provenance/interface/BranchMapper.h"
#include "DataFormats/Provenance/interface/BranchMapperRegistry.h"
#include <ostream>

/*----------------------------------------------------------------------

$Id: BranchMapper.cc,v 1.1.2.1 2008/04/25 17:21:36 wmtan Exp $

----------------------------------------------------------------------*/

namespace edm {
  BranchMapper::BranchMapper() :
    mapping_(),
    highWaterMark_()
  { }

  BranchMapperID
  BranchMapper::id() const
  {
    expand();
    // This implementation is ripe for optimization.
    std::ostringstream oss;
    for (std::vector<BranchID>::const_iterator 
	   i = mapping_.begin(),
	   e = mapping_.end();
	 i != e;
	 ++i) {
	oss << *i << ' ';
      }
    
    std::string stringrep = oss.str();
    cms::Digest md5alg(stringrep);
    return BranchMapperID(md5alg.digest().toString());
  }


  void
  BranchMapper::write(std::ostream& os) const {
    // This is grossly inadequate, but it is not critical for the
    // first pass.
    expand();
    for (std::vector<BranchID>::const_iterator 
	   i = mapping_.begin(),
	   e = mapping_.end();
	 i != e;
	 ++i) {
	os << *i << ' ';
      }
  }

  void
  BranchMapper::insert(ProductID const& pid, BranchID const& bid) {
    branchToProduct_.insert(std::make_pair(bid, pid));
    if (pid.id() <= mapping().size()) {
      mapping_[pid.id() - 1] = bid;
    }
    if (pid > highWaterMark_) {
      highWaterMark_ = pid; 
    }
  }
    
  ProductID 
  BranchMapper::branchToProduct(BranchID const& bid) const {
    std::map<BranchID, ProductID>::const_iterator it = branchToProduct_.find(bid);
    if (it == branchToProduct_.end()) return ProductID();
    return it->second;
  }

  BranchID 
  BranchMapper::productToBranch(ProductID const& pid) const {
    if (pid.id() > mapping().size()) {
      expand();
    }
    return mapping_[pid.id() - 1];
  }

  void
  BranchMapper::expand() const {
    size_t oldSize = mapping().size();
    size_t newSize = highWaterMark_.id();
    std::vector<BranchID> & mappingR = const_cast<std::vector<BranchID> &>(mapping_);
    if (newSize > oldSize) {
      mappingR.resize(newSize);
    }
    for (std::map<BranchID, ProductID>::const_iterator it = branchToProduct_.begin(),
	itEnd = branchToProduct_.end();
	it != itEnd; ++it) {
      mappingR[it->second.id() - 1] = it->first;
    }
  }

  bool
  operator==(BranchMapper const& a, BranchMapper const& b) {
    a.expand();
    b.expand();
    return
      a.mapping() == b.mapping();
  }
}
