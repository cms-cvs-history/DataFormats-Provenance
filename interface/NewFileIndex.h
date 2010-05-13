#ifndef DataFormats_Provenance_NewFileIndex_h
#define DataFormats_Provenance_NewFileIndex_h

/*----------------------------------------------------------------------

NewFileIndex.h 

----------------------------------------------------------------------*/

#include "DataFormats/Provenance/interface/ProcessHistoryID.h"
#include "DataFormats/Provenance/interface/RunID.h"
#include "DataFormats/Provenance/interface/LuminosityBlockID.h"
#include "DataFormats/Provenance/interface/EventID.h"
#include "DataFormats/Provenance/interface/Transient.h"

#include <vector>
#include <cassert>
#include <iosfwd>

namespace edm {

  class NewFileIndex {

    public:
      typedef long long EntryNumber_t;

      NewFileIndex();
      ~NewFileIndex() {}

      void addEntry(ProcessHistoryID processHistoryID,
                    RunNumber_t run,
                    LuminosityBlockNumber_t lumi,
                    EventNumber_t event,
                    EntryNumber_t entry);

      enum EntryType {kRun, kLumi, kEvent, kEnd};

      class Element {
        public:
          static int const invalidIndex = -1;
	  static EntryNumber_t const invalidEntry = -1LL;

          Element() : processHistoryIDIndex_(invalidIndex), run_(0U),
                      lumi_(0U), event_(0U), entry_(invalidEntry) {
          }
          Element(int index, RunNumber_t run, LuminosityBlockNumber_t lumi, EventNumber_t event, long long entry) :
            processHistoryIDIndex_(index),
            run_(run), lumi_(lumi), 
            event_(event), entry_(entry) {
            assert(lumi_ != 0U || event_ == 0U);
	  }
          Element(int index, RunNumber_t run, LuminosityBlockNumber_t lumi, EventNumber_t event) :
            processHistoryIDIndex_(index),
            run_(run), lumi_(lumi), event_(event), entry_(invalidEntry) {}
          EntryType getEntryType() const {
            return lumi_ == 0U ? kRun : (event_ == 0U ? kLumi : kEvent); 
          }

          int processHistoryIDIndex() const { return processHistoryIDIndex_; }
          RunNumber_t run() const { return run_; }
          LuminosityBlockNumber_t lumi() const { return lumi_; }
          EventNumber_t event() const { return event_; }
          EntryNumber_t entry() const { return entry_; }

          void setProcessHistoryIDIndex(int v) { processHistoryIDIndex_ = v; }
          void setRun(RunNumber_t v) { run_ = v; }
          void setLumi(LuminosityBlockNumber_t v) { lumi_ = v; }
          void setEvent(EventNumber_t v) { event_ = v; }
          void setEntry(EntryNumber_t v) { entry_ = v; }

      private:

          int processHistoryIDIndex_;
          RunNumber_t run_;
          LuminosityBlockNumber_t lumi_;
          EventNumber_t event_;
          EntryNumber_t entry_;
      };

      typedef std::vector<Element>::const_iterator const_iterator;
      typedef std::vector<Element>::iterator iterator;

      void sortBy_Index_Run_Lumi_Event();
      void sortBy_Index_Run_Lumi_Entry();

      const_iterator
      findPosition(RunNumber_t run, LuminosityBlockNumber_t lumi = 0U, EventNumber_t event = 0U) const;

      const_iterator
      findEventPosition(RunNumber_t run, LuminosityBlockNumber_t lumi, EventNumber_t event) const;

      const_iterator
      findEventEntryPosition(RunNumber_t run, LuminosityBlockNumber_t lumi, EventNumber_t event, EntryNumber_t entry) const;

      const_iterator
      findLumiPosition(RunNumber_t run, LuminosityBlockNumber_t lumi) const;

      const_iterator
      findRunPosition(RunNumber_t run) const;

      const_iterator
      findNextRun(const_iterator const& iter) const;

      const_iterator
      findNextLumiOrRun(const_iterator const& iter) const;

      bool
      containsItem(RunNumber_t run, LuminosityBlockNumber_t lumi, EventNumber_t event) const {
	return event ? containsEvent(run, lumi, event) : (lumi ? containsLumi(run, lumi) : containsRun(run));
      }

      bool
      containsEvent(RunNumber_t run, LuminosityBlockNumber_t lumi, EventNumber_t event) const {
	return findEventPosition(run, lumi, event) != entries_.end();
      }

      bool
      containsLumi(RunNumber_t run, LuminosityBlockNumber_t lumi) const {
        return findLumiPosition(run, lumi) != entries_.end();
      }

      bool
      containsRun(RunNumber_t run) const {
        return findRunPosition(run) != entries_.end();
      }

      const_iterator begin() const {return entries_.begin();}

      const_iterator end() const {return entries_.end();}

      iterator begin() {return entries_.begin();}

      iterator end() {return entries_.end();}

      iterator erase(iterator pos) {return entries_.erase(pos);}

      iterator erase(iterator begin, iterator end) {return entries_.erase(begin, end);}

      std::vector<Element>::size_type size() const {return entries_.size();}

      bool empty() const {return entries_.empty();}

      bool allEventsInEntryOrder() const;

      std::vector<ProcessHistoryID> const& processHistoryIDs() const { return processHistoryIDs_; }

      void fixIndexes(std::vector<ProcessHistoryID> & processHistoryIDs);

      enum SortState {kNotSorted, kSorted_Index_Run_Lumi_Event, kSorted_Index_Run_Lumi_Entry};

      struct Transients {
	Transients();
	bool allInEntryOrder_;
	bool resultCached_;
	SortState sortState_;
        int previousAddedIndex_;
      };

    private:

      bool& allInEntryOrder() const {return transients_.get().allInEntryOrder_;}
      bool& resultCached() const {return transients_.get().resultCached_;}
      SortState& sortState() const {return transients_.get().sortState_;}
      int& previousAddedIndex() const {return transients_.get().previousAddedIndex_;}

      std::vector<Element> entries_;
      mutable Transient<Transients> transients_;
      std::vector<ProcessHistoryID> processHistoryIDs_;
  };

  bool operator<(NewFileIndex::Element const& lh, NewFileIndex::Element const& rh);

  inline
  bool operator>(NewFileIndex::Element const& lh, NewFileIndex::Element const& rh) {return rh < lh;}

  inline
  bool operator>=(NewFileIndex::Element const& lh, NewFileIndex::Element const& rh) {return !(lh < rh);}

  inline
  bool operator<=(NewFileIndex::Element const& lh, NewFileIndex::Element const& rh) {return !(rh < lh);}

  inline
  bool operator==(NewFileIndex::Element const& lh, NewFileIndex::Element const& rh) {return !(lh < rh || rh < lh);}

  inline
  bool operator!=(NewFileIndex::Element const& lh, NewFileIndex::Element const& rh) {return lh < rh || rh < lh;}

  class Compare_Index_Run_Lumi_Entry {
  public:
    bool operator()(NewFileIndex::Element const& lh, NewFileIndex::Element const& rh);
  };

  class Compare_Index_Run_Lumi {
  public:
    bool operator()(NewFileIndex::Element const& lh, NewFileIndex::Element const& rh);
  };

  class Compare_Index_Run {
  public:
    bool operator()(NewFileIndex::Element const& lh, NewFileIndex::Element const& rh);
  };

  class Compare_Index {
  public:
    bool operator()(NewFileIndex::Element const& lh, NewFileIndex::Element const& rh);
  };

  std::ostream&
  operator<<(std::ostream& os, NewFileIndex const& fileIndex);
}

#endif
