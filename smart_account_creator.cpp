#include "include/smart_account_creator.hpp"

using namespace eosio;

class sac : public contract {
public:
  sac(account_name self) : eosio::contract(self) {}


  void transfer(const account_name sender, const account_name receiver) {
    const auto transfer = unpack_action_data<currency::transfer>();
    if (transfer.from == _self || transfer.to != _self) {
      // This is an outgoing transfer, do nothing.
      return;
    }

    // Parse Memo: Memo must have format "account_name-owner_key(-active_key)"
    eosio_assert(transfer.quantity.symbol == CORE_SYMBOL, "Must be CORE_SYMBOL");
    eosio_assert(transfer.quantity.is_valid(), "Invalid token transfer");
    eosio_assert(transfer.quantity.amount > 0, "Quantity must be positive");

    eosio_assert(transfer.memo.length() == 120 || transfer.memo.length() == 66, "Malformed Memo (not right length)");
    const string account_string = transfer.memo.substr(0, 12);
    const account_name account_to_create = string_to_name(account_string.c_str());
    eosio_assert(transfer.memo[12] == ':' || transfer.memo[12] == '-', "Malformed Memo [12] == : or -");

    const string owner_key_str = transfer.memo.substr(13, 53);
    string active_key_str;
    if(transfer.memo[66] == ':' || transfer.memo[66] == '-') {
      active_key_str = transfer.memo.substr(67, 53);  // Active key provided.
    } else {
      active_key_str = owner_key_str;  // Active key becomes the same as owner.
    }

    const auto owner_pubkey  = abieos::string_to_public_key(owner_key_str);
    const auto active_pubkey = abieos::string_to_public_key(active_key_str);

    const auto owner_auth  = authority{  1, {{owner_pubkey,  1}}, {}, {}  };
    const auto active_auth = authority{  1, {{active_pubkey, 1}}, {}, {}  };

    const auto amount = rambytes_price(3 * 1024);
    const auto ram_replace = rambytes_price(256);
    const auto cpu = asset(1000);
    const auto net = asset(1000);

    const auto remaining_balance = transfer.quantity - cpu - net - amount - ram_replace;

    eosio_assert(remaining_balance.amount >= 0, "Not enough money");

    // Create account.
    INLINE_ACTION_SENDER(call::eosio, newaccount)
      (N(eosio), {{_self, N(active)}},
      {_self, account_to_create, owner_auth, active_auth});

    // Buy ram for account.
    INLINE_ACTION_SENDER(eosiosystem::system_contract, buyram)
      (N(eosio), {{_self, N(active)}},
      {_self, account_to_create, amount});

    // Replace lost ram.
    INLINE_ACTION_SENDER(eosiosystem::system_contract, buyram)
      (N(eosio), {{_self, N(active)}},
      {_self, _self, ram_replace});

    // Delegate and transfer cpu and net.
    INLINE_ACTION_SENDER(eosiosystem::system_contract, delegatebw)
      (N(eosio), {{_self, N(active)}},
      {_self, account_to_create, net, cpu, 1});

    if (remaining_balance.amount > 0) {
      // Transfer remaining balance to new account.
      INLINE_ACTION_SENDER(eosio::token, transfer)
        (N(eosio.token), {{_self, N(active)}},
        {_self, account_to_create, remaining_balance, std::string("Initial balance")});
    }
  }
};


#define EOSIO_ABI_EX(TYPE, MEMBERS)                                            \
  extern "C" {                                                                 \
  void apply(uint64_t receiver, uint64_t code, uint64_t action) {              \
    if (action == N(onerror)) {                                                \
      /* onerror is only valid if it is for the "eosio" code account and       \
       * authorized by "eosio"'s "active permission */                         \
      eosio_assert(code == N(eosio), "onerror action's are only valid from "   \
                                     "the \"eosio\" system account");          \
    }                                                                          \
    auto self = receiver;                                                      \
    if (code == self || code == N(eosio.token) || action == N(onerror)) {      \
      TYPE thiscontract(self);                                                 \
      switch (action) { EOSIO_API(TYPE, MEMBERS) }                             \
      /* does not allow destructor of thiscontract to run: eosio_exit(0); */   \
    }                                                                          \
  }                                                                            \
  }

EOSIO_ABI_EX(sac, (transfer))
