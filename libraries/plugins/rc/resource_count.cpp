
#include <hive/plugins/rc/resource_count.hpp>
#include <hive/plugins/rc/resource_sizes.hpp>

#include <hive/protocol/operations.hpp>

namespace hive { namespace plugins { namespace rc {

using namespace hive::protocol;

struct count_operation_visitor
{
  typedef void result_type;

  mutable int32_t  market_op_count = 0;
  mutable int32_t  new_account_op_count = 0;
  mutable int64_t  state_bytes_count = 0;
  mutable int64_t  execution_time_count = 0;
  mutable bool     subsidized_op = false;
  mutable uint32_t subsidized_signatures = 0;

  const state_object_size_info& _w;
  const operation_exec_info& _e;
  const fc::time_point_sec _now;

  count_operation_visitor( const state_object_size_info& w, const operation_exec_info& e, const fc::time_point_sec now )
    : _w(w), _e(e), _now(now) {}

  int64_t get_authority_dynamic_size( const authority& auth )const
  {
    int64_t size = _w.authority_account_member_size * auth.account_auths.size()
      + _w.authority_key_member_size * auth.key_auths.size();
    return size;
  }

  void operator()( const account_create_operation& op )const
  {
    state_bytes_count +=
        _w.account_create_base_size
      + _w.account_json_metadata_char_size * op.json_metadata.size()
      + get_authority_dynamic_size( op.owner )
      + get_authority_dynamic_size( op.active )
      + get_authority_dynamic_size( op.posting );
    //note: initial delegation from fee was only active before RC - compare with account_create_with_delegation_operation
    execution_time_count += _e.account_create_time;
  }

  void operator()( const account_create_with_delegation_operation& op )const
  {
    state_bytes_count +=
        _w.account_create_base_size
      + _w.account_json_metadata_char_size * op.json_metadata.size()
      + get_authority_dynamic_size( op.owner )
      + get_authority_dynamic_size( op.active )
      + get_authority_dynamic_size( op.posting );
    //compare with account_create_operation and delegate_vesting_shares_operation
    state_bytes_count += _w.vesting_delegation_object_size;
    execution_time_count += _e.account_create_with_delegation_time;
  }

  void operator()( const account_witness_vote_operation& op )const
  {
    if( op.approve )
      state_bytes_count += _w.account_witness_vote_size;
    execution_time_count += _e.account_witness_vote_time;
  }

  void operator()( const comment_operation& op )const
  {
    state_bytes_count +=
        _w.comment_base_size
      + _w.comment_permlink_char_size * op.permlink.size();
    execution_time_count += _e.comment_time;
  }

  void operator()( const comment_payout_beneficiaries& bens )const
  {
    state_bytes_count += _w.comment_beneficiaries_member_size * bens.beneficiaries.size();
  }

  void operator()( const comment_options_operation& op )const
  {
    for( const comment_options_extension& e : op.extensions )
    {
      e.visit( *this );
    }
    execution_time_count += _e.comment_options_time;
  }

  void operator()( const convert_operation& op ) const
  {
    state_bytes_count += _w.convert_size;
    execution_time_count += _e.convert_time;
  }

  void operator()( const collateralized_convert_operation& op ) const
  {
    state_bytes_count += _w.collateralized_convert_size;
    execution_time_count += _e.collateralized_convert_time;
  }

  void operator()( const create_claimed_account_operation& op )const
  {
    state_bytes_count +=
        _w.account_create_base_size
      + _w.account_json_metadata_char_size * op.json_metadata.size()
      + get_authority_dynamic_size( op.owner )
      + get_authority_dynamic_size( op.active )
      + get_authority_dynamic_size( op.posting );
    //basically the same as account_create_operation
    execution_time_count += _e.create_claimed_account_time;
  }

  void operator()( const decline_voting_rights_operation& op )const
  {
    if( op.decline )
      state_bytes_count += _w.decline_voting_rights_size;
    execution_time_count += _e.decline_voting_rights_time;
  }

  void operator()( const delegate_vesting_shares_operation& op )const
  {
    if( op.vesting_shares.amount > 0 )
      state_bytes_count += _w.delegate_vesting_shares_size;
    else
      state_bytes_count += _w.vesting_delegation_expiration_object_size;
    execution_time_count += _e.delegate_vesting_shares_time;
  }

  void operator()( const escrow_transfer_operation& op )const
  {
    state_bytes_count += _w.escrow_transfer_size;
    execution_time_count += _e.escrow_transfer_time;
  }

  void operator()( const limit_order_create_operation& op )const
  {
    if( !op.fill_or_kill )
      state_bytes_count += _w.limit_order_create_size;
    execution_time_count += _e.limit_order_create_time;
    market_op_count++;
  }

  void operator()( const limit_order_create2_operation& op )const
  {
    if( !op.fill_or_kill )
      state_bytes_count += _w.limit_order_create_size;
    execution_time_count += _e.limit_order_create2_time;
    market_op_count++;
  }

  void operator()( const request_account_recovery_operation& op )const
  {
    if( op.new_owner_authority.weight_threshold != 0 )
      state_bytes_count += _w.request_account_recovery_size;
    execution_time_count += _e.request_account_recovery_time;
  }

  void operator()( const set_withdraw_vesting_route_operation& op )const
  {
    FC_TODO( "Change RC state bytes computation to take SMT's into account" )
    if( op.percent != 0 )
      state_bytes_count += _w.set_withdraw_vesting_route_size;
    execution_time_count += _e.set_withdraw_vesting_route_time;
  }

  void operator()( const vote_operation& op )const
  {
    FC_TODO( "Change RC state bytes computation to take SMT's into account" )
    state_bytes_count += _w.vote_size;
    execution_time_count += _e.vote_time;
  }

  void operator()( const witness_update_operation& op )const
  {
    //TODO: compute differential cost
    state_bytes_count +=
        _w.witness_update_base_size
      + _w.witness_update_url_char_size * op.url.size();
    execution_time_count += _e.witness_update_time;
  }

  void operator()( const transfer_operation& )const
  {
    FC_TODO( "Change RC state bytes computation to take SMT's into account" )
    execution_time_count += _e.transfer_time;
    market_op_count++;
  }

  void operator()( const transfer_to_vesting_operation& )const
  {
    FC_TODO( "Change RC state bytes computation to take SMT's into account" )
    state_bytes_count += _w.transfer_to_vesting_size;
    execution_time_count += _e.transfer_to_vesting_time;
    market_op_count++;
  }

  void operator()( const transfer_to_savings_operation& )const
  {
    FC_TODO( "Change RC state bytes computation to take SMT's into account" )
    execution_time_count += _e.transfer_to_savings_time;
  }

  void operator()( const transfer_from_savings_operation& )const
  {
    FC_TODO( "Change RC state bytes computation to take SMT's into account" )
    state_bytes_count += _w.transfer_from_savings_size;
    execution_time_count += _e.transfer_from_savings_time;
  }

  void operator()( const claim_reward_balance_operation& op )const
  {
    FC_TODO( "Change RC state bytes computation to take SMT's into account" )
    execution_time_count += _e.claim_reward_balance_time;
  }

  void operator()( const withdraw_vesting_operation& op )const
  {
    FC_TODO( "Change RC state bytes computation to take SMT's into account" )
    execution_time_count += _e.withdraw_vesting_time;
  }

  void operator()( const account_update_operation& op )const
  {
    //TODO: compute differential cost
    if( op.owner )
      state_bytes_count += get_authority_dynamic_size( *op.owner ) + _w.owner_authority_history_object_size;
    if( op.active )
      state_bytes_count += get_authority_dynamic_size( *op.active );
    if( op.posting )
      state_bytes_count += get_authority_dynamic_size( *op.posting );
    state_bytes_count += _w.account_json_metadata_char_size * op.json_metadata.size();
    execution_time_count += _e.account_update_time;
  }

  void operator()( const account_update2_operation& op )const
  {
    //TODO: compute differential cost
    if( op.owner )
      state_bytes_count += get_authority_dynamic_size( *op.owner ) + _w.owner_authority_history_object_size;
    if( op.active )
      state_bytes_count += get_authority_dynamic_size( *op.active );
    if( op.posting )
      state_bytes_count += get_authority_dynamic_size( *op.posting );
    state_bytes_count += _w.account_json_metadata_char_size
      * ( op.json_metadata.size() + op.posting_json_metadata.size() );
    execution_time_count += _e.account_update2_time;
  }

  void operator()( const account_witness_proxy_operation& )const
  {
    execution_time_count += _e.account_witness_proxy_time;
  }

  void operator()( const cancel_transfer_from_savings_operation& )const
  {
    execution_time_count += _e.cancel_transfer_from_savings_time;
  }

  void operator()( const change_recovery_account_operation& )const
  {
    state_bytes_count += _w.change_recovery_account_size;
    execution_time_count += _e.change_recovery_account_time;
  }

  void operator()( const claim_account_operation& o )const
  {
    execution_time_count += _e.claim_account_time;
    if( o.fee.amount == 0 )
      new_account_op_count++;
  }

  void operator()( const custom_operation& )const
  {
    //no point in adding state cost of witness_custom_op_object because in only lives up to end of block
    execution_time_count += _e.custom_time;
  }

  void operator()( const custom_json_operation& op )const
  {
    //no point in adding state cost of witness_custom_op_object because in only lives up to end of block
    execution_time_count += _e.custom_json_time;
  }

  void operator()( const custom_binary_operation& o )const
  {
    //no point in adding state cost of witness_custom_op_object because in only lives up to end of block
    //this operation is (soft-fork) obsolete
    execution_time_count += _e.custom_binary_time;
  }

  void operator()( const delete_comment_operation& )const
  {
    execution_time_count += _e.delete_comment_time;
  }

  void operator()( const escrow_approve_operation& )const
  {
    //while approval prolongs escrow_object's life it should be already paid by person that initiated transfer
    execution_time_count += _e.escrow_approve_time;
  }

  void operator()( const escrow_dispute_operation& )const
  {
    execution_time_count += _e.escrow_dispute_time;
  }

  void operator()( const escrow_release_operation& )const
  {
    execution_time_count += _e.escrow_release_time;
  }

  void operator()( const feed_publish_operation& )const
  {
    execution_time_count += _e.feed_publish_time;
  }

  void operator()( const limit_order_cancel_operation& )const
  {
    execution_time_count += _e.limit_order_cancel_time;
  }

  void operator()( const witness_set_properties_operation& )const
  {
    execution_time_count += _e.witness_set_properties_time;
  }

#ifdef HIVE_ENABLE_SMT
  void operator()( const claim_reward_balance2_operation& op )const
  {
    FC_TODO( "Change RC state bytes computation to take SMT's into account" )
    execution_time_count += _e.claim_reward_balance2_time;
  }

  void operator()( const smt_setup_operation& op )const
  {
    FC_TODO( "Change RC state bytes computation to take SMT's into account" )
    execution_time_count += _e.smt_setup_time;
  }

  void operator()( const smt_setup_emissions_operation& op )const
  {
    FC_TODO( "Change RC state bytes computation to take SMT's into account" )
    execution_time_count += _e.smt_setup_emissions_time;
  }

  void operator()( const smt_set_setup_parameters_operation& op )const
  {
    FC_TODO( "Change RC state bytes computation to take SMT's into account" )
    execution_time_count += _e.smt_set_setup_parameters_time;
  }

  void operator()( const smt_set_runtime_parameters_operation& op )const
  {
    FC_TODO( "Change RC state bytes computation to take SMT's into account" )
    execution_time_count += _e.smt_set_runtime_parameters_time;
  }

  void operator()( const smt_create_operation& op )const
  {
    FC_TODO( "Change RC state bytes computation to take SMT's into account" )
    execution_time_count += _e.smt_create_time;
  }

  void operator()( const allowed_vote_assets& )const
  {
    FC_TODO( "Change RC state bytes computation to take SMT's into account" )
  }

  void operator()( const smt_contribute_operation& op ) const
  {
    FC_TODO( "Change RC state bytes computation to take SMT's into account" )
    execution_time_count += _e.smt_contribute_time;
  }
#endif

  void operator()( const create_proposal_operation& op ) const
  {
    FC_ASSERT( op.end_date > _now );
    uint32_t lifetime = ( op.end_date.sec_since_epoch() - _now.sec_since_epoch() + 3600 - 1 ) / 3600; //round up to full hours
    int64_t proposal_size =
        _w.create_proposal_base_size
      + _w.create_proposal_subject_permlink_char_size * ( op.subject.size() + op.permlink.size() );
    state_bytes_count += proposal_size * lifetime;
    execution_time_count += _e.create_proposal_time;
  }

  void operator()( const update_proposal_operation& op ) const
  {
    //state bytes were pretty much paid during creation (since extending end date is forbidden)
    execution_time_count += _e.update_proposal_time;
  }

  void operator()( const update_proposal_votes_operation& op ) const
  {
    //TODO: compute differential cost (?)
    if( op.approve )
      state_bytes_count += _w.update_proposal_votes_member_size * op.proposal_ids.size();
    execution_time_count += _e.update_proposal_votes_time;
  }

  void operator()(const remove_proposal_operation&) const
  {
    execution_time_count += _e.remove_proposal_time;
  }

  void operator()( const recurrent_transfer_operation& op )const
  {
    if( op.amount.amount > 0 )
    {
      uint32_t lifetime = op.recurrence * op.executions; //since recurrence is in hours, so is lifetime
      state_bytes_count += _w.recurrent_transfer_base_size * lifetime;
    }
    execution_time_count += _e.recurrent_transfer_base_time
      + _e.recurrent_transfer_processing_time * op.executions;
    market_op_count++;
  }

  void operator()( const recover_account_operation& op ) const
  {
    subsidized_op = true;
    subsidized_signatures = 2; //needs recent and new signature
    //note: multisig stolen accounts are not subsidized, circumventing cost of setting up
    //new multisig is also not allowed

    //cost still calculated for case when it is not subsidized
    //TODO: compute differential cost
    state_bytes_count += get_authority_dynamic_size( op.new_owner_authority ) + _w.owner_authority_history_object_size;
    execution_time_count += _e.recover_account_time;
  }

  // Time critical or simply operations that were outdated when RC was started in HF20 - no extra cost
  void operator()( const pow_operation& ) const {}
  void operator()( const pow2_operation& ) const {}
  void operator()( const report_over_production_operation& ) const {}
  void operator()( const reset_account_operation& ) const {}
  void operator()( const set_reset_account_operation& ) const {}

  // Virtual Ops (their costs, be it hived or HM, should be added to operations that spawn them)
  void operator()( const virtual_operation& ) const {}

  // Optional Actions
#ifdef IS_TEST_NET
  void operator()( const example_optional_action& ) const {}
#endif


  // TODO:
  // Should following ops be market ops?
  // withdraw_vesting, convert, collateralized_convert, set_withdraw_vesting_route, limit_order_create2
  // escrow_transfer, escrow_dispute, escrow_release, escrow_approve,
  // transfer_to_savings, transfer_from_savings, cancel_transfer_from_savings,
  // claim_reward_balance, delegate_vesting_shares, any SMT operations
};

typedef count_operation_visitor count_optional_action_visitor;

void count_resources(
  const signed_transaction& tx,
  const size_t size,
  count_resources_result& result,
  const fc::time_point_sec now
  )
{
  static const state_object_size_info size_info;
  static const operation_exec_info exec_info;
  const int64_t tx_size = int64_t( size );
  count_operation_visitor vtor( size_info, exec_info, now );

  for( const operation& op : tx.operations )
  {
    op.visit( vtor );
  }

  auto prevent_negative = []( count_resources_result& result )
  {
    for( auto& usage : result )
      if( usage < 0 )
        usage = 0;
  };

  if( vtor.subsidized_op && tx.operations.size() == 1 && tx.signatures.size() <= vtor.subsidized_signatures )
  {
    // transactions with single subsidized operation with normal amount of signatures are free
    // (for now just account recovery operation, but we might have more in the future)
    prevent_negative( result );
    return;
  }
  
  result[ resource_history_bytes ] += tx_size;

  result[ resource_new_accounts ] += vtor.new_account_op_count;

  if( vtor.market_op_count > 0 )
    result[ resource_market_bytes ] += tx_size;

  result[ resource_state_bytes ] += vtor.state_bytes_count
    + size_info.transaction_base_size + size_info.transaction_byte_size * tx_size; //cost of temporary tx storage

  result[ resource_execution_time ] += vtor.execution_time_count
    + exec_info.transaction_time + exec_info.verify_authority_time * tx.signatures.size();

  prevent_negative( result );
}

void count_resources(
  const optional_automated_action& action,
  const size_t size,
  count_resources_result& result,
  const fc::time_point_sec now
){
  static const state_object_size_info size_info;
  static const operation_exec_info exec_info;
  const int64_t action_size = int64_t( size );
  count_optional_action_visitor vtor( size_info, exec_info, now );

  //subsidized operations not expected (but even if they were, we won't subsidize them here, at least for now)

  result[ resource_history_bytes ] += action_size;

  action.visit( vtor );

  // Not expecting actions to create accounts, but better to add it for completeness
  result[ resource_new_accounts ] += vtor.new_account_op_count;

  result[ resource_state_bytes ] += vtor.state_bytes_count;

  result[ resource_execution_time ] += vtor.execution_time_count;

  for( auto& usage : result )
    if( usage < 0 )
      usage = 0;
}

} } } // hive::plugins::rc
