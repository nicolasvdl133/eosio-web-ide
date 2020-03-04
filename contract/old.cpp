#include <eosio/eosio.hpp>
#include <eosio/system.hpp>
#include <eosio/multi_index.hpp>



// --------------------------------------The contract -------------------------------------------------
using namespace eosio;

class mechelenbon : eosio::contract {
  public:
    // Use contract's constructor
    using contract::contract;
    mechelenbon( name self, name code, datastream<const char*> ds ):
    contract(self, code, ds),
    _bonnen(self, self.value)
      {}


    
    // EEN HANDELAAR TOEVOEGEN
    [[eosio::action]] void handelaaradd(uint64_t id, eosio::name user) {
         handelaar_table table{get_self(), 0};

        // Check user
        require_auth(eosio::name("mechelenbon")); // tabel met users die handelaars kunnen toevoegen

        // Create an ID if user didn't specify one
        eosio::check(id < 1'000'000'000ull, "user-specified id is too big");
        if (!id)
            id = std::max(table.available_primary_key(), 1'000'000'000ull);

        // handelaar record toevoegen
        table.emplace(get_self(), [&](auto& handelaar) {
            handelaar.id       = id;
            handelaar.user     = user;
        });
    }


        // EEN BON AANMAKEN
    [[eosio::action]] void createbon( eosio::name buyer, eosio::name beneficiary, double amount) {
        // bon table{get_self(), 0};

        // Check user
        require_auth(eosio::name("mechelenbon"));// tabel met users die bonnen kunne aanmaken


        eosio::check(amount > 5, "user-specified amount is too low");
        // Create an ID if user didn't specify one ( !!!! id aanpassen)
        //eosio::check(id < 1'000'000'000ull, "user-specified id is too big");
        //if (!id)
        uint64_t id = std::max(_bonnen.available_primary_key(), 1'000'000'000ull);

        // bon record toevoegen
        _bonnen.emplace(get_self(), [&](auto& bon) {
            bon.id       = id;
            bon.buyer     = buyer;
            bon.beneficiary = beneficiary;
            bon.date  = eosio::time_point_sec(eosio::current_time_point());
            bon.amount = amount;
            bon.accepted  = true;
        });
    }
    
    [[eosio::action]] void acceptbon(eosio::name user , uint64_t bonid) {
    
        
        require_auth(user);
        //bon table{get_self(), 0};
        //controle of bon al reeds geacepteerd is
        auto iterator = _bonnen.find(bonid);
        _bonnen.modify(iterator, user, [&]( auto& row ) {
            row.accepted = true;
        });
    }




/*

    // een bon gebruiken bij een handelaar
    [[eosio::action]] void usebon(double amount, eosio::name user, eosio::name reciver) {
        //bon verminderen indien bon niet genoeg is ? volgende bon met laagste datum gebruiken
        //bon toevoegen in tabel gebruikte bonnen bij handelaar
        require_auth(user);
        

        auto iterator = _bonnen.get_index(name("bonnen"));
        auto slitr = iterator.lower_bound(0);

        
        while(slitr != iterator.end & amount != 0){
            print(iterator)
        }


        //table.modify(iterator, user, [&]( auto& row ) {
          //row.amount =  row.amount - amount;
        //});

    }
    */
private:
    //----------------------------handelaar tabel--------------------------------------------------------
    struct [[eosio::table("handelaar"), eosio::contract("mechelenbon")]] handelaar {
        uint64_t    id       = {}; // Non-0
        eosio::name user     = {};

        uint64_t primary_key() const { return id; }

    };

    using handelaar_table = eosio::multi_index<
        "handelaar"_n, handelaar>;   // , eosio::indexed_by<"byid"_n, const_mem_fun<id, uint64_t, &id::primary_key>
    //-----------------------------------------------------------------------------------------------------

    //----------------------------bonnen tabel-------------------------------------------------------------
    struct [[eosio::table("bonnen"), eosio::contract("mechelenbon")]] bon {
        uint64_t                id          = {}; // Non-0
        double                  amount      = {};
        eosio::time_point_sec   date        = {};
        bool                    accepted    = {};
        eosio::name             beneficiary = {};
        eosio::name             buyer       = {};

        uint64_t primary_key() const { return id; }
        uint64_t get_beneficiary()const { return beneficiary.value; }
        uint64_t get_date()const { return date.utc_seconds; }


    };

    //using bon_table = eosio::multi_index<"bon"_n, bon>;   // , eosio::indexed_by<"byid"_n, const_mem_fun<id, uint64_t, &id::primary_key>

    typedef eosio::multi_index<
        "bonnen"_n, bon,
        eosio::indexed_by<"beneficiary"_n, eosio::const_mem_fun<bon, uint64_t, &bon::get_beneficiary>>,
        eosio::indexed_by<"date"_n, eosio::const_mem_fun<bon, uint64_t, &bon::get_date>>> bonnen;


    bonnen _bonnen;

    //-----------------------------------------------------------------------------------------------------


    //----------------------------gebruikte bonnen tabel-------------------------------------------------------------
    struct [[eosio::table("usedbon"), eosio::contract("mechelenbon")]] usedbon {
        uint64_t                id          = {}; // Non-0
        uint64_t                bonid       = {}; // Non-0
        double                  amount      = {}; // used amount
        eosio::time_point_sec   date        = {}; // date of use
        eosio::name             beneficiary = {};
        

        uint64_t primary_key() const { return id; }

    };

    using usedbon_table = eosio::multi_index<
        "usedbon"_n, usedbon>;   // , eosio::indexed_by<"byid"_n, const_mem_fun<id, uint64_t, &id::primary_key>

    //-----------------------------------------------------------------------------------------------------



};

