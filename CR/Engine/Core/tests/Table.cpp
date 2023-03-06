#include <doctest/doctest.h>

import CR.Engine.Core.Table;

import<type_traits>;
import<utility>;

namespace {
	struct PlayerHealth {
		inline static int numConstructed{0};
		inline static int numDestructed{0};
		PlayerHealth() { numConstructed++; }
		~PlayerHealth() { numDestructed++; }

		uint32_t Health{100};
		uint32_t Armor{100};
	};
	static_assert(std::is_standard_layout_v<PlayerHealth>);

	struct PlayerMoney {
		inline static int numConstructed{0};
		inline static int numDestructed{0};
		PlayerMoney() { numConstructed++; }
		~PlayerMoney() { numDestructed++; }

		uint32_t Copper{0};
		uint32_t Silver{0};
		uint32_t Gold{0};
		uint32_t Platinum{0};
	};
	static_assert(std::is_standard_layout_v<PlayerMoney>);

	struct PlayerStats {
		inline static int numConstructed{0};
		inline static int numDestructed{0};
		PlayerStats() { numConstructed++; }
		~PlayerStats() { numDestructed++; }

		float CritChance{5.0f};
		float BonusWeaponDmg{0.0f};
		float BonusMagicDmg{0.0f};
	};
	static_assert(std::is_standard_layout_v<PlayerStats>);
}    // namespace

TEST_CASE("table_simple") {
	using t_Table = CR::Engine::Core::Table<16, uint32_t, PlayerHealth>;
	t_Table table{"simple table"};

	REQUIRE(table.GetIndex(0) == t_Table::c_unused);

	int preConstructed = PlayerHealth::numConstructed;
	int preDestructed  = PlayerHealth::numDestructed;

	uint16_t index1 = table.insert(0);
	REQUIRE(table.GetIndex(0) != t_Table::c_unused);
	REQUIRE(preConstructed == PlayerHealth::numConstructed);
	REQUIRE(preDestructed == PlayerHealth::numDestructed);

	// first one damaged
	PlayerHealth* row = &table.GetValue<PlayerHealth>(index1);
	row->Health       = 2;
	row->Armor        = 100;

	// add a second one
	[[maybe_unused]] uint16_t index2 = table.insert(1);
	REQUIRE(table.GetIndex(1) != t_Table::c_unused);

	// reduce armor for all
	auto view = table.GetView<PlayerHealth>();

	// by value, so should not change the source data
	for(auto [health] : view) { health.Armor = 50; }

	// check it has expected values
	row = &table.GetValue<PlayerHealth>(index1);
	REQUIRE(row->Health == 2);
	REQUIRE(row->Armor == 100);

	// now check second row
	row = &table.GetValue<PlayerHealth>(index2);
	REQUIRE(row->Health == 100);
	REQUIRE(row->Armor == 100);

	// by reference, so should change the source data
	for(auto& [health] : view) { health.Armor = 50; }

	// check it has expected values
	row = &table.GetValue<PlayerHealth>(index1);
	REQUIRE(row->Health == 2);
	REQUIRE(row->Armor == 50);

	// now check second row
	row = &table.GetValue<PlayerHealth>(index2);
	REQUIRE(row->Health == 100);
	REQUIRE(row->Armor == 50);

	table.erase(index1);
	table.erase(index2);

	// test out const access too
	auto constColumnSet = std::as_const(table).GetView<PlayerHealth>();
	for(auto& [health] : constColumnSet) {
		REQUIRE(health.Armor == 50);
		// shouldnt compile
		// health.Armor = 20;
	}

	// should have never created or destroyed any.
	REQUIRE(preConstructed == PlayerHealth::numConstructed);
	REQUIRE(preDestructed == PlayerHealth::numDestructed);
}

TEST_CASE("table_multiple") {
	using t_Table = CR::Engine::Core::Table<16, std::string, PlayerHealth, PlayerMoney, PlayerStats>;
	t_Table table{"a table"};

	REQUIRE(table.GetIndex("knight") == t_Table::c_unused);

	int preConstructed =
	    PlayerHealth::numConstructed + PlayerMoney::numConstructed + PlayerStats::numConstructed;
	int preDestructed = PlayerHealth::numDestructed + PlayerMoney::numDestructed + PlayerStats::numDestructed;

	uint16_t knightIndex = table.insert("knight");
	REQUIRE(table.GetIndex("knight") != t_Table::c_unused);
	REQUIRE(preConstructed ==
	        PlayerHealth::numConstructed + PlayerMoney::numConstructed + PlayerStats::numConstructed);
	REQUIRE(preDestructed ==
	        PlayerHealth::numDestructed + PlayerMoney::numDestructed + PlayerStats::numDestructed);

	// add a second one
	uint16_t wizardIndex = table.insert("wizard");
	REQUIRE(table.GetIndex("wizard") != t_Table::c_unused);

	// change some values for 2 columns
	auto view = table.GetView<PlayerHealth, PlayerMoney>();

	for(auto&& [health, money] : view) {
		health.Armor = 50;
		money.Gold   = 100;
	}

	// now check second row
	{
		auto& row = table.GetValue<PlayerHealth>(wizardIndex);
		REQUIRE(row.Armor == 50);
	}
	{
		auto& row = table.GetValue<PlayerMoney>(wizardIndex);
		REQUIRE(row.Gold == 100);
	}

	// iterate over table instead of a view
	for(auto&& [health, money, stats] : table) {
		health.Health = 100;
		money.Silver  = 100;
		stats.CritChance += 1.0f;
	}

	table.erase(knightIndex);
	table.erase(wizardIndex);

	// should have never created or destroyed any.
	REQUIRE(preConstructed ==
	        PlayerHealth::numConstructed + PlayerMoney::numConstructed + PlayerStats::numConstructed);
	REQUIRE(preDestructed ==
	        PlayerHealth::numDestructed + PlayerMoney::numDestructed + PlayerStats::numDestructed);
}
