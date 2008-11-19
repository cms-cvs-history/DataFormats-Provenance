#ifndef DataFormats_Provenance_ParameterSetIDList_h
#define DataFormats_Provenance_ParameterSetIDList_h

/*----------------------------------------------------------------------
  
ParameterSetIDList: 
ParameterSetIDLists: 
        one table stored per File
	table ParameterSetIDLists keyed by ProcessInfo::branchListIndex_;
	entry ParameterSetIDList keyed by ProductID::productIndex_;

----------------------------------------------------------------------*/

#include <vector>
#include "DataFormats/Provenance/interface/ParameterSetID.h"

namespace edm {
  typedef std::vector<ParameterSetID> ParameterSetIDList;
  typedef std::vector<ParameterSetIDList> ParameterSetIDLists;
}
#endif
