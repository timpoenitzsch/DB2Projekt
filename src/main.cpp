#define CATCH_CONFIG_MAIN
#include "core/column.hpp"
//include your compressed column implementations here
#include "config.hpp"

#include "tests/utils.hpp"

#include <catch2/catch.hpp>

template<typename T>
struct Column_Test_Fixture
{
    using ValueType = typename T::value_type;
    Column_Test_Fixture() :
        col_one(getAttributeString<ValueType>(), getAttributeType<ValueType>()),
        col_two(getAttributeString<ValueType>(), getAttributeType<ValueType>()),
        reference_data(100){};

    T col_one;
    T col_two;
    std::vector<ValueType> reference_data;
};

using namespace CoGaDB;

TEMPLATE_PRODUCT_TEST_CASE_METHOD(
    Column_Test_Fixture,
    "Template test case method with test types specified inside std::tuple",
    "[class][template]",
    (Column /*TODO: insert your column types here, separated by comma*/),
    (int, double, std::string))
{
    using ValueType = typename Column_Test_Fixture<TestType>::ValueType;
    auto &col_one = Column_Test_Fixture<TestType>::col_one;
    auto &col_two = Column_Test_Fixture<TestType>::col_two;
    auto &reference_data = Column_Test_Fixture<TestType>::reference_data;

    /****** INSERT TEST ******/

    REQUIRE_NOTHROW(fill_column<ValueType>(col_one, reference_data));

    REQUIRE(reference_data.size() == col_one.size());
    REQUIRE_THAT(col_one, isEqual<TestType>(reference_data));

    /****** VIRTUAL COPY CONSTRUCTOR TEST ******/
    std::cout << "VIRTUAL COPY CONSTRUCTOR TEST...";

    ColumnPtr copy = col_one.copy();
    REQUIRE(copy);
    // REQUIRE(*(std::static_pointer_cast<TestType>(copy)) == col_one);

    /****** UPDATE TEST ******/

    std::uniform_int_distribution dist(0, 100);
    TID tid = dist(gen);
    auto new_value = get_rand_value<ValueType>();
    std::cout << "UPDATE TEST: Update value on Position '" << tid << "' to new value '" << new_value << "'...";

    reference_data[tid] = new_value;

    REQUIRE_NOTHROW(col_one.update(tid, new_value));
    REQUIRE_THAT(col_one, isEqual<TestType>(reference_data));

    /****** DELETE TEST ******/
    tid = dist(gen);
    std::cout << "DELETE TEST: Delete value on Position '" << tid << "'..."; // << std::endl;

    reference_data.erase(reference_data.begin() + tid);

    REQUIRE_NOTHROW(col_one.remove(tid));
    REQUIRE_THAT(col_one, isEqual<TestType>(reference_data));

    /****** STORE AND LOAD TEST ******/
    REQUIRE_NOTHROW(col_one.store(DATA_PATH));

    col_one.clearContent();
    REQUIRE(col_one.size() == 0);

    REQUIRE_NOTHROW(col_two.load(DATA_PATH));
    REQUIRE_THAT(col_one, isEqual<TestType>(reference_data));
}
//	/****** BULK UPDATE TEST ******/
//	{
//		std::cout << "BULK UPDATE TEST..."; // << std::endl;
//		std::shared_ptr<Column<int> > uncompressed_col (new Column<int>("int column",INT));
//		std::shared_ptr<Column<int> > compressed_col (new Column<int>("int column",INT));
//		//std::shared_ptr<DictionaryCompressedColumn<int> > compressed_col (new
// DictionaryCompressedColumn<int>("compressed int column",INT));

//		uncompressed_col->insert(reference_data.begin(),reference_data.end());
//		compressed_col->insert(reference_data.begin(),reference_data.end());

//		bool result = *(std::static_pointer_cast<ColumnBaseTyped<int>
//>(uncompressed_col))==*(std::static_pointer_cast<ColumnBaseTyped<int> >(compressed_col)); 		if(!result){
// std::cerr << std::endl << "operator== TEST FAILED!" << std::endl; 			return false;
//		}
//		PositionListPtr tids (new PositionList());
//		int new_value=rand()%100;
//	   for(unsigned int i=0;i<10;i++){
//	 		tids->push_back(rand()%uncompressed_col->size());
//	   }
//
//		uncompressed_col->update(tids,new_value);
//		compressed_col->update(tids,new_value);

//		result = *(std::static_pointer_cast<ColumnBaseTyped<int>
//>(uncompressed_col))==*(std::static_pointer_cast<ColumnBaseTyped<int> >(compressed_col)); 		if(!result){
// std::cerr << std::endl << "BULK UPDATE TEST FAILED!" << std::endl; 			 return false;
//		}
//		std::cout << "SUCCESS"<< std::endl;

//	}

//	/****** BULK DELETE TEST ******/
//	{
//		std::cout << "BULK DELETE TEST..."; // << std::endl;
//		std::shared_ptr<Column<int> > uncompressed_col (new Column<int>("int column",INT));
//		std::shared_ptr<Column<int> > compressed_col (new Column<int>("int column",INT));

//		//std::shared_ptr<DictionaryCompressedColumn<int> > compressed_col (new
// DictionaryCompressedColumn<int>("compressed int column",INT));

//		uncompressed_col->insert(reference_data.begin(),reference_data.end());
//		compressed_col->insert(reference_data.begin(),reference_data.end());

//		bool result = *(std::static_pointer_cast<ColumnBaseTyped<int>
//>(uncompressed_col))==*(std::static_pointer_cast<ColumnBaseTyped<int> >(compressed_col)); 		if(!result){
// std::cerr << std::endl << "operator== TEST FAILED!" << std::endl; 			return false;
//		}

//		PositionListPtr tids (new PositionList());

//	   for(unsigned int i=0;i<10;i++){
//	 		tids->push_back(rand()%uncompressed_col->size());
//	   }
//
//		uncompressed_col->remove(tids);
//		compressed_col->remove(tids);

//		result = *(std::static_pointer_cast<ColumnBaseTyped<int>
//>(uncompressed_col))==*(std::static_pointer_cast<ColumnBaseTyped<int> >(compressed_col)); 		if(!result){
// std::cerr << "BULK DELETE TEST FAILED!" << std::endl; 			 return false;
//		}
//		std::cout << "SUCCESS"<< std::endl;

//	}

// return 0;
//}
