// Copyright (c) 2012-2018, The CryptoNote developers, The Bytecoin developers, [ ] developers.
// Licensed under the GNU Lesser General Public License. See LICENSE for details.

#include "Core/Wallet.hpp"
#include "crypto/crypto.hpp"
#include "platform/PathTools.hpp"

#include "test_wallet_file.hpp"

using namespace std;

// test01.simplewallet.wallet
// format - simplewallet with cache
// no password
// created by simplewallet from cryptonote-2.1.2, several tx received and sent
// 24xTx43fFtNBUn5f6Fj1wC7y8JsbD4N1XS2s3Q8HzWxtfvERccTPX6e5ua1mf55Wm7Z4MiaWT7LPeiBxPtD8kU9V7z3kuex

// test02.wallet
// format - legacy walletd with cache (file truncated to 1000000 bytes due to git limitations)
// no password
// created by rpc-wallet-2.1.2, several tx received and sent
// 25HYsSBvERcePEb7LWrECYiPw4vpeG3MWCBxB3LbNC43issv7LEDp8gat9CRkq1ZusM85yZA6y2gTBfdUpJwyJKdDb8W8dk
// 29gEx3NRgooBHLdzQUGicXBtMdAtHaaDtf6aGZjbsgJrCSFVaRCT9SYat9CRkq1ZusM85yZA6y2gTBfdUpJwyJKdDZriJ8Y
// 23acTzmNBWFSDK12QNVucEdjRjTwPmXHsPwyXaYtc1yP9FviK5e5bMiat9CRkq1ZusM85yZA6y2gTBfdUpJwyJKdDaQapEx

// test03.wallet
// format - legacy walletd with cache (file truncated to 1000000 bytes due to git limitations)
// password - test03
// created by rpc-wallet-2.1.2, several tx received and sent
// 21vbqHu4mDLGyo7Q959fbggLU4Z6cXMfaDpUJuHQSY9b4bRfYGbv9hF4zcz3DBpH1y4kUop2HPKPsNb9WLBYE6U16w1V12t

// test04.wallet
// format - legacy walletd with cache and contacts (file truncated to 1000000 bytes due to git limitations)
// password - test04
// created by GUI wallet 1.1.9.3, several tx received and sent
// 27TfAw84qKYEktQW9QvV94VkjNnRJuQ9JTrirjbQC5ASdRS233RAtENBNEZrGCSjPAFBNBReUsaQ8Jo82GTHLU4xQ28tTWU

// test05.wallet
// format - new walletd
// password - test05
// created by walletd 3.0.0
// 23ryfTTpryt8q3h5NFoMGs4BjM6jGmdQqUrtSuU4nM3oP8CKmXXwu6CcEYrwtUm2rx43LvihFEhKEfDagjQxWoLwDTX6XpC

// test05v.wallet - view-only version of test05.wallet

// test06.wallet
// format - new walletd
// no password
// created by walletd 3.0.0
// 27j6TP7du1SWKk4fVQa138VNhy339xa76jEBRajhHYnVeLaLs5HmSkEZ4FiLuLy87hgWYkSinGntREBMq3dvui11NkhsuUJ
// 23kJeyCgzH6JkTJqq1NNjrhXFYrfsUYvw2a9HhiEqVtRXK86HAu3uWWZ4FiLuLy87hgWYkSinGntREBMq3dvui11NiUKftd
// 28acio4hR2cjQPSBj8oYbjSYkFvVaPdQTH44HsigJQiSgr6S5BaAZkzZ4FiLuLy87hgWYkSinGntREBMq3dvui11Ng9YMtq

// test06v.wallet - view-only version of test06.wallet

static void test_body(const cryptonote::Currency &currency, const std::string &path, const std::string &password,
    const std::vector<std::string> &addresses, bool view_only) {
	cryptonote::Wallet wallet("test_wallet_file.tmp", password);
	if (wallet.is_view_only() != view_only)
		throw std::runtime_error("view_only test failed for " + path);
	auto records = wallet.get_records();
	if (!crypto::keys_match(wallet.get_view_secret_key(), wallet.get_view_public_key()))
		throw std::runtime_error("view keys do not match for " + path);
	for (auto &&a : addresses) {
		cryptonote::AccountPublicAddress address;
		if (!currency.parse_account_address_string(a, address))
			throw std::runtime_error("failed to parse address " + a);
		if (address.view_public_key != wallet.get_view_public_key())
			throw std::runtime_error("view_public_key test failed for " + path);
		auto rit = records.find(address.spend_public_key);
		if (rit == records.end())
			throw std::runtime_error("spend_public_key not found for " + path);
		if (view_only && rit->second.spend_secret_key != crypto::SecretKey{})
			throw std::runtime_error("non empty secret spend key for " + path);
		if (!view_only && !crypto::keys_match(rit->second.spend_secret_key, rit->second.spend_public_key))
			throw std::runtime_error("spend keys do not match for " + path);
		if (address.spend_public_key != rit->second.spend_public_key)
			throw std::runtime_error("spend_public_key test failed for " + path);
		records.erase(rit);
	}
	if (!records.empty())
		throw std::runtime_error("excess wallet records for " + path);
}

static void test_single_file(const cryptonote::Currency &currency, const std::string &path, const std::string &password,
    const std::vector<std::string> &addresses, bool view_only) {
	platform::copy_file("test_wallet_file.tmp", path);
	test_body(currency, "test_wallet_file.tmp", password, addresses, view_only);
	{
		platform::FileStream fs("test_wallet_file.tmp", platform::FileStream::READ_EXISTING);
		auto si = fs.seek(0, SEEK_END);
		if (si != cryptonote::Wallet::wallet_file_size(addresses.size()))
			throw std::runtime_error("truncated/overwritten wallet size wrong " + path);
	}
	test_body(currency, "test_wallet_file.tmp", password, addresses, view_only);
}

void test_wallet_file(const std::string &path_prefix) {
	cryptonote::Currency currency(false);

	test_single_file(currency, path_prefix + "/test01.simplewallet.wallet", "",
	    {"24xTx43fFtNBUn5f6Fj1wC7y8JsbD4N1XS2s3Q8HzWxtfvERccTPX6e5ua1mf55Wm7Z4MiaWT7LPeiBxPtD8kU9V7z3kuex"}, false);
	test_single_file(currency, path_prefix + "/test02.wallet", "",
	    {"25HYsSBvERcePEb7LWrECYiPw4vpeG3MWCBxB3LbNC43issv7LEDp8gat9CRkq1ZusM85yZA6y2gTBfdUpJwyJKdDb8W8dk",
	        "29gEx3NRgooBHLdzQUGicXBtMdAtHaaDtf6aGZjbsgJrCSFVaRCT9SYat9CRkq1ZusM85yZA6y2gTBfdUpJwyJKdDZriJ8Y",
	        "23acTzmNBWFSDK12QNVucEdjRjTwPmXHsPwyXaYtc1yP9FviK5e5bMiat9CRkq1ZusM85yZA6y2gTBfdUpJwyJKdDaQapEx"},
	    false);
	test_single_file(currency, path_prefix + "/test03.wallet", "test03",
	    {"21vbqHu4mDLGyo7Q959fbggLU4Z6cXMfaDpUJuHQSY9b4bRfYGbv9hF4zcz3DBpH1y4kUop2HPKPsNb9WLBYE6U16w1V12t"}, false);
	test_single_file(currency, path_prefix + "/test04.wallet", "test04",
	    {"27TfAw84qKYEktQW9QvV94VkjNnRJuQ9JTrirjbQC5ASdRS233RAtENBNEZrGCSjPAFBNBReUsaQ8Jo82GTHLU4xQ28tTWU"}, false);
	test_single_file(currency, path_prefix + "/test05.wallet", "test05",
	    {"23ryfTTpryt8q3h5NFoMGs4BjM6jGmdQqUrtSuU4nM3oP8CKmXXwu6CcEYrwtUm2rx43LvihFEhKEfDagjQxWoLwDTX6XpC"}, false);
	test_single_file(currency, path_prefix + "/test05v.wallet", "test05",
	    {"23ryfTTpryt8q3h5NFoMGs4BjM6jGmdQqUrtSuU4nM3oP8CKmXXwu6CcEYrwtUm2rx43LvihFEhKEfDagjQxWoLwDTX6XpC"}, true);
	test_single_file(currency, path_prefix + "/test06.wallet", "",
	    {"27j6TP7du1SWKk4fVQa138VNhy339xa76jEBRajhHYnVeLaLs5HmSkEZ4FiLuLy87hgWYkSinGntREBMq3dvui11NkhsuUJ",
	        "23kJeyCgzH6JkTJqq1NNjrhXFYrfsUYvw2a9HhiEqVtRXK86HAu3uWWZ4FiLuLy87hgWYkSinGntREBMq3dvui11NiUKftd",
	        "28acio4hR2cjQPSBj8oYbjSYkFvVaPdQTH44HsigJQiSgr6S5BaAZkzZ4FiLuLy87hgWYkSinGntREBMq3dvui11Ng9YMtq"},
	    false);
	test_single_file(currency, path_prefix + "/test06v.wallet", "",
	    {"27j6TP7du1SWKk4fVQa138VNhy339xa76jEBRajhHYnVeLaLs5HmSkEZ4FiLuLy87hgWYkSinGntREBMq3dvui11NkhsuUJ",
	        "23kJeyCgzH6JkTJqq1NNjrhXFYrfsUYvw2a9HhiEqVtRXK86HAu3uWWZ4FiLuLy87hgWYkSinGntREBMq3dvui11NiUKftd",
	        "28acio4hR2cjQPSBj8oYbjSYkFvVaPdQTH44HsigJQiSgr6S5BaAZkzZ4FiLuLy87hgWYkSinGntREBMq3dvui11Ng9YMtq"},
	    true);
}