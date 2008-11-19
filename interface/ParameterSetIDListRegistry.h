#ifndef FWCore_Framework_ParameterSetIDListRegistry_h
#define FWCore_Framework_ParameterSetIDListRegistry_h

#include "FWCore/Utilities/interface/ThreadSafeIndexedRegistry.h"
#include "DataFormats/Provenance/interface/ParameterSetIDList.h"

namespace edm {
  typedef edm::detail::ThreadSafeIndexedRegistry<ParameterSetIDList> ParameterSetIDListRegistry;
  typedef ParameterSetIDListRegistry::collection_type ParameterSetIDLists;
}

#endif
