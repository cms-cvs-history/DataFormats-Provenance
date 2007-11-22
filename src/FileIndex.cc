#include "DataFormats/Provenance/interface/FileIndex.h"
#include "FWCore/Utilities/interface/Algorithms.h"

#include <algorithm>

namespace edm {
  FileIndex::FileIndex() : entries_(), eventsSorted_(false), sortedCached_(false){}

  void
  FileIndex::addEntry(RunNumber_t run, LuminosityBlockNumber_t lumi, EventNumber_t event, EntryNumber_t entry) {
    entries_.push_back(FileIndex::Element(run, lumi, event, entry));
  }

  void FileIndex::sort() {
    stable_sort_all(entries_);
  }

  bool FileIndex::eventsSorted() const {
    if (!sortedCached_) {
      sortedCached_ = true;
      EntryNumber_t maxEntry = -1LL;
      for (std::vector<FileIndex::Element>::const_iterator it = entries_.begin(), itEnd = entries_.end(); it != itEnd; ++it) {
        if (it->getEntryType() == kEvent) {
	  if (it->entry_ < maxEntry) {
	    eventsSorted_ = false;
	    return eventsSorted_;
          }
	  maxEntry = it->entry_;
        }
      }
      eventsSorted_ = true;
    }
    return eventsSorted_;
  }

  FileIndex::const_iterator
  FileIndex::findPosition(RunNumber_t run, LuminosityBlockNumber_t lumi, EventNumber_t event) const {
    Element el(run, lumi, event);
    const_iterator it = std::lower_bound(entries_.begin(), entries_.end(), el);
    bool lumiMissing = (lumi == 0 && event != 0);
    if (lumiMissing) {
      const_iterator itEnd = entries_.end();
      while (it->event_ < event && it->run_ <= run && it != itEnd) ++it;
    }
    return it;
  }

  bool operator<(FileIndex::Element const& lh, FileIndex::Element const& rh) {
    if(lh.run_ == rh.run_) {
      if(lh.lumi_ == rh.lumi_) {
	return lh.event_ < rh.event_;
      }
      return lh.lumi_ < rh.lumi_;
    }
    return lh.run_ < rh.run_;
  }
}
