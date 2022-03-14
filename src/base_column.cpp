
#include <core/base_column.hpp>
#include <core/column.hpp>
#include <iostream>
#include <utility>


namespace CoGaDB{

	ColumnBase::ColumnBase(std::string name, AttributeType db_type) : name_(std::move(name)), db_type_(db_type){

	}

	ColumnBase::~ColumnBase()= default;

	AttributeType ColumnBase::getType() const noexcept{
		return db_type_;
	}

	std::string ColumnBase::getName() const noexcept{
		return name_;
	}

	ColumnPtr createColumn(AttributeType type, const std::string& name){

	ColumnPtr ptr;
	if(type==INT){
		ptr=ColumnPtr(new Column<int>(name,INT));
	}else if(type==FLOAT){
		ptr=ColumnPtr(new Column<float>(name,FLOAT));
	}else if(type==VARCHAR){
		ptr=ColumnPtr(new Column<std::string>(name,VARCHAR));
	}else if(type==BOOLEAN){
		//ptr=ColumnPtr(new Column<bool>(name,BOOLEAN));
        std::cerr << "Fatal Error! invalid AttributeType: " << type << " for Column: " << name
			  << " Note: bool is currently not supported, will be added again in the future!"<< std::endl;
	}else{
		std::cerr << "Fatal Error! invalid AttributeType: " << type << " for Column: " << name << std::endl;
	}

	return ptr;
	}

}; //end namespace CogaDB

