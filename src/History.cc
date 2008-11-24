#include "DataFormats/Provenance/interface/History.h"

namespace edm
{
  
  History::size_type
  History::size() const {
    return eventSelections_.size();
  }

  void 
  History::addEventSelectionEntry(EventSelectionID const& eventSelection) {
    eventSelections_.push_back(eventSelection);
  }

  void 
  History::addBranchListIndexEntry(BranchListIndex const& branchListIndex) {
    branchListIndexes_.push_back(branchListIndex);
  }

  void 
  History::addParameterSetListIndexEntry(ParameterSetListIndex const& parameterSetListIndex) {
    parameterSetListIndexes_.push_back(parameterSetListIndex);
  }

  EventSelectionID const&
  History::getEventSelectionID(History::size_type i) const {
    return eventSelections_[i];
  }

  EventSelectionIDVector const&
  History::eventSelectionIDs() const {
    return eventSelections_;
  }

  ProcessHistoryID const&
  History::processHistoryID() const {
    return processHistoryID_;
  }

  ProcessHistoryID &
  History::processHistoryID() {
    return processHistoryID_;
  }

}
