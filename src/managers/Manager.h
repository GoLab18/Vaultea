#include "Constants.h"
#include "storage/page/Constants.h"
#include "storage/page/Pager.h"

#include <map>
#include <unordered_map>

using namespace vault::managers;
using namespace vault::storage::page;

class Manager {
public:
  explicit Manager(Pager &pager, PageId rootPage);

  void loadFreeSpaceMap();

protected:
  Pager &pager;

  PageId rootPage;
  PageId tailPage;

  std::multimap<FreeSpace, PageId> freeSpaceMap;
  std::unordered_map<PageId, std::multimap<FreeSpace, PageId>::iterator>
      pagePositions;

  void insertPage(PageId id, FreeSpace fs);
  void updatePage(PageId id, FreeSpace oldFs, FreeSpace newFs);
  void removePage(PageId id);
};
