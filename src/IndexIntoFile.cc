#include "DataFormats/Provenance/interface/IndexIntoFile.h"
#include "FWCore/Utilities/interface/Algorithms.h"

#include <algorithm>
#include <ostream>
#include <iomanip>
#include <map>

namespace edm {

  IndexIntoFile::IndexIntoFile() : entries_(),
                           transients_(),
                           processHistoryIDs_() {
  }

  // The default value for sortState_ reflects the fact that
  // the index is always sorted using Index, Run, Lumi, and Event
  // number by the PoolOutputModule before being written out.
  // In the other case when we create a new IndexIntoFile, the
  // vector is empty, which is consistent with it having been
  // sorted.

  IndexIntoFile::Transients::Transients() : allInEntryOrder_(false),
                                        resultCached_(false),
                                        sortState_(kSorted_Index_Run_Lumi_Event),
                                        previousAddedIndex_(Element::invalidIndex) {
  }

  void
  IndexIntoFile::addEntry(ProcessHistoryID const& processHistoryID,
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
    entries_.push_back(IndexIntoFile::Element(index, run, lumi, event, entry));
    resultCached() = false;
    sortState() = kNotSorted;
    previousAddedIndex() = index;
  }

  void IndexIntoFile::sortBy_Index_Run_Lumi_Event() {
    stable_sort_all(entries_);
    resultCached() = false;
    sortState() = kSorted_Index_Run_Lumi_Event;
  }

  void IndexIntoFile::sortBy_Index_Run_Lumi_Entry() {
    stable_sort_all(entries_, Compare_Index_Run_Lumi_Entry());
    resultCached() = false;
    sortState() = kSorted_Index_Run_Lumi_Entry;
  }

  bool IndexIntoFile::allEventsInEntryOrder() const {
    if(!resultCached()) {
      resultCached() = true;
      EntryNumber_t maxEntry = Element::invalidEntry;
      for(std::vector<IndexIntoFile::Element>::const_iterator it = entries_.begin(), itEnd = entries_.end(); it != itEnd; ++it) {
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

  IndexIntoFile::const_iterator
  IndexIntoFile::findPosition(RunNumber_t run, LuminosityBlockNumber_t lumi, EventNumber_t event) const {

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
          return it;
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

  IndexIntoFile::const_iterator
  IndexIntoFile::findEventPosition(RunNumber_t run, LuminosityBlockNumber_t lumi, EventNumber_t event) const {
    return findPosition(run, lumi, event);
  }

  IndexIntoFile::const_iterator
  IndexIntoFile::findLumiPosition(RunNumber_t run, LuminosityBlockNumber_t lumi) const {
    return findPosition(run, lumi, 0U);
  }

  IndexIntoFile::const_iterator
  IndexIntoFile::findRunPosition(RunNumber_t run) const {
    return findPosition(run, 0U, 0U);
  }

  IndexIntoFile::const_iterator
  IndexIntoFile::findEventEntryPosition(RunNumber_t run,
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

  IndexIntoFile::const_iterator
  IndexIntoFile::findNextRun(IndexIntoFile::const_iterator const& iter) const {
    return std::upper_bound(iter, entries_.end(), *iter, Compare_Index_Run());
  }

  IndexIntoFile::const_iterator
  IndexIntoFile::findNextLumiOrRun(IndexIntoFile::const_iterator const& iter) const {
    return std::upper_bound(iter, entries_.end(), *iter, Compare_Index_Run_Lumi());
  }

  void
  IndexIntoFile::fixIndexes(std::vector<ProcessHistoryID> & processHistoryIDs) {

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

  bool operator<(IndexIntoFile::Element const& lh, IndexIntoFile::Element const& rh) {
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

  bool Compare_Index_Run_Lumi_Entry::operator()(IndexIntoFile::Element const& lh, IndexIntoFile::Element const& rh) {
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

  bool Compare_Index_Run_Lumi::operator()(IndexIntoFile::Element const& lh, IndexIntoFile::Element const& rh) {
    if (lh.processHistoryIDIndex() == rh.processHistoryIDIndex()) {
      if(lh.run() == rh.run()) {
        return lh.lumi() < rh.lumi();
      }
      return lh.run() < rh.run();
    }
    return lh.processHistoryIDIndex() < rh.processHistoryIDIndex();
  }

  bool Compare_Index_Run::operator()(IndexIntoFile::Element const& lh, IndexIntoFile::Element const& rh) {
    if (lh.processHistoryIDIndex() == rh.processHistoryIDIndex()) {
      return lh.run() < rh.run();
    }
    return lh.processHistoryIDIndex() < rh.processHistoryIDIndex();
  }

  bool Compare_Index::operator()(IndexIntoFile::Element const& lh, IndexIntoFile::Element const& rh) {
    return lh.processHistoryIDIndex() < rh.processHistoryIDIndex();
  }

  std::ostream&
  operator<<(std::ostream& os, IndexIntoFile const& fileIndex) {

    os << "\nPrinting IndexIntoFile contents.  This includes a list of all Runs, LuminosityBlocks\n"
       << "and Events stored in the root file.\n\n";
    os << std::setw(15) << "Process History"
       << std::setw(15) << "Run"
       << std::setw(15) << "Lumi"
       << std::setw(15) << "Event"
       << std::setw(19) << "TTree Entry"
       << "\n";
    for(std::vector<IndexIntoFile::Element>::const_iterator it = fileIndex.begin(), itEnd = fileIndex.end(); it != itEnd; ++it) {
      if(it->getEntryType() == IndexIntoFile::kEvent) {
        os << std::setw(15) << it->processHistoryIDIndex()
           << std::setw(15) << it->run()
           << std::setw(15) << it ->lumi()
           << std::setw(15) << it->event()
           << std::setw(19) << it->entry()
           << "\n";
      }
      else if(it->getEntryType() == IndexIntoFile::kLumi) {
        os << std::setw(15) << it->processHistoryIDIndex()
           << std::setw(15) << it->run()
           << std::setw(15) << it ->lumi()
           << std::setw(15) << " "
           << std::setw(19) << it->entry() << "  (LuminosityBlock)"
           << "\n";
      }
      else if(it->getEntryType() == IndexIntoFile::kRun) {
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
