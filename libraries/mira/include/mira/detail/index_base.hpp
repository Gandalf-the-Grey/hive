/* Copyright 2003-2017 Joaquin M Lopez Munoz.
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 *
 * See http://www.boost.org/libs/multi_index for library home page.
 */

#pragma once

#include <boost/config.hpp> /* keep it first to prevent nasty warns in MSVC */
#include <boost/core/addressof.hpp>
#include <boost/detail/allocator_utilities.hpp>
#include <boost/detail/no_exceptions_support.hpp>
#include <boost/detail/workaround.hpp>
#include <boost/move/core.hpp>
#include <boost/move/utility.hpp>
#include <boost/mpl/vector.hpp>
#include <mira/detail/copy_map.hpp>
#include <mira/detail/do_not_copy_elements_tag.hpp>
#include <mira/detail/node_type.hpp>
#include <boost/multi_index/detail/vartempl_support.hpp>
#include <mira/multi_index_container_fwd.hpp>
#include <boost/tuple/tuple.hpp>
#include <utility>

#include <mira/detail/index_loader.hpp>
#include <mira/detail/index_saver.hpp>

#include <rocksdb/db.h>
#include <rocksdb/options.h>
#include <rocksdb/slice.h>
#include <rocksdb/utilities/write_batch_with_index.h>

namespace mira{

namespace multi_index{

namespace detail{

/* The role of this class is threefold:
 *   - tops the linear hierarchy of indices.
 *   - terminates some cascading backbone function calls (insert_, etc.),
 *   - grants access to the backbone functions of the final
 *     multi_index_container class (for access restriction reasons, these
 *     cannot be called directly from the index classes.)
 */

struct lvalue_tag{};
struct rvalue_tag{};
struct emplaced_tag{};

template<typename Value,typename IndexSpecifierList,typename Allocator>
class index_base
{
protected:
  typedef index_node_base<Value,Allocator>    node_type;
  typedef typename multi_index_node_type<
    Value,IndexSpecifierList,Allocator>::type final_node_type;
  typedef multi_index_container<
    Value,IndexSpecifierList,Allocator>       final_type;
  typedef boost::tuples::null_type            ctor_args_list;
  typedef typename
  boost::detail::allocator::rebind_to<
    Allocator,
    typename Allocator::value_type
  >::type                                     final_allocator_type;
  typedef boost::mpl::vector0<>               index_type_list;
  typedef boost::mpl::vector0<>               iterator_type_list;
  typedef boost::mpl::vector0<>               const_iterator_type_list;
  typedef copy_map<
    final_node_type,
    final_allocator_type>                     copy_map_type;

  typedef index_saver<
    node_type,
    final_allocator_type>                     index_saver_type;
  typedef index_loader<
    node_type,
    final_node_type,
    final_allocator_type>                     index_loader_type;

private:
  typedef Value                               value_type;

protected:
  explicit index_base(const ctor_args_list&,const Allocator&){}

   typedef boost::true_type                  is_terminal_node;
   typedef boost::false_type                 id_type;
   typedef boost::false_type                 id_from_value;
   typedef boost::false_type                 primary_index_type;
   typedef boost::false_type                 iterator;

   db_ptr                                    _db;
   ::rocksdb::WriteBatch                     _write_buffer;
   column_handles                            _handles;

   static const size_t                       COLUMN_INDEX = 0;

  index_base(
    const index_base<Value,IndexSpecifierList,Allocator>&,
    do_not_copy_elements_tag)
  {}

   ~index_base()
   {
      cleanup_column_handles();
   }

   void flush() {}

  void copy_(
    const index_base<Value,IndexSpecifierList,Allocator>&,const copy_map_type&)
  {}

  final_node_type* insert_(const value_type& v,final_node_type*& x,lvalue_tag)
  {
    x=final().allocate_node();
    BOOST_TRY{
      boost::detail::allocator::construct(boost::addressof(x->value()),v);
    }
    BOOST_CATCH(...){
      final().deallocate_node(x);
      BOOST_RETHROW;
    }
    BOOST_CATCH_END
    return x;
  }

  final_node_type* insert_(const value_type& v,final_node_type*& x,rvalue_tag)
  {
    x=final().allocate_node();
    BOOST_TRY{
      /* This shoud have used a modified, T&&-compatible version of
       * boost::detail::allocator::construct, but
       * <boost/detail/allocator_utilities.hpp> is too old and venerable to
       * mess with; besides, it is a general internal utility and the imperfect
       * perfect forwarding emulation of Boost.Move might break other libs.
       */

      new (boost::addressof(x->value()))
        value_type(boost::move(const_cast<value_type&>(v)));
    }
    BOOST_CATCH(...){
      final().deallocate_node(x);
      BOOST_RETHROW;
    }
    BOOST_CATCH_END
    return x;
  }

  final_node_type* insert_(const value_type&,final_node_type*& x,emplaced_tag)
  {
    return x;
  }

  final_node_type* insert_(
    const value_type& v,node_type*,final_node_type*& x,lvalue_tag)
  {
    return insert_(v,x,lvalue_tag());
  }

  final_node_type* insert_(
    const value_type& v,node_type*,final_node_type*& x,rvalue_tag)
  {
    return insert_(v,x,rvalue_tag());
  }

  final_node_type* insert_(
    const value_type&,node_type*,final_node_type*& x,emplaced_tag)
  {
    return x;
  }

  bool insert_rocksdb_( const Value& v )
  {
     return true;
  }

  void erase_(value_type& x) {}

  void delete_node_(node_type* x)
  {
    boost::detail::allocator::destroy(boost::addressof(x->value()));
  }

  void clear_(){}

  void swap_(index_base<Value,IndexSpecifierList,Allocator>&){}

  void swap_elements_(index_base<Value,IndexSpecifierList,Allocator>&){}

  bool replace_(const value_type& v,node_type* x,lvalue_tag)
  {
    x->value()=v;
    return true;
  }

  bool replace_(const value_type& v,node_type* x,rvalue_tag)
  {
    x->value()=boost::move(const_cast<value_type&>(v));
    return true;
  }

   template< typename Modifier >
   bool modify_( Modifier& mod, value_type& v )
   {
      mod( v );
      return true;
   }

  //bool modify_(node_type*){return true;}

  //bool modify_rollback_(node_type*){return true;}

  bool check_rollback_(node_type*)const{return true;}

  /* serialization */

  template<typename Archive>
  void save_(Archive&,const unsigned int,const index_saver_type&)const{}

  template<typename Archive>
  void load_(Archive&,const unsigned int,const index_loader_type&){}

#if defined(BOOST_MULTI_INDEX_ENABLE_INVARIANT_CHECKING)
  /* invariant stuff */

  bool invariant_()const{return true;}
#endif

   void populate_column_definitions_( column_definitions& defs )const
   {
      // Do nothing
      //*
      defs.emplace_back(
         ::rocksdb::kDefaultColumnFamilyName,
         ::rocksdb::ColumnFamilyOptions()
      );
      //*/
   }

   void cleanup_column_handles()
   {
      for( auto* h : _handles )
         delete h;

      _handles.clear();
   }

  /* access to backbone memfuns of Final class */

  final_type&       final(){return *static_cast<final_type*>(this);}
  const final_type& final()const{return *static_cast<const final_type*>(this);}

  final_node_type* final_header()const{return final().header();}

  bool        final_empty_()const{return final().empty_();}
  std::size_t final_size_()const{return final().size_();}
  std::size_t final_max_size_()const{return final().max_size_();}

  std::pair<final_node_type*,bool> final_insert_(const value_type& x)
    {return final().insert_(x);}
  std::pair<final_node_type*,bool> final_insert_rv_(const value_type& x)
    {return final().insert_rv_(x);}
  template<typename T>
  std::pair<final_node_type*,bool> final_insert_ref_(const T& t)
    {return final().insert_ref_(t);}
  template<typename T>
  std::pair<final_node_type*,bool> final_insert_ref_(T& t)
    {return final().insert_ref_(t);}

  template<BOOST_MULTI_INDEX_TEMPLATE_PARAM_PACK>
  std::pair<final_node_type*,bool> final_emplace_(
    BOOST_MULTI_INDEX_FUNCTION_PARAM_PACK)
  {
    return final().emplace_(BOOST_MULTI_INDEX_FORWARD_PARAM_PACK);
  }

  template<BOOST_MULTI_INDEX_TEMPLATE_PARAM_PACK>
  bool final_emplace_rocksdb_(
    BOOST_MULTI_INDEX_FUNCTION_PARAM_PACK)
  {
    return final().emplace_rocksdb_(BOOST_MULTI_INDEX_FORWARD_PARAM_PACK);
  }

  std::pair<final_node_type*,bool> final_insert_(
    const value_type& x,final_node_type* position)
    {return final().insert_(x,position);}
  std::pair<final_node_type*,bool> final_insert_rv_(
    const value_type& x,final_node_type* position)
    {return final().insert_rv_(x,position);}
  template<typename T>
  std::pair<final_node_type*,bool> final_insert_ref_(
    const T& t,final_node_type* position)
    {return final().insert_ref_(t,position);}
  template<typename T>
  std::pair<final_node_type*,bool> final_insert_ref_(
    T& t,final_node_type* position)
    {return final().insert_ref_(t,position);}

  template<BOOST_MULTI_INDEX_TEMPLATE_PARAM_PACK>
  std::pair<final_node_type*,bool> final_emplace_hint_(
    final_node_type* position,BOOST_MULTI_INDEX_FUNCTION_PARAM_PACK)
  {
    return final().emplace_hint_(
      position,BOOST_MULTI_INDEX_FORWARD_PARAM_PACK);
  }

   void final_erase_( value_type& v )
   {
      final().erase_( v );
   }
/*
  void final_delete_node_(final_node_type* x){final().delete_node_(x);}
  void final_delete_all_nodes_(){final().delete_all_nodes_();}
*/
   void final_clear_() { final().clear_(); }
/*
  void final_swap_(final_type& x){final().swap_(x);}

  bool final_replace_(
    const value_type& k,final_node_type* x)
    {return final().replace_(k,x);}
  bool final_replace_rv_(
    const value_type& k,final_node_type* x)
    {return final().replace_rv_(k,x);}
*/
   template< typename Modifier >
   bool final_modify_( Modifier& mod, value_type& x )
   {
      return final().modify_( mod, x );
   }

   template< typename Modifier, typename Rollback >
   bool final_modify_( Modifier& mod, Rollback& back, value_type& x )
   {
      return final().modify_( mod, back, x );
   }

   size_t final_get_column_size()
   {
      return final().get_column_size();
   }

#if defined(BOOST_MULTI_INDEX_ENABLE_INVARIANT_CHECKING)
  void final_check_invariant_()const{final().check_invariant_();}
#endif
};

} /* namespace multi_index::detail */

} /* namespace multi_index */

} /* namespace mira */
