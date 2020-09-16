#include <eosiolib/eosio.hpp>
#include <eosiolib/asset.hpp>
#include <eosiolib/system.hpp>
#include <eosiolib/time.hpp>
#include <eosiolib/multi_index.hpp>
#include <eosiolib/contract.hpp>
#include <eosiolib/crypto.hpp>

class [[eosio::contract]] reg : public eosio::contract {

public:
    reg( eosio::name receiver, eosio::name code, eosio::datastream<const char*> ds ): eosio::contract(receiver, code, ds)
    {}

    [[eosio::action]] void addguest(eosio::name username, eosio::name registrator, eosio::public_key public_key, eosio::asset d_cpu, eosio::asset d_net);
    [[eosio::action]] void update();
    

    static void recieve(eosio::name payer, eosio::name username, eosio::asset quantity, uint64_t code);
    

    void apply(uint64_t receiver, uint64_t code, uint64_t action);
    
    

    static constexpr eosio::name _me = "registrator"_n;
    static const uint64_t _GUEST_EXPIRATION = 1209600; //14 days
    static constexpr eosio::symbol _MIN_SYMBOL = eosio::symbol(eosio::symbol_code("UNIT"),4);

    static const uint64_t _MIN_AMOUNT = 10000; 
        
    static const uint64_t _BASKET = 3;
    
    
    


    static uint128_t combine_ids(const uint64_t &x, const uint64_t &y) {
        return (uint128_t{x} << 64) | y;
    };

   struct permission_level_weight {
      eosio::permission_level  permission;
      uint16_t          weight;

      // explicit serialization macro is not necessary, used here only to improve compilation time
      EOSLIB_SERIALIZE( permission_level_weight, (permission)(weight) )
   };

   struct key_weight {
      eosio::public_key  key;
      uint16_t           weight;

      // explicit serialization macro is not necessary, used here only to improve compilation time
      EOSLIB_SERIALIZE( key_weight, (key)(weight) )
   };

   struct wait_weight {
      uint32_t           wait_sec;
      uint16_t           weight;

      // explicit serialization macro is not necessary, used here only to improve compilation time
      EOSLIB_SERIALIZE( wait_weight, (wait_sec)(weight) )
   };

   struct authority {
      uint32_t                              threshold = 0;
      std::vector<key_weight>               keys;
      std::vector<permission_level_weight>  accounts;
      std::vector<wait_weight>              waits;

      // explicit serialization macro is not necessary, used here only to improve compilation time
      EOSLIB_SERIALIZE( authority, (threshold)(keys)(accounts)(waits) )
   };

    struct [[eosio::table]] guests {
        eosio::name username;
        
        eosio::name registrator;
        eosio::public_key public_key;
        eosio::asset d_cpu;
        eosio::asset d_net;

        eosio::time_point_sec expiration;

        eosio::asset to_pay;
        
        uint64_t primary_key() const {return username.value;}
        uint64_t byexpr() const {return expiration.sec_since_epoch();}

        EOSLIB_SERIALIZE(guests, (username)(registrator)(public_key)(d_cpu)(d_net)(expiration)(to_pay))
    };

    typedef eosio::multi_index<"guests"_n, guests,
       eosio::indexed_by< "byexpr"_n, eosio::const_mem_fun<guests, uint64_t, 
                      &guests::byexpr>>
    > guests_index;


    struct [[eosio::table]] reserved {
        eosio::name username;
        eosio::name registrator;

        uint64_t primary_key() const {return username.value;}

        EOSLIB_SERIALIZE(reserved, (username)(registrator))
    };

    typedef eosio::multi_index<"reserved"_n, reserved> reserved_index;
 


    struct [[eosio::table]] lifetimed {
        eosio::name username;
        eosio::name registrator;

        uint64_t primary_key() const {return username.value;}

        EOSLIB_SERIALIZE(lifetimed, (username)(registrator))
    };

    typedef eosio::multi_index<"lifetimed"_n, lifetimed> lifetimed_index;
 


};
