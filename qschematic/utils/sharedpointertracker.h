#include <memory>
#include <QDebug>

#define TINKER__PTR_TRACKER__LOG_LEVEL 1
#define TINKER__MAKE_SH__USERSPACE_TRACKER true

/// QDebug stream-op additions
template <typename ...T>
auto operator <<(QDebug debug, const std::weak_ptr<T...>& val) -> QDebug
{
    auto raw_ptr = val.lock().get();
    debug << "std::weak_ptr{ "
          << raw_ptr
          << ", use_count: "
          << val.use_count()
          << " }";
    return debug;
}

template <typename ...T>
auto operator <<(QDebug debug, const std::shared_ptr<T...>& val) -> QDebug
{
    debug << "std::shared_ptr{ "
          << val.get()
          << ", use_count: "
          << val.use_count()
          << " }";
    return debug;
}

inline
    auto operator <<(QDebug debug, const std::string& val) -> QDebug
{
    debug << val.c_str();
    return debug;
}



namespace QSchematic {
    class Item;
}

/**
 * Global management of item-shared-ptr's to allow more insightful tracking and debugging
 */

namespace SharedPtrTracker {

using PtrTrackerBaseT = QSchematic::Item;
using ImmortalPtrRegType = std::vector<std::shared_ptr<PtrTrackerBaseT>>;
using PtrRegistryType = std::unordered_map<const PtrTrackerBaseT*, std::weak_ptr<PtrTrackerBaseT>>;

extern PtrRegistryType      _global_items_shared_ptr_registry;
extern ImmortalPtrRegType   _global_immortal_shared_ptr_registry;
extern int                  _global_alloc_counter;


inline
auto _dbg_inspect_shptr_registry() -> void
{
    auto _d = qDebug();
    _d << endl << "{";
    for (auto pair : _global_items_shared_ptr_registry ) {
        _d << "    " << pair << endl;
    }
    _d << "}" << endl;
}

template <typename InPtrT>
auto obtain_weak_pointer(InPtrT* ptr) -> std::weak_ptr<PtrTrackerBaseT>
{
    auto qg_ptr = static_cast<const PtrTrackerBaseT*>(ptr);
    if ( _global_items_shared_ptr_registry.find(qg_ptr) != end(_global_items_shared_ptr_registry) ) {
        auto w_ptr = _global_items_shared_ptr_registry.at(qg_ptr);
        if ( not w_ptr.expired() ) {
            #if TINKER__PTR_TRACKER__LOG_LEVEL >= 3
                qDebug() << "SHPTR-REG => got ptr" << ptr << " => " << w_ptr;
            #endif
            return w_ptr;
        }
        else {
            #if TINKER__PTR_TRACKER__LOG_LEVEL >= 2
                qDebug() << "SHPTR-REG => got ptr, but it's not alive anymore!" << ptr;
            #endif
            return {};
        }
    }
    else {
        #if TINKER__PTR_TRACKER__LOG_LEVEL >= 2
            qDebug() << "SHPTR-REG => no matching shared-ptr found!" << ptr;
        #endif
        return {};
    }
}

template <typename WantedT = PtrTrackerBaseT, typename InPtrT>
auto obtain_shared_pointer(InPtrT* ptr) -> std::shared_ptr<WantedT>
{
    auto w_ptr = obtain_weak_pointer(ptr);
    auto sh_ptr = std::dynamic_pointer_cast<WantedT>(w_ptr.lock());
    return sh_ptr;
}

template <typename InPtrT>
auto assert_expired(InPtrT* ptr) -> bool
{
    auto w_ptr = obtain_weak_pointer(ptr);
    return w_ptr.expired();
}

template <typename T>
auto eternalize_pointer_life(std::shared_ptr<T> ptr) -> void {
    _global_immortal_shared_ptr_registry.push_back(ptr);
}

template <typename InstanceT, typename ...ArgsT>
auto mk_sh(ArgsT&& ...args) -> std::shared_ptr<InstanceT>
{
    #if TINKER__MAKE_SH__USERSPACE_TRACKER == true
        #if TINKER__PTR_TRACKER__LOG_LEVEL >= 3
            auto count = ++_global_alloc_counter;
            qDebug() << "mk_sh<...>() -> (" << count << ")";
        #endif
        auto sh_ptr = std::make_shared<InstanceT>(args...);
        auto root_type_ptr = static_cast<PtrTrackerBaseT*>(sh_ptr.get());

        #if TINKER__PTR_TRACKER__LOG_LEVEL >= 3
            qDebug() << "mk_sh... (" << count << ")" << root_type_ptr << sh_ptr;
        #endif

        _global_items_shared_ptr_registry.insert({root_type_ptr, sh_ptr});

        #if TINKER__PTR_TRACKER__LOG_LEVEL >= 3
            qDebug() << "mk_sh<...>() -> / (" << count << ") registry.size = " << _global_items_shared_ptr_registry.size() << endl;
        #endif

        return sh_ptr;

    #else
        return std::make_shared<InstanceT>(args...);
    #endif
}


// TODO: explicit _maybe version, others will assert-throw if expectations unmet
// TODO: cleanup_stale()
// TODO: register_untracked_shptr(shptr/weakptr)
// TODO: mk_sh<>() add capability of tracking TIMESTAMP, SERIAL and/or call-stack-point
// TODO: assert_shptr_expired(shptr)
// TODO: assert_shptr_alive(shptr)

}



namespace QSchematic {

template <typename T, typename ...ArgsT>
auto mk_sh(ArgsT&&... args) -> std::shared_ptr<T>
{
    return SharedPtrTracker::mk_sh<T>(std::forward<ArgsT>(args)...);
}

template <typename WantedT, typename T>
auto adopt_origin_instance(std::shared_ptr<T> val) -> std::shared_ptr<WantedT>
{
    return std::dynamic_pointer_cast<WantedT>(val);
}

template <typename T>
auto adopt_origin_instance(std::shared_ptr<T> val) -> std::shared_ptr<T>
{
    return val;
}

}