#include <eosiolib/eosio.hpp>

using std::string;
using namespace eosio;

class [[eosio::contract]] addressbook : public eosio::contract {
    public:
        // constructor
        // 親クラスeosio::contractのコンストラクタを初期化
        addressbook(name receiver, name code, datastream<const char *> ds) : contract(receiver, code, ds) {}

        // マルチインデックステーブルにデータを追加
        [[eosio::action]]
        void create(name user, string name, string address, string tel) {
            require_auth(user);

            // マルチインデックステーブルのインスタンスを作成
            // 第一引数：_code スマートコントラクトのアカウント
            // 第二引数：user.value スコープ
            address_index addresses(_code, user.value);
            auto new_id = addresses.available_primary_key();

            // emplace関数を呼び出してデータを作成
            // 第一引数：処理に必要なRAMを負担するアカウントを指定（今回はエンドユーザー）
            // 第二引数：コールバック関数
            addresses.emplace(user, [&]( auto& row ) {
                row.id = new_id;
                row.name = name;
                row.address = address;
                row.tel = tel;
            });

            create_history(user, new_id, "create", name + "|" + address + "|" + tel);
        }

        // マルチインデックステーブルのデータ更新
        [[eosio::action]]
        void update(name user, uint64_t id, string name, string address, string tel) {
            require_auth(user);

            // マルチインデックステーブルのインスタンスを作成
            address_index addresses(_code, user.value);
            auto iterator = addresses.find(id);

            // 該当するデータがなかった場合は、iteratorがend()になる。
            eosio_assert(iterator != addresses.end(), "Record does not exist");

            // modify関数を呼び出してデータを更新
            addresses.modify(iterator, user, [&](auto &row) {
                row.name = name;
                row.address = address;
                row.tel = tel;
            });
        }

        // マルチインデックステーブルのデータ削除
        [[eosio::action]]
        void destroy(name user, uint64_t id) {
            require_auth(user);

            // マルチインデックステーブルのインスタンスを作成
            address_index addresses(_code, user.value);
            auto iterator = addresses.find(id);
            eosio_assert(iterator != addresses.end(), "Record does not exist");

            addresses.erase(iterator);
        }

    private:
        // マルチインデックステーブルを構造体で定義
        // 構造体の構造についてはアップデート不可
        struct [[eosio::table]] person {
            uint64_t id;
            string name;
            string address;
            string tel;

            auto primary_key() const { return id; }
        };

        // typedef 既存の型名 新規の型名
        // typedef eosio::multi_index<"名前"_n, 構造体型名> 新しい型名
        typedef eosio::multi_index<"people"_n, person> address_index;

        // addressbook_hisotory.cpp の createアクションを呼び出す
        void create_history(name user, uint64_t addressbook_id, string history_type, string history_content) {
            
            // 第一引数：呼び出しに使う権限レベル（eosio.codeにactive権限を許可)
            // 第二引数：呼び出し先のアカウント
            // 第三引数：呼び出し先のアクション
            // 第四引数：呼び出し先アクションに渡す引数
            action(
                permission_level{get_self(), "active"_n},
                name{"history"},
//                name{"eosbkhistory"},
                "create"_n,
                std::make_tuple(user, addressbook_id, history_type, history_content)
            ).send();
        }
};

// dispathにcreate, update, destroy アクションを追加
EOSIO_DISPATCH(addressbook,(create)(update)(destroy))