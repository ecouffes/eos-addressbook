#include <eosiolib/eosio.hpp>

using std::string;
using namespace eosio;

class [[eosio::contract]] addressbook_history : public eosio::contract {
    public:
        // constructor
        // 親クラスeosio::contractのコンストラクタを初期化
        addressbook_history(name receiver, name code, datastream<const char *> ds) : contract(receiver, code, ds){}

        // マルチインデックステーブルにデータを追加
        [[eosio::action]]
        // history_typeはcreateとかupdateとか
        void create(name user, uint64_t addressbook_id, string history_type, string history_content) {
            require_auth(name{"addressbook"});
//            require_auth(name{"eosaddressbk"});

            // マルチインデックステーブルのインスタンスを作成
            // 第一引数：_code スマートコントラクトのアカウント
            // 第二引数：_code.value スコープ
            history_index histories(_code, _code.value);

            histories.emplace(_code, [&](auto &row) {
                row.id = histories.available_primary_key();
                row.addressbook_id = addressbook_id;
                row.user = user.to_string();
                row.history_type = history_type;
                row.history_content = history_content;
            });
        }

        // マルチインデックステーブルのデータ削除
        [[eosio::action]]
        void destroy(uint64_t id) {
            require_auth(name{"addressbook"});
//            require_auth(name{"eosaddressbk"});

            history_index histories(_code, _code.value);
            auto iterator = histories.find(id);
            eosio_assert(iterator != histories.end(), "Record does not exist");

            histories.erase(iterator);
        }

    private:
        // マルチインデックステーブルを構造体で定義
        // 構造体の構造についてはアップデート不可
        struct [[eosio::table]] history {
            uint64_t id;
            uint64_t addressbook_id;
            string user;
            string history_type;
            string history_content;

            auto primary_key() const { return id; }
        };

        // typedef 既存の型名 新規の型名
        // typedef eosio::multi_index<"テーブル名"_n, 構造体型名> 新しい型名
        typedef eosio::multi_index<"histories"_n, history> history_index;
};

EOSIO_DISPATCH(addressbook_history, (create)(destroy))
