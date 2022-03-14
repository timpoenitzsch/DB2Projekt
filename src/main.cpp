#include <string>
#include <core/global_definitions.hpp>
#include <core/base_column.hpp>
#include <core/column_base_typed.hpp>
#include <core/column.hpp>
#include <core/compressed_column.hpp>

/*this is the include for the example compressed column with empty implementation*/
#include <compression/dictionary_compressed_column.hpp>

#include  "unittest.hpp"

using namespace CoGaDB;

int main(){
	/*Adapt the Column to your implemented method*/
            std::cout <<"Dic: "<< std::endl;
	if(!unittest<Column, int>()){
		std::cout << "At least one Unittest Failed!" << std::endl;	
		return -1;	
	}
	std::cout << "Unitests Passed!" << std::endl;

	if(!unittest<Column,float>()){
		std::cout << "At least one Unittest Failed!" << std::endl;	
		return -1;	
	}
	std::cout << "Unitests Passed!" << std::endl;

	if(!unittest<Column ,std::string>()){
		std::cout << "At least one Unittest Failed!" << std::endl;	
		return -1;	
	}
	std::cout << "Unitests Passed!" << std::endl;

//	/****** BULK UPDATE TEST ******/
//	{
//		std::cout << "BULK UPDATE TEST..."; // << std::endl;
//		std::shared_ptr<Column<int> > uncompressed_col (new Column<int>("int column",INT));
//		std::shared_ptr<Column<int> > compressed_col (new Column<int>("int column",INT));
//		//std::shared_ptr<DictionaryCompressedColumn<int> > compressed_col (new DictionaryCompressedColumn<int>("compressed int column",INT));


//		uncompressed_col->insert(reference_data.begin(),reference_data.end()); 
//		compressed_col->insert(reference_data.begin(),reference_data.end()); 

//		bool result = *(std::static_pointer_cast<ColumnBaseTyped<int> >(uncompressed_col))==*(std::static_pointer_cast<ColumnBaseTyped<int> >(compressed_col));
//		if(!result){ 
//			std::cerr << std::endl << "operator== TEST FAILED!" << std::endl;	
//			return false;
//		}
//		PositionListPtr tids (new PositionList());
//		int new_value=rand()%100;
//	   for(unsigned int i=0;i<10;i++){
//	 		tids->push_back(rand()%uncompressed_col->size());
//	   }
//		
//		uncompressed_col->update(tids,new_value); 
//		compressed_col->update(tids,new_value); 

//		result = *(std::static_pointer_cast<ColumnBaseTyped<int> >(uncompressed_col))==*(std::static_pointer_cast<ColumnBaseTyped<int> >(compressed_col));
//		if(!result){
//			 std::cerr << std::endl << "BULK UPDATE TEST FAILED!" << std::endl;	
//			 return false;	
//		}
//		std::cout << "SUCCESS"<< std::endl;	

//	}

//	/****** BULK DELETE TEST ******/
//	{
//		std::cout << "BULK DELETE TEST..."; // << std::endl;
//		std::shared_ptr<Column<int> > uncompressed_col (new Column<int>("int column",INT));
//		std::shared_ptr<Column<int> > compressed_col (new Column<int>("int column",INT));

//		//std::shared_ptr<DictionaryCompressedColumn<int> > compressed_col (new DictionaryCompressedColumn<int>("compressed int column",INT));

//		uncompressed_col->insert(reference_data.begin(),reference_data.end()); 
//		compressed_col->insert(reference_data.begin(),reference_data.end()); 

//		bool result = *(std::static_pointer_cast<ColumnBaseTyped<int> >(uncompressed_col))==*(std::static_pointer_cast<ColumnBaseTyped<int> >(compressed_col));
//		if(!result){ 
//			std::cerr << std::endl << "operator== TEST FAILED!" << std::endl;	
//			return false;
//		}

//		PositionListPtr tids (new PositionList());

//	   for(unsigned int i=0;i<10;i++){
//	 		tids->push_back(rand()%uncompressed_col->size());
//	   }
//		
//		uncompressed_col->remove(tids); 
//		compressed_col->remove(tids); 

//		result = *(std::static_pointer_cast<ColumnBaseTyped<int> >(uncompressed_col))==*(std::static_pointer_cast<ColumnBaseTyped<int> >(compressed_col));
//		if(!result){
//			 std::cerr << "BULK DELETE TEST FAILED!" << std::endl;	
//			 return false;	
//		}
//		std::cout << "SUCCESS"<< std::endl;	

//	}


 return 0;
}


