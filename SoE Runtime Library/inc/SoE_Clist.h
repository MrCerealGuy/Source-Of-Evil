/*

 * PROJECT     : Source of Evil Runtime Library v1.0 
 * PROGRAMMER  : Andreas Zahnleiter (Mafuba)    

*/

#ifndef SOE_CLIST_H
#define SOE_CLIST_H

template<class TYPE> class SoECList
{	
public:
    SoECList(void);
    virtual ~SoECList(void);

    void         Clear( BOOL deleteMe = TRUE );
    void         Add( TYPE* newElement , BOOL Last = TRUE );
    void         Insert( TYPE* newElement, DWORD pos );
    TYPE*        Get( DWORD Pos );
    inline TYPE* GetFirst( void );
	inline TYPE* GetLast( void );
    TYPE*        Next( TYPE* &Search );
	TYPE*        Prev( TYPE* &Search );
    TYPE*        Remove( TYPE* &search, BOOL deleteMe = TRUE );
    TYPE*        Remove( DWORD Pos, BOOL deleteMe = TRUE );
    void         CreateLookup( void );
    LONG         GetCount( void );

private:
    TYPE* m_First;     // Pointer to the first element in the list
    TYPE* m_Last;      // Pointer to the last element in the list
    TYPE** m_PtrList;  // Pointer to a list of pointer used as a lookup table
    LONG m_Count;      // Count of number of elements in the list
};

template<class TYPE> SoECList<TYPE>::SoECList(void)
{	
    m_Last    = NULL;
    m_First   = NULL;
	m_Count   = 0;
	m_PtrList = NULL;
}

template<class TYPE> SoECList<TYPE>::~SoECList(void)
{	
    Clear(FALSE);
	delete [] m_PtrList;
}

template<class TYPE> void SoECList<TYPE>::Add( TYPE* newElement , BOOL Last)
{	
    //Cannot add a NULL to the list.
	if( newElement == NULL ) 
        return; 

	if( m_First == NULL )
	{	
        // If no elements in list, make this the first
		newElement->SetNext(NULL);
		newElement->SetPrev(NULL);
		m_Last  = newElement;
		m_First = newElement;
	}
	else
	{
		if( Last )
		{
            // Insert at end of the list
			newElement->SetNext(NULL);	
			newElement->SetPrev(m_Last);
			m_Last->SetNext(newElement);
			m_Last = newElement;
		} 
        else
		{
            // Insert at the beginning of the list
			newElement->SetNext(m_First);
			newElement->SetPrev(NULL);
			m_First->SetPrev(newElement);
			m_First = newElement;
		}
	}

	m_Count++;

	// If a lookup table was created delete it
	if( m_PtrList != NULL)
    {
	    delete [] m_PtrList;
        m_PtrList = NULL;
    }
}

template<class TYPE> void SoECList<TYPE>::Insert( TYPE* newElement, DWORD pos )
{
    //Cannot add a NULL to the list.
    if( newElement == NULL ) 
        return; 

    if( pos < 0 ) 
        pos=0;

    if( pos >= m_Count )
    {
        // Add to end of the list
        Add( newElement );
    }
    else
    {
        TYPE *Search = Get( pos );

        if( Search->GetPrev() != NULL )
            Search->GetPrev()->SetNext(newElement);
        else
            m_First = newElement;

        newElement->SetNext(Search);
        newElement->SetPrev(Search->GetPrev());
        Search->SetPrev(newElement);
        m_Count++;

	    // If a lookup table was created delete it
		if( m_PtrList != NULL)
        {
	        delete [] m_PtrList;
            m_PtrList = NULL;
        }
    }
}

template<class TYPE> void SoECList<TYPE>::Clear( BOOL deleteMe )
{	
    if( m_First != NULL )
	{	
        TYPE* Search = m_First;
	
		do
		{	
            if( (Search->GetPrev() != NULL) && (deleteMe == TRUE) )
			    delete Search->GetPrev();

		}while( (Search = Search->GetNext()) != NULL );

		m_First = m_Last = NULL;
		m_Count=0;

        // If a lookup table was created delete it
		if( m_PtrList != NULL)
        {
	        delete [] m_PtrList;
            m_PtrList = NULL;
        }
	}
}

template<class TYPE> TYPE* SoECList<TYPE>::Next( TYPE* &Search )
{	
    TYPE* tmp = NULL;

	if( Search != NULL ) 
        tmp = Search->GetNext();

	Search = tmp;

	return tmp;
}

template<class TYPE> TYPE* SoECList<TYPE>::Prev( TYPE* &Search )
{	
    TYPE* tmp = NULL;

	if( Search != NULL ) 
        tmp = Search->GetPrev();

	Search = tmp;

	return tmp;
}

template<class TYPE> inline TYPE* SoECList<TYPE>::GetFirst(void)
{	
    return m_First;	
}

template<class TYPE> inline TYPE* SoECList<TYPE>::GetLast(void)
{	
    return m_Last;	
}

template<class TYPE> inline LONG SoECList<TYPE>::GetCount(void)
{	
    return m_Count;	
}

template<class TYPE> TYPE* SoECList<TYPE>::Get( DWORD Pos )
{	
    TYPE* tmp = m_First;

	if( tmp != NULL )
	{	
        if( m_PtrList != NULL )
		{
			return m_PtrList[Pos];
		}
		else
		{
			if (Pos == 0) 
                return m_First;

			if (Pos == (DWORD)(m_Count - 1)) 
                return m_Last;

            // Start at the first element and GetNext 'pos' times
			for( int i=0; i<(int)Pos && tmp; i++ )
                tmp = tmp->GetNext();

			return tmp;
		}

	}

	return NULL;
}

template<class TYPE> TYPE* SoECList<TYPE>::Remove( TYPE* &Search, BOOL deleteMe )
{	
    TYPE* tmp;

    // If value passed in is NULL return NULL
	if (Search == NULL) 
        return NULL; 

	if( Search->GetPrev() != NULL )
		Search->GetPrev()->SetNext(Search->GetNext());
	else 
		m_First = Search->GetNext();


	if( Search->GetNext() != NULL )
		Search->GetNext()->SetPrev(Search->GetPrev());
	else
		m_Last = Search->GetPrev();

	tmp = Search->GetPrev();

    if( deleteMe == TRUE )
	    delete Search;

	m_Count--;

	if( m_PtrList != NULL)
    {
	    delete [] m_PtrList;
        m_PtrList = NULL;
    }

	Search=tmp; 

	return tmp;
}

template<class TYPE> TYPE* SoECList<TYPE>::Remove( DWORD Pos, BOOL deleteMe )
{	
    TYPE* T = Get(Pos);
	T = Remove(T, deleteMe);

	return T;
}

template<class TYPE> void SoECList<TYPE>::CreateLookup( void )
{	
    // Delete the lookup table
    if( m_PtrList != NULL)
    {
	    delete [] m_PtrList;
        m_PtrList = NULL;
    }

	m_PtrList = new TYPE*[m_Count];

	if( m_First != NULL )
	{	
        TYPE* Search = m_First;
		int i=0;

		do
		{	
            m_PtrList[i] = Search;
			i++;

		}while( (Search = Search->GetNext()) != NULL );
	}
}

#endif SOE_CLIST_H