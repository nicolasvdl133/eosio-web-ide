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
    _bonnen(self, self.value)
      {}


    ACTION createbon( eosio::name buyer, eosio::name beneficiary, double amount)
    {
         // Check user
        require_auth(eosio::name("mechelenbon")); // tabel met users die bonnen kunne aanmaken
       
        eosio::check(amount > 5, "user-specified amount is too low");    
        uint64_t id = std::max(_bonnen.available_primary_key(), 1'000'000'000ull);

        print(id);

        auto idx = _bonnen.emplace(_self, [&](auto& bon) {
            bon.id       = id;
            bon.buyer     = buyer;
            bon.beneficiary = beneficiary;
            bon.date  = eosio::time_point_sec(eosio::current_time_point());
            bon.amount = amount;
            bon.accepted  = true;
        });

    }



    ACTION spendbon(eosio::name spender, eosio::name reciver, double amount)
    {
         // Check user
        require_auth(spender); // tabel met users die bonnen kunne aanmaken
        
        bool delbon = true;
        while(delbon){
            auto iterator = _bonnen.get_index<name("beneficiary")>();
            auto aditr = iterator.lower_bound(spender.value);
            delbon = false;
            print("ok1");
            check(aditr != iterator.end() || aditr->beneficiary == spender, "user doesnt have any bons to spend");

            if( aditr == iterator.end() || aditr->beneficiary != spender ) {
                
                }else {
                    print("ok2");
                    iterator.modify(aditr, spender, [&]( auto& row ){
                    
                        if(row.amount<amount){
                            amount -= row.amount; 
                            delbon = true;
                            print("ok3");
                        }
                        else {
                            row.amount = row.amount - amount;
                            amount=0;
                            print("ok4");
                        }
                    
                    });   
                    if(delbon){
                        iterator.erase(aditr);
                        print("ok5");
                    }
                }

            print("ok6");

        }







       /*bool nxtbn;
       auto iterator = _bonnen.get_index<name("beneficiary")>();
       auto aditr = iterator.lower_bound(spender.value);
       //print(spender.value);

       check(aditr != iterator.end() || aditr->beneficiary == spender, "user doesnt have any bons to spend");



       if( aditr == iterator.end() || aditr->beneficiary != spender ) {

           print("not found");
       }
       else{
           while(amount!=0){

            


               //amount = deprbon(spender, reciver, amount);
           }
            


       }

        print(amount);

       //check(iterator != _bonnen.end(), "Record does not exist");
       //_bonnen.erase(iterator);*/

    
    }



    
    private:

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
        name("bonnen"), bon,
        eosio::indexed_by<name("beneficiary"), eosio::const_mem_fun<bon, uint64_t, &bon::get_beneficiary>>,
        eosio::indexed_by<name("date"), eosio::const_mem_fun<bon, uint64_t, &bon::get_date>>> bonnen;


    bonnen _bonnen;

    //-----------------------------------------------------------------------------------------------------


    //---------------------------------------------functies---------------------------------------------------

    double deprbon(eosio::name spender, eosio::name reciver, double amount){
                double delbon;
                auto iterator = _bonnen.get_index<name("beneficiary")>();
                auto aditr = iterator.lower_bound(spender.value);


                if( aditr == iterator.end() || aditr->beneficiary != spender ) {

                    iterator.modify(aditr, spender, [&]( auto& row ){
                    
                        if(row.amount<amount){
                            amount -= row.amount; 
                            delbon = true;
                        }
                        else {
                            row.amount = row.amount - amount;
                        }
                    
                    });   
                    if(delbon){
                        iterator.erase(aditr);
                        return amount;
                    }else {
                    
                    return 0;
                    }
                }
                return 0;
    }



};