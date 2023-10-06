import test_tools as tt
from hive_local_tools import run_for


@run_for("testnet", "mainnet_5m", "live_mainnet")
def test_get_order_book(node, should_prepare):
    preparation_for_testnet_node(node, should_prepare)
    node.api.condenser.get_order_book(100)


@run_for("testnet", "mainnet_5m", "live_mainnet")
def test_get_order_book_with_default_argument(node, should_prepare):
    preparation_for_testnet_node(node, should_prepare)
    node.api.condenser.get_order_book()


def preparation_for_testnet_node(node, should_prepare):
    if should_prepare:
        wallet = tt.Wallet(attach_to=node)
        wallet.create_account("alice", hives=tt.Asset.Test(100), vests=tt.Asset.Test(100))
        wallet.create_account("bob", hives=tt.Asset.Test(100), vests=tt.Asset.Test(100), hbds=tt.Asset.Tbd(100))

        wallet.api.create_order("alice", 1, tt.Asset.Test(100), tt.Asset.Tbd(50), False, 3600)
        wallet.api.create_order("bob", 2, tt.Asset.Tbd(20), tt.Asset.Test(100), False, 3600)
