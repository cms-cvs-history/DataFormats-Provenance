#include "DataFormats/Provenance/interface/History.h"

namespace edm
{
  
  History::size_type
  History::size() const {
    return eventSelections_.size();
  }

  ProcessIndex
  History::currentProcessIndex() const {
    return branchListIndexes_.size() - 1;
  }

  void 
  History::addEntry(EventSelectionID const& eventSelection) {
    eventSelections_.push_back(eventSelection);
  }

  void 
  History::addEntry(BranchListIndex const& branchListIndex) {
    branchListIndexes_.push_back(branchListIndex);
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
