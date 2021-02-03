#pragma once

#include <eosio/asset.hpp>

namespace mindswap {

    using namespace eosio;

    const name id = "mindswap"_n;
    const name code = "mindswapswap"_n;
    const std::string description = "Mindswap Converter";

    /**
     * pair
     */
    struct [[table]] stat_row {
        asset           supply;
        asset           max_supply;
        name            issuer;
        extended_asset  pool1;
        extended_asset  pool2;
        int32_t         fee;
        name            fee_contract;
        uint64_t        curve;

        uint64_t primary_key() const { return supply.symbol.code().raw(); }
    };
    typedef multi_index< "stat"_n, stat_row > stat;


    /**
     * ## STATIC `get_amount_out`
     *
     * Given an input amount of an asset and pair id, returns the calculated return
     *
     * ### params
     *
     * - `{asset} in` - input amount
     * - `{symbol} out_sym` - out symbol
     * - `{string} pair_id` - pair_id
     *
     * ### example
     *
     * ```c++
     * // Inputs
     * const asset in = asset { 10000, "EOS" };
     * const symbol out_sym = symbol {"IQ,3"};
     * const string pair_id = "EOSIQ"
     *
     * // Calculation
     * const uint64_t amount_out = mindswap::get_amount_out( in, out_sym, pair_id );
     * // => 9996
     * ```
     */
    static asset get_amount_out( const asset in, symbol out_sym, const string& pair_id )
    {
        check(in.amount > 0, "MindswapLibrary: INSUFFICIENT_INPUT_AMOUNT");

        symbol_code liq_symcode {pair_id};
        check(liq_symcode.is_valid(), "MindswapLibrary: Invalid pair_id");

        mindswap::stat _stat( mindswap::code, liq_symcode.raw() );
        auto row = _stat.get(liq_symcode.raw(), "MindswapLibrary: Invalid Mindswap symbol");

        if(row.pool1.quantity.symbol != in.symbol) std::swap(row.pool1, row.pool2);
        check( row.pool1.quantity.symbol == in.symbol && row.pool2.quantity.symbol == out_sym, "MindswapLibrary: invalid pair");
        check( row.pool1.quantity.amount > 0 && row.pool2.quantity.amount > 0, "MindswapLibrary: INSUFFICIENT_LIQUIDITY");

        auto am_in = in.amount;
        auto res_in = row.pool1.quantity.amount + in.amount;
        auto res_out = row.pool2.quantity.amount;

        auto am_out = am_in * res_out / res_in;
        auto fee = (9999 + am_out * row.fee)/10000;
        am_out -= fee;

        check(am_out > 0, "MindswapLibrary: wrong return");

        return asset { am_out, out_sym };
    }
}