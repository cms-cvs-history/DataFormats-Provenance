#ifndef DataFormats_Provenance_ParameterSetListIndex_h
#define DataFormats_Provenance_ParameterSetListIndex_h

/*----------------------------------------------------------------------
  
ProcessInfo: 
ProcessInfoVector: 
	table keyed by ProductID::processIndex_.
	processIndex_ is monotonically increasing with time.
        one table stored per Occurrence(Event. Lumi, Run)

----------------------------------------------------------------------*/

#include <vector>

namespace edm {
  typedef unsigned short ParameterSetListIndex;
  typedef std::vector<ParameterSetListIndex> ParameterSetListIndexes;
}
#endif
