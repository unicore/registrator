#include "registrator.hpp"

using namespace eosio;


  [[eosio::action]] void reg::addguest(eosio::name username, eosio::name registrator, eosio::public_key public_key, eosio::asset d_cpu, eosio::asset d_net){
    require_auth(registrator);
    
    guests_index guests(_me, _me.value);
    
    auto guest = guests.find(username.value);
    
    eosio::check(d_net.symbol == _MIN_SYMBOL, "Wrong symbol");
    eosio::check(d_cpu.symbol == _MIN_SYMBOL, "Wrong symbol");
    
    eosio::check( is_account( username ), "User account does not exist");

    reserved_index reserve(_me, _me.value);
    auto raccount = reserve.find(username.value);
    
    eosio::check(raccount == reserve.end(), "account has been already registered throw registrator community");
    eosio::check(guest == guests.end(), "account already registered");
    
    guests.emplace(registrator, [&](auto &a){
      a.username = username;
      a.registrator = registrator;
      a.expiration = eosio::time_point_sec(now() + _GUEST_EXPIRATION);
      a.to_pay = eosio::asset(_MIN_AMOUNT, _MIN_SYMBOL);
      a.public_key = public_key;
      a.d_cpu = d_cpu;
      a.d_net = d_net;
    });

  }


  [[eosio::action]] void reg::update(){
    
    guests_index guests(_me, _me.value);

    auto guests_by_exp = guests.template get_index<"byexpr"_n>();

    auto guest = guests_by_exp.begin();

    uint64_t count = 0;
    std::vector<eosio::name> list_for_delete;
    bool not_expired = false;

    while (guest != guests_by_exp.end() && (count < _BASKET) && (!not_expired) ){
      if (guest->expiration < eosio::time_point_sec(now())){
        authority newauth;
        newauth.threshold = 1;
        eosio::permission_level permission(_me, eosio::name("eosio.code"));
        permission_level_weight accountpermission{permission, 1};
        newauth.accounts.emplace_back(accountpermission);

        //Change active authority of guest to registrator@eosio.code
        eosio::action(eosio::permission_level(guest->username, eosio::name("owner")), 
          eosio::name("eosio"), eosio::name("updateauth"), std::tuple(guest->username, 
            eosio::name("active"), eosio::name("owner"), newauth) 
        ).send();
        

        list_for_delete.push_back(guest->username);
        
             
      } else {
        not_expired = true;
      }

      count++;
      guest++;   
    }

    for (auto username : list_for_delete){
      auto guest_for_delete = guests.find(username.value);
      

      reserved_index reserve(_me, _me.value);
      
      reserve.emplace(_me, [&](auto &r){
        r.username = username;
        r.registrator = guest_for_delete->registrator;
      });

      guests.erase(guest_for_delete);

    }
  }


  void reg::recieve(eosio::name payer, eosio::name username, eosio::asset quantity, uint64_t code){
    require_auth(payer);

    eosio::check(code == "eosio.token"_n.value, "Only system token contract is available for buy guests now;");

    guests_index guests(_me, _me.value);
    auto guest = guests.find(username.value);

    eosio::check((guest->to_pay).amount == quantity.amount, "Wrong amount");
    eosio::check((guest->to_pay).symbol == quantity.symbol, "Wrong symbol");
        
    authority newauth;
    newauth.threshold = 1;
    
    key_weight k_weight{guest->public_key, 1};
    
    newauth.keys.emplace_back(k_weight);

    eosio::action(eosio::permission_level(guest->username, eosio::name("owner")), eosio::name("eosio"), eosio::name("updateauth"), std::tuple(eosio::name(guest->username), eosio::name("owner"), eosio::name(""), newauth) ).send();

    lifetimed_index lifetimed(_me, _me.value);
    lifetimed.emplace(_me, [&](auto &l){
      l.username = username;
      l.registrator = guest -> registrator;
    });

    guests.erase(guest);
  }


extern "C" {
   
   /// The apply method implements the dispatch of events to this contract
   void apply( uint64_t receiver, uint64_t code, uint64_t action ) {
        if (code == reg::_me.value) {
          if (action == "addguest"_n.value){
            execute_action(name(receiver), name(code), &reg::addguest);
          } else if (action == "update"_n.value){
            execute_action(name(receiver), name(code), &reg::update);
          }    
        } else {
          if (action == "transfer"_n.value){
            
            struct transfer{
                eosio::name from;
                eosio::name to;
                eosio::asset quantity;
                std::string memo;
            };

            auto op = eosio::unpack_action_data<transfer>();

            if (op.to == reg::_me){
              eosio::name username = eosio::name(op.memo.c_str());
              
              reg::recieve(op.from, username, op.quantity, code);
            }
          }
        }
  };
};
