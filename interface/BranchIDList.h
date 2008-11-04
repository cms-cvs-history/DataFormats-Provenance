#ifndef DataFormats_Provenance_BranchIDList_h
#define DataFormats_Provenance_BranchIDList_h

/*----------------------------------------------------------------------
  
BranchIDList: 
BranchIDListVector: 
        one table stored per File
	table BranchIDListVector keyed by ProcessInfo::branchListIndex_;
	entry BranchIDList keyed by ProductID::productIndex_;

----------------------------------------------------------------------*/

#include <vector>
#include "DataFormats/Provenance/interface/BranchID.h"

namespace edm {
  typedef std::vector<BranchID> BranchIDList;
  typedef std::vector<BranchIDList> BranchIDListVector;
}
#endif
