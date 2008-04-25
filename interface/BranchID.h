#ifndef DataFormats_Provenance_BranchID_h
#define DataFormats_Provenance_BranchID_h

/*----------------------------------------------------------------------
  
BranchID: A unique identifier for each branch.

$Id: BranchID.h,v 1.2 2007/04/01 15:39:03 wmtan Exp $

----------------------------------------------------------------------*/

#include <iosfwd>
#include <string>

namespace edm {
  struct BranchID {
    typedef std::string ID;
    BranchID();
    explicit BranchID(std::string const& str);
    ID id() const { return id_; }
    ID id_;
    bool operator<(BranchID const& rh) const {return id_ < rh.id_;}
    bool operator>(BranchID const& rh) const {return id_ > rh.id_;}
    bool operator==(BranchID const& rh) const {return id_ == rh.id_;}
    bool operator!=(BranchID const& rh) const {return id_ != rh.id_;}
  };

  std::ostream&
  operator<<(std::ostream& os, BranchID const& id);
}
#endif
