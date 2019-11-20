#include "sharedpointertracker.h"

namespace SharedPtrTracker {

/**
 * global storage for shared-ptr's to allow more insightful tracking and debugging
 */
PtrRegistryType     _global_items_shared_ptr_registry = {};
ImmortalPtrRegType  _global_immortal_shared_ptr_registry = {};
int                 _global_alloc_counter = 0;

}

