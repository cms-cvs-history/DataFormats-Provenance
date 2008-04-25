#include "DataFormats/Provenance/interface/BranchMapper.h"
#include "DataFormats/Provenance/interface/BranchMapperRegistry.h"
#include <ostream>

/*----------------------------------------------------------------------

$Id: BranchMapper.cc,v 1.3 2008/02/08 17:34:39 wdd Exp $

----------------------------------------------------------------------*/

namespace edm {
  BranchMapper::BranchMapper() :
    mapping_()
  { }

  BranchMapperID
  BranchMapper::id() const
  {
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
    for (std::vector<BranchID>::const_iterator 
	   i = mapping_.begin(),
	   e = mapping_.end();
	 i != e;
	 ++i) {
	os << *i << ' ';
      }
  }
    
  bool
  operator==(BranchMapper const& a, BranchMapper const& b) {
    return
      a.mapping() == b.mapping();
  }
}
