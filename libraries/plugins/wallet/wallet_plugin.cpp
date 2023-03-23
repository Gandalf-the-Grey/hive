#include <hive/plugins/wallet/wallet_plugin.hpp>

#include <hive/chain/database_exceptions.hpp>

#include <boost/filesystem.hpp>
#include <chrono>

#include <fc/io/json.hpp>

namespace hive { namespace plugins { namespace wallet {

namespace bpo = boost::program_options;
namespace bfs = boost::filesystem;

wallet_plugin::wallet_plugin() {}

wallet_manager& wallet_plugin::get_wallet_manager() {
   return *wallet_manager_ptr;
}

void wallet_plugin::set_program_options(boost::program_options::options_description& cli, boost::program_options::options_description& cfg) {
   cfg.add_options()
         ("wallet-dir", bpo::value<boost::filesystem::path>()->default_value("."),
          "The path of the wallet files (absolute path or relative to application data dir)")
         ("unlock-timeout", bpo::value<int64_t>()->default_value(900),
          "Timeout for unlocked wallet in seconds (default 900 (15 minutes)). "
          "Wallets will automatically lock after specified number of seconds of inactivity. "
          "Activity is defined as any wallet command e.g. list-wallets.")
         ("yubihsm-url", bpo::value<string>()->value_name("URL"),
          "Override default URL of http://localhost:12345 for connecting to yubihsm-connector")
         ("yubihsm-authkey", bpo::value<uint16_t>()->value_name("key_num"),
          "Enables YubiHSM support using given Authkey")
         ;
}

void wallet_plugin::plugin_initialize(const boost::program_options::variables_map& options) {
   ilog("initializing wallet plugin");
   try {
      wallet_manager_ptr = std::make_unique<wallet_manager>();

      if (options.count("wallet-dir")) {
         auto dir = options.at("wallet-dir").as<boost::filesystem::path>();
         if (dir.is_relative())
            dir = appbase::app().data_dir() / dir;
         if( !bfs::exists(dir) )
            bfs::create_directories(dir);
         wallet_manager_ptr->set_dir(dir);
      }
      if (options.count("unlock-timeout")) {
         auto timeout = options.at("unlock-timeout").as<int64_t>();
         HIVE_ASSERT(timeout > 0, hive::chain::invalid_lock_timeout_exception, "Please specify a positive timeout ${t}", ("t", timeout));
         std::chrono::seconds t(timeout);
         wallet_manager_ptr->set_timeout(t);
      }
   } FC_LOG_AND_RETHROW()
}

}}}
