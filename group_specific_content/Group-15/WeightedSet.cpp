#define CATCH_CONFIG_MAIN
#include "../../third-party/Catch/single_include/catch2/catch.hpp"

#include "../../group_specific_content/Group-15/WeightedSet.hpp"

void MakeSmallTree(cse498::WeightedSet<std::string>& wSet) {
	wSet.Insert("A", 2.0);
	wSet.Insert("B", 3.0);
	wSet.Insert("C", 5.0);
	wSet.Insert("D", 7.0);
}

void MakeLargerTree(cse498::WeightedSet<int>& wSet) {
	wSet.Insert(1, 11.0);	
	wSet.Insert(2, 2.0);
	wSet.Insert(3, 2.0);
	wSet.Insert(4, 0.0);
	wSet.Insert(5, 5.5);
	wSet.Insert(6, 13.5);
	wSet.Insert(7, 17.0);
	wSet.Insert(8, 1.0);
	wSet.Insert(9, 25.0);
	wSet.Insert(10, 23.0);
}

void MakeDecimalTree(cse498::WeightedSet<char>& wSet) {
	wSet.Insert('a', 2.000000000000001);
	wSet.Insert('b', 3.023344563390414);
	wSet.Insert('c', 4.999999999999999);
	wSet.Insert('d', 7.003450023394053);
}

void MakeTinyTree(cse498::WeightedSet<int>& wSet) {
	wSet.Insert(1, 1e-12); //Tolerance
	wSet.Insert(2, 1e-15);
	wSet.Insert(3, 0.000000009);
	wSet.Insert(4, 0.000000010);
}

void MakeHugeTree(cse498::WeightedSet<int>& wSet) {
    std::mt19937_64 rng(12345);
    std::uniform_real_distribution<double> dist(0.1, 10.0);
	for (int i = 1; i <= 100000; i++) {
		 wSet.Insert(i, dist(rng));   // every item gets equal weight
    }
}

TEST_CASE("Test WeightedSet Constructor", "[core]")
{
	cse498::WeightedSet<int> wSet;

	CHECK(wSet.GetSize() == 0);
}

TEST_CASE("Test Insert", "[core]"){
	
	SECTION("Small Tree") {
		cse498::WeightedSet<std::string> wSet;
		MakeSmallTree(wSet);
		CHECK(wSet.GetSize() == 4);
	}

	SECTION("Larger Tree"){
		cse498::WeightedSet<int> wSet;
		MakeLargerTree(wSet);
		CHECK(wSet.GetSize() == 10);
	}
	
	SECTION("Bad Inserts") {
		cse498::WeightedSet<std::string> wSet;
		MakeSmallTree(wSet);
		
		REQUIRE_THROWS_AS (
        wSet.Insert("E", -3.5),
        std::invalid_argument
   		);

		REQUIRE_THROWS_AS (
        wSet.Insert("A", 5.0),
        std::invalid_argument
   		);
	}
}

TEST_CASE("Test Sums", "[core]"){
	SECTION("Small Tree") {
		cse498::WeightedSet<std::string> wSet;
		MakeSmallTree(wSet);
		CHECK(wSet.GetItemSum("A") == Approx(17.0)); // A = weight (2.0) + left sum (10.0) + right sum (5.0)
		CHECK(wSet.GetItemSum("B") == Approx(10.0)); //B = weight (3.0) + Left sum (7.0) + right sum (0.0)
		CHECK(wSet.GetItemSum("C") == Approx(5.0)); //C is a leaf, sum = weight (5.0)
		CHECK(wSet.GetItemSum("D") == Approx(7.0)); //D is a leaf, sum = weight (7.0)
	}

	SECTION("Larger Tree"){
		cse498::WeightedSet<int> wSet;
		MakeLargerTree(wSet);

		CHECK(wSet.GetItemSum(1) == Approx(100.0));
		CHECK(wSet.GetItemSum(2) == Approx(56.5));
		CHECK(wSet.GetItemSum(3) == Approx(32.5));
		CHECK(wSet.GetItemSum(4) == Approx(26));
		CHECK(wSet.GetItemSum(5) == Approx(28.5));
		CHECK(wSet.GetItemSum(6) == Approx(13.5));
		CHECK(wSet.GetItemSum(7) == Approx(17.0));
		CHECK(wSet.GetItemSum(8) == Approx(1.0));
		CHECK(wSet.GetItemSum(9) == Approx(25.0));
		CHECK(wSet.GetItemSum(10) == Approx(23.0));
	}

	SECTION("Decimal tree"){
		cse498::WeightedSet<char> wSet;
		MakeDecimalTree(wSet);

		CHECK(wSet.GetItemSum('a') == Approx(2.000000000001 + 3.023344563390414 
			+ 7.003450023394053 + 4.999999999999999).margin(1e-12));
		CHECK(wSet.GetItemSum('b') == Approx(3.023344563390414 + 7.003450023394053).margin(1e-12));
		CHECK(wSet.GetItemSum('c') == Approx(4.999999999999999).margin(1e-12));
		CHECK(wSet.GetItemSum('d') == Approx(7.003450023394053).margin(1e-12));
	}

	SECTION ("Tiny Tree"){
		cse498::WeightedSet<int> wSet;
		MakeTinyTree(wSet);

		CHECK(wSet.GetItemSum(1) == Approx(0.000000010 + 0.000000009).margin(1e-12));
		CHECK(wSet.GetItemSum(2) == Approx(0.000000010).margin(1e-12));
		CHECK(wSet.GetItemSum(3) == Approx(0.000000009).margin(1e-12));
		CHECK(wSet.GetItemSum(4) == Approx(0.000000010).margin(1e-12));
	}
}

TEST_CASE("Test Update", "[core]"){
	
	SECTION("Small Tree"){
		cse498::WeightedSet<std::string> wSet;
		MakeSmallTree(wSet);

		wSet.Update("D", 10.4561);
		CHECK(wSet.GetWeight("D") == Approx(10.4561));
		CHECK(wSet.GetItemSum("A") == Approx(2.0 + 3.0 + 10.4561 + 5.0));
		CHECK(wSet.GetItemSum("B") == Approx(3.0 + 10.4561));
		CHECK(wSet.GetItemSum("C") == Approx(5.0));
		CHECK(wSet.GetItemSum("D") == Approx(10.4561));

		wSet.Update("A", 2.5327);
		CHECK(wSet.GetWeight("A") == Approx(2.5327));
		CHECK(wSet.GetItemSum("A") == Approx(2.5327 + 3.0 + 10.4561 + 5.0));
		CHECK(wSet.GetItemSum("B") == Approx(3.0 + 10.4561));
		CHECK(wSet.GetItemSum("C") == Approx(5.0));
		CHECK(wSet.GetItemSum("D") == Approx(10.4561));

		wSet.Update("C", 0.0);
		CHECK(wSet.GetWeight("C") == Approx(0.0));
		CHECK(wSet.GetItemSum("A") == Approx(2.5327 + 3.0 + 10.4561 + 0.0));
		CHECK(wSet.GetItemSum("B") == Approx(3.0 + 10.4561));
		CHECK(wSet.GetItemSum("C") == Approx(0.0));
		CHECK(wSet.GetItemSum("D") == Approx(10.4561));
	}

	SECTION ("Bad Updates") {
		cse498::WeightedSet<std::string> wSet;
		MakeSmallTree(wSet);
		
		REQUIRE_THROWS_AS (
        wSet.Update("A", -3.5),
        std::invalid_argument
   		);

		REQUIRE_THROWS_AS (
        wSet.Update("E", 5.0),
        std::invalid_argument
   		);
	}
}

TEST_CASE("Test Sample", "[core]"){
	SECTION("Small tree"){
		cse498::WeightedSet<std::string> wSet;
		MakeSmallTree(wSet);

		CHECK(wSet.Sample(0.0) == "D");
		CHECK(wSet.Sample(7.0) == "D"); //D in [0.0, 7.0]
		CHECK(wSet.Sample(7.000000000001) == "D"); //Tolerance
		CHECK(wSet.Sample(7.000000000002) == "B"); //B in (7.0, 10.0]
		CHECK(wSet.Sample(10.0) == "B");
		CHECK(wSet.Sample(10.001) == "A"); //A in (10, 12]
		CHECK(wSet.Sample(12.0) == "A");
		CHECK(wSet.Sample(12.000001) == "C"); //C in (12, 17]
		CHECK(wSet.Sample(17) == "C");
	}

	SECTION("Larger Tree"){
		cse498::WeightedSet<int> wSet;
		MakeLargerTree(wSet); 

		CHECK(wSet.Sample(0.0) == 8);
		CHECK(wSet.Sample(1.0) == 8);
		CHECK(wSet.Sample(1.000000000002) == 9); //item 4 gets skipped because w = 0
		CHECK(wSet.Sample(26.0) == 9);
		CHECK(wSet.Sample(26.000000000002) == 2);
		CHECK(wSet.Sample(28.0) == 2);
		CHECK(wSet.Sample(28.000000000002) == 10);
		CHECK(wSet.Sample(51) == 10);
		CHECK(wSet.Sample(51.000000000002) == 5);
		CHECK(wSet.Sample(56.5) == 5);
		CHECK(wSet.Sample(56.500000000002) == 1);
		CHECK(wSet.Sample(67.5) == 1);
		CHECK(wSet.Sample(67.500000000002) == 6);
		CHECK(wSet.Sample(81.0) == 6);
		CHECK(wSet.Sample(81.000000000002) == 3);
		CHECK(wSet.Sample(83.0) == 3);
		CHECK(wSet.Sample(83.000000000002) == 7);
		CHECK(wSet.Sample(100) == 7);
	}

	SECTION("Decimal Tree"){
		cse498::WeightedSet<char> wSet;
		MakeDecimalTree(wSet);

		CHECK(wSet.Sample(0.0) == 'd');
		CHECK(wSet.Sample(7.003450023394053) == 'd');
		CHECK(wSet.Sample(7.003450023394053 + 2e-12) == 'b');
		CHECK(wSet.Sample(7.003450023394053 + 3.023344563390414) == 'b');
		CHECK(wSet.Sample(7.003450023394053 + 3.023344563390414 + 2e-12) == 'a');
		CHECK(wSet.Sample(7.003450023394053 + 3.023344563390414 
			+ 2.000000000000001) == 'a');
		CHECK(wSet.Sample(7.003450023394053 + 3.023344563390414 
			+ 2.000000000000001 + 2e-12) == 'c');
		CHECK(wSet.Sample(7.003450023394053 + 3.023344563390414 
			+ 2.000000000000001 + 4.999999999999999) == 'c');
	}

	SECTION("Tiny Tree"){
		cse498::WeightedSet<int> wSet;
		MakeTinyTree(wSet);

		CHECK(wSet.Sample(0.0) == 4);
		CHECK(wSet.Sample(0.000000010) == 4);
		CHECK(wSet.Sample(0.000000010 + 2e-12) == 3);
		CHECK(wSet.Sample(0.000000010 + 0.000000009) == 3);
	}

	SECTION("Weight Zero In Small tree"){
		cse498::WeightedSet<std::string> wSet;
		MakeSmallTree(wSet);

		REQUIRE_THROWS_AS (
        wSet.Sample(-7.0),
        std::invalid_argument
   		);

		REQUIRE_THROWS_AS (
        wSet.Sample(20.0),
        std::invalid_argument
   		);

		//leftmost leaf
		wSet.Update("D", 0.0);
		CHECK(wSet.Sample(0.0) == "B");
		CHECK(wSet.Sample(3.0) == "B");
		CHECK(wSet.Sample(3.0 + 2.0) == "A");
		CHECK(wSet.Sample(3.0 + 2.0 + 5.0) == "C");

		//Leftmost leaf + it's parent
		wSet.Update("B", 0.0);
		CHECK(wSet.Sample(0.0) == "A");
		CHECK(wSet.Sample(2.0) == "A");
		CHECK(wSet.Sample(7.0) == "C");

		//Left side + root
		wSet.Update("A", 0.0);
		CHECK(wSet.Sample(0.0) == "C");
		CHECK(wSet.Sample(5.0) == "C");

		//All 0
		wSet.Update("C", 0.0);
		REQUIRE_THROWS_AS (
        wSet.Sample(0.0),
        std::runtime_error
   		);

		wSet.Update("D", 7.0);
		wSet.Update("B", 3.0);
		wSet.Update("A", 2.0);

		//Right leaf 0
		CHECK(wSet.Sample(0.0) == "D");
		CHECK(wSet.Sample(7.0) == "D");
		CHECK(wSet.Sample(7.0 + 3.0) == "B");
		CHECK(wSet.Sample(7.0 + 2.0 + 3.0) == "A");
		REQUIRE_THROWS_AS (
        wSet.Sample(7.0 + 2.0 + 3.0 + 5.0),
        std::invalid_argument
   		);

		//Right side + root
		wSet.Update("A", 0.0);
		CHECK(wSet.Sample(0.0) == "D");
		CHECK(wSet.Sample(7.0) == "D");
		CHECK(wSet.Sample(7.0 + 3.0) == "B");
		REQUIRE_THROWS_AS (
        wSet.Sample(7.0 + 2.0 + 3.0),
        std::invalid_argument
   		);
	}

	SECTION("Weight Zero In Larger Tree"){
		cse498::WeightedSet<int> wSet;
		MakeLargerTree(wSet);

		//Remove 4th row leaf nodes
		wSet.Update(8, 0.0);
		wSet.Update(9, 0.0);
		wSet.Update(10, 0.0);

		//Make item 4 nonzero
		wSet.Update(4, 4.0);

		CHECK(wSet.Sample(0.0) == 4);
		CHECK(wSet.Sample(4.0) == 4);
		CHECK(wSet.Sample(6.0) == 2);
		CHECK(wSet.Sample(11.5) == 5);
		CHECK(wSet.Sample(22.5) == 1);
		CHECK(wSet.Sample(36) == 6);
		CHECK(wSet.Sample(38) == 3);
		CHECK(wSet.Sample(55) == 7);

		//Right parent
		wSet.Update(3, 0.0);
		CHECK(wSet.Sample(0.0) == 4);
		CHECK(wSet.Sample(4.0) == 4);
		CHECK(wSet.Sample(6.0) == 2);
		CHECK(wSet.Sample(11.5) == 5);
		CHECK(wSet.Sample(22.5) == 1);
		CHECK(wSet.Sample(36) == 6);
		CHECK(wSet.Sample(53) == 7);

		//Right parent + Right Right leaf
		wSet.Update(7, 0.0);
		CHECK(wSet.Sample(0.0) == 4);
		CHECK(wSet.Sample(4.0) == 4);
		CHECK(wSet.Sample(6.0) == 2);
		CHECK(wSet.Sample(11.5) == 5);
		CHECK(wSet.Sample(22.5) == 1);
		CHECK(wSet.Sample(36) == 6);

		//Right parent + Right Left leaf
		wSet.Update(7, 17.0);
		wSet.Update(6, 0.0);
		CHECK(wSet.Sample(0.0) == 4);
		CHECK(wSet.Sample(4.0) == 4);
		CHECK(wSet.Sample(6.0) == 2);
		CHECK(wSet.Sample(11.5) == 5);
		CHECK(wSet.Sample(22.5) == 1);
		CHECK(wSet.Sample(39.5) == 7);

		wSet.Update(7, 0.0); //Right side deleted

		//Add full left side back
		wSet.Update(8, 1.0);
		wSet.Update(9, 25.0);
		wSet.Update(10, 23.0);

		//Item 2 deleted
		wSet.Update(2, 0.0);
		CHECK(wSet.Sample(0.0) == 8);
		CHECK(wSet.Sample(1.0) == 8);
		CHECK(wSet.Sample(26.0) == 9);
		CHECK(wSet.Sample(49) == 10);
		CHECK(wSet.Sample(54.5) == 5);
		CHECK(wSet.Sample(65.5) == 1);
	}
}

TEST_CASE("Sample meets worst-case time budget", "[core]")
{
    cse498::WeightedSet<int> wSet;
    MakeHugeTree(wSet);          // 100k items

    double total = wSet.GetItemSum(1);

    std::mt19937_64 rng(42);
    std::uniform_real_distribution<double> dist(0.0, total);

    const int N = 1000;          // number of samples to test

    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < N; i++) {
        wSet.Sample(dist(rng));
    }

    auto end = std::chrono::high_resolution_clock::now();
    double ms =
        std::chrono::duration<double, std::milli>(end - start).count();

    CHECK(ms < 3.0);
}