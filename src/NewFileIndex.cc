#include "DataFormats/Provenance/interface/NewFileIndex.h"
#include "FWCore/Utilities/interface/Algorithms.h"

#include <algorithm>
#include <ostream>
#include <iomanip>
#include <map>

namespace edm {

  NewFileIndex::NewFileIndex() : entries_(),
                           transients_(),
                           processHistoryIDs_() {
  }

  // The default value for sortState_ reflects the fact that
  // the index is always sorted using Index, Run, Lumi, and Event
  // number by the PoolOutputModule before being written out.
  // In the other case when we create a new NewFileIndex, the
  // vector is empty, which is consistent with it having been
  // sorted.

  NewFileIndex::Transients::Transients() : allInEntryOrder_(false),
                                        resultCached_(false),
                                        sortState_(kSorted_Index_Run_Lumi_Event),
                                        previousAddedIndex_(Element::invalidIndex) {
  }

  void
  NewFileIndex::addEntry(ProcessHistoryID processHistoryID,
                      RunNumber_t run,
                      LuminosityBlockNumber_t lumi,
                      EventNumber_t event,
                      EntryNumber_t entry) {
    int index;
    // First see if the ProcessHistoryID is the same as the previous one.
    // This is just a performance optimization.  We expect to usually get
    // many in a row that are the same.
    if (previousAddedIndex() != Element::invalidIndex &&
        processHistoryID == processHistoryIDs_[previousAddedIndex()]) {
      index = previousAddedIndex();
    }
    // If it was not the same as the previous one then search through the
    // entire vector.  If it is not there, it needs to be added at the
    // end.
    else {
      index = 0;
      while (index < static_cast<int>(processHistoryIDs_.size()) && 
             processHistoryIDs_[index] != processHistoryID) {
        ++index;        
      }
      if (index == static_cast<int>(processHistoryIDs_.size())) {
        processHistoryIDs_.push_back(processHistoryID);
      }
    }
    entries_.push_back(NewFileIndex::Element(index, run, lumi, event, entry));
    resultCached() = false;
    sortState() = kNotSorted;
    previousAddedIndex() = index;
  }

  void NewFileIndex::sortBy_Index_Run_Lumi_Event() {
    stable_sort_all(entries_);
    resultCached() = false;
    sortState() = kSorted_Index_Run_Lumi_Event;
  }

  void NewFileIndex::sortBy_Index_Run_Lumi_Entry() {
    stable_sort_all(entries_, Compare_Index_Run_Lumi_Entry());
    resultCached() = false;
    sortState() = kSorted_Index_Run_Lumi_Entry;
  }

  bool NewFileIndex::allEventsInEntryOrder() const {
    if(!resultCached()) {
      resultCached() = true;
      EntryNumber_t maxEntry = Element::invalidEntry;
      for(std::vector<NewFileIndex::Element>::const_iterator it = entries_.begin(), itEnd = entries_.end(); it != itEnd; ++it) {
        if(it->getEntryType() == kEvent) {
	  if(it->entry() < maxEntry) {
	    allInEntryOrder() = false;
	    return false;
          }
	  maxEntry = it->entry();
        }
      }
      allInEntryOrder() = true;
      return true;
    }
    return allInEntryOrder();
  }

  NewFileIndex::const_iterator
  NewFileIndex::findPosition(RunNumber_t run, LuminosityBlockNumber_t lumi, EventNumber_t event) const {

    assert(sortState() != kNotSorted);

    bool lumiMissing = (lumi == 0 && event != 0);

    const_iterator it;
    const_iterator iEnd = entries_.end();
    const_iterator phEnd;

    // Loop over ranges of entries with the same ProcessHistoryID
    for (const_iterator phBegin = entries_.begin();
         phBegin != iEnd;
         phBegin = phEnd) {

      Element el(phBegin->processHistoryIDIndex(), run, lumi, event);
      phEnd = std::upper_bound(phBegin, iEnd, el, Compare_Index());

      if (!lumiMissing) {
        if (sortState() == kSorted_Index_Run_Lumi_Event) {

          it = std::lower_bound(phBegin, phEnd, el);
          if (it == phEnd) continue;
          if (it->run() == run && it->lumi() == lumi && it->event() == event) return it;
        }

        else { // sortState() == kSorted_Index_Run_Lumi_Entry
	  std::pair<const_iterator, const_iterator> lumiRange = 
              std::equal_range(phBegin, phEnd, el, Compare_Index_Run_Lumi());
	  if (lumiRange.first == phEnd) continue;
	  it = std::find(lumiRange.first, lumiRange.second, el);
          if (it  == lumiRange.second) continue;
        }
      }
      else { // if (lumiMissing)

	std::pair<const_iterator, const_iterator> runRange = 
            std::equal_range(phBegin, phEnd, el, Compare_Index_Run());
	if (runRange.first == phEnd) continue;

        const_iterator lumiEnd;
        for (const_iterator lumiBegin = runRange.first;
             lumiBegin != runRange.second;
             lumiBegin = lumiEnd) {

          Element elWithLumi(phBegin->processHistoryIDIndex(), run, lumiBegin->lumi(), event);
          lumiEnd = std::upper_bound(lumiBegin, runRange.second, elWithLumi, Compare_Index_Run_Lumi());

          if (sortState() == kSorted_Index_Run_Lumi_Event) {
            it = std::lower_bound(lumiBegin, lumiEnd, elWithLumi);
          }
          else {
            it = std::find(lumiBegin, lumiEnd, elWithLumi);
          }  
          if (it == lumiEnd) continue;
          if (it->event() == event) return it;
        }
      }
    } // Loop over ProcessHistoryIDs
    return iEnd;
  }

  NewFileIndex::const_iterator
  NewFileIndex::findEventPosition(RunNumber_t run, LuminosityBlockNumber_t lumi, EventNumber_t event) const {
    return findPosition(run, lumi, event);
  }

  NewFileIndex::const_iterator
  NewFileIndex::findLumiPosition(RunNumber_t run, LuminosityBlockNumber_t lumi) const {
    return findPosition(run, lumi, 0U);
  }

  NewFileIndex::const_iterator
  NewFileIndex::findRunPosition(RunNumber_t run) const {
    return findPosition(run, 0U, 0U);
  }

  NewFileIndex::const_iterator
  NewFileIndex::findEventEntryPosition(RunNumber_t run,
                                    LuminosityBlockNumber_t lumi,
                                    EventNumber_t event,
                                    EntryNumber_t entry) const {
    assert(sortState() != kNotSorted);

    bool lumiMissing = (lumi == 0 && event != 0);

    const_iterator it;
    const_iterator iEnd = entries_.end();
    const_iterator phEnd;

    // Loop over ranges of entries with the same ProcessHistoryID
    for (const_iterator phBegin = entries_.begin();
         phBegin != iEnd;
         phBegin = phEnd) {

      Element el(phBegin->processHistoryIDIndex(), run, lumi, event, entry);
      phEnd = std::upper_bound(phBegin, iEnd, el, Compare_Index());

      if (!lumiMissing) {
        if (sortState() == kSorted_Index_Run_Lumi_Event) {

          it = std::lower_bound(phBegin, phEnd, el);
          while (it != phEnd &&
                 it->run() == run &&
                 it->lumi() == lumi &&
                 it->event() == event) {
            if (entry == it->entry()) return it;
            ++it;
          }
        }

        else { // sortState() == kSorted_Index_Run_Lumi_Entry
	  it = std::lower_bound(phBegin, phEnd, el, Compare_Index_Run_Lumi_Entry());
          if (it != phEnd &&
              it->run() == run &&
              it->lumi() == lumi &&
              it->event() == event &&
              it->entry() == entry) return it;
       }
      }
      else { // lumiMissing was true

	std::pair<const_iterator, const_iterator> runRange = 
            std::equal_range(phBegin, phEnd, el, Compare_Index_Run());
	if (runRange.first == phEnd) continue;

        const_iterator lumiEnd;
        for (const_iterator lumiBegin = runRange.first;
             lumiBegin != runRange.second;
             lumiBegin = lumiEnd) {

	  Element elWithLumi(phBegin->processHistoryIDIndex(), run, lumiBegin->lumi(), event, entry);
          lumiEnd = std::upper_bound(lumiBegin, runRange.second, elWithLumi, Compare_Index_Run_Lumi());

          if (sortState() == kSorted_Index_Run_Lumi_Event) {
            it = std::lower_bound(lumiBegin, lumiEnd, elWithLumi);
            while (it != lumiEnd &&
                   it->event() == event) {
              if (entry == it->entry()) return it;
              ++it;
            }
          }
	  else {
	    it = std::lower_bound(lumiBegin, lumiEnd, elWithLumi, Compare_Index_Run_Lumi_Entry());
            if (it != lumiEnd &&
                it->event() == event &&
                it->entry() == entry) return it;
          }
        }
      } // lumiMissing 
    } // Loop over ProcessHistoryIDs
    return iEnd; // Did not find it
  }

  NewFileIndex::const_iterator
  NewFileIndex::findNextRun(NewFileIndex::const_iterator const& iter) const {
    return std::upper_bound(iter, entries_.end(), *iter, Compare_Index_Run());
  }

  NewFileIndex::const_iterator
  NewFileIndex::findNextLumiOrRun(NewFileIndex::const_iterator const& iter) const {
    return std::upper_bound(iter, entries_.end(), *iter, Compare_Index_Run_Lumi());
  }

  void
  NewFileIndex::fixIndexes(std::vector<ProcessHistoryID> & processHistoryIDs) {

    std::map<int, int> oldToNewIndex;
    for (std::vector<ProcessHistoryID>::const_iterator iter = processHistoryIDs_.begin(),
                                                       iEnd = processHistoryIDs_.end();
         iter != iEnd;
         ++iter) {
      std::vector<ProcessHistoryID>::const_iterator iterExisting =
        std::find(processHistoryIDs.begin(), processHistoryIDs.end(), *iter);
      if (iterExisting == processHistoryIDs.end()) {
        oldToNewIndex[iter - processHistoryIDs_.begin()] = processHistoryIDs.size();
        processHistoryIDs.push_back(*iter);
      }
      else {
        oldToNewIndex[iter - processHistoryIDs_.begin()] = iterExisting - processHistoryIDs.begin();
      }
    }
    processHistoryIDs_ = processHistoryIDs;

    for (std::vector<Element>::iterator iter = entries_.begin(),
	                                iEnd = entries_.end();
         iter != iEnd;
         ++iter) {
      iter->setProcessHistoryIDIndex(oldToNewIndex[iter->processHistoryIDIndex()]);
    }
  }

  bool operator<(NewFileIndex::Element const& lh, NewFileIndex::Element const& rh) {
    if (lh.processHistoryIDIndex() == rh.processHistoryIDIndex()) {
      if(lh.run() == rh.run()) {
        if(lh.lumi() == rh.lumi()) {
	  return lh.event() < rh.event();
        }
        return lh.lumi() < rh.lumi();
      }
      return lh.run() < rh.run();
    }
    return lh.processHistoryIDIndex() < rh.processHistoryIDIndex();
  }

  bool Compare_Index_Run_Lumi_Entry::operator()(NewFileIndex::Element const& lh, NewFileIndex::Element const& rh) {
    if (lh.processHistoryIDIndex() == rh.processHistoryIDIndex()) {
      if(lh.run() == rh.run()) {
        if(lh.lumi() == rh.lumi()) {
          // Both are Events
          if (lh.event() != 0U && rh.event() != 0U) {
            return lh.entry() < rh.entry();
          }
          // Both are not Events, both are Runs or both are Lumis
          else if (lh.event() == 0U && rh.event() == 0U) {
            return lh.entry() < rh.entry();
          }
          // Runs or Lumis come before Events
          else if (lh.event() == 0U) {
	    return true;
          }
          else {
	    return false;
	  }
        }
        return lh.lumi() < rh.lumi();
      }
      return lh.run() < rh.run();
    }
    return lh.processHistoryIDIndex() < rh.processHistoryIDIndex();
  }

  bool Compare_Index_Run_Lumi::operator()(NewFileIndex::Element const& lh, NewFileIndex::Element const& rh) {
    if (lh.processHistoryIDIndex() == rh.processHistoryIDIndex()) {
      if(lh.run() == rh.run()) {
        return lh.lumi() < rh.lumi();
      }
      return lh.run() < rh.run();
    }
    return lh.processHistoryIDIndex() < rh.processHistoryIDIndex();
  }

  bool Compare_Index_Run::operator()(NewFileIndex::Element const& lh, NewFileIndex::Element const& rh) {
    if (lh.processHistoryIDIndex() == rh.processHistoryIDIndex()) {
      return lh.run() < rh.run();
    }
    return lh.processHistoryIDIndex() < rh.processHistoryIDIndex();
  }

  bool Compare_Index::operator()(NewFileIndex::Element const& lh, NewFileIndex::Element const& rh) {
    return lh.processHistoryIDIndex() < rh.processHistoryIDIndex();
  }

  std::ostream&
  operator<<(std::ostream& os, NewFileIndex const& fileIndex) {

    os << "\nPrinting NewFileIndex contents.  This includes a list of all Runs, LuminosityBlocks\n"
       << "and Events stored in the root file.\n\n";
    os << std::setw(15) << "Process History"
       << std::setw(15) << "Run"
       << std::setw(15) << "Lumi"
       << std::setw(15) << "Event"
       << std::setw(19) << "TTree Entry"
       << "\n";
    for(std::vector<NewFileIndex::Element>::const_iterator it = fileIndex.begin(), itEnd = fileIndex.end(); it != itEnd; ++it) {
      if(it->getEntryType() == NewFileIndex::kEvent) {
        os << std::setw(15) << it->processHistoryIDIndex()
           << std::setw(15) << it->run()
           << std::setw(15) << it ->lumi()
           << std::setw(15) << it->event()
           << std::setw(19) << it->entry()
           << "\n";
      }
      else if(it->getEntryType() == NewFileIndex::kLumi) {
        os << std::setw(15) << it->processHistoryIDIndex()
           << std::setw(15) << it->run()
           << std::setw(15) << it ->lumi()
           << std::setw(15) << " "
           << std::setw(19) << it->entry() << "  (LuminosityBlock)"
           << "\n";
      }
      else if(it->getEntryType() == NewFileIndex::kRun) {
        os << std::setw(15) << it->processHistoryIDIndex()
           << std::setw(15) << it->run()
           << std::setw(15) << " "
           << std::setw(15) << " "
           << std::setw(19) << it->entry() << "  (Run)"
           << "\n";
      }
    }
    os << "\nProcess History IDs (the first value on each line above is an index into this list of IDs)\n";
    int i = 0;
    for (std::vector<ProcessHistoryID>::const_iterator iter = fileIndex.processHistoryIDs().begin(),
                                                       iEnd = fileIndex.processHistoryIDs().end();
         iter != iEnd;
         ++iter, ++i) {
      os << "  " << i << "  " << *iter << "\n";
    }
    return os;
  }
}
