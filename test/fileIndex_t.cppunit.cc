/*
 *  fileIndex_t.cppunit.cc
 */

#include <cppunit/extensions/HelperMacros.h>

#include "DataFormats/Provenance/interface/FileIndex.h"

class testFileIndex: public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(testFileIndex);
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

  bool areEntryVectorsTheSame(edm::FileIndex &i1, edm::FileIndex &i2);
};

///registration of the test so that the runner can find it
CPPUNIT_TEST_SUITE_REGISTRATION(testFileIndex);

void testFileIndex::constructAndInsertTest()
{
  edm::FileIndex fileIndex;
  CPPUNIT_ASSERT(fileIndex.empty());
  CPPUNIT_ASSERT(fileIndex.size() == 0);
  CPPUNIT_ASSERT(fileIndex.begin() == fileIndex.end());

  fileIndex.addEntry(100, 101, 102,   1);
  fileIndex.addEntry(200, 201,   0,   2);
  fileIndex.addEntry(300,   0,   0,   3);
  fileIndex.addEntry(100, 101, 103,   2);

  CPPUNIT_ASSERT(!fileIndex.empty());
  CPPUNIT_ASSERT(fileIndex.size() == 4);

  edm::FileIndex::const_iterator iter = fileIndex.begin();
  CPPUNIT_ASSERT(iter->run_ == 100);
  CPPUNIT_ASSERT(iter->lumi_ == 101);
  CPPUNIT_ASSERT(iter->event_ == 102);
  CPPUNIT_ASSERT(iter->entry_ == 1);
  CPPUNIT_ASSERT(iter->getEntryType() == edm::FileIndex::kEvent);

  ++iter;
  CPPUNIT_ASSERT(iter->getEntryType() == edm::FileIndex::kLumi);

  ++iter;
  CPPUNIT_ASSERT(iter->run_ == 300);
  CPPUNIT_ASSERT(iter->lumi_ == 0);
  CPPUNIT_ASSERT(iter->event_ == 0);
  CPPUNIT_ASSERT(iter->entry_ == 3);
  CPPUNIT_ASSERT(iter->getEntryType() == edm::FileIndex::kRun);

  ++iter;
  ++iter;
  CPPUNIT_ASSERT(iter == fileIndex.end());

  CPPUNIT_ASSERT(fileIndex.allEventsInEntryOrder() == true);
  CPPUNIT_ASSERT(fileIndex.allEventsInEntryOrder() == true);
}

void testFileIndex::eventSortAndSearchTest()
{
  // Note this contains some illegal duplicates
  // For the moment there is nothing to prevent
  // these from occurring so we handle this using
  // a stable_sort for now ...  They will not bother
  // the FileIndex.

  edm::FileIndex fileIndex;
  fileIndex.addEntry(3, 3, 2, 5);
  fileIndex.addEntry(3, 3, 1, 4);
  fileIndex.addEntry(3, 3, 3, 3);
  fileIndex.addEntry(3, 3, 0, 6);
  fileIndex.addEntry(3, 0, 0, 7);
  fileIndex.addEntry(1, 0, 0, 8);
  fileIndex.addEntry(3, 1, 0, 9);
  fileIndex.addEntry(3, 0, 0, 1);
  fileIndex.addEntry(3, 3, 0, 1);
  fileIndex.addEntry(3, 3, 1, 1);
  fileIndex.addEntry(1, 2, 2, 2);
  fileIndex.addEntry(1, 2, 4, 1);
  fileIndex.addEntry(1, 2, 0, 2);
  fileIndex.addEntry(1, 2, 1, 2);

  fileIndex.sortBy_Run_Lumi_Event();

  edm::FileIndex shouldBe;
  shouldBe.addEntry(1, 0, 0, 8);
  shouldBe.addEntry(1, 2, 0, 2);
  shouldBe.addEntry(1, 2, 1, 2);
  shouldBe.addEntry(1, 2, 2, 2);
  shouldBe.addEntry(1, 2, 4, 1);
  shouldBe.addEntry(3, 0, 0, 7);
  shouldBe.addEntry(3, 0, 0, 1);
  shouldBe.addEntry(3, 1, 0, 9);
  shouldBe.addEntry(3, 3, 0, 6);
  shouldBe.addEntry(3, 3, 0, 1);
  shouldBe.addEntry(3, 3, 1, 4);
  shouldBe.addEntry(3, 3, 1, 1);
  shouldBe.addEntry(3, 3, 2, 5);
  shouldBe.addEntry(3, 3, 3, 3);

  CPPUNIT_ASSERT(areEntryVectorsTheSame(fileIndex, shouldBe));

  CPPUNIT_ASSERT(fileIndex.allEventsInEntryOrder() == false);
  CPPUNIT_ASSERT(fileIndex.allEventsInEntryOrder() == false);

  edm::FileIndex::const_iterator iter = fileIndex.findPosition( 3, 3, 2);
  CPPUNIT_ASSERT((iter - fileIndex.begin()) == 12);
  CPPUNIT_ASSERT(iter->run_ == 3);
  CPPUNIT_ASSERT(iter->lumi_ == 3);
  CPPUNIT_ASSERT(iter->event_ == 2);
  CPPUNIT_ASSERT(iter->entry_ == 5);

  iter = fileIndex.findPosition( 3, 3, 7);
  CPPUNIT_ASSERT(iter == fileIndex.end());

  iter = fileIndex.findPosition( 1, 2, 3);
  CPPUNIT_ASSERT((iter - fileIndex.begin()) == 4);

  iter = fileIndex.findPosition( 3, 3, 0);
  CPPUNIT_ASSERT((iter - fileIndex.begin()) == 8);

  iter = fileIndex.findPosition( 1, 1, 0);
  CPPUNIT_ASSERT((iter - fileIndex.begin()) == 1);

  iter = fileIndex.findPosition( 1, 0, 0);
  CPPUNIT_ASSERT((iter - fileIndex.begin()) == 0);

  iter = fileIndex.findPosition( 2, 0, 0);
  CPPUNIT_ASSERT((iter - fileIndex.begin()) == 5);

  iter = fileIndex.findPosition( 2, 0, 1);
  CPPUNIT_ASSERT((iter - fileIndex.begin()) == 5);

  iter = fileIndex.findPosition( 1, 2, 3);
  CPPUNIT_ASSERT((iter - fileIndex.begin()) == 4);

  iter = fileIndex.findPosition( 3, 0, 3);
  CPPUNIT_ASSERT((iter - fileIndex.begin()) == 13);

  iter = fileIndex.findEventPosition( 3, 3, 2, true);
  CPPUNIT_ASSERT((iter - fileIndex.begin()) == 12);
  CPPUNIT_ASSERT(fileIndex.containsEvent(3, 3, 2, true));

  iter = fileIndex.findEventPosition( 1, 2, 3, true);
  CPPUNIT_ASSERT(iter == fileIndex.end());
  CPPUNIT_ASSERT(!fileIndex.containsEvent(1, 2, 3, true));

  iter = fileIndex.findEventPosition( 1, 2, 3, false);
  CPPUNIT_ASSERT((iter - fileIndex.begin()) == 4);
  CPPUNIT_ASSERT(fileIndex.containsEvent(1, 2, 3, false));

  iter = fileIndex.findEventPosition( 3, 0, 1, true);
  CPPUNIT_ASSERT((iter - fileIndex.begin()) == 10);

  iter = fileIndex.findLumiPosition( 3, 1, true);
  CPPUNIT_ASSERT((iter - fileIndex.begin()) == 7);

  iter = fileIndex.findLumiPosition( 3, 2, true);
  CPPUNIT_ASSERT(iter == fileIndex.end());

  iter = fileIndex.findLumiPosition( 3, 1, false);
  CPPUNIT_ASSERT((iter - fileIndex.begin()) == 7);

  iter = fileIndex.findLumiPosition( 3, 2, false);
  CPPUNIT_ASSERT((iter - fileIndex.begin()) == 8);

  iter = fileIndex.findRunPosition( 3, true);
  CPPUNIT_ASSERT((iter - fileIndex.begin()) == 5);

  iter = fileIndex.findRunPosition( 2, true);
  CPPUNIT_ASSERT(iter == fileIndex.end());

  iter = fileIndex.findRunPosition( 2, false);
  CPPUNIT_ASSERT((iter - fileIndex.begin()) == 5);

  iter = fileIndex.findLumiOrRunPosition( 1, 2);
  CPPUNIT_ASSERT((iter - fileIndex.begin()) == 1);

  iter = fileIndex.findLumiOrRunPosition( 3, 0);
  CPPUNIT_ASSERT((iter - fileIndex.begin()) == 5);

  iter = fileIndex.findLumiOrRunPosition( 3, 4);
  CPPUNIT_ASSERT(iter == fileIndex.end());

  iter = fileIndex.findLumiOrRunPosition( 3, 2);
  CPPUNIT_ASSERT((iter - fileIndex.begin()) == 8);

  iter = fileIndex.findLumiOrRunPosition( 2, 1);
  CPPUNIT_ASSERT((iter - fileIndex.begin()) == 5);

}

void testFileIndex::eventEntrySortAndSearchTest()
{

  // Note this contains some illegal duplicates
  // For the moment there is nothing to prevent
  // these from occurring so we handle this using
  // a stable_sort for now ...  They will not bother
  // the FileIndex.

  edm::FileIndex fileIndex;
  fileIndex.addEntry(3, 3, 2, 5);
  fileIndex.addEntry(3, 3, 1, 4);
  fileIndex.addEntry(3, 3, 3, 3);
  fileIndex.addEntry(3, 3, 0, 6);
  fileIndex.addEntry(3, 0, 0, 7);
  fileIndex.addEntry(1, 0, 0, 8);
  fileIndex.addEntry(3, 1, 0, 9);
  fileIndex.addEntry(3, 0, 0, 1);
  fileIndex.addEntry(3, 3, 0, 1);
  fileIndex.addEntry(3, 3, 1, 1);
  fileIndex.addEntry(1, 2, 2, 2);
  fileIndex.addEntry(1, 2, 4, 1);
  fileIndex.addEntry(1, 2, 0, 2);
  fileIndex.addEntry(1, 2, 1, 2);

  fileIndex.sortBy_Run_Lumi_EventEntry();

  edm::FileIndex shouldBe;
  shouldBe.addEntry(1, 0, 0, 8);
  shouldBe.addEntry(1, 2, 0, 2);
  shouldBe.addEntry(1, 2, 4, 1);
  shouldBe.addEntry(1, 2, 2, 2);
  shouldBe.addEntry(1, 2, 1, 2);
  shouldBe.addEntry(3, 0, 0, 7);
  shouldBe.addEntry(3, 0, 0, 1);
  shouldBe.addEntry(3, 1, 0, 9);
  shouldBe.addEntry(3, 3, 0, 6);
  shouldBe.addEntry(3, 3, 0, 1);
  shouldBe.addEntry(3, 3, 1, 1);
  shouldBe.addEntry(3, 3, 3, 3);
  shouldBe.addEntry(3, 3, 1, 4);
  shouldBe.addEntry(3, 3, 2, 5);

  CPPUNIT_ASSERT(areEntryVectorsTheSame(fileIndex, shouldBe));

  edm::FileIndex::const_iterator iter = fileIndex.findLumiPosition( 3, 1, true);
  CPPUNIT_ASSERT((iter - fileIndex.begin()) == 7);

  iter = fileIndex.findLumiPosition( 3, 2, true);
  CPPUNIT_ASSERT(iter == fileIndex.end());

  iter = fileIndex.findLumiPosition( 3, 1, false);
  CPPUNIT_ASSERT((iter - fileIndex.begin()) == 7);

  iter = fileIndex.findLumiPosition( 3, 2, false);
  CPPUNIT_ASSERT((iter - fileIndex.begin()) == 8);

  iter = fileIndex.findRunPosition( 3, true);
  CPPUNIT_ASSERT((iter - fileIndex.begin()) == 5);

  iter = fileIndex.findRunPosition( 2, true);
  CPPUNIT_ASSERT(iter == fileIndex.end());

  iter = fileIndex.findRunPosition( 2, false);
  CPPUNIT_ASSERT((iter - fileIndex.begin()) == 5);

  iter = fileIndex.findLumiOrRunPosition( 1, 2);
  CPPUNIT_ASSERT((iter - fileIndex.begin()) == 1);

  iter = fileIndex.findLumiOrRunPosition( 3, 0);
  CPPUNIT_ASSERT((iter - fileIndex.begin()) == 5);

  iter = fileIndex.findLumiOrRunPosition( 3, 4);
  CPPUNIT_ASSERT(iter == fileIndex.end());

  iter = fileIndex.findLumiOrRunPosition( 3, 2);
  CPPUNIT_ASSERT((iter - fileIndex.begin()) == 8);

  iter = fileIndex.findLumiOrRunPosition( 2, 1);
  CPPUNIT_ASSERT((iter - fileIndex.begin()) == 5);
}

bool testFileIndex::areEntryVectorsTheSame(edm::FileIndex &i1, edm::FileIndex &i2) {
  if (i1.size() != i2.size()) return false;
  for (edm::FileIndex::const_iterator iter1 = i1.begin(), iter2 = i2.begin();
       iter1 != i1.end(); 
       ++iter1, ++iter2) {
    if (iter1->run_ != iter2->run_ ||
        iter1->lumi_ != iter2->lumi_ ||
        iter1->event_ != iter2->event_ ||
        iter1->entry_ != iter2->entry_) {
      return false;
    }
  }
  return true;
}