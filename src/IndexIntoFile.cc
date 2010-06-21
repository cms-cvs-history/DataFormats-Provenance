#include "DataFormats/Provenance/interface/IndexIntoFile.h"
#include "FWCore/Utilities/interface/Algorithms.h"

#include <algorithm>
#include <ostream>
#include <iomanip>
#include <map>

namespace edm {

  IndexIntoFile::IndexIntoFile() : transients_(),
                                   processHistoryIDs_(),
                                   runOrLumiEntries_() {
  }

  IndexIntoFile::~IndexIntoFile() {
  }

  ProcessHistoryID const& IndexIntoFile::processHistoryID(int i) const {
    return processHistoryIDs_.at(i);
  }

  std::vector<ProcessHistoryID> const& IndexIntoFile::processHistoryIDs() const {
    return processHistoryIDs_;
  }

  void
  IndexIntoFile::addEntry(ProcessHistoryID const& processHistoryID,
                          RunNumber_t run,
                          LuminosityBlockNumber_t lumi,
                          EventNumber_t event,
                          EntryNumber_t entry) {
    int index = 0;
    // First see if the ProcessHistoryID is the same as the previous one.
    // This is just a performance optimization.  We expect to usually get
    // many in a row that are the same.
    if (previousAddedIndex() != invalidIndex &&
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
    previousAddedIndex() = index;

    resultCached() = false;

    assert((currentRun() == run && currentIndex() == index) || currentRun() == invalidRun);
    if (lumi == invalidLumi) {
      assert(currentLumi() == invalidLumi);
      currentIndex() = invalidIndex;
      currentRun() = invalidRun;
      currentLumi() = invalidLumi;
      std::pair<IndexRunKey, EntryNumber_t> firstRunEntry(IndexRunKey(index, run), entry);
      runToFirstEntry().insert(firstRunEntry);
      RunOrLumiEntry runEntry(runToFirstEntry()[IndexRunKey(index, run)], invalidEntry, entry, index, run, lumi, invalidEntry, invalidEntry);
      runOrLumiEntries_.push_back(runEntry);
    }
    else {
      assert(currentLumi() == lumi || currentLumi() == invalidLumi);
      if (currentRun() == invalidRun) {
        currentRun() = run;
        currentIndex() = index;
      }
      if (event == invalidEvent) {
        currentLumi() = invalidLumi;
        std::pair<IndexRunLumiKey, EntryNumber_t> firstLumiEntry(IndexRunLumiKey(index, run, lumi), entry);
        lumiToFirstEntry().insert(firstLumiEntry);
        RunOrLumiEntry lumiEntry(invalidEntry, lumiToFirstEntry()[IndexRunLumiKey(index, run, lumi)],
                                 entry, index, run, lumi, beginEvents(), endEvents());
        runOrLumiEntries_.push_back(lumiEntry);
        beginEvents() = invalidEntry;
        endEvents() = invalidEntry;
      }
      else {
        if (beginEvents() == invalidEntry) {
          currentLumi() = lumi;
          beginEvents() = entry;
          endEvents() = beginEvents() + 1;
        }
        else {
          assert(currentLumi() == lumi);
          assert(entry == endEvents());
          ++endEvents();
        }
      }
    }
  }

  void IndexIntoFile::fillRunOrLumiIndexes() {
    int index = 0;
    for (std::vector<RunOrLumiEntry>::const_iterator iter = runOrLumiEntries_.begin(),
	                                             iEnd = runOrLumiEntries_.end();
         iter != iEnd;
         ++iter, ++index) {
      runOrLumiIndexes().push_back(RunOrLumiIndexes(iter->processHistoryIDIndex(),
                                                    iter->run(),
                                                    iter->lumi(),
                                                    index));
    }
    stable_sort_all(runOrLumiIndexes());

    long long beginEventNumbers = 0;

    std::vector<RunOrLumiIndexes>::iterator beginOfLumi = runOrLumiIndexes().begin();
    std::vector<RunOrLumiIndexes>::iterator endOfLumi = beginOfLumi;
    std::vector<RunOrLumiIndexes>::iterator iEnd = runOrLumiIndexes().end();
    while (true) {
      while (beginOfLumi != iEnd && beginOfLumi->lumi() == invalidLumi) {
        ++beginOfLumi;
      }
      if (beginOfLumi == iEnd) break;

      endOfLumi = beginOfLumi + 1;
      while (endOfLumi != iEnd &&
             beginOfLumi->processHistoryIDIndex() == endOfLumi->processHistoryIDIndex() &&
             beginOfLumi->run() == endOfLumi->run() &&
             beginOfLumi->lumi() == endOfLumi->lumi()) {
        ++endOfLumi;
      }
      int nEvents = 0;
      for (std::vector<RunOrLumiIndexes>::iterator iter = beginOfLumi;
           iter != endOfLumi;
	     ++iter) {
        if (runOrLumiEntries_[iter->indexToGetEntry()].beginEvents() != invalidEntry) {
          nEvents += runOrLumiEntries_[iter->indexToGetEntry()].endEvents() -
	             runOrLumiEntries_[iter->indexToGetEntry()].beginEvents();
        }
      }
      for (std::vector<RunOrLumiIndexes>::iterator iter = beginOfLumi;
           iter != endOfLumi;
	     ++iter) {
        iter->setBeginEventNumbers(beginEventNumbers);
        iter->setEndEventNumbers(beginEventNumbers + nEvents);
      }
      beginEventNumbers += nEvents;
      beginOfLumi = endOfLumi;
    }
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

    for (std::vector<RunOrLumiEntry>::iterator iter = runOrLumiEntries_.begin(),
	                                       iEnd = runOrLumiEntries_.end();
         iter != iEnd;
         ++iter) {
      iter->setProcessHistoryIDIndex(oldToNewIndex[iter->processHistoryIDIndex()]);
    }
  }

  void IndexIntoFile::sortVector_Run_Or_Lumi_Entries() {
    for (std::vector<RunOrLumiEntry>::iterator iter = runOrLumiEntries_.begin(),
                                               iEnd = runOrLumiEntries_.end();
         iter != iEnd;
         ++iter) {
      std::map<IndexRunKey, EntryNumber_t>::const_iterator firstRunEntry = 
        runToFirstEntry().find(IndexRunKey(iter->processHistoryIDIndex(), iter->run()));
      assert(firstRunEntry != runToFirstEntry().end());

      iter->setOrderPHIDRun(firstRunEntry->second);
    }
    stable_sort_all(runOrLumiEntries_);
  }

  void IndexIntoFile::sortEvents() {
    std::vector<RunOrLumiIndexes>::iterator beginOfLumi = runOrLumiIndexes().begin();
    std::vector<RunOrLumiIndexes>::iterator endOfLumi = beginOfLumi;
    std::vector<RunOrLumiIndexes>::iterator iEnd = runOrLumiIndexes().end();
    while (true) {
      while (beginOfLumi != iEnd && beginOfLumi->lumi() == invalidLumi) {
        ++beginOfLumi;
      }
      if (beginOfLumi == iEnd) break;

      endOfLumi = beginOfLumi + 1;
      while (endOfLumi != iEnd &&
             beginOfLumi->processHistoryIDIndex() == endOfLumi->processHistoryIDIndex() &&
             beginOfLumi->run() == endOfLumi->run() &&
             beginOfLumi->lumi() == endOfLumi->lumi()) {
        ++endOfLumi;
      }
      std::sort(eventNumbers().begin() + beginOfLumi->beginEventNumbers(),
                eventNumbers().begin() + beginOfLumi->endEventNumbers());
      beginOfLumi = endOfLumi;
    }
  }

  void IndexIntoFile::sortEventEntries() {
    std::vector<RunOrLumiIndexes>::iterator beginOfLumi = runOrLumiIndexes().begin();
    std::vector<RunOrLumiIndexes>::iterator endOfLumi = beginOfLumi;
    std::vector<RunOrLumiIndexes>::iterator iEnd = runOrLumiIndexes().end();
    while (true) {
      while (beginOfLumi != iEnd && beginOfLumi->lumi() == invalidLumi) {
        ++beginOfLumi;
      }
      if (beginOfLumi == iEnd) break;

      endOfLumi = beginOfLumi + 1;
      while (endOfLumi != iEnd &&
             beginOfLumi->processHistoryIDIndex() == endOfLumi->processHistoryIDIndex() &&
             beginOfLumi->run() == endOfLumi->run() &&
             beginOfLumi->lumi() == endOfLumi->lumi()) {
        ++endOfLumi;
      }
      std::sort(eventEntries().begin() + beginOfLumi->beginEventNumbers(),
                eventEntries().begin() + beginOfLumi->endEventNumbers());
      beginOfLumi = endOfLumi;
    }
  }

  bool IndexIntoFile::allEventsInEntryOrder(bool sortMode) const {
    if(!resultCached()) {
      resultCached() = true;
      EntryNumber_t maxEntry = invalidEntry;
      for(IndexIntoFileItr it = begin(sortMode), itEnd = end(sortMode); it != itEnd; ++it) {
        if(it.getEntryType() == kEvent) {
	  if(it.entry() < maxEntry) {
	    allInEntryOrder() = false;
	    return false;
          }
	  maxEntry = it.entry();
        }
      }
      allInEntryOrder() = true;
      return true;
    }
    return allInEntryOrder();
  }

  IndexIntoFile::IndexIntoFileItr IndexIntoFile::begin(bool sortMode) const {
    if (empty()) {
      return end(sortMode);
    }   
    IndexIntoFileItr iter(this,
                          sortMode,
                          kRun,
                          0,
                          invalidIndex,
                          invalidIndex,
                          0,
                          0);
    iter.initializeRun();
    return iter;
  }

  IndexIntoFile::IndexIntoFileItr IndexIntoFile::end(bool sortMode) const {
    return IndexIntoFileItr(this,
                            sortMode,
                            kEnd,
                            invalidIndex,
                            invalidIndex,
                            invalidIndex,
                            0,
                            0);
  }

  bool IndexIntoFile::empty() const {
    return runOrLumiEntries().empty();
  }

  IndexIntoFile::IndexIntoFileItr
  IndexIntoFile::findPosition(RunNumber_t run, LuminosityBlockNumber_t lumi, EventNumber_t event) const {

    assert(runOrLumiEntries().size() == runOrLumiIndexes().size());

    bool lumiMissing = (lumi == 0 && event != 0);

    std::vector<RunOrLumiIndexes>::const_iterator it;
    std::vector<RunOrLumiIndexes>::const_iterator iEnd = runOrLumiIndexes().end();
    std::vector<RunOrLumiIndexes>::const_iterator phEnd;

    // Loop over ranges of entries with the same ProcessHistoryID
    for (std::vector<RunOrLumiIndexes>::const_iterator phBegin = runOrLumiIndexes().begin();
         phBegin != iEnd;
         phBegin = phEnd) {

      RunOrLumiIndexes el(phBegin->processHistoryIDIndex(), run, lumi, 0);
      phEnd = std::upper_bound(phBegin, iEnd, el, Compare_Index());

      std::vector<RunOrLumiIndexes>::const_iterator iRun = std::lower_bound(phBegin, phEnd, el, Compare_Index_Run());

      if (iRun == phEnd || iRun->run() != run) continue;

      if (lumi == invalidLumi && event == invalidEvent) {
        IndexIntoFileItr indexItr(this,
                                  true,
                                  kRun,
                                  iRun - runOrLumiIndexes().begin(),
                                  invalidIndex,
                                  invalidIndex,
                                  0,
                                  0);
        indexItr.initializeRun();
        return indexItr;
      }

      std::vector<RunOrLumiIndexes>::const_iterator iRunEnd = std::upper_bound(iRun, phEnd, el, Compare_Index_Run());
      if (!lumiMissing) {

        std::vector<RunOrLumiIndexes>::const_iterator iLumi = std::lower_bound(iRun, iRunEnd, el);
        if (iLumi == iRunEnd || iLumi->lumi() != lumi) continue;

        if (event == invalidEvent) {
          IndexIntoFileItr indexItr(this,
                                    true,
                                    kRun,
                                    iRun - runOrLumiIndexes().begin(),
                                    iLumi - runOrLumiIndexes().begin(),
                                    invalidIndex,
                                    0,
                                    0);
          indexItr.initializeLumi();
          return indexItr;
        }

        long long beginEventNumbers = iLumi->beginEventNumbers();
        long long endEventNumbers = iLumi->endEventNumbers();
        if (beginEventNumbers >= endEventNumbers) continue;
        std::vector<EventNumber_t>::const_iterator eventIter = std::lower_bound(eventNumbers().begin() + beginEventNumbers,
                                                                                eventNumbers().begin() + endEventNumbers,
                                                                                event);
        if (eventIter == (eventNumbers().begin() + endEventNumbers) ||
            *eventIter != event) continue;

        return IndexIntoFileItr(this,
                                true,
                                kRun,
                                iRun - runOrLumiIndexes().begin(),
                                iLumi - runOrLumiIndexes().begin(),
                                iLumi - runOrLumiIndexes().begin(),
                                eventIter - eventNumbers().begin() - beginEventNumbers,
                                endEventNumbers - beginEventNumbers);
      }
      if (lumiMissing) {

        std::vector<RunOrLumiIndexes>::const_iterator iLumi = iRun;
        while (iLumi != iRunEnd && iLumi->lumi() == invalidLumi) {
          ++iLumi;
        }
        if (iLumi == iRunEnd) continue;

        std::vector<RunOrLumiIndexes>::const_iterator lumiEnd;
        for ( ;
             iLumi != iRunEnd;
             iLumi = lumiEnd) {

          RunOrLumiIndexes elWithLumi(phBegin->processHistoryIDIndex(), run, iLumi->lumi(), 0);
          lumiEnd = std::upper_bound(iLumi, iRunEnd, elWithLumi);

          long long beginEventNumbers = iLumi->beginEventNumbers();
          long long endEventNumbers = iLumi->endEventNumbers();
          if (beginEventNumbers >= endEventNumbers) continue;
          if (!eventNumbers().empty()) {
            std::vector<EventNumber_t>::const_iterator eventIter = std::lower_bound(eventNumbers().begin() + beginEventNumbers,
                                                                                    eventNumbers().begin() + endEventNumbers,
                                                                                    event);
            if (eventIter == (eventNumbers().begin() + endEventNumbers) ||
                *eventIter != event) continue;

            return IndexIntoFileItr(this,
                                    true,
                                    kRun,
                                    iRun - runOrLumiIndexes().begin(),
                                    iLumi - runOrLumiIndexes().begin(),
                                    iLumi - runOrLumiIndexes().begin(),
                                    eventIter - eventNumbers().begin() - beginEventNumbers,
                                    endEventNumbers - beginEventNumbers);
          }
          else {
            std::vector<EventEntry>::const_iterator eventIter = std::lower_bound(eventEntries().begin() + beginEventNumbers,
                                                                                 eventEntries().begin() + endEventNumbers,
                                                                                 EventEntry(event, invalidEntry));
            if (eventIter == (eventEntries().begin() + endEventNumbers) ||
                eventIter->event() != event) continue;

            return IndexIntoFileItr(this,
                                    true,
                                    kRun,
                                    iRun - runOrLumiIndexes().begin(),
                                    iLumi - runOrLumiIndexes().begin(),
                                    iLumi - runOrLumiIndexes().begin(),
                                    eventIter - eventEntries().begin() - beginEventNumbers,
                                    endEventNumbers - beginEventNumbers);
          }
        }
      }
    } // Loop over ProcessHistoryIDs

    return IndexIntoFileItr(this,
                            true,
                            kEnd,
                            invalidIndex,
                            invalidIndex,
                            invalidIndex,
                            0,
                            0);

  }

  IndexIntoFile::IndexIntoFileItr
  IndexIntoFile::findEventPosition(RunNumber_t run, LuminosityBlockNumber_t lumi, EventNumber_t event) const {
    assert(event != invalidEvent);
    IndexIntoFileItr iter = findPosition(run, lumi, event);
    iter.advanceToEvent();
    return iter;
  }

  IndexIntoFile::IndexIntoFileItr
  IndexIntoFile::findLumiPosition(RunNumber_t run, LuminosityBlockNumber_t lumi) const {
    assert(lumi != invalidLumi);
    IndexIntoFileItr iter = findPosition(run, lumi, 0U);
    iter.advanceToLumi();
    return iter;
  }

  IndexIntoFile::IndexIntoFileItr
  IndexIntoFile::findRunPosition(RunNumber_t run) const {
    return findPosition(run, 0U, 0U);
  }

  bool
  IndexIntoFile::containsItem(RunNumber_t run, LuminosityBlockNumber_t lumi, EventNumber_t event) const {
	return event ? containsEvent(run, lumi, event) : (lumi ? containsLumi(run, lumi) : containsRun(run));
  }

  bool
  IndexIntoFile::containsEvent(RunNumber_t run, LuminosityBlockNumber_t lumi, EventNumber_t event) const {
	return findEventPosition(run, lumi, event).getEntryType() != kEnd;
  }

  bool
  IndexIntoFile::containsLumi(RunNumber_t run, LuminosityBlockNumber_t lumi) const {
    return findLumiPosition(run, lumi).getEntryType() != kEnd;
  }

  bool
  IndexIntoFile::containsRun(RunNumber_t run) const {
    return findRunPosition(run).getEntryType() != kEnd;
  }

  IndexIntoFile::SortedRunOrLumiItr IndexIntoFile::beginRunOrLumi() {
    return SortedRunOrLumiItr(this, 0);
  }

  IndexIntoFile::SortedRunOrLumiItr IndexIntoFile::endRunOrLumi() {
    return SortedRunOrLumiItr(this, runOrLumiIndexes().size());
  }

  IndexIntoFile::RunOrLumiEntry::RunOrLumiEntry() :
    orderPHIDRun_(invalidEntry),
    orderPHIDRunLumi_(invalidEntry),
    entry_(invalidEntry),
    processHistoryIDIndex_(invalidIndex),
    run_(invalidRun),
    lumi_(invalidLumi),
    beginEvents_(invalidEntry),
    endEvents_(invalidEntry) {
  }

  IndexIntoFile::RunOrLumiEntry::RunOrLumiEntry(EntryNumber_t orderPHIDRun,
                                                EntryNumber_t orderPHIDRunLumi,
                                                EntryNumber_t entry,
                                                int processHistoryIDIndex,
                                                RunNumber_t run,
                                                LuminosityBlockNumber_t lumi,
                                                EntryNumber_t beginEvents,
                                                EntryNumber_t endEvents) :
    orderPHIDRun_(orderPHIDRun),
    orderPHIDRunLumi_(orderPHIDRunLumi),
    entry_(entry),
    processHistoryIDIndex_(processHistoryIDIndex),
    run_(run),
    lumi_(lumi),
    beginEvents_(beginEvents),
    endEvents_(endEvents) {
  }

  IndexIntoFile::RunOrLumiIndexes::RunOrLumiIndexes(int processHistoryIDIndex,
                                                    RunNumber_t run,
                                                    LuminosityBlockNumber_t lumi,
                                                    int indexToGetEntry) :
    processHistoryIDIndex_(processHistoryIDIndex),
    run_(run),
    lumi_(lumi),
    indexToGetEntry_(indexToGetEntry),
    beginEventNumbers_(-1),
    endEventNumbers_(-1)
  {
  }

  IndexIntoFile::SortedRunOrLumiItr::SortedRunOrLumiItr(IndexIntoFile const* indexIntoFile, unsigned runOrLumi) :
    indexIntoFile_(indexIntoFile), runOrLumi_(runOrLumi) {
    assert(runOrLumi_ <= indexIntoFile_->runOrLumiIndexes().size());
  }

  bool IndexIntoFile::SortedRunOrLumiItr::operator==(SortedRunOrLumiItr const& right) const {
    return indexIntoFile_ == right.indexIntoFile() &&
           runOrLumi_ == right.runOrLumi();
  }

  bool IndexIntoFile::SortedRunOrLumiItr::operator!=(SortedRunOrLumiItr const& right) const {
    return indexIntoFile_ != right.indexIntoFile() ||
           runOrLumi_ != right.runOrLumi();
  }

  IndexIntoFile::SortedRunOrLumiItr & IndexIntoFile::SortedRunOrLumiItr::operator++() {
    if (runOrLumi_ != indexIntoFile_->runOrLumiIndexes().size()) {
      ++runOrLumi_;
    }
    return *this;
  }

  bool IndexIntoFile::SortedRunOrLumiItr::isRun() {
    return indexIntoFile_->runOrLumiIndexes().at(runOrLumi_).lumi() == 0;
  }

  void IndexIntoFile::SortedRunOrLumiItr::getRange(long long & beginEventNumbers,
                long long & endEventNumbers,
                EntryNumber_t & beginEventEntry,
                EntryNumber_t & endEventEntry) {
    beginEventNumbers = indexIntoFile_->runOrLumiIndexes().at(runOrLumi_).beginEventNumbers();
    endEventNumbers = indexIntoFile_->runOrLumiIndexes().at(runOrLumi_).endEventNumbers();

    int indexToGetEntry = indexIntoFile_->runOrLumiIndexes().at(runOrLumi_).indexToGetEntry();
    beginEventEntry = indexIntoFile_->runOrLumiEntries_.at(indexToGetEntry).beginEvents();
    endEventEntry = indexIntoFile_->runOrLumiEntries_.at(indexToGetEntry).endEvents();
  }

  IndexIntoFile::IndexIntoFileItrImpl::IndexIntoFileItrImpl(IndexIntoFile const* indexIntoFile) :
    indexIntoFile_(indexIntoFile),
    size_(static_cast<int>(indexIntoFile_->runOrLumiEntries_.size())),
    type_(kEnd),
    indexToRun_(invalidIndex),
    indexToLumi_(invalidIndex),
    indexToEventRange_(invalidIndex),
    indexToEvent_(0),
    nEvents_(0) {

    if (size_ == 0) {
      return;
    }
    type_ = kRun;
    assert(indexIntoFile_->runOrLumiEntries_[0].isRun());
    initializeRun();
  }

  IndexIntoFile::IndexIntoFileItrImpl::IndexIntoFileItrImpl(IndexIntoFile const* indexIntoFile,
                       EntryType entryType,
                       int indexToRun,
                       int indexToLumi,
                       int indexToEventRange,
                       long long indexToEvent,
                       long long nEvents) :
    indexIntoFile_(indexIntoFile),
    size_(static_cast<int>(indexIntoFile_->runOrLumiEntries_.size())),
    type_(entryType),
    indexToRun_(indexToRun),
    indexToLumi_(indexToLumi),
    indexToEventRange_(indexToEventRange),
    indexToEvent_(indexToEvent),
    nEvents_(nEvents) {
  }

  void IndexIntoFile::IndexIntoFileItrImpl::next () {

    if (type_ == kEvent) {
      if ((indexToEvent_ + 1)  < nEvents_) {
        ++indexToEvent_;
      }
      else {
        bool found = nextEventRange();

        if (!found) {
          type_ = getRunOrLumiEntryType(indexToLumi_ + 1);

          if (type_ == kLumi) {
            ++indexToLumi_;
            initializeLumi();
          }
          else if (type_ == kRun) {
            indexToRun_ = indexToLumi_ + 1;
            initializeRun();
          }
          else {
            setInvalid(); // type_ is kEnd
          }
        }
      }
    }
    else if (type_ == kLumi) {

      bool hasEvents = lumiHasEvents();

      if (indexToLumi_ + 1 == size_) {
        if (hasEvents) {
          type_ = kEvent;
        }
        else {
          type_ = kEnd;
          setInvalid();
        }
      }
      else {

        EntryType nextType = getRunOrLumiEntryType(indexToLumi_ + 1);
        bool sameLumi = isSameLumi(indexToLumi_, indexToLumi_ + 1);

        if (sameLumi && nextType != kRun) {
          ++indexToLumi_;
        }
        else if (hasEvents) {
          type_ = kEvent;
        }
        else if (nextType == kRun) {
          type_ = kRun;
          indexToRun_ = indexToLumi_ + 1;
          initializeRun();
        }
        else {
          ++indexToLumi_;
          initializeLumi();
        }
      }
	  }
    else if (type_ == kRun) {
      EntryType nextType = getRunOrLumiEntryType(indexToRun_ + 1);
      bool sameRun = isSameRun(indexToRun_, indexToRun_ + 1);
      if (nextType == kRun && sameRun) {
        ++indexToRun_;
      }
      else if (nextType == kRun && !sameRun) {
        ++indexToRun_;
        initializeRun();
      }
      else {
        type_ = kLumi;
      }
    }
  }

  void IndexIntoFile::IndexIntoFileItrImpl::skipEventForward(RunNumber_t & runOfSkippedEvent,
                                                             LuminosityBlockNumber_t & lumiOfSkippedEvent,
                                                             EntryNumber_t & skippedEventEntry) {
    if (indexToEvent_  < nEvents_) {

      runOfSkippedEvent = run();
      lumiOfSkippedEvent = peekAheadAtLumi();
      skippedEventEntry = peekAheadAtEventEntry();

      if ((indexToEvent_ + 1)  < nEvents_) {
        ++indexToEvent_;
        return;
      }

      if (nextEventRange()) {
        return;
      }
      else if (type_ == kRun) {
        if (skipLumiInRun()) {
          return;
        }
      }
    }
    if (type_ == kRun) {
      while (skipLumiInRun()) {
        if (indexToEvent_  < nEvents_) {
          skipEventForward(runOfSkippedEvent, lumiOfSkippedEvent, skippedEventEntry);
          return;
        }
      }
    }
    while (type_ != kEvent && type_ != kEnd) {
      next();
    }
    if (type_ == kEnd) {
      runOfSkippedEvent = invalidRun;
      lumiOfSkippedEvent = invalidLumi;
      skippedEventEntry = invalidEntry;
      return;
    }
    runOfSkippedEvent = run();
    lumiOfSkippedEvent = lumi();
    skippedEventEntry = entry();
    next();
    return;
  }

  void IndexIntoFile::IndexIntoFileItrImpl::advanceToNextRun() {
    if (type_ == kEnd) return;
    for (int i = 1; indexToRun_ + i < size_; ++i) {
      if (getRunOrLumiEntryType(indexToRun_ + i) == kRun) {
	      if (!isSameRun(indexToRun_, indexToRun_ + i)) {
          type_ = kRun;
          indexToRun_ += i;
          initializeRun();
          return;
        }
      }
    }
    type_ = kEnd;
    setInvalid();
  }

  void IndexIntoFile::IndexIntoFileItrImpl::advanceToNextLumiOrRun() {
    if (type_ == kEnd) return;
    int startSearch = indexToLumi_;
    if (startSearch == invalidIndex) startSearch = indexToRun_;
    for (int i = 1; startSearch + i < size_; ++i) {
      if (getRunOrLumiEntryType(startSearch + i) == kRun) {
        if (!isSameRun(indexToRun_, startSearch + i)) {
          type_ = kRun;
          indexToRun_ = startSearch + i;
          initializeRun();
          return;
        }
      }
      if (indexToLumi_ != invalidIndex && isSameLumi(indexToLumi_, startSearch + i)) {
        continue;
      }
      type_ = kLumi;
      indexToLumi_ = startSearch + i;
      initializeLumi();
    }
    type_ = kEnd;
    setInvalid();
   }

  void IndexIntoFile::IndexIntoFileItrImpl::initializeRun() {

    indexToLumi_ = invalidIndex;
    indexToEventRange_ = invalidIndex;
    indexToEvent_ = 0;
    nEvents_ = 0;

    for (int i = 1; (i + indexToRun_) < size_; ++i) {
      EntryType entryType = getRunOrLumiEntryType(indexToRun_ + i);
      bool sameRun = isSameRun(indexToRun_, indexToRun_ + i);

      if (entryType == kRun) {
        if (sameRun) {
          continue;
        }
        else {
          break;
        }
      }
      else {
        indexToLumi_ = indexToRun_ + i;
        initializeLumi();
        return;
      }
    }
  }

  bool IndexIntoFile::IndexIntoFileItrImpl::operator==(IndexIntoFileItrImpl const& right) const {
    return (indexIntoFile_ == right.indexIntoFile_ &&
            size_ == right.size_ &&
            type_ == right.type_ &&
            indexToRun_ == right.indexToRun_ &&
            indexToLumi_ == right.indexToLumi_ &&
            indexToEventRange_ == right.indexToEventRange_ &&
            indexToEvent_ == right.indexToEvent_ &&
            nEvents_ == right.nEvents_);
  }

  void IndexIntoFile::IndexIntoFileItrImpl::setInvalid() {
    indexToRun_ = invalidIndex;
    indexToLumi_ = invalidIndex;
    indexToEventRange_ = invalidIndex;
    indexToEvent_ = 0;
    nEvents_ = 0;
  }

  IndexIntoFile::IndexIntoFileItrNoSort::IndexIntoFileItrNoSort(IndexIntoFile const* indexIntoFile) :
    IndexIntoFileItrImpl(indexIntoFile)
  {
  }

  IndexIntoFile::IndexIntoFileItrNoSort::IndexIntoFileItrNoSort(IndexIntoFile const* indexIntoFile,
                         EntryType entryType,
                         int indexToRun,
                         int indexToLumi,
                         int indexToEventRange,
                         long long indexToEvent,
                         long long nEvents) :
    IndexIntoFileItrImpl(indexIntoFile,
                         entryType,
                         indexToRun,
                         indexToLumi,
                         indexToEventRange,
                         indexToEvent,
                         nEvents)
  {
  }

  IndexIntoFile::IndexIntoFileItrImpl*
  IndexIntoFile::IndexIntoFileItrNoSort::clone() const {
    return new IndexIntoFileItrNoSort(*this);
  }

  int
  IndexIntoFile::IndexIntoFileItrNoSort::processHistoryIDIndex() const {
    if (type_ == kEnd) return invalidIndex;
    return indexIntoFile_->runOrLumiEntries()[indexToRun_].processHistoryIDIndex();
  }

  RunNumber_t IndexIntoFile::IndexIntoFileItrNoSort::run() const {
    if (type_ == kEnd) return invalidRun;
    return indexIntoFile_->runOrLumiEntries()[indexToRun_].run();
  }

  LuminosityBlockNumber_t IndexIntoFile::IndexIntoFileItrNoSort::lumi() const {
    if (type_ == kEnd || type_ == kRun) return invalidLumi;
    return indexIntoFile_->runOrLumiEntries()[indexToLumi_].lumi();
  }

  IndexIntoFile::EntryNumber_t IndexIntoFile::IndexIntoFileItrNoSort::entry() const {
    if (type_ == kEnd) return invalidEntry;
    if (type_ == kRun) return indexIntoFile_->runOrLumiEntries_[indexToRun_].entry();
    if (type_ == kLumi) return indexIntoFile_->runOrLumiEntries_[indexToLumi_].entry();
    return
      indexIntoFile_->runOrLumiEntries()[indexToEventRange_].beginEvents() +
      indexToEvent_;
  }

  LuminosityBlockNumber_t IndexIntoFile::IndexIntoFileItrNoSort::peekAheadAtLumi() const {
    if (indexToLumi_ == invalidIndex) return invalidLumi;
    return indexIntoFile_->runOrLumiEntries()[indexToLumi_].lumi();
  }

  IndexIntoFile::EntryNumber_t IndexIntoFile::IndexIntoFileItrNoSort::peekAheadAtEventEntry() const {
    if (indexToLumi_ == invalidIndex) return invalidEntry;
    if (indexToEvent_ >= nEvents_) return invalidEntry;
    return
      indexIntoFile_->runOrLumiEntries()[indexToEventRange_].beginEvents() +
      indexToEvent_;
  }

  void IndexIntoFile::IndexIntoFileItrNoSort::initializeLumi_() {

    indexToEventRange_ = invalidIndex;
    indexToEvent_ = 0;
    nEvents_ = 0;

    for (int i = 0; indexToLumi_ + i < size_; ++i) {
      if (indexIntoFile_->runOrLumiEntries_[indexToLumi_ + i].isRun()) {
        break;
      }
      else if (indexIntoFile_->runOrLumiEntries_[indexToLumi_ + i].lumi() ==
               indexIntoFile_->runOrLumiEntries_[indexToLumi_].lumi()) {
        if (indexIntoFile_->runOrLumiEntries_[indexToLumi_ + i].beginEvents() == invalidEntry) {
          continue;
        }
        indexToEventRange_ = indexToLumi_ + i;
        indexToEvent_ = 0;
        nEvents_ = indexIntoFile_->runOrLumiEntries_[indexToEventRange_].endEvents() -
                   indexIntoFile_->runOrLumiEntries_[indexToEventRange_].beginEvents();
        break;
      }
      else {
        break;
      }
    }
  }

  bool IndexIntoFile::IndexIntoFileItrNoSort::nextEventRange() {
    // Look for the next event range, same lumi but different entry 
    for(int i = 1; indexToEventRange_ + i <= size_; ++i) {
      if (indexIntoFile_->runOrLumiEntries_[indexToEventRange_ + i ].isRun()) {
        return false;  // hit next run
      }
      else if (indexIntoFile_->runOrLumiEntries_[indexToEventRange_ + i].lumi() ==
               indexIntoFile_->runOrLumiEntries_[indexToEventRange_].lumi()) {
        if (indexIntoFile_->runOrLumiEntries_[indexToEventRange_ + i].beginEvents() == invalidEntry) {
          continue; // same lumi but has no events, keep looking
        }
        indexToEventRange_ += i;
        indexToEvent_ = 0;
        nEvents_ = indexIntoFile_->runOrLumiEntries_[indexToEventRange_].endEvents() -
                   indexIntoFile_->runOrLumiEntries_[indexToEventRange_].beginEvents();
        return true; // found more events in this lumi
      }
      return false; // hit next lumi
    }
    return false; // hit the end of the IndexIntoFile
  }

  bool IndexIntoFile::IndexIntoFileItrNoSort::skipLumiInRun() { 
    for(int i = 1; indexToEventRange_ + i <= size_; ++i) {
      if (indexIntoFile_->runOrLumiEntries_[indexToEventRange_ + i ].isRun()) {
        return false;  // hit next run
      }
      else if (indexIntoFile_->runOrLumiEntries_[indexToEventRange_ + i].lumi() ==
               indexIntoFile_->runOrLumiEntries_[indexToEventRange_].lumi()) {
        continue;
      }
      indexToLumi_ = indexToEventRange_ + i;
      initializeLumi();
      return true; // hit next lumi
    }
    return false; // hit the end of the IndexIntoFile
  }

  IndexIntoFile::EntryType IndexIntoFile::IndexIntoFileItrNoSort::getRunOrLumiEntryType(int index) const {
    if (index < 0 || index >= size_) {
      return kEnd;
    }
    else if (indexIntoFile_->runOrLumiEntries_[index].isRun()) {
      return kRun;
    }
    return kLumi;
  }

  bool IndexIntoFile::IndexIntoFileItrNoSort::lumiHasEvents() const {
    return indexIntoFile_->runOrLumiEntries()[indexToLumi_].beginEvents() != invalidEntry;
  }

  bool IndexIntoFile::IndexIntoFileItrNoSort::isSameLumi(int index1, int index2) const {
    if (index1 < 0 || index1 >= size_ || index2 < 0 || index2 >= size_) {
      return false;
    }
    return indexIntoFile_->runOrLumiEntries()[index1].lumi() ==
           indexIntoFile_->runOrLumiEntries()[index2].lumi();
  }

  bool IndexIntoFile::IndexIntoFileItrNoSort::isSameRun(int index1, int index2) const {
    if (index1 < 0 || index1 >= size_ || index2 < 0 || index2 >= size_) {
      return false;
    }
    return indexIntoFile_->runOrLumiEntries()[index1].run() ==
           indexIntoFile_->runOrLumiEntries()[index2].run() &&
           indexIntoFile_->runOrLumiEntries()[index1].processHistoryIDIndex() ==
           indexIntoFile_->runOrLumiEntries()[index2].processHistoryIDIndex();
  }

  IndexIntoFile::IndexIntoFileItrSorted::IndexIntoFileItrSorted(IndexIntoFile const* indexIntoFile) :
    IndexIntoFileItrImpl(indexIntoFile) {
  }

  IndexIntoFile::IndexIntoFileItrSorted::IndexIntoFileItrSorted(IndexIntoFile const* indexIntoFile,
                         EntryType entryType,
                         int indexToRun,
                         int indexToLumi,
                         int indexToEventRange,
                         long long indexToEvent,
                         long long nEvents) :
    IndexIntoFileItrImpl(indexIntoFile,
                         entryType,
                         indexToRun,
                         indexToLumi,
                         indexToEventRange,
                         indexToEvent,
                         nEvents) {
  }

  IndexIntoFile::IndexIntoFileItrImpl* IndexIntoFile::IndexIntoFileItrSorted::clone() const {
    return new IndexIntoFileItrSorted(*this);
  }

  int IndexIntoFile::IndexIntoFileItrSorted::processHistoryIDIndex() const {
    if (type_ == kEnd) return invalidIndex;
    return indexIntoFile_->runOrLumiIndexes()[indexToRun_].processHistoryIDIndex();
  }

  RunNumber_t IndexIntoFile::IndexIntoFileItrSorted::run() const {
    if (type_ == kEnd) return invalidRun;
    return indexIntoFile_->runOrLumiIndexes()[indexToRun_].run();
  }

  LuminosityBlockNumber_t IndexIntoFile::IndexIntoFileItrSorted::lumi() const {
    if (type_ == kEnd || type_ == kRun) return invalidLumi;
    return indexIntoFile_->runOrLumiIndexes()[indexToLumi_].lumi();
  }

  IndexIntoFile::EntryNumber_t IndexIntoFile::IndexIntoFileItrSorted::entry() const {
    if (type_ == kEnd) return invalidEntry;
    if (type_ == kRun) {
      int i =  indexIntoFile_->runOrLumiIndexes()[indexToRun_].indexToGetEntry();
      return indexIntoFile_->runOrLumiEntries()[i].entry();
    }
    if (type_ == kLumi) {
      int i =  indexIntoFile_->runOrLumiIndexes()[indexToLumi_].indexToGetEntry();
      return indexIntoFile_->runOrLumiEntries()[i].entry();
    }
    long long eventNumberIndex = 
      indexIntoFile_->runOrLumiIndexes()[indexToEventRange_].beginEventNumbers() +
      indexToEvent_;
    return indexIntoFile_->eventEntries().at(eventNumberIndex).entry();
  }

  LuminosityBlockNumber_t IndexIntoFile::IndexIntoFileItrSorted::peekAheadAtLumi() const {
    if (indexToLumi_ == invalidIndex) return invalidLumi;
    return indexIntoFile_->runOrLumiIndexes()[indexToLumi_].lumi();
  }

  IndexIntoFile::EntryNumber_t IndexIntoFile::IndexIntoFileItrSorted::peekAheadAtEventEntry() const {
    if (indexToLumi_ == invalidIndex) return invalidEntry;
    if (indexToEvent_ >= nEvents_) return invalidEntry;
    long long eventNumberIndex = 
      indexIntoFile_->runOrLumiIndexes()[indexToEventRange_].beginEventNumbers() +
      indexToEvent_;
    return indexIntoFile_->eventEntries().at(eventNumberIndex).entry();
  }

  void IndexIntoFile::IndexIntoFileItrSorted::initializeLumi_() {
    indexToEventRange_ = indexToLumi_;
    indexToEvent_ = 0;
    nEvents_ = 
      indexIntoFile_->runOrLumiIndexes()[indexToLumi_].endEventNumbers() -
      indexIntoFile_->runOrLumiIndexes()[indexToLumi_].beginEventNumbers();
    if (nEvents_ == 0){
      indexToEventRange_ = invalidIndex;
    }
  }

  bool IndexIntoFile::IndexIntoFileItrSorted::nextEventRange() {
    return false;          
  }

  bool IndexIntoFile::IndexIntoFileItrSorted::skipLumiInRun() { 
    for(int i = 1; indexToEventRange_ + i <= size_; ++i) {
      if (indexIntoFile_->runOrLumiIndexes()[indexToEventRange_ + i ].isRun()) {
        return false;  // hit next run
      }
      else if (indexIntoFile_->runOrLumiIndexes()[indexToEventRange_ + i].lumi() ==
               indexIntoFile_->runOrLumiIndexes()[indexToEventRange_].lumi()) {
        continue;
      }
      indexToLumi_ = indexToEventRange_ + i;
      initializeLumi();
      return true; // hit next lumi
    }
    return false; // hit the end of the IndexIntoFile
  }

  IndexIntoFile::EntryType IndexIntoFile::IndexIntoFileItrSorted::getRunOrLumiEntryType(int index) const {
    if (index < 0 || index >= size_) {
      return kEnd;
    }
    else if (indexIntoFile_->runOrLumiIndexes()[index].isRun()) {
      return kRun;
    }
    return kLumi;
  }

  bool IndexIntoFile::IndexIntoFileItrSorted::lumiHasEvents() const {
    return indexIntoFile_->runOrLumiIndexes()[indexToLumi_].beginEventNumbers() !=
           indexIntoFile_->runOrLumiIndexes()[indexToLumi_].endEventNumbers();
  }

  bool IndexIntoFile::IndexIntoFileItrSorted::isSameLumi(int index1, int index2) const {
    if (index1 < 0 || index1 >= size_ || index2 < 0 || index2 >= size_) {
      return false;
    }
    return indexIntoFile_->runOrLumiIndexes()[index1].lumi() ==
           indexIntoFile_->runOrLumiIndexes()[index2].lumi();
  }

  bool IndexIntoFile::IndexIntoFileItrSorted::isSameRun(int index1, int index2) const {
    if (index1 < 0 || index1 >= size_ || index2 < 0 || index2 >= size_) {
      return false;
    }
    return indexIntoFile_->runOrLumiIndexes()[index1].run() ==
           indexIntoFile_->runOrLumiIndexes()[index2].run() &&
           indexIntoFile_->runOrLumiIndexes()[index1].processHistoryIDIndex() ==
           indexIntoFile_->runOrLumiIndexes()[index2].processHistoryIDIndex();
  }

  IndexIntoFile::IndexIntoFileItr::IndexIntoFileItr(IndexIntoFile const* indexIntoFile, bool sortMode) :
    impl_() {
    if (sortMode) {
      value_ptr<IndexIntoFileItrImpl> temp(new IndexIntoFileItrSorted(indexIntoFile));
      swap(temp, impl_);
    }
    else {
      value_ptr<IndexIntoFileItrImpl> temp(new IndexIntoFileItrNoSort(indexIntoFile));
      swap(temp, impl_);
    }
  }

  IndexIntoFile::IndexIntoFileItr::IndexIntoFileItr(IndexIntoFile const* indexIntoFile,
                   bool sortMode,
                   EntryType entryType,
                   int indexToRun,
                   int indexToLumi,
                   int indexToEventRange,
                   long long indexToEvent,
                   long long nEvents) :
    impl_() {
    if (sortMode) {
      value_ptr<IndexIntoFileItrImpl> temp(new IndexIntoFileItrSorted(indexIntoFile,
                                                                      entryType,
                                                                      indexToRun,
                                                                      indexToLumi,
                                                                      indexToEventRange,
                                                                      indexToEvent,
                                                                      nEvents
                                                                     ));
      swap(temp, impl_);
    }
    else {
      value_ptr<IndexIntoFileItrImpl> temp(new IndexIntoFileItrNoSort(indexIntoFile,
                                                                      entryType,
                                                                      indexToRun,
                                                                      indexToLumi,
                                                                      indexToEventRange,
                                                                      indexToEvent,
                                                                      nEvents));
      swap(temp, impl_);
    }
  }

  void IndexIntoFile::IndexIntoFileItr::advanceToEvent() {
    for (EntryType entryType = getEntryType();
         entryType != kEnd && entryType != kEvent;
         entryType = getEntryType()) {
	    impl_->next();
    }
  }

  void IndexIntoFile::IndexIntoFileItr::advanceToLumi() {
    for (EntryType entryType = getEntryType();
         entryType != kEnd && entryType != kLumi;
         entryType = getEntryType()) {
	    impl_->next();
    }
  }

  IndexIntoFile::Transients::Transients() : allInEntryOrder_(false),
                                            resultCached_(false),
                                            previousAddedIndex_(invalidIndex),
                                            runToFirstEntry_(),
                                            lumiToFirstEntry_(),
                                            beginEvents_(invalidEntry),
                                            endEvents_(invalidEntry),
                                            currentIndex_(invalidIndex),
                                            currentRun_(invalidRun),
                                            currentLumi_(invalidLumi) {
  }

  bool Compare_Index_Run::operator()(IndexIntoFile::RunOrLumiIndexes const& lh, IndexIntoFile::RunOrLumiIndexes const& rh) {
    if (lh.processHistoryIDIndex() == rh.processHistoryIDIndex()) {
      return lh.run() < rh.run();
    }
    return lh.processHistoryIDIndex() < rh.processHistoryIDIndex();
  }

  bool Compare_Index::operator()(IndexIntoFile::RunOrLumiIndexes const& lh, IndexIntoFile::RunOrLumiIndexes const& rh) {
    return lh.processHistoryIDIndex() < rh.processHistoryIDIndex();
  }

  std::ostream&
  operator<<(std::ostream& os, IndexIntoFile const& fileIndex) {
    // Need to fix this, used by edmFileUtil
    /*
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
    */
    return os;
  }
}
