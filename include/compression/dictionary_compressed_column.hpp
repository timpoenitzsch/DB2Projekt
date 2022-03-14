
/*! \example dictionary_compressed_column.hpp
 * This is an example of how to implement a compression technique in our framework. One has to inherit from an abstract base class CoGaDB::CompressedColumn and implement the pure virtual methods.
 */

#pragma once

#include <core/compressed_column.hpp>

namespace CoGaDB{
	

/*!
 *  \brief     This class represents a dictionary compressed column with type T, is the base class for all compressed typed column classes.
 */	
template<class T>
class DictionaryCompressedColumn : public CompressedColumn<T>{
	public:
	/***************** constructors and destructor *****************/
	DictionaryCompressedColumn(const std::string& name, AttributeType db_type);
	virtual ~DictionaryCompressedColumn();

	virtual bool insert(const std::any& new_Value);
	virtual bool insert(const T& new_value);
	template <typename InputIterator>
	bool insert(InputIterator first, InputIterator last);

	virtual bool update(TID tid, const std::any& new_value);
	virtual bool update(PositionListPtr tid, const std::any& new_value);
	
	virtual bool remove(TID tid);
	//assumes tid list is sorted ascending
	virtual bool remove(PositionListPtr tid);
	virtual bool clearContent();

	virtual std::any get(TID tid);
	//virtual const std::any* const getRawData()=0;
	virtual void print() const noexcept;
	[[nodiscard]] virtual size_t size() const noexcept;
	[[nodiscard]] virtual unsigned int getSizeinBytes() const noexcept;

	[[nodiscard]] virtual ColumnPtr copy() const;

	virtual bool store(const std::string& path);
	virtual bool load(const std::string& path);


	
	virtual T& operator[](int index);

};


/***************** Start of Implementation Section ******************/

	
	template<class T>
	DictionaryCompressedColumn<T>::DictionaryCompressedColumn(const std::string& name, AttributeType db_type) : CompressedColumn<T>(name, db_type){

	}

	template<class T>
	DictionaryCompressedColumn<T>::~DictionaryCompressedColumn()= default;

	template<class T>
	bool DictionaryCompressedColumn<T>::insert(const std::any&){

		return false;
	}

	template<class T>
	bool DictionaryCompressedColumn<T>::insert(const T&){
		return false;
	}

	template <typename T> 
	template <typename InputIterator>
	bool DictionaryCompressedColumn<T>::insert(InputIterator , InputIterator ){
		
		return true;
	}

	template<class T>
	std::any DictionaryCompressedColumn<T>::get(TID ){

		return {};
	}

	template<class T>
	void DictionaryCompressedColumn<T>::print() const noexcept{

	}
	template<class T>
	size_t DictionaryCompressedColumn<T>::size() const noexcept{

		return 0;
	}
	template<class T>
	ColumnPtr DictionaryCompressedColumn<T>::copy() const{

		return {};
	}

	template<class T>
	bool DictionaryCompressedColumn<T>::update(TID , const std::any& ){
		return false;
	}

	template<class T>
	bool DictionaryCompressedColumn<T>::update(PositionListPtr , const std::any& ){
		return false;		
	}
	
	template<class T>
	bool DictionaryCompressedColumn<T>::remove(TID){
		return false;	
	}
	
	template<class T>
	bool DictionaryCompressedColumn<T>::remove(PositionListPtr){
		return false;			
	}

	template<class T>
	bool DictionaryCompressedColumn<T>::clearContent(){
		return false;
	}

	template<class T>
	bool DictionaryCompressedColumn<T>::store(const std::string&){
		return false;
	}
	template<class T>
	bool DictionaryCompressedColumn<T>::load(const std::string&){
		return false;
	}

	template<class T>
	T& DictionaryCompressedColumn<T>::operator[](const int){
		static T t;
		return t;
	}

	template<class T>
	unsigned int DictionaryCompressedColumn<T>::getSizeinBytes() const noexcept{
		return 0; //return values_.capacity()*sizeof(T);
	}

/***************** End of Implementation Section ******************/



}; //end namespace CogaDB

