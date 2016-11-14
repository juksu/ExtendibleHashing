#ifndef EXTHASH
#define EXTHASH

#include <iostream>
#include "Container.h"

/*
 * Class ExthashEmptyException
 */
class ExthashEmptyException : public ContainerException {
	public:
		virtual const char * what() const noexcept override { return "Exthash: empty"; }
};


/*
 * class Exthash
 */
template <typename E, size_t B = 16>	// B defines Blocksize
class Exthash : public Container<E>		// Exthash inherits public from the abstract Container class
{
	/*
	 * struct Block
	 */
	struct Block
	{
		// local depth
		size_t localDepth;
		// number of entries in block
		size_t bSize;
		// Array with values
		E values[B];
		
		Block(): localDepth{0}, bSize{0} {}
		
		// an append function, adds e into the values[]-array
		// returns true if value could be inserted, otherwise (for example values[] is full) false
		bool append( const E& e )
		{
			// space in block
			if( bSize < B )
			{
				values[bSize] = e;
				bSize++;
				return true;
			}
			// block is full
			else
				return false;
		}
		
		bool member( const E& e )
		{
			for( size_t i = 0; i < bSize; i++ )
				if( e == values[i] )
					return true;
			
			return false;
		}
		
		bool remove( const E& e )
		{
			for( size_t i = 0; i < bSize; i++ )
			{
				if( e == values[i] )
				{
					bSize--;
					
					// move the value at the end of the array to the position of the deleted value
					// do that only if the deleted value was not at the end of the array
					if( i != bSize )
						values[i] = values[bSize];
					return true;
				}
			}
			return false;
		}
		
		// a print function which prints localDepth, bSize and all the values in the Block to the stream
		std::ostream& print( std::ostream& o ) const
		{
			o << "localDepth " << localDepth << std::endl;
			o << "bSize " << bSize << std::endl;
			for( size_t i = 0; i < bSize; i++ )
				o << values[i] << ", ";
			return o;
		}
	};
	/*
	 * struct Block END
	 */
	
	
	// size of datastructure
	size_t n;
	// global depth
	size_t globalDepth;
	// size of index array
	size_t indexsize;
	// index to pointers of block elements
	// index, an array to Block pointers
	Block** index;

	/*
	 * Function declarations
	 */
	void indexExpansion();	
	void splitBlock( size_t pos ) ;
	void quickSort( E* sort, size_t left, size_t right ) const;
	
	public: 
		// Constructor
		Exthash();
		Exthash( std::initializer_list<E> el );
		
		// Destructor
		virtual ~Exthash() ;
		
		// use Containerfunction add( const E& e )
		// by defining of a function all functions with the same name in parent class will be hidden
		// we do not want that for add and remove therefore import them by "using"
		// using imports all functions with that name. part of them still can be overriden.
		using Container<E>::add;
		// override Containerfunction add( const E e[], size_t len )
		virtual void add( const E e[], size_t len ) override;
		
		// use Containerfunction remove( const E& e )
		using Container<E>::remove;
		// override Containerfunction remove( const E e[], size_t len )
		virtual void remove( const E e[], size_t len ) override;
		
		// override Containerfunction member( const E e[] )
		//virtual bool member( const E& e ) const override;
		virtual bool member(const E& e) const override;
		
		// override Containerfunction size()
		virtual size_t size() const override;
		
		// do not define a function bool empty() since we don't want to overwrite the function in the Container.h
		
		// override Containerfunction apply(std::function<void(const E&)> f, Order order = dontcare)
		virtual size_t apply( std::function<void( const E& )> f, Order order = dontcare ) const override;
		
		// override Containerfunction min()
		virtual E min() const override;
		
		// override Containerfunction max()
		virtual E max() const override;
		
		// override Containerfunction print()
		virtual std::ostream& print( std::ostream& o ) const override;
		/*
		 * Function declarations END
		 */
}; // Class Exthash END


/*
 * define declared functions
 * at least all pure virtual function of Container class must be implemented
 */

// create index of size 1 and first Block in constructor
template <typename E, size_t B>
Exthash<E,B>::Exthash() : n{0}, globalDepth{0}, indexsize{1}, index{new Block*[indexsize]}
{ 
	index[0] = new Block;
	index[0]->localDepth = globalDepth;
}

template <typename E, size_t B>
Exthash<E,B>::Exthash( std::initializer_list<E> el ) : Exthash() 
{
	for (auto e: el) add(e);
}

template <typename E, size_t B>
Exthash<E,B>::~Exthash()
{
	// delete all Blocks
	// work from back to front
	// if i is within 2^t (t = local depth) it means the entry is in this range unique (it is referenced the last time) and can be deleted
	// else decrease i till the last reference of this block is reached
	for( size_t i = indexsize; i > 0; i-- )
		if( i <= (1 << index[i-1]->localDepth) )
			delete index[i-1];
	
	delete[] index;
}

template <typename E, size_t B>
void Exthash<E,B>::add( const E e[], size_t len ) 
{
	size_t pos;
	
	for( size_t i = 0; i < len; i++ )
	{
		// get position
		pos = hashValue( e[i] ) % indexsize;
		
		// add the value only if it does not exist already
		if( !(index[pos]->member( e[i] )) )
		{
			// space in Block
			if( index[pos]->bSize < B )
			{
				index[pos]->append( e[i] );
				n++;
			}
			
			// Block is full
			else 
			{
				// if localDepth equals (or is by any accident larger as) globalDepth a index expansion is required
				if( index[pos]->localDepth >= globalDepth )
					indexExpansion();

				// split Block
				splitBlock( pos );
				
				// try to add the value again by jumping back to loop
				i--;
				continue;
			}
		}
	}
}

template <typename E, size_t B>
void Exthash<E,B>::remove( const E e[], size_t len )
{	
	size_t pos;
	
	for( size_t i = 0; i < len; i++ )
	{
		// get position
		pos = hashValue( e[i] ) % indexsize;
		
		// remove value from Block (no Blockmerge required)
		if( index[pos]->remove( e[i] ) )
			n--;
	}
}

template <typename E, size_t B>
bool Exthash<E,B>::member( const E& e ) const
{
	size_t pos = hashValue( e ) % indexsize;
	
	return index[pos]->member( e );	
}

template <typename E, size_t B>
size_t Exthash<E,B>::size() const
{
	return n;
}

template <typename E, size_t B>
size_t Exthash<E,B>::apply( std::function<void( const E& )> f, Order order ) const 
{
	// dont get even started if empty
	if( n == 0 )
		return 0;

	size_t k = 0;
	size_t i, j, t;
	if( order == dontcare )
	{
		// get all values
		for( i = 0; i < indexsize; i++ )
		{
			// check if this is the first time the Block is referenced
			// if so insert values into f
			// the referenced Block is only within 2^t unique where t = localDepth
			// if i < 2^t then it is the first time this block is referenced, if not ignore it
			t = index[i]->localDepth;
			if( i < (1 << t) )
			{
				for( j = 0; j < index[i]->bSize; j++ )
				{
					try
					{
						f(index[i]->values[j]);
					} catch(...)
					{	// if an ellipsis (...) is used as the parameter of catch it will catch any exception
						// stop execution and return value of succesful applies
						return k;
					}
					k++;
				}
			}		
		}
	}
	else
	{
		/// TODO: maybe even better to sort pointers? (less extra space)
		E* sortArray = new E[n];
		
		// get all values, same as with order == dontcare but this time to sortArray
		for( i = 0; i < indexsize; i++ )
		{
			t = index[i]->localDepth;
			if( i < (1 << t) )
			{
				for( j = 0; j < index[i]->bSize; j++ )
				{
					sortArray[k] = index[i]->values[j];
					k++;
				}
			}		
		}
		k = 0;
		
		quickSort( sortArray, 0, n-1 );
	
		if( order == ascending )
		{
			for( i = 0; i < n; i++ )
			{
				try
				{
					f(sortArray[i]);
				} catch(...)
				{
					delete[] sortArray;
					return k;
				}
				k++;
			}
		}
		else 	//  order == descending
		{
			for( i = n; i > 0; i-- )
			{
				try
				{
					f(sortArray[i-1]);
				} catch(...)
				{
					delete[] sortArray;
					return k;
				}
				k++;
			}			
			
		}
		delete[] sortArray;
	}
	return k;
}

template <typename E, size_t B>
E Exthash<E,B>::min() const
{
	if ( n == 0 ) 
		throw ExthashEmptyException();
	
	// set min to the first value in the table
	// Note: it is possible that there is no entry in index[0]->values[0]
	// However, the first value in a block will always be on position values[0]
	// Find the first entry with value
	size_t i, j, t;
	for( i = 0; i < indexsize; i++ )
		if( index[i]->bSize != 0 )
			break;
	
	E* min = &(index[i]->values[0]);
	
	// search through index
	for( ; i < indexsize; i++ )
	{
		t = index[i]->localDepth;
		if( i < (1 << t) )
			for( j = 0; j < index[i]->bSize; j++ )
				if( *min > index[i]->values[j] )
					min = &(index[i]->values[j]);		
	}
	
	return *min;
}

template <typename E, size_t B>
E Exthash<E,B>::max() const
{
	if ( n == 0 ) 
		throw ExthashEmptyException();
	
	// set max to the first value in the table
	// Note: it is possible that there is no entry in index[0]->values[0]
	// However, the first value in a block will always be on position values[0]
	// Find the first entry with value
	size_t i, j, t;
	for( i = 0; i < indexsize; i++ )
		if( index[i]->bSize != 0 )
			break;
	
	E* max = &(index[i]->values[0]);
	
	// search through index
	for( ; i < indexsize; i++ )
	{
		t = index[i]->localDepth;
		if( i < (1 << t) )
			for( j = 0; j < index[i]->bSize; j++ )
				if( index[i]->values[j] > *max )
					max = &(index[i]->values[j]);		
	}
	
	return *max;
}

template <typename E, size_t B>
std::ostream& Exthash<E,B>::print( std::ostream& o ) const
{
	o << std::endl << "size " << size() << std::endl;
	o << "blocksize " << B << std::endl;
	o << "globalDepth " << globalDepth << std::endl;
	o << "indexsize " << indexsize << std::endl << std::endl;
	
	bool br;
	
	for( size_t i = 0; i < indexsize; i++ )
	{
		br = false;
		
		o << "index entry " << i << std::endl;
		
		for( size_t j = 0; j < i; j++ )
		{
			if( index[i] == index[j] )
			{
				o << "this block was first referenced in index[" << j << "]" << std::endl << std::endl;
				br = true;
				break;
			}
		}
		if( br )
			continue;	
		
		o << "address " << index[i] << std::endl;
		index[i]->print( o );
		o << std::endl << std::endl;
	}
	o << "end";
	return o;
}

template <typename E, size_t B>
void Exthash<E,B>::indexExpansion()
{
		// helper block
		Block** help = new Block*[ indexsize*2 ];
		
		// copy locations into new index
		// the first and second half of the index are identical (index still refers to old size)
		for( size_t i = 0; i < indexsize; i++ )
		{
			help[i] = index[i];	
			help[indexsize + i] = index[i];
		}
		
		// delete old index
		delete[] index;
		
		index = help;
		globalDepth++;
		indexsize *= 2; 
}

template <typename E, size_t B>
void Exthash<E,B>::splitBlock( size_t pos )
{
	// the block which is pointed to from index[pos] has to be split
	// first find the first index entry which points to this block
	// this entry is in the first 2^t indexentries where t = localDepth
	// more precisely the first entry pointing to the block is pos % 2^t
	// calculate 2^t by bit-shift
	size_t t = index[pos]->localDepth;
	pos = pos % (1 << t);
	
	// the next index entry pointing to this Block can be found at pos + 2^t (in fact every pos + i*2^t entry is pointing to this block)
	// this index entry will be the first pointing to the new block
	size_t newPos = pos + (1 << t);
	
	// create two Blocks, one which takes the role of the new Block, one which takes the role of the "old" Block
	Block* newBlock1 = new Block();
	newBlock1->localDepth = t + 1;
	Block* newBlock2 = new Block();
	newBlock2->localDepth = t + 1;
	
	// make the old Block available here
	Block* oldBlock = index[pos];
	
	// set the new index references
	// newPos will be always the later one, so as long newPos < indexsize we are fine
	while( newPos < indexsize )
	{
		index[pos] = newBlock1;
		index[newPos] = newBlock2;
		
		// go the the next entry (p = p + 2^(t+1))
		pos += (1 << (t + 1));
		newPos += (1 << (t + 1));
	}
	
	// insert the entries in the old block into the new block simply by add-function
	for( size_t i = 0; i < oldBlock->bSize; i++ )
	{
		// pos must point to one of the two new blocks
		pos = hashValue( oldBlock->values[i] ) % indexsize;
		
		// obviously there must be space in block and the value does not exist already so we can ignore all controll
		index[pos]-> append( oldBlock->values[i] );
	}
	
	// everything done, delete the old block
	delete oldBlock;
}

template <typename E, size_t B>
void Exthash<E,B>::quickSort( E* sort, size_t left, size_t right ) const
{
	// get sure that left < right, if this is not the case we wanted to sort a array of length <= 1 (we are at the end of recursive call)
	if( left < right )
	{
		// take left value as pivot
		size_t pivot = left;
		size_t l = left;
		size_t r = right;
		E help;
		
		while( true )
		{
			// sink from left
			// sink as long l < r and value on position l is smaller (or equal since we don't want to change pivot with itself) than pivot
			// i > j || i == j is equal to !(j > i)
			while( l < r &&	!(sort[l] > sort[pivot]) )
				l++;
			
			// sink from right
			// sink as long r > l and value on position r is larger (or equal since we don't want to change pivot with itself) than pivot
			while( r > l && !(sort[pivot] > sort[r]) )
				r--;
				
			// if l != r swap sort[l] with sort[r]
			// else we reached the final destination, put pivot on right place and exit loop
			if( l != r )
			{
				help=sort[l];
				sort[l] = sort[r];
				sort[r] = help;
			}
			else
			{
				// case if we reached end of sort with the last value being smaller then the pivot
				if( sort[pivot] > sort[l] )
				{
					help = sort[l];
					sort[l] = sort[pivot];
					sort[pivot] = help;
					
					// sort left of pivot
					// l-1 may be negative which would cause an underflow
					if( l > 1 )
						quickSort( sort, left, l-1 ); 	
					// sort right of pivot
					quickSort( sort, r, right );					
				}
				else // case if sort[l] is the first value which is greater than sort[pivot]
				{
					help = sort[l-1];
					sort[l-1] = sort[pivot];
					sort[pivot] = help;
					
					// sort left of pivot
					// l-2 may be negative which would cause an underflow
					if( l > 2 )
						quickSort( sort, left, l-2 ); 	
					// sort right of pivot
					quickSort( sort, r, right );
				}
				break;		// exit loop
			}				
		}
	}
}
/*
 * define declared functions END
 */

#endif //EXTHASH
