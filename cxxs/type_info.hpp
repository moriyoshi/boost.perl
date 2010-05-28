#include <boost/intrusive/rbtree_algorithms.hpp>
#include <boost/type_traits/is_pod.hpp>
#include <boost/range/begin.hpp>
#include <boost/range/end.hpp>
#include <boost/noncopyable.hpp>
#include <typeinfo>
#include <vector>
#include <string>

#include "cxxs/demangle.hpp"

#ifndef CXXS_TYPE_INFO_DEFINED
#define CXXS_TYPE_INFO_DEFINED

namespace cxxs {

namespace detail {

    inline std::string demangle_if_possible(char const* name)
    {
#if CXXS_DEMANGLER_AVAILABLE
        return demangle(name);
#else
        return std::string(name);
#endif
    }

    struct type_info_rbtree_node_traits;

} // namespace detail


class type_info: public boost::noncopyable {
    friend struct detail::type_info_rbtree_node_traits;
public:
    type_info(): id_(0), name_(), bases_() {}

    type_info(std::type_info const& id)
        : id_(&id), name_(detail::demangle_if_possible(id.name())),
          bases_() {}

    template<typename Tset>
    type_info(std::type_info const& id, Tset const& bases)
        : id_(&id), name_(detail::demangle_if_possible(id.name())),
          bases_(boost::begin(bases), boost::end(bases)) {}

    std::type_info const& id() const {
        return *id_;
    }

    std::string const& name() const {
        return name_;
    }

    std::vector<type_info const*> const& bases() {
        return bases_;
    }

    bool operator<(type_info const& rhs) {
        return id_ < rhs.id_;
    }

private: 
    struct {
        type_info* parent;
        type_info* left;
        type_info* right;
        unsigned int color:1;
    } rbtree_head_;

    std::type_info const* id_;
    std::string const name_;
    std::vector<type_info const*> const bases_; 
};

namespace detail {
    struct type_info_rbtree_node_traits {
        typedef type_info node;
        typedef type_info* node_ptr;
        typedef type_info const* const_node_ptr;
        typedef unsigned int color;

        static node_ptr get_parent(const_node_ptr n) {
            return n->rbtree_head_.parent;
        }

        static void set_parent(node_ptr n, node_ptr v) {
            n->rbtree_head_.parent = v;
        }

        static node_ptr get_left(const_node_ptr n) {
            return n->rbtree_head_.left;
        }

        static void set_left(node_ptr n, node_ptr v) {
            n->rbtree_head_.left = v;
        }

        static node_ptr get_right(const_node_ptr n) {
            return n->rbtree_head_.right;
        }

        static void set_right(node_ptr n, node_ptr v) {
            n->rbtree_head_.right = v;
        }

        static color get_color(const_node_ptr n) {
            return n->rbtree_head_.color;
        }

        static void set_color(node_ptr n, color v) {
            n->rbtree_head_.color = v;
        }

        static color black() { return 0; }

        static color red() { return 1; }

        struct node_compare {
            bool operator()(const_node_ptr a, const_node_ptr b) {
                return &a->id() < &b->id();
            }
        };

        struct key_node_compare {
            bool operator()(std::type_info const& key, const_node_ptr n) {
                return &key < &n->id();
            }

            bool operator()(const_node_ptr n, std::type_info const& key) {
                return &n->id() < &key;
            }
        };
    };
} // namespace detail

} // namespace cxxs

#endif /* CXXS_TYPE_INFO_DEFINED */

#ifndef CXXS_TYPE_INFO_REGISTRY_DEFINED
#define CXXS_TYPE_INFO_REGISTRY_DEFINED

namespace cxxs {

class type_info_registry {
    typedef boost::intrusive::rbtree_algorithms<
            detail::type_info_rbtree_node_traits> rbtree_algo_type;

private:
    struct node_deleter { void operator()(type_info* p) { delete p; } };

public:

    ~type_info_registry() {
        rbtree_algo_type::clear_and_dispose(&root, node_deleter());
    }

    type_info const* lookup(std::type_info const& id) {
        type_info const* retval(rbtree_algo_type::find(&root, id,
            detail::type_info_rbtree_node_traits::key_node_compare()));
        return retval == &root ? 0: retval;
    }

    bool add(type_info* n) {
        rbtree_algo_type::insert_commit_data cd;
        std::pair<type_info*, bool> r(
            rbtree_algo_type::insert_unique_check(&root, n->id(),
                detail::type_info_rbtree_node_traits::key_node_compare(),
                cd));
        if (r.second) {
            rbtree_algo_type::insert_unique_commit(&root, n, cd);
            return true;
        } else {
            delete n;
            return false;
        }
    }

    type_info_registry() {
        rbtree_algo_type::init_header(&root);
    }

private:
    type_info root;
};

} // namespace cxxs

#endif /* CXXS_TYPE_INFO_REGISTRY_DEFINED */

#ifndef CXXS_TYPE_INFO_UTIL_FUNCTIIONS_DEFINED
#define CXXS_TYPE_INFO_UTIL_FUNCTIIONS_DEFINED

#include "cxxs/errors.hpp"

namespace cxxs {

inline type_info_registry& get_global_type_info_registry() {
    static type_info_registry singleton;
    return singleton;
}

inline type_info const& get_type_info(std::type_info const& id)
{
    type_info const* retval(get_global_type_info_registry().lookup(id));
    if (!retval)
        throw type_not_registered(id);
    return *retval;
}

template<typename T>
inline type_info const& get_type_info()
{
    std::type_info const& id(typeid(T));
    type_info_registry& reg(get_global_type_info_registry());

    type_info const* retval(get_global_type_info_registry().lookup(id));
    if (!retval) {
        if (boost::is_pod<T>::value) {
            retval = new type_info(id);
            reg.add(retval);
        } else {
            throw type_not_registered(id);
        }
    }
    return retval;
}

} // namespace cxxs

#endif /* CXXS_TYPE_INFO_UTIL_FUNCTIIONS_DEFINED */
