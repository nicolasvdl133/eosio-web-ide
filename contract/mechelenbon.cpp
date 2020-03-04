#include <eosio/eosio.hpp>
#include <eosio/system.hpp>
#include <eosio/multi_index.hpp>

using namespace eosio;

class mechelenbon : eosio::contract {
  public:
    // Use contract's constructor
    using contract::contract;
    mechelenbon( name self, name code, datastream<const char*> ds ):
    contract(self, code, ds),
    _bonnen(self, self.value),
    _handelaars(self, self.value)
      {}

    // EEN HANDELAAR TOEVOEGEN  
    [[eosio::action]] void handelaaradd(uint64_t id, eosio::name user) {

        require_auth(eosio::name("mechelenbon")); // Check user
        check(is_account(user), "handelaar account does not exist");// check if account exist
        auto iterator = _handelaars.get_index<name("user")>(); // via user index handelaar terug vinden
        auto aditr = iterator.lower_bound(user.value);

        check(aditr == iterator.end() || aditr->user != user, "user is already in table handelaars"); //controle of handelaar al in tbl zit
        eosio::check(id < 1'000ull, "user-specified id is too big");         // Create an ID if user didn't specify one
        if (!id)
            id = std::max(_handelaars.available_primary_key(), 1'000ull);

        _handelaars.emplace(_self, [&](auto& handelaar) {         // handelaar record toevoegen
            handelaar.id       = id;
            handelaar.user     = user;
        });
    }

    //EEN BON ACCEPTEREN
    [[eosio::action]] void acceptbon(eosio::name user , uint64_t bonid) {
        require_auth(user);
        auto iterator = _bonnen.find(bonid);
        check(iterator->beneficiary == user, "you dont have the richt to accept this bon");
        check(iterator->accepted == false, "bon is already accepted");
        _bonnen.modify(iterator, user, [&]( auto& row ) {
            row.accepted = true;
        });
    }

    //EEN BON AANMAKEN
    ACTION createbon( eosio::name buyer, eosio::name beneficiary, double amount)
    {
        require_auth(eosio::name("mechelenbon")); //Check user
        check(is_account(buyer), "buyer account does not exist");
        check(is_account(beneficiary), "beneficiary account does not exist");
        eosio::check(amount > 5, "user-specified amount is too low");    
        uint64_t id = std::max(_bonnen.available_primary_key(), 1ull);
        auto idx = _bonnen.emplace(_self, [&](auto& bon) {
            bon.id          = id;
            bon.buyer       = buyer;
            bon.beneficiary = beneficiary;
            bon.date        = eosio::time_point_sec(eosio::current_time_point());
            bon.amount      = amount;
            bon.accepted    = true;
        });
    }

    //EEN BON UITGEVEN
    ACTION spendbon(eosio::name spender, eosio::name reciver, double amount)
    {
        require_auth(spender); // Check user auth
        check(is_account(reciver), "reciver account does not exist"); // Check account exist
        check(is_account(spender), "spender account does not exist");  

        usedbon_table _usedbon{get_self(), 0}; // tbl usedbon intantieren
        auto iterator = _handelaars.get_index<name("user")>(); // via user index handelaar terug vinden
        auto aditr = iterator.lower_bound(reciver.value);
        check(aditr != iterator.end() || aditr->user == reciver, "payment reciver needs to be in the table handelaars"); // check if it not at end and reciver is handelaar
        double usedamount = amount;
        bool delbon = true;
        while(delbon){
            auto iterator = _bonnen.get_index<name("beneficiary")>();
            auto aditr = iterator.lower_bound(spender.value);
            delbon = false;
            print("debug:entered loop");
            check(aditr != iterator.end() || aditr->beneficiary == spender, "user doesnt have any bons to spend");
            check(aditr->accepted == true, "you first need to accept the bon before you can spend it");
            if( aditr == iterator.end() & aditr->beneficiary != spender ) {
                }else {
                    print("debug: enter modify");
                    iterator.modify(aditr, spender, [&]( auto& row ){
                    uint64_t id = std::max(_usedbon.available_primary_key(), 1ull);
                        if(row.amount<=usedamount){
                            usedamount -= row.amount; 
                            delbon = true;
                            _usedbon.emplace(get_self(), [&](auto& usedbon) {
                                usedbon.id          = id;
                                usedbon.bonid       = row.id; // Non-0
                                usedbon.amount      = row.amount; // used amount
                                usedbon.date        = eosio::time_point_sec(eosio::current_time_point()); // date of use
                                usedbon.beneficiary = reciver;
                            });
                            print("debug: entered bon < amount");
                        }
                        else {
                            row.amount = row.amount - usedamount;
                             _usedbon.emplace(get_self(), [&](auto& usedbon) {
                                usedbon.id          = id;
                                usedbon.bonid       = row.id; // Non-0
                                usedbon.amount      = usedamount; // used amount
                                usedbon.date        = eosio::time_point_sec(eosio::current_time_point()); // date of use
                                usedbon.beneficiary = reciver;
                            });
                            usedamount=0; // FIX
                            print("debug: entered bon > amount");
                        }
                    });   
                    if(delbon){
                        iterator.erase(aditr);
                        print("debug: entered bon delete");
                    }
                }
        }
        print("debug: payment end");
    }

    //EEN BON GEVEN
    ACTION givebon(eosio::name giver,eosio::name benef , uint64_t bonid) {
        require_auth(giver);
        check(is_account(benef), "beneficiary account does not exist");
        check(is_account(giver), "giver account does not exist");
        auto iterator = _bonnen.find(bonid);
        check(iterator->beneficiary == giver, "you dont have the rights to that bon");
        check(iterator->accepted == true, "you first need to accept the bon before you can give it to someone else");
        _bonnen.modify(iterator, giver, [&]( auto& row ) { 
            if(row.beneficiary == giver){ //overbodig
                row.beneficiary = benef;
                row.accepted = false;
            }
        });
    }

    //----------------------------tables-------------------------------------------------------------
    private:
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
    typedef eosio::multi_index<
        name("bonnen"), bon,
        eosio::indexed_by<name("beneficiary"), eosio::const_mem_fun<bon, uint64_t, &bon::get_beneficiary>>,
        eosio::indexed_by<name("date"), eosio::const_mem_fun<bon, uint64_t, &bon::get_date>>> bonnen;
    bonnen _bonnen;

    struct [[eosio::table("handelaars"), eosio::contract("mechelenbon")]] handelaar {
        uint64_t    id       = {}; // Non-0
        eosio::name user     = {};

        uint64_t primary_key() const { return id; }
        uint64_t get_handelaar()const { return user.value; }
    };
    typedef eosio::multi_index<
        name("handelaars"), handelaar,
        eosio::indexed_by<name("user"), eosio::const_mem_fun<handelaar, uint64_t, &handelaar::get_handelaar>>
        > handelaars;
    handelaars _handelaars; 
    
    struct [[eosio::table("usedbon"), eosio::contract("mechelenbon")]] usedbon {
        uint64_t                id          = {}; // Non-0
        uint64_t                bonid       = {}; // Non-0
        double                  amount      = {}; // used amount
        eosio::time_point_sec   date        = {}; // date of use
        eosio::name             beneficiary = {};
        uint64_t primary_key() const { return id; }
    };
    using usedbon_table = eosio::multi_index<
        "usedbon"_n, usedbon>;
};