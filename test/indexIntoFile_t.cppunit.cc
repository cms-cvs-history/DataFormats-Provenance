/*
 *  indexIntoFile_t.cppunit.cc
 */

#include <cppunit/extensions/HelperMacros.h>

#include "DataFormats/Provenance/interface/IndexIntoFile.h"
#include "DataFormats/Provenance/interface/ProcessHistoryID.h"

namespace {
  edm::ProcessHistoryID phid = edm::ProcessHistoryID();
}

class testIndexIntoFile: public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(testIndexIntoFile);
  CPPUNIT_TEST(constructAndInsertTest);
  CPPUNIT_TEST(eventSortAndSearchTest);
  CPPUNIT_TEST(eventEntrySortAndSearchTest);
  CPPUNIT_TEST_SUITE_END();

 public:
  void setUp() { }
  void tearDown() { }

  void constructAndInsertTest();
  void eventSortAndSearchTest();
  void eventEntrySortAndSearchTest();

  bool areEntryVectorsTheSame(edm::IndexIntoFile &i1, edm::IndexIntoFile &i2);
};

///registration of the test so that the runner can find it
CPPUNIT_TEST_SUITE_REGISTRATION(testIndexIntoFile);

void testIndexIntoFile::constructAndInsertTest()
{
  edm::IndexIntoFile indexIntoFile;
  CPPUNIT_ASSERT(indexIntoFile.empty());
  CPPUNIT_ASSERT(indexIntoFile.size() == 0);
  CPPUNIT_ASSERT(indexIntoFile.begin() == indexIntoFile.end());

  indexIntoFile.addEntry(phid, 100, 101, 102,   1);
  indexIntoFile.addEntry(phid, 200, 201,   0,   2);
  indexIntoFile.addEntry(phid, 300,   0,   0,   3);
  indexIntoFile.addEntry(phid, 100, 101, 103,   2);

  CPPUNIT_ASSERT(!indexIntoFile.empty());
  CPPUNIT_ASSERT(indexIntoFile.size() == 4);

  edm::IndexIntoFile::const_iterator iter = indexIntoFile.begin();
  CPPUNIT_ASSERT(iter->run() == 100);
  CPPUNIT_ASSERT(iter->lumi() == 101);
  CPPUNIT_ASSERT(iter->event() == 102);
  CPPUNIT_ASSERT(iter->entry() == 1);
  CPPUNIT_ASSERT(iter->getEntryType() == edm::IndexIntoFile::kEvent);

  ++iter;
  CPPUNIT_ASSERT(iter->getEntryType() == edm::IndexIntoFile::kLumi);

  ++iter;
  CPPUNIT_ASSERT(iter->run() == 300);
  CPPUNIT_ASSERT(iter->lumi() == 0);
  CPPUNIT_ASSERT(iter->event() == 0);
  CPPUNIT_ASSERT(iter->entry() == 3);
  CPPUNIT_ASSERT(iter->getEntryType() == edm::IndexIntoFile::kRun);

  ++iter;
  ++iter;
  CPPUNIT_ASSERT(iter == indexIntoFile.end());

  CPPUNIT_ASSERT(indexIntoFile.allEventsInEntryOrder() == true);
  CPPUNIT_ASSERT(indexIntoFile.allEventsInEntryOrder() == true);
}

void testIndexIntoFile::eventSortAndSearchTest()
{
  // Note this contains some illegal duplicates
  // For the moment there is nothing to prevent
  // these from occurring so we handle this using
  // a stable_sort for now ...  They will not bother
  // the IndexIntoFile.

  edm::IndexIntoFile indexIntoFile;
  indexIntoFile.addEntry(phid, 3, 3, 2, 5);
  indexIntoFile.addEntry(phid, 3, 3, 1, 4);
  indexIntoFile.addEntry(phid, 3, 3, 3, 3);
  indexIntoFile.addEntry(phid, 3, 3, 0, 6);
  indexIntoFile.addEntry(phid, 3, 0, 0, 7);
  indexIntoFile.addEntry(phid, 1, 0, 0, 8);
  indexIntoFile.addEntry(phid, 3, 1, 0, 9);
  indexIntoFile.addEntry(phid, 3, 0, 0, 1);
  indexIntoFile.addEntry(phid, 3, 3, 0, 1);
  indexIntoFile.addEntry(phid, 3, 3, 1, 1);
  indexIntoFile.addEntry(phid, 1, 2, 2, 2);
  indexIntoFile.addEntry(phid, 1, 2, 4, 1);
  indexIntoFile.addEntry(phid, 1, 2, 0, 2);
  indexIntoFile.addEntry(phid, 1, 2, 1, 2);

  indexIntoFile.sortBy_Index_Run_Lumi_Event();

  edm::IndexIntoFile shouldBe;
  shouldBe.addEntry(phid, 1, 0, 0, 8);
  shouldBe.addEntry(phid, 1, 2, 0, 2);
  shouldBe.addEntry(phid, 1, 2, 1, 2);
  shouldBe.addEntry(phid, 1, 2, 2, 2);
  shouldBe.addEntry(phid, 1, 2, 4, 1);
  shouldBe.addEntry(phid, 3, 0, 0, 7);
  shouldBe.addEntry(phid, 3, 0, 0, 1);
  shouldBe.addEntry(phid, 3, 1, 0, 9);
  shouldBe.addEntry(phid, 3, 3, 0, 6);
  shouldBe.addEntry(phid, 3, 3, 0, 1);
  shouldBe.addEntry(phid, 3, 3, 1, 4);
  shouldBe.addEntry(phid, 3, 3, 1, 1);
  shouldBe.addEntry(phid, 3, 3, 2, 5);
  shouldBe.addEntry(phid, 3, 3, 3, 3);

  CPPUNIT_ASSERT(areEntryVectorsTheSame(indexIntoFile, shouldBe));

  CPPUNIT_ASSERT(indexIntoFile.allEventsInEntryOrder() == false);
  CPPUNIT_ASSERT(indexIntoFile.allEventsInEntryOrder() == false);

  edm::IndexIntoFile::const_iterator iter = indexIntoFile.findPosition( 3, 3, 2);
  CPPUNIT_ASSERT((iter - indexIntoFile.begin()) == 12);
  CPPUNIT_ASSERT(iter->run() == 3);
  CPPUNIT_ASSERT(iter->lumi() == 3);
  CPPUNIT_ASSERT(iter->event() == 2);
  CPPUNIT_ASSERT(iter->entry() == 5);

  iter = indexIntoFile.findPosition( 3, 3, 7);
  CPPUNIT_ASSERT(iter == indexIntoFile.end());

  iter = indexIntoFile.findPosition( 1, 2, 3);
  CPPUNIT_ASSERT(iter == indexIntoFile.end());

  iter = indexIntoFile.findPosition( 3, 3, 0);
  CPPUNIT_ASSERT((iter - indexIntoFile.begin()) == 8);

  iter = indexIntoFile.findPosition( 1, 1, 0);
  CPPUNIT_ASSERT(iter == indexIntoFile.end());

  iter = indexIntoFile.findPosition( 1, 0, 0);
  CPPUNIT_ASSERT((iter - indexIntoFile.begin()) == 0);

  iter = indexIntoFile.findPosition( 2, 0, 0);
  CPPUNIT_ASSERT(iter == indexIntoFile.end());

  iter = indexIntoFile.findPosition( 2, 0, 1);
  CPPUNIT_ASSERT(iter == indexIntoFile.end());

  iter = indexIntoFile.findPosition( 1, 2, 3);
  CPPUNIT_ASSERT(iter == indexIntoFile.end());

  iter = indexIntoFile.findPosition( 3, 0, 3);
  CPPUNIT_ASSERT((iter - indexIntoFile.begin()) == 13);

  iter = indexIntoFile.findEventPosition( 3, 3, 2);
  CPPUNIT_ASSERT((iter - indexIntoFile.begin()) == 12);
  CPPUNIT_ASSERT(indexIntoFile.containsEvent(3, 3, 2));

  iter = indexIntoFile.findEventPosition( 1, 2, 3);
  CPPUNIT_ASSERT(iter == indexIntoFile.end());
  CPPUNIT_ASSERT(!indexIntoFile.containsEvent(1, 2, 3));

  iter = indexIntoFile.findEventPosition( 3, 0, 1);
  CPPUNIT_ASSERT((iter - indexIntoFile.begin()) == 10);

  iter = indexIntoFile.findLumiPosition( 3, 1);
  CPPUNIT_ASSERT((iter - indexIntoFile.begin()) == 7);

  iter = indexIntoFile.findLumiPosition( 3, 2);
  CPPUNIT_ASSERT(iter == indexIntoFile.end());

  CPPUNIT_ASSERT(indexIntoFile.containsLumi(3, 3));
  CPPUNIT_ASSERT(!indexIntoFile.containsLumi(2, 3));

  iter = indexIntoFile.findRunPosition( 3);
  CPPUNIT_ASSERT((iter - indexIntoFile.begin()) == 5);

  iter = indexIntoFile.findRunPosition( 2);
  CPPUNIT_ASSERT(iter == indexIntoFile.end());

  CPPUNIT_ASSERT(indexIntoFile.containsRun(3));
  CPPUNIT_ASSERT(!indexIntoFile.containsRun(2));
}

void testIndexIntoFile::eventEntrySortAndSearchTest()
{

  // Note this contains some illegal duplicates
  // For the moment there is nothing to prevent
  // these from occurring so we handle this using
  // a stable_sort for now ...  They will not bother
  // the IndexIntoFile.

  edm::IndexIntoFile indexIntoFile;
  indexIntoFile.addEntry(phid, 3, 3, 2, 5);
  indexIntoFile.addEntry(phid, 3, 3, 1, 4);
  indexIntoFile.addEntry(phid, 3, 3, 3, 3);
  indexIntoFile.addEntry(phid, 3, 3, 0, 6);
  indexIntoFile.addEntry(phid, 3, 0, 0, 7);
  indexIntoFile.addEntry(phid, 1, 0, 0, 8);
  indexIntoFile.addEntry(phid, 3, 1, 0, 9);
  indexIntoFile.addEntry(phid, 3, 0, 0, 1);
  indexIntoFile.addEntry(phid, 3, 3, 0, 1);
  indexIntoFile.addEntry(phid, 3, 3, 1, 1);
  indexIntoFile.addEntry(phid, 1, 2, 2, 2);
  indexIntoFile.addEntry(phid, 1, 2, 4, 1);
  indexIntoFile.addEntry(phid, 1, 2, 0, 2);
  indexIntoFile.addEntry(phid, 1, 2, 1, 2);

  indexIntoFile.sortBy_Index_Run_Lumi_Entry();

  edm::IndexIntoFile shouldBe;
  shouldBe.addEntry(phid, 1, 0, 0, 8);
  shouldBe.addEntry(phid, 1, 2, 0, 2);
  shouldBe.addEntry(phid, 1, 2, 4, 1);
  shouldBe.addEntry(phid, 1, 2, 2, 2);
  shouldBe.addEntry(phid, 1, 2, 1, 2);
  shouldBe.addEntry(phid, 3, 0, 0, 1);
  shouldBe.addEntry(phid, 3, 0, 0, 7);
  shouldBe.addEntry(phid, 3, 1, 0, 9);
  shouldBe.addEntry(phid, 3, 3, 0, 1);
  shouldBe.addEntry(phid, 3, 3, 0, 6);
  shouldBe.addEntry(phid, 3, 3, 1, 1);
  shouldBe.addEntry(phid, 3, 3, 3, 3);
  shouldBe.addEntry(phid, 3, 3, 1, 4);
  shouldBe.addEntry(phid, 3, 3, 2, 5);

  CPPUNIT_ASSERT(areEntryVectorsTheSame(indexIntoFile, shouldBe));

  edm::IndexIntoFile::const_iterator iter = indexIntoFile.findPosition( 3, 1);
  CPPUNIT_ASSERT((iter - indexIntoFile.begin()) == 7);

  iter = indexIntoFile.findLumiPosition( 3, 2);
  CPPUNIT_ASSERT(iter == indexIntoFile.end());

  iter = indexIntoFile.findRunPosition( 3);
  CPPUNIT_ASSERT((iter - indexIntoFile.begin()) == 5);

  iter = indexIntoFile.findRunPosition( 2);
  CPPUNIT_ASSERT(iter == indexIntoFile.end());
}

bool testIndexIntoFile::areEntryVectorsTheSame(edm::IndexIntoFile &i1, edm::IndexIntoFile &i2) {
  if (i1.size() != i2.size()) return false;
  for (edm::IndexIntoFile::const_iterator iter1 = i1.begin(), iter2 = i2.begin();
       iter1 != i1.end(); 
       ++iter1, ++iter2) {
    if (iter1->run() != iter2->run() ||
        iter1->lumi() != iter2->lumi() ||
        iter1->event() != iter2->event() ||
        iter1->entry() != iter2->entry()) {
      return false;
    }
  }
  return true;
}
