#ifndef __SOE_VECTOR_H__
#define __SOE_VECTOR_H__

#include <stdlib.h>

template<class T> class vector
{
private:

	int size;
	T*  data;

public:

  vector();
	~vector();

	T& Get(int i);
	void Set(T e, int i);
	int GetCount(void);
	T& operator[](int i) { return (Get(i)); }
	const T& operator[](int i) const { return (Get(i)); }

private:

	void realloc(int s);
	bool is_valid(void);
	
};

template<class T> vector<T>::vector()
{
	size = 1;
	data = (T*)malloc(sizeof(T));
}

template<class T> vector<T>::~vector()
{
	//if (is_valid()) free(data);
}

template<class T> void vector<T>::realloc(int s)
{
	if (s >= 1)
	{
		data = (T*)::realloc((void*)data,s*sizeof(T));
		size += s;
	}
}

template<class T> T& vector<T>::Get(int i)
{
	if ((i >= 0) && (i < size))
		return (data[i]);
	else 
		return (data[0]);
}

template<class T> int vector<T>::GetCount(void)
{
	return (size);
}

template<class T> bool vector<T>::is_valid(void)
{
	return ((size <= 0) ? false : true);
}

template<class T> void vector<T>::Set(T e, int i)
{
	if (is_valid())
	{
		if (i >= size) realloc(size+1);
		data[i] = e;
	}
}

#endif  