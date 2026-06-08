#include "DataManager.h"

DataManager::DataManager(Pager &pager, PageId rootPage)
    : SlottedManager(pager, rootPage, PageType::Data) {}

RecordRef DataManager::insert(const RawBytes &data) {
  return insertRecord(data);
}

RawBytes DataManager::read(const RecordRef &ref) { return readRecord(ref); }

std::optional<RecordRef> DataManager::update(const RecordRef &ref,
                                             const RawBytes &data) {
  return updateRecord(ref, data);
}

void DataManager::remove(const RecordRef &ref) { deleteRecord(ref); }

PageId DataManager::rootPage() const { return getRootPage(); }
